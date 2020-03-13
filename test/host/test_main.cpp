#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "HelioController.h"
#include "HelioMath.h"
#include "SolarCalculator.h"

using namespace heliocontrol;

uint32_t helioTestMillis = 0;

class FakeAxis : public AxisDriver {
 public:
  FakeAxis() : position_(0.0f), target_(0.0f), begun_(false) {}

  bool begin(float initialAngle, float) override {
    position_ = initialAngle;
    target_ = initialAngle;
    begun_ = true;
    return true;
  }
  void command(float angle) override { target_ = angle; }
  void update() override { position_ = target_; }
  void stop() override { target_ = position_; }
  float position() const override { return position_; }
  float target() const override { return target_; }
  bool ready() const override { return begun_; }

 private:
  float position_;
  float target_;
  bool begun_;
};

class FakeLight : public LightSensor {
 public:
  FakeLight() : ok_(true) { set(600, 600, 600, 600); }

  bool begin() override { return true; }
  bool read(LightReading &reading) override {
    reading = reading_;
    return ok_;
  }
  void set(uint16_t topLeft, uint16_t topRight, uint16_t bottomLeft,
           uint16_t bottomRight) {
    reading_.topLeft = topLeft;
    reading_.topRight = topRight;
    reading_.bottomLeft = bottomLeft;
    reading_.bottomRight = bottomRight;
    reading_.average = static_cast<uint16_t>(
        (static_cast<uint32_t>(topLeft) + topRight + bottomLeft +
         bottomRight) /
        4UL);
  }
  void fail() { ok_ = false; }

 private:
  LightReading reading_;
  bool ok_;
};

class FakeSafety : public SafetyInputs {
 public:
  FakeSafety() { memset(&status_, 0, sizeof(status_)); }
  bool begin() override { return true; }
  bool read(SafetyStatus &status) override {
    status = status_;
    return true;
  }
  SafetyStatus status_;
};

ControllerConfig fastConfig() {
  ControllerConfig config = defaultControllerConfig();
  config.tracking.updateIntervalMs = 50;
  config.tracking.darknessDelayMs = 100;
  config.tracking.lightRecoveryDelayMs = 0;
  config.tracking.windRecoveryDelayMs = 100;
  return config;
}

void testTrackingAndLimits() {
  helioTestMillis = 0;
  FakeAxis azimuth;
  FakeAxis elevation;
  FakeLight light;
  FakeSafety safety;
  light.set(900, 300, 900, 300);

  HelioController controller(azimuth, elevation, light, &safety);
  ControllerConfig config = fastConfig();
  assert(controller.begin(config));
  controller.update();

  assert(controller.state() == ControllerState::TRACKING);
  assert(azimuth.target() == config.azimuth.initialAngle +
                                  config.tracking.stepDegrees);
  assert(elevation.target() == config.elevation.initialAngle);

  assert(!controller.setManualTarget(181.0f, 45.0f));
  assert(controller.setManualTarget(100.0f, 45.0f));
  assert(controller.mode() == OperatingMode::MANUAL);
}

void testNightParking() {
  helioTestMillis = 0;
  FakeAxis azimuth;
  FakeAxis elevation;
  FakeLight light;
  light.set(10, 10, 10, 10);

  HelioController controller(azimuth, elevation, light);
  ControllerConfig config = fastConfig();
  assert(controller.begin(config));
  controller.update();
  assert(controller.state() == ControllerState::WAITING_FOR_LIGHT);

  helioTestMillis = 100;
  controller.update();
  assert(controller.state() == ControllerState::NIGHT_PARK);
  assert(azimuth.target() == config.azimuth.parkAngle);
  assert(elevation.target() == config.elevation.parkAngle);
}

void testWindAndEmergency() {
  helioTestMillis = 0;
  FakeAxis azimuth;
  FakeAxis elevation;
  FakeLight light;
  FakeSafety safety;
  HelioController controller(azimuth, elevation, light, &safety);
  ControllerConfig config = fastConfig();
  assert(controller.begin(config));

  safety.status_.wind = true;
  controller.update();
  assert(controller.state() == ControllerState::WIND_STOW);
  assert(azimuth.target() == config.azimuth.stowAngle);
  assert(elevation.target() == config.elevation.stowAngle);

  safety.status_.wind = false;
  safety.status_.emergencyStop = true;
  controller.update();
  assert(controller.state() == ControllerState::FAULT);
  assert((controller.status().faults & FAULT_EMERGENCY) != 0);
  assert(!controller.clearFaults());

  safety.status_.emergencyStop = false;
  controller.update();
  assert(controller.clearFaults());
  assert(controller.mode() == OperatingMode::STOPPED);
}

void testSensorFailure() {
  helioTestMillis = 0;
  FakeAxis azimuth;
  FakeAxis elevation;
  FakeLight light;
  HelioController controller(azimuth, elevation, light);
  assert(controller.begin(fastConfig()));
  light.fail();
  controller.update();
  assert(controller.state() == ControllerState::FAULT);
  assert((controller.status().faults & FAULT_SENSOR) != 0);
}

void testSolarCalculator() {
  const LocalDateTime invalid = {2026, 2, 30, 12, 0, 0};
  const GeoLocation laPlata = {-34.9214f, -57.9544f, -180};
  assert(!SolarCalculator::calculate(invalid, laPlata).valid);

  const LocalDateTime noon = {2026, 1, 15, 12, 0, 0};
  const SolarCoordinates sun = SolarCalculator::calculate(noon, laPlata);
  assert(sun.valid);
  assert(sun.aboveHorizon);
  assert(sun.azimuthDegrees >= 0.0f && sun.azimuthDegrees < 360.0f);
  assert(sun.elevationDegrees > 50.0f && sun.elevationDegrees < 90.0f);

  assert(normalizedDifference(1000, 0, 1000) == 100);
  assert(normalizedDifference(0, 1000, 1000) == -100);
  assert(normalizedDifference(500, 500, 1000) == 0);
}

int main() {
  testTrackingAndLimits();
  testNightParking();
  testWindAndEmergency();
  testSensorFailure();
  testSolarCalculator();
  puts("Todas las pruebas de lógica pasaron.");
  return 0;
}
