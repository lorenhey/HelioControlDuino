# Hardware y cableado

## Configuración de referencia

El firmware incluido está preparado para un seguidor de dos ejes accionado por
servos de posición de 0° a 180°. Para motores DC, actuadores lineales o motores
paso a paso se debe implementar la interfaz `AxisDriver`; la lógica de
seguimiento no necesita cambios.

### Pines predeterminados

| Señal | Arduino Uno/Nano/Mega | Estado inicial |
| --- | --- | --- |
| LDR superior izquierda | A0 | habilitado |
| LDR superior derecha | A1 | habilitado |
| LDR inferior izquierda | A2 | habilitado |
| LDR inferior derecha | A3 | habilitado |
| Señal servo azimut | D9 | habilitado |
| Señal servo elevación | D10 | habilitado |
| Alarma de viento | configurable | deshabilitado |
| Parada de emergencia | configurable | deshabilitado |
| Finales de carrera | configurables | deshabilitados |

Todos los pines se cambian en
`examples/CompleteController/config.h`.

## Sensor de cuatro LDR

Colocá los cuatro sensores en una matriz de 2 × 2 y un separador opaco en cruz
entre ellos:

```text
          parte superior
        ┌────────┬────────┐
        │   A0   │   A1   │
        │ sup izq│ sup der│
        ├────────┼────────┤
        │   A2   │   A3   │
        │ inf izq│ inf der│
        └────────┴────────┘
          salida de cables
```

Cada LDR forma un divisor resistivo:

```text
5 V ── LDR ──┬── entrada analógica
             │
            10 kΩ
             │
GND ─────────┘
```

Con este circuito, una iluminación mayor produce una lectura mayor. Si el LDR y
la resistencia están intercambiados, configurá `LDR_INVERT_READINGS = true`.

El separador debe ser perpendicular a la placa de sensores, tener entre 15 y
30 mm de alto y proyectar una diferencia apreciable cuando el conjunto no
apunta al Sol. Los cuatro LDR deben ser del mismo modelo y quedar lo más cerca
posible entre sí.

## Servos y alimentación

Las salidas D9 y D10 son únicamente señales de control. Conectá:

- señal de azimut a D9;
- señal de elevación a D10;
- positivo de ambos servos a una fuente regulada externa;
- negativos de la fuente, los servos y Arduino a una masa común.

Agregá un capacitor electrolítico cerca de los servos y dimensioná cables,
fuente y fusible para la corriente de bloqueo. Nunca alimentes un servo de
potencia desde el regulador de la placa.

## Entradas de seguridad

Las entradas opcionales trabajan por defecto como contactos normalmente
cerrados hacia masa mediante `INPUT_PULLUP` y lógica activa en nivel bajo. Esta
configuración permite detectar un cable cortado como condición inactiva sólo si
se añade supervisión eléctrica externa; en instalaciones críticas se recomienda
un relé de seguridad dedicado.

Ejemplo de asignación:

```cpp
static const int8_t PIN_WIND_ALARM = 2;
static const int8_t PIN_EMERGENCY_STOP = 3;
static const int8_t PIN_AZIMUTH_MINIMUM = 4;
static const int8_t PIN_AZIMUTH_MAXIMUM = 5;
static const int8_t PIN_ELEVATION_MINIMUM = 6;
static const int8_t PIN_ELEVATION_MAXIMUM = 7;
```

El disparo por viento ordena la posición de resguardo y el sistema sólo retoma
la operación después del tiempo de recuperación configurado. La parada de
emergencia queda registrada como falla; se libera con `RESET` después de retirar
la condición insegura.

## Actuadores de potencia

No conectes relés, motores, contactores ni actuadores lineales directamente a
los pines. Usá drivers con aislamiento y protecciones adecuadas. Una instalación
exterior debe contemplar, como mínimo:

- finales de carrera cableados también en el circuito de potencia;
- fusibles o disyuntores y protección contra sobrecorriente;
- corte físico de emergencia;
- sensor de viento independiente;
- puesta a tierra, protección contra sobretensiones y gabinete estanco;
- topes mecánicos que no dependan del software.
