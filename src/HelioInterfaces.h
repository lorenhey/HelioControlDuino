#ifndef HELIO_CONTROL_DUINO_INTERFACES_H
#define HELIO_CONTROL_DUINO_INTERFACES_H

#include "HelioTypes.h"

namespace heliocontrol {

class AxisDriver {
 public:
  virtual ~AxisDriver() {}
  virtual bool begin(float initialAngle,
                     float maximumSpeedDegreesPerSecond) = 0;
  virtual void command(float angle) = 0;
  virtual void update() = 0;
  virtual void stop() = 0;
  virtual float position() const = 0;
  virtual float target() const = 0;
  virtual bool ready() const = 0;
};

class LightSensor {
 public:
  virtual ~LightSensor() {}
  virtual bool begin() = 0;
  virtual bool read(LightReading &reading) = 0;
};

class SafetyInputs {
 public:
  virtual ~SafetyInputs() {}
  virtual bool begin() = 0;
  virtual bool read(SafetyStatus &status) = 0;
};

}  // namespace heliocontrol

#endif
