#include <HelioControlDuino.h>

using namespace heliocontrol;

ServoAxis azimuthAxis(9);
ServoAxis elevationAxis(10);
FourLdrSensor lightSensor(A0, A1, A2, A3);
HelioController tracker(azimuthAxis, elevationAxis, lightSensor);

void setup() {
  Serial.begin(115200);

  ControllerConfig config = defaultControllerConfig();
  // Cambie estos signos si un servo se mueve en el sentido incorrecto.
  config.tracking.azimuthDirection = 1;
  config.tracking.elevationDirection = 1;

  if (!tracker.begin(config)) {
    Serial.println(F("No se pudo iniciar HelioControlDuino."));
  }
}

void loop() { tracker.update(); }
