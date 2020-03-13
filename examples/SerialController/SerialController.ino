#include <HelioControlDuino.h>

using namespace heliocontrol;

ServoAxis azimuthAxis(9);
ServoAxis elevationAxis(10);
FourLdrSensor lightSensor(A0, A1, A2, A3, 8, 0.25f);
DigitalSafetyInputs safety(defaultSafetyPins());
HelioController tracker(azimuthAxis, elevationAxis, lightSensor, &safety);
SerialConsole console(Serial, tracker);

void setup() {
  Serial.begin(115200);
  tracker.begin(defaultControllerConfig());
  console.begin();
}

void loop() {
  tracker.update();
  console.poll();
}
