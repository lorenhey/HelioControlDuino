# API de la biblioteca

Incluí la cabecera única:

```cpp
#include <HelioControlDuino.h>
using namespace heliocontrol;
```

## Componentes principales

- `HelioController`: máquina de estados, seguimiento y prioridades de seguridad.
- `ServoAxis`: driver gradual para servos de posición.
- `FourLdrSensor`: adquisición, sobremuestreo y filtrado de cuatro LDR.
- `DigitalSafetyInputs`: viento, emergencia y cuatro finales de carrera.
- `SerialConsole`: interfaz de operación por cualquier objeto `Stream`.
- `SolarCalculator`: azimut y elevación solar a partir de fecha y ubicación.

## Configuración mínima

```cpp
ServoAxis azimuth(9);
ServoAxis elevation(10);
FourLdrSensor light(A0, A1, A2, A3);
HelioController tracker(azimuth, elevation, light);

void setup() {
  ControllerConfig config = defaultControllerConfig();
  tracker.begin(config);
}

void loop() {
  tracker.update();
}
```

`update()` debe ejecutarse con frecuencia. No usa `delay()` ni bloquea durante
el movimiento.

## Hardware personalizado

Para usar otro actuador, implementá `AxisDriver`. Para usar otro sistema de
sensado o seguridad, implementá `LightSensor` o `SafetyInputs`. Las interfaces
no reservan memoria dinámica y pueden usarse en placas AVR con poca RAM.

Un `AxisDriver` debe:

- aceptar un objetivo dentro de su rango físico;
- avanzar de forma no bloqueante desde `update()`;
- informar su posición y objetivo;
- retener o detener el eje cuando se llama `stop()`.

## Posición solar

```cpp
LocalDateTime time = {2026, 1, 15, 12, 0, 0};
GeoLocation place = {-34.9214f, -57.9544f, -180};
SolarCoordinates sun = SolarCalculator::calculate(time, place);
```

El horario es local y `utcOffsetMinutes` incluye el desplazamiento horario
vigente. El azimut se expresa de 0° a 360° en sentido horario desde el norte y
la elevación respecto del horizonte. El cálculo no obtiene la hora por sí mismo;
un modo astronómico permanente necesita un RTC o una fuente horaria equivalente.
