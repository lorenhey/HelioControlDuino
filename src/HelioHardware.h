#ifndef HELIO_CONTROL_DUINO_HARDWARE_H
#define HELIO_CONTROL_DUINO_HARDWARE_H

#include <Servo.h>

#include "HelioInterfaces.h"

namespace heliocontrol {

class ServoAxis : public AxisDriver {
 public:
  explicit ServoAxis(uint8_t pin, uint16_t minimumPulseUs = 544,
                     uint16_t maximumPulseUs = 2400);

  bool begin(float initialAngle,
             float maximumSpeedDegreesPerSecond) override;
  void command(float angle) override;
  void update() override;
  void stop() override;
  float position() const override;
  float target() const override;
  bool ready() const override;
  void detach();

 private:
  Servo servo_;
  uint8_t pin_;
  uint16_t minimumPulseUs_;
  uint16_t maximumPulseUs_;
  float position_;
  float target_;
  float maximumSpeed_;
  uint32_t lastUpdateMs_;
  bool begun_;
};

class FourLdrSensor : public LightSensor {
 public:
  FourLdrSensor(uint8_t topLeftPin, uint8_t topRightPin,
                uint8_t bottomLeftPin, uint8_t bottomRightPin,
                uint8_t samplesPerReading = 8,
                float smoothingFactor = 0.25f,
                bool invertReadings = false, uint16_t adcMaximum = 1023);

  bool begin() override;
  bool read(LightReading &reading) override;

 private:
  uint16_t samplePin(uint8_t pin) const;
  uint16_t filter(uint8_t index, uint16_t sample);

  uint8_t pins_[4];
  uint8_t samplesPerReading_;
  float smoothingFactor_;
  bool invertReadings_;
  uint16_t adcMaximum_;
  float filtered_[4];
  bool hasReading_;
};

struct SafetyPins {
  int8_t wind;
  int8_t emergencyStop;
  int8_t azimuthMinimum;
  int8_t azimuthMaximum;
  int8_t elevationMinimum;
  int8_t elevationMaximum;
  bool activeLow;
  bool useInternalPullups;
};

SafetyPins defaultSafetyPins();

class DigitalSafetyInputs : public SafetyInputs {
 public:
  explicit DigitalSafetyInputs(const SafetyPins &pins);

  bool begin() override;
  bool read(SafetyStatus &status) override;

 private:
  void configurePin(int8_t pin) const;
  bool active(int8_t pin) const;

  SafetyPins pins_;
  bool begun_;
};

}  // namespace heliocontrol

#endif
