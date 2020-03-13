#include "HelioController.h"

#include <string.h>

#include "HelioMath.h"

namespace heliocontrol {

HelioController::HelioController(AxisDriver &azimuthAxis,
                                 AxisDriver &elevationAxis,
                                 LightSensor &lightSensor,
                                 SafetyInputs *safetyInputs)
    : azimuthAxis_(azimuthAxis),
      elevationAxis_(elevationAxis),
      lightSensor_(lightSensor),
      safetyInputs_(safetyInputs),
      begun_(false),
      darkTimerActive_(false),
      lightRecoveryActive_(false),
      windRecoveryActive_(false),
      windWasActive_(false),
      darkSince_(0),
      lightSince_(0),
      windClearSince_(0),
      lastAutomaticUpdate_(0) {
  memset(&config_, 0, sizeof(config_));
  memset(&status_, 0, sizeof(status_));
  status_.mode = OperatingMode::STOPPED;
  status_.state = ControllerState::STARTING;
}

bool HelioController::validAxisConfig(const AxisConfig &axis) const {
  return axis.minimumAngle >= 0.0f && axis.maximumAngle <= 180.0f &&
         axis.minimumAngle < axis.maximumAngle &&
         axis.parkAngle >= axis.minimumAngle &&
         axis.parkAngle <= axis.maximumAngle &&
         axis.stowAngle >= axis.minimumAngle &&
         axis.stowAngle <= axis.maximumAngle &&
         axis.initialAngle >= axis.minimumAngle &&
         axis.initialAngle <= axis.maximumAngle &&
         axis.maximumSpeedDegreesPerSecond > 0.0f;
}

bool HelioController::validConfig(const ControllerConfig &config) const {
  const TrackingConfig &tracking = config.tracking;
  return validAxisConfig(config.azimuth) &&
         validAxisConfig(config.elevation) &&
         tracking.darknessThreshold <= 1023 &&
         tracking.deadbandPercent <= 100 && tracking.stepDegrees > 0.0f &&
         tracking.stepDegrees <= 30.0f &&
         (tracking.azimuthDirection == -1 ||
          tracking.azimuthDirection == 1) &&
         (tracking.elevationDirection == -1 ||
          tracking.elevationDirection == 1) &&
         tracking.updateIntervalMs >= 50;
}

bool HelioController::begin(const ControllerConfig &config) {
  config_ = config;
  status_.mode = OperatingMode::AUTOMATIC;
  status_.state = ControllerState::STARTING;
  status_.faults = FAULT_NONE;

  if (!validConfig(config_)) {
    setFault(FAULT_INVALID_CONFIG);
    return false;
  }

  if (!lightSensor_.begin()) {
    setFault(FAULT_SENSOR);
    return false;
  }

  if (safetyInputs_ != nullptr && !safetyInputs_->begin()) {
    setFault(FAULT_SAFETY_INPUT);
    return false;
  }

  const bool azimuthReady = azimuthAxis_.begin(
      config_.azimuth.initialAngle,
      config_.azimuth.maximumSpeedDegreesPerSecond);
  const bool elevationReady = elevationAxis_.begin(
      config_.elevation.initialAngle,
      config_.elevation.maximumSpeedDegreesPerSecond);

  if (!azimuthReady || !elevationReady) {
    setFault(FAULT_AXIS);
    return false;
  }

  begun_ = true;
  status_.state = ControllerState::WAITING_FOR_LIGHT;
  lastAutomaticUpdate_ = millis() - config_.tracking.updateIntervalMs;
  refreshStatus();
  return true;
}

void HelioController::update() {
  azimuthAxis_.update();
  elevationAxis_.update();

  if (!begun_) {
    return;
  }

  const uint32_t now = millis();

  if (safetyInputs_ != nullptr) {
    SafetyStatus latest;
    memset(&latest, 0, sizeof(latest));
    if (!safetyInputs_->read(latest)) {
      setFault(FAULT_SAFETY_INPUT);
      refreshStatus();
      return;
    }
    status_.safety = latest;
  }

  if (status_.safety.emergencyStop) {
    setFault(FAULT_EMERGENCY);
    refreshStatus();
    return;
  }

  if ((status_.safety.azimuthMinimum &&
       status_.safety.azimuthMaximum) ||
      (status_.safety.elevationMinimum &&
       status_.safety.elevationMaximum)) {
    setFault(FAULT_LIMIT_CONFLICT);
    refreshStatus();
    return;
  }

  if (status_.faults != FAULT_NONE) {
    status_.state = ControllerState::FAULT;
    refreshStatus();
    return;
  }

  if (handleWind(now)) {
    refreshStatus();
    return;
  }

  switch (status_.mode) {
    case OperatingMode::AUTOMATIC:
      runAutomatic(now);
      break;

    case OperatingMode::PARK:
      commandPark();
      status_.state = ControllerState::PARKED;
      break;

    case OperatingMode::STOW:
      commandStow();
      status_.state = ControllerState::WIND_STOW;
      break;

    case OperatingMode::MANUAL:
      status_.state = ControllerState::MANUAL;
      break;

    case OperatingMode::STOPPED:
      azimuthAxis_.stop();
      elevationAxis_.stop();
      status_.state = ControllerState::STOPPED;
      break;
  }

  refreshStatus();
}

bool HelioController::handleWind(uint32_t now) {
  if (status_.safety.wind) {
    windWasActive_ = true;
    windRecoveryActive_ = false;
    commandStow();
    status_.state = ControllerState::WIND_STOW;
    return true;
  }

  if (!windWasActive_) {
    windRecoveryActive_ = false;
    return false;
  }

  if (!windRecoveryActive_) {
    windRecoveryActive_ = true;
    windClearSince_ = now;
  }

  if (!elapsed(now, windClearSince_, config_.tracking.windRecoveryDelayMs)) {
    commandStow();
    status_.state = ControllerState::WIND_STOW;
    return true;
  }

  windRecoveryActive_ = false;
  windWasActive_ = false;
  return false;
}

void HelioController::runAutomatic(uint32_t now) {
  if (!elapsed(now, lastAutomaticUpdate_,
               config_.tracking.updateIntervalMs)) {
    return;
  }
  lastAutomaticUpdate_ = now;

  LightReading reading;
  memset(&reading, 0, sizeof(reading));
  if (!lightSensor_.read(reading)) {
    setFault(FAULT_SENSOR);
    return;
  }

  status_.light = reading;
  ++status_.updateCount;

  if (reading.average < config_.tracking.darknessThreshold) {
    lightRecoveryActive_ = false;
    if (!darkTimerActive_) {
      darkTimerActive_ = true;
      darkSince_ = now;
    }

    if (elapsed(now, darkSince_, config_.tracking.darknessDelayMs)) {
      commandPark();
      status_.state = ControllerState::NIGHT_PARK;
    } else {
      status_.state = ControllerState::WAITING_FOR_LIGHT;
    }
    return;
  }

  darkTimerActive_ = false;

  if (status_.state == ControllerState::NIGHT_PARK ||
      status_.state == ControllerState::WAITING_FOR_LIGHT) {
    if (!lightRecoveryActive_) {
      lightRecoveryActive_ = true;
      lightSince_ = now;
    }

    if (!elapsed(now, lightSince_,
                 config_.tracking.lightRecoveryDelayMs)) {
      status_.state = ControllerState::WAITING_FOR_LIGHT;
      return;
    }
  }

  lightRecoveryActive_ = false;
  moveTowardLight(reading);
  status_.state = ControllerState::TRACKING;
}

void HelioController::moveTowardLight(const LightReading &reading) {
  const uint32_t total = static_cast<uint32_t>(reading.topLeft) +
                         reading.topRight + reading.bottomLeft +
                         reading.bottomRight;
  const int32_t left = static_cast<int32_t>(reading.topLeft) +
                       reading.bottomLeft;
  const int32_t right = static_cast<int32_t>(reading.topRight) +
                        reading.bottomRight;
  const int32_t top = static_cast<int32_t>(reading.topLeft) +
                      reading.topRight;
  const int32_t bottom = static_cast<int32_t>(reading.bottomLeft) +
                         reading.bottomRight;

  status_.azimuthErrorPercent = normalizedDifference(left, right, total);
  status_.elevationErrorPercent = normalizedDifference(top, bottom, total);

  float azimuth = azimuthAxis_.target();
  float elevation = elevationAxis_.target();
  const int16_t deadband = config_.tracking.deadbandPercent;

  if (status_.azimuthErrorPercent > deadband) {
    azimuth += config_.tracking.stepDegrees *
               config_.tracking.azimuthDirection;
  } else if (status_.azimuthErrorPercent < -deadband) {
    azimuth -= config_.tracking.stepDegrees *
               config_.tracking.azimuthDirection;
  }

  if (status_.elevationErrorPercent > deadband) {
    elevation += config_.tracking.stepDegrees *
                 config_.tracking.elevationDirection;
  } else if (status_.elevationErrorPercent < -deadband) {
    elevation -= config_.tracking.stepDegrees *
                 config_.tracking.elevationDirection;
  }

  commandClamped(azimuth, elevation);
}

void HelioController::setMode(OperatingMode mode) {
  if (!begun_ || status_.faults != FAULT_NONE) {
    return;
  }

  status_.mode = mode;
  lightRecoveryActive_ = false;
  darkTimerActive_ = false;

  if (mode == OperatingMode::PARK) {
    commandPark();
  } else if (mode == OperatingMode::STOW) {
    commandStow();
  } else if (mode == OperatingMode::STOPPED) {
    azimuthAxis_.stop();
    elevationAxis_.stop();
  }
}

bool HelioController::setManualTarget(float azimuth, float elevation) {
  if (!begun_ || status_.faults != FAULT_NONE ||
      status_.safety.emergencyStop || status_.safety.wind) {
    return false;
  }

  if (azimuth < config_.azimuth.minimumAngle ||
      azimuth > config_.azimuth.maximumAngle ||
      elevation < config_.elevation.minimumAngle ||
      elevation > config_.elevation.maximumAngle) {
    return false;
  }

  status_.mode = OperatingMode::MANUAL;
  commandClamped(azimuth, elevation);
  status_.state = ControllerState::MANUAL;
  return true;
}

bool HelioController::clearFaults() {
  if (!begun_ || status_.safety.emergencyStop ||
      (status_.safety.azimuthMinimum && status_.safety.azimuthMaximum) ||
      (status_.safety.elevationMinimum &&
       status_.safety.elevationMaximum)) {
    return false;
  }

  status_.faults = FAULT_NONE;
  status_.mode = OperatingMode::STOPPED;
  status_.state = ControllerState::STOPPED;
  azimuthAxis_.stop();
  elevationAxis_.stop();
  return true;
}

void HelioController::setFault(uint8_t fault) {
  status_.faults |= fault;
  status_.mode = OperatingMode::STOPPED;
  status_.state = ControllerState::FAULT;
  azimuthAxis_.stop();
  elevationAxis_.stop();
}

void HelioController::commandPark() {
  commandClamped(config_.azimuth.parkAngle,
                 config_.elevation.parkAngle);
}

void HelioController::commandStow() {
  commandClamped(config_.azimuth.stowAngle,
                 config_.elevation.stowAngle);
}

void HelioController::commandClamped(float azimuth, float elevation) {
  azimuth = clampAngle(azimuth, config_.azimuth.minimumAngle,
                       config_.azimuth.maximumAngle);
  elevation = clampAngle(elevation, config_.elevation.minimumAngle,
                         config_.elevation.maximumAngle);

  if (status_.safety.azimuthMinimum && azimuth < azimuthAxis_.position()) {
    azimuth = azimuthAxis_.position();
  }
  if (status_.safety.azimuthMaximum && azimuth > azimuthAxis_.position()) {
    azimuth = azimuthAxis_.position();
  }
  if (status_.safety.elevationMinimum &&
      elevation < elevationAxis_.position()) {
    elevation = elevationAxis_.position();
  }
  if (status_.safety.elevationMaximum &&
      elevation > elevationAxis_.position()) {
    elevation = elevationAxis_.position();
  }

  azimuthAxis_.command(azimuth);
  elevationAxis_.command(elevation);
}

void HelioController::refreshStatus() {
  status_.azimuthPosition = azimuthAxis_.position();
  status_.elevationPosition = elevationAxis_.position();
  status_.azimuthTarget = azimuthAxis_.target();
  status_.elevationTarget = elevationAxis_.target();
}

OperatingMode HelioController::mode() const { return status_.mode; }

ControllerState HelioController::state() const { return status_.state; }

const ControllerStatus &HelioController::status() const { return status_; }

const ControllerConfig &HelioController::config() const { return config_; }

}  // namespace heliocontrol
