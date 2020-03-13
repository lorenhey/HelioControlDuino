# Diseño del controlador

## Prioridades

El ciclo principal actualiza siempre los drivers de eje y después evalúa, en
orden, las condiciones que pueden impedir el seguimiento:

```text
parada de emergencia o conflicto de límites
                    │
                    ▼
                  FALLA

alarma de viento ──► RESGUARDO ──► demora de recuperación
                                         │
                                         ▼
orden manual ──► STOP / PARK / STOW / MOVE
                                         │
                                         ▼
luz insuficiente ──► espera ──► estacionamiento nocturno
                                         │
                                         ▼
                              seguimiento automático
```

## Control por luz

Para cada eje se calcula una diferencia normalizada:

```text
error azimut = 100 × (izquierda - derecha) / suma de los cuatro LDR
error elevación = 100 × (arriba - abajo) / suma de los cuatro LDR
```

La normalización hace que la zona muerta sea útil tanto con cielo despejado
como nublado. Sólo se ordena un paso cuando el valor absoluto supera esa zona.
El `ServoAxis` limita después la velocidad física del objetivo.

## Temporización

Todas las demoras se calculan con restas de enteros sin signo, por lo que siguen
funcionando correctamente cuando `millis()` desborda. Ninguna transición usa
esperas bloqueantes.

## Alcance

La implementación de referencia conoce la posición ordenada al servo, no la
posición mecánica real. Para estructuras pesadas o sistemas donde una falla de
movimiento sea peligrosa se debe usar un `AxisDriver` con encoder y supervisión
independiente.
