#ifndef HELIO_CONTROL_DUINO_COMPLETE_CONTROLLER_CONFIG_H
#define HELIO_CONTROL_DUINO_COMPLETE_CONTROLLER_CONFIG_H

#include <HelioControlDuino.h>

using namespace heliocontrol;

// Sensores: vistos de frente, con el cableado saliendo hacia abajo.
static const uint8_t PIN_LDR_TOP_LEFT = A0;
static const uint8_t PIN_LDR_TOP_RIGHT = A1;
static const uint8_t PIN_LDR_BOTTOM_LEFT = A2;
static const uint8_t PIN_LDR_BOTTOM_RIGHT = A3;

static const uint8_t PIN_SERVO_AZIMUTH = 9;
static const uint8_t PIN_SERVO_ELEVATION = 10;
static const uint8_t PIN_STATUS_LED = LED_BUILTIN;

// Use PIN_UNUSED para toda entrada que no esté instalada.
static const int8_t PIN_WIND_ALARM = PIN_UNUSED;
static const int8_t PIN_EMERGENCY_STOP = PIN_UNUSED;
static const int8_t PIN_AZIMUTH_MINIMUM = PIN_UNUSED;
static const int8_t PIN_AZIMUTH_MAXIMUM = PIN_UNUSED;
static const int8_t PIN_ELEVATION_MINIMUM = PIN_UNUSED;
static const int8_t PIN_ELEVATION_MAXIMUM = PIN_UNUSED;

static const uint8_t LDR_SAMPLES = 8;
static const float LDR_SMOOTHING = 0.25f;
// Cambiar a true si más luz produce una lectura analógica más baja.
static const bool LDR_INVERT_READINGS = false;
static const uint16_t ADC_MAXIMUM = 1023;

inline ControllerConfig makeTrackerConfig() {
  ControllerConfig config = defaultControllerConfig();

  config.azimuth.minimumAngle = 10.0f;
  config.azimuth.maximumAngle = 170.0f;
  config.azimuth.initialAngle = 90.0f;
  config.azimuth.parkAngle = 90.0f;
  config.azimuth.stowAngle = 90.0f;
  config.azimuth.maximumSpeedDegreesPerSecond = 20.0f;

  config.elevation.minimumAngle = 15.0f;
  config.elevation.maximumAngle = 90.0f;
  config.elevation.initialAngle = 20.0f;
  config.elevation.parkAngle = 20.0f;
  config.elevation.stowAngle = 15.0f;
  config.elevation.maximumSpeedDegreesPerSecond = 15.0f;

  config.tracking.darknessThreshold = 120;
  config.tracking.deadbandPercent = 6;
  config.tracking.stepDegrees = 1.0f;
  // Cambiar el signo si un eje se mueve en sentido contrario.
  config.tracking.azimuthDirection = 1;
  config.tracking.elevationDirection = 1;
  config.tracking.updateIntervalMs = 500;
  config.tracking.darknessDelayMs = 5UL * 60UL * 1000UL;
  config.tracking.lightRecoveryDelayMs = 30UL * 1000UL;
  config.tracking.windRecoveryDelayMs = 60UL * 1000UL;

  return config;
}

inline SafetyPins makeSafetyPins() {
  SafetyPins pins = defaultSafetyPins();
  pins.wind = PIN_WIND_ALARM;
  pins.emergencyStop = PIN_EMERGENCY_STOP;
  pins.azimuthMinimum = PIN_AZIMUTH_MINIMUM;
  pins.azimuthMaximum = PIN_AZIMUTH_MAXIMUM;
  pins.elevationMinimum = PIN_ELEVATION_MINIMUM;
  pins.elevationMaximum = PIN_ELEVATION_MAXIMUM;
  pins.activeLow = true;
  pins.useInternalPullups = true;
  return pins;
}

#endif
