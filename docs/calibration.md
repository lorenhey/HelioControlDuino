# Calibración y primera puesta en marcha

Realizá estas pruebas sin el panel acoplado a la transmisión. Un valor angular
incorrecto puede llevar el mecanismo contra un tope.

## 1. Comprobar los LDR

1. Cargá el firmware completo y abrí el monitor serie a 115200 baudios.
2. Ejecutá `STATUS` con iluminación uniforme.
3. Tapá cada LDR por separado y comprobá que baja la lectura media.
4. Si sube al taparlo, configurá `LDR_INVERT_READINGS = true`.
5. Corregí cualquier sensor intercambiado antes de continuar.

La orden `STATUS` muestra `LIGHT` como el promedio y `ERR` como los errores de
azimut y elevación en porcentaje. Con luz uniforme ambos errores deben quedar
cerca de cero.

## 2. Determinar el sentido de los ejes

1. Ejecutá `MOVE 100 30` y observá el sentido de azimut.
2. Ejecutá `MOVE 80 40` y observá ambos movimientos.
3. Volvé a `MOVE 90 20`.
4. Si un eje corrige alejándose de la luz, cambiá su dirección de `1` a `-1`:

```cpp
config.tracking.azimuthDirection = -1;
config.tracking.elevationDirection = 1;
```

## 3. Definir límites seguros

Medí el recorrido útil sin alcanzar los topes mecánicos y configurá:

```cpp
config.azimuth.minimumAngle = 10.0f;
config.azimuth.maximumAngle = 170.0f;
config.elevation.minimumAngle = 15.0f;
config.elevation.maximumAngle = 90.0f;
```

Las posiciones inicial, nocturna y de resguardo deben estar dentro de esos
intervalos. La configuración se rechaza si no cumple esa condición.

## 4. Ajustar estabilidad

- Aumentá `deadbandPercent` si el sistema oscila con nubes o reflejos.
- Reducí `stepDegrees` para movimientos más finos.
- Bajá `maximumSpeedDegreesPerSecond` si la estructura vibra.
- Aumentá `LDR_SMOOTHING` para reaccionar más rápido; reducí el valor para
  filtrar más ruido.

Valores iniciales recomendados:

| Parámetro | Valor |
| --- | --- |
| Zona muerta | 6 % |
| Paso de corrección | 1° |
| Intervalo | 500 ms |
| Suavizado | 0,25 |
| Velocidad azimut | 20°/s |
| Velocidad elevación | 15°/s |

## 5. Calibrar la oscuridad

Consultá `LIGHT` a pleno día y al anochecer. Elegí
`darknessThreshold` por encima del valor nocturno y muy por debajo del valor de
día nublado. La demora predeterminada de cinco minutos evita estacionamientos
por sombras breves.

## 6. Probar la seguridad

Con el mecanismo descargado:

1. activá cada final de carrera y verificá que no permita avanzar hacia ese
   extremo;
2. activá viento y comprobá el desplazamiento a `stowAngle`;
3. activá la parada de emergencia y verificá `STATE=FAULT`;
4. liberá la entrada, ejecutá `RESET` y luego `AUTO`;
5. simulá un sensor desconectado y revisá que el comportamiento mecánico siga
   siendo seguro.

Repetí la prueba de seguridad con una persona junto al corte físico de energía
antes de acoplar el panel.
