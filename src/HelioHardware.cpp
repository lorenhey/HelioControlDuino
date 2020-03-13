#include "HelioHardware.h"

#include <math.h>

#include "HelioMath.h"

namespace heliocontrol {

ServoAxis::ServoAxis(uint8_t pin, uint16_t minimumPulseUs,
                     uint16_t maximumPulseUs)
    : pin_(pin),
      minimumPulseUs_(minimumPulseUs),
      maximumPulseUs_(maximumPulseUs),
      position_(90.0f),
      target_(90.0f),
      maximumSpeed_(20.0f),
      lastUpdateMs_(0),
      begun_(false) {}

bool ServoAxis::begin(float initialAngle,
                      float maximumSpeedDegreesPerSecond) {
  if (maximumSpeedDegreesPerSecond <= 0.0f || minimumPulseUs_ == 0 ||
      maximumPulseUs_ <= minimumPulseUs_) {
    return false;
  }

  servo_.attach(pin_, minimumPulseUs_, maximumPulseUs_);
  if (!servo_.attached()) {
    return false;
  }

  position_ = clampAngle(initialAngle, 0.0f, 180.0f);
  target_ = position_;
  maximumSpeed_ = maximumSpeedDegreesPerSecond;
  servo_.write(static_cast<int>(position_ + 0.5f));
  lastUpdateMs_ = millis();
  begun_ = true;
  return true;
}

void ServoAxis::command(float angle) {
  if (!begun_) {
    return;
  }
  target_ = clampAngle(angle, 0.0f, 180.0f);
}

void ServoAxis::update() {
  if (!begun_) {
    return;
  }

  const uint32_t now = millis();
  uint32_t elapsedMs = static_cast<uint32_t>(now - lastUpdateMs_);
  if (elapsedMs == 0) {
    return;
  }
  if (elapsedMs > 250) {
    elapsedMs = 250;
  }
  lastUpdateMs_ = now;

  const float maximumStep =
      maximumSpeed_ * (static_cast<float>(elapsedMs) / 1000.0f);
  const float difference = target_ - position_;

  if (difference > maximumStep) {
    position_ += maximumStep;
  } else if (difference < -maximumStep) {
    position_ -= maximumStep;
  } else {
    position_ = target_;
  }

  servo_.write(static_cast<int>(position_ + 0.5f));
}

void ServoAxis::stop() {
  if (begun_) {
    target_ = position_;
  }
}

float ServoAxis::position() const { return position_; }

float ServoAxis::target() const { return target_; }

bool ServoAxis::ready() const { return begun_; }

void ServoAxis::detach() {
  if (servo_.attached()) {
    servo_.detach();
  }
  begun_ = false;
}

FourLdrSensor::FourLdrSensor(uint8_t topLeftPin, uint8_t topRightPin,
                             uint8_t bottomLeftPin,
                             uint8_t bottomRightPin,
                             uint8_t samplesPerReading,
                             float smoothingFactor,
                             bool invertReadings, uint16_t adcMaximum)
    : samplesPerReading_(samplesPerReading),
      smoothingFactor_(smoothingFactor),
      invertReadings_(invertReadings),
      adcMaximum_(adcMaximum),
      hasReading_(false) {
  pins_[0] = topLeftPin;
  pins_[1] = topRightPin;
  pins_[2] = bottomLeftPin;
  pins_[3] = bottomRightPin;
  for (uint8_t index = 0; index < 4; ++index) {
    filtered_[index] = 0.0f;
  }
}

bool FourLdrSensor::begin() {
  if (samplesPerReading_ == 0 || samplesPerReading_ > 64 ||
      smoothingFactor_ <= 0.0f || smoothingFactor_ > 1.0f ||
      adcMaximum_ == 0) {
    return false;
  }

  for (uint8_t index = 0; index < 4; ++index) {
    pinMode(pins_[index], INPUT);
  }
  hasReading_ = false;
  return true;
}

uint16_t FourLdrSensor::samplePin(uint8_t pin) const {
  uint32_t total = 0;
  for (uint8_t sample = 0; sample < samplesPerReading_; ++sample) {
    total += static_cast<uint16_t>(analogRead(pin));
  }
  uint16_t value = static_cast<uint16_t>(total / samplesPerReading_);
  if (value > adcMaximum_) {
    value = adcMaximum_;
  }
  return invertReadings_ ? adcMaximum_ - value : value;
}

uint16_t FourLdrSensor::filter(uint8_t index, uint16_t sample) {
  if (!hasReading_) {
    filtered_[index] = static_cast<float>(sample);
  } else {
    filtered_[index] += smoothingFactor_ *
                        (static_cast<float>(sample) - filtered_[index]);
  }
  return static_cast<uint16_t>(filtered_[index] + 0.5f);
}

bool FourLdrSensor::read(LightReading &reading) {
  const uint16_t topLeft = filter(0, samplePin(pins_[0]));
  const uint16_t topRight = filter(1, samplePin(pins_[1]));
  const uint16_t bottomLeft = filter(2, samplePin(pins_[2]));
  const uint16_t bottomRight = filter(3, samplePin(pins_[3]));
  hasReading_ = true;

  reading.topLeft = topLeft;
  reading.topRight = topRight;
  reading.bottomLeft = bottomLeft;
  reading.bottomRight = bottomRight;
  reading.average = static_cast<uint16_t>(
      (static_cast<uint32_t>(topLeft) + topRight + bottomLeft +
       bottomRight) /
      4UL);
  return true;
}

SafetyPins defaultSafetyPins() {
  SafetyPins pins;
  pins.wind = PIN_UNUSED;
  pins.emergencyStop = PIN_UNUSED;
  pins.azimuthMinimum = PIN_UNUSED;
  pins.azimuthMaximum = PIN_UNUSED;
  pins.elevationMinimum = PIN_UNUSED;
  pins.elevationMaximum = PIN_UNUSED;
  pins.activeLow = true;
  pins.useInternalPullups = true;
  return pins;
}

DigitalSafetyInputs::DigitalSafetyInputs(const SafetyPins &pins)
    : pins_(pins), begun_(false) {}

void DigitalSafetyInputs::configurePin(int8_t pin) const {
  if (pin != PIN_UNUSED) {
    pinMode(static_cast<uint8_t>(pin),
            pins_.useInternalPullups ? INPUT_PULLUP : INPUT);
  }
}

bool DigitalSafetyInputs::begin() {
  configurePin(pins_.wind);
  configurePin(pins_.emergencyStop);
  configurePin(pins_.azimuthMinimum);
  configurePin(pins_.azimuthMaximum);
  configurePin(pins_.elevationMinimum);
  configurePin(pins_.elevationMaximum);
  begun_ = true;
  return true;
}

bool DigitalSafetyInputs::active(int8_t pin) const {
  if (pin == PIN_UNUSED) {
    return false;
  }
  const bool high = digitalRead(static_cast<uint8_t>(pin)) == HIGH;
  return pins_.activeLow ? !high : high;
}

bool DigitalSafetyInputs::read(SafetyStatus &status) {
  if (!begun_) {
    return false;
  }
  status.wind = active(pins_.wind);
  status.emergencyStop = active(pins_.emergencyStop);
  status.azimuthMinimum = active(pins_.azimuthMinimum);
  status.azimuthMaximum = active(pins_.azimuthMaximum);
  status.elevationMinimum = active(pins_.elevationMinimum);
  status.elevationMaximum = active(pins_.elevationMaximum);
  return true;
}

}  // namespace heliocontrol
