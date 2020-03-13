#include "HelioTypes.h"

namespace heliocontrol {

ControllerConfig defaultControllerConfig() {
  ControllerConfig config;

  config.azimuth.minimumAngle = 10.0f;
  config.azimuth.maximumAngle = 170.0f;
  config.azimuth.parkAngle = 90.0f;
  config.azimuth.stowAngle = 90.0f;
  config.azimuth.initialAngle = 90.0f;
  config.azimuth.maximumSpeedDegreesPerSecond = 20.0f;

  config.elevation.minimumAngle = 15.0f;
  config.elevation.maximumAngle = 90.0f;
  config.elevation.parkAngle = 20.0f;
  config.elevation.stowAngle = 15.0f;
  config.elevation.initialAngle = 20.0f;
  config.elevation.maximumSpeedDegreesPerSecond = 15.0f;

  config.tracking.darknessThreshold = 120;
  config.tracking.deadbandPercent = 6;
  config.tracking.stepDegrees = 1.0f;
  config.tracking.azimuthDirection = 1;
  config.tracking.elevationDirection = 1;
  config.tracking.updateIntervalMs = 500;
  config.tracking.darknessDelayMs = 300000UL;
  config.tracking.lightRecoveryDelayMs = 30000UL;
  config.tracking.windRecoveryDelayMs = 60000UL;

  return config;
}

const __FlashStringHelper *modeName(OperatingMode mode) {
  switch (mode) {
    case OperatingMode::AUTOMATIC:
      return F("AUTO");
    case OperatingMode::PARK:
      return F("PARK");
    case OperatingMode::STOW:
      return F("STOW");
    case OperatingMode::MANUAL:
      return F("MANUAL");
    case OperatingMode::STOPPED:
      return F("STOP");
  }
  return F("UNKNOWN");
}

const __FlashStringHelper *stateName(ControllerState state) {
  switch (state) {
    case ControllerState::STARTING:
      return F("STARTING");
    case ControllerState::TRACKING:
      return F("TRACKING");
    case ControllerState::WAITING_FOR_LIGHT:
      return F("WAITING_LIGHT");
    case ControllerState::NIGHT_PARK:
      return F("NIGHT_PARK");
    case ControllerState::PARKED:
      return F("PARKED");
    case ControllerState::WIND_STOW:
      return F("WIND_STOW");
    case ControllerState::MANUAL:
      return F("MANUAL");
    case ControllerState::STOPPED:
      return F("STOPPED");
    case ControllerState::FAULT:
      return F("FAULT");
  }
  return F("UNKNOWN");
}

}  // namespace heliocontrol
