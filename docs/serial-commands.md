# Consola serie

El firmware usa 115200 baudios, 8 bits, sin paridad y un bit de parada. Cada
orden termina con `LF`, `CR` o ambos. No distingue mayúsculas de minúsculas.

| Comando | Acción |
| --- | --- |
| `HELP` o `?` | Lista resumida de órdenes |
| `STATUS` | Estado, ángulos, luz, errores y seguridad |
| `AUTO` | Habilita el seguimiento automático |
| `PARK` | Lleva ambos ejes a la posición de estacionamiento |
| `STOW` | Lleva ambos ejes a la posición de resguardo |
| `STOP` | Mantiene la posición actual |
| `MOVE <az> <el>` | Fija un objetivo manual en grados |
| `RESET` | Borra fallas si ya no existe una condición insegura |
| `VERSION` | Informa la versión instalada |

Ejemplo:

```text
> MOVE 105 42
OK objetivo manual aceptado
> STATUS
MODE=MANUAL STATE=MANUAL FAULTS=0x0 AZ=96.4/105.0 EL=35.8/42.0 LIGHT=638 ERR=1,-2 WIND=0 ESTOP=0
```

`AZ` y `EL` muestran `posición/objetivo`. La posición es una estimación del
ángulo ordenado al servo; no es una medición física salvo que el driver de eje
usado implemente realimentación.

## Códigos de falla

`FAULTS` es una máscara hexadecimal:

| Bit | Hex | Significado |
| --- | --- | --- |
| 0 | `0x01` | Configuración inválida |
| 1 | `0x02` | Error al leer el sensor de luz |
| 2 | `0x04` | Parada de emergencia |
| 3 | `0x08` | Finales mínimo y máximo activos a la vez |
| 4 | `0x10` | No se pudo iniciar un eje |
| 5 | `0x20` | Error en las entradas de seguridad |

Las fallas detienen el seguimiento. `RESET` sólo se acepta cuando las entradas
de emergencia y límites vuelven a un estado coherente; después el controlador
queda detenido y requiere `AUTO`, `PARK`, `STOW` o `MOVE`.
