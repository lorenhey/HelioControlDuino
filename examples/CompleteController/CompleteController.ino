#include <HelioControlDuino.h>

#include "config.h"

using namespace heliocontrol;

const ControllerConfig TRACKER_CONFIG = makeTrackerConfig();
const SafetyPins SAFETY_PINS = makeSafetyPins();

ServoAxis azimuthAxis(PIN_SERVO_AZIMUTH);
ServoAxis elevationAxis(PIN_SERVO_ELEVATION);
FourLdrSensor lightSensor(
    PIN_LDR_TOP_LEFT, PIN_LDR_TOP_RIGHT, PIN_LDR_BOTTOM_LEFT,
    PIN_LDR_BOTTOM_RIGHT, LDR_SAMPLES, LDR_SMOOTHING,
    LDR_INVERT_READINGS, ADC_MAXIMUM);
DigitalSafetyInputs safetyInputs(SAFETY_PINS);
HelioController tracker(azimuthAxis, elevationAxis, lightSensor,
                        &safetyInputs);
SerialConsole console(Serial, tracker);

uint32_t lastLedChange = 0;
bool ledOn = false;

uint32_t ledIntervalFor(ControllerState state) {
  switch (state) {
    case ControllerState::FAULT:
      return 100;
    case ControllerState::WIND_STOW:
      return 250;
    case ControllerState::TRACKING:
      return 1000;
    case ControllerState::NIGHT_PARK:
    case ControllerState::PARKED:
      return 2000;
    default:
      return 500;
  }
}

void updateStatusLed() {
  const uint32_t now = millis();
  const uint32_t interval = ledIntervalFor(tracker.state());
  if (static_cast<uint32_t>(now - lastLedChange) >= interval) {
    lastLedChange = now;
    ledOn = !ledOn;
    digitalWrite(PIN_STATUS_LED, ledOn ? HIGH : LOW);
  }
}

void setup() {
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  Serial.begin(115200);
  const uint32_t serialWaitStarted = millis();
  while (!Serial && static_cast<uint32_t>(millis() - serialWaitStarted) <
                        2000UL) {
  }

  const bool ready = tracker.begin(TRACKER_CONFIG);
  console.begin();
  if (!ready) {
    Serial.println(F("ERROR no se pudo iniciar; ejecute STATUS"));
  } else {
    Serial.println(F("OK controlador listo"));
  }
  console.printStatus();
}

void loop() {
  tracker.update();
  console.poll();
  updateStatusLed();
}
