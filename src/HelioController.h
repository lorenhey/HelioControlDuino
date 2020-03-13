#ifndef HELIO_CONTROL_DUINO_CONTROLLER_H
#define HELIO_CONTROL_DUINO_CONTROLLER_H

#include "HelioInterfaces.h"

namespace heliocontrol {

class HelioController {
 public:
  HelioController(AxisDriver &azimuthAxis, AxisDriver &elevationAxis,
                  LightSensor &lightSensor,
                  SafetyInputs *safetyInputs = nullptr);

  bool begin(const ControllerConfig &config);
  void update();

  void setMode(OperatingMode mode);
  bool setManualTarget(float azimuth, float elevation);
  bool clearFaults();

  OperatingMode mode() const;
  ControllerState state() const;
  const ControllerStatus &status() const;
  const ControllerConfig &config() const;

 private:
  bool validAxisConfig(const AxisConfig &axis) const;
  bool validConfig(const ControllerConfig &config) const;
  void setFault(uint8_t fault);
  void commandPark();
  void commandStow();
  void commandClamped(float azimuth, float elevation);
  void runAutomatic(uint32_t now);
  void moveTowardLight(const LightReading &reading);
  bool handleWind(uint32_t now);
  void refreshStatus();

  AxisDriver &azimuthAxis_;
  AxisDriver &elevationAxis_;
  LightSensor &lightSensor_;
  SafetyInputs *safetyInputs_;

  ControllerConfig config_;
  ControllerStatus status_;
  bool begun_;
  bool darkTimerActive_;
  bool lightRecoveryActive_;
  bool windRecoveryActive_;
  bool windWasActive_;
  uint32_t darkSince_;
  uint32_t lightSince_;
  uint32_t windClearSince_;
  uint32_t lastAutomaticUpdate_;
};

}  // namespace heliocontrol

#endif
