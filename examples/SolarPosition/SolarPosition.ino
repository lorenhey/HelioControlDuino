#include <HelioControlDuino.h>

using namespace heliocontrol;

void setup() {
  Serial.begin(115200);

  // Hora local y ubicación de La Plata, Argentina.
  const LocalDateTime time = {2026, 1, 15, 12, 0, 0};
  const GeoLocation location = {-34.9214f, -57.9544f, -180};
  const SolarCoordinates sun = SolarCalculator::calculate(time, location);

  if (!sun.valid) {
    Serial.println(F("Fecha o ubicacion invalida."));
    return;
  }

  Serial.print(F("Azimut: "));
  Serial.println(sun.azimuthDegrees, 2);
  Serial.print(F("Elevacion: "));
  Serial.println(sun.elevationDegrees, 2);
  Serial.print(F("Sobre el horizonte: "));
  Serial.println(sun.aboveHorizon ? F("si") : F("no"));
}

void loop() {}
