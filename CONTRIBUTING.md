# Contribuir a HelioControlDuino

Gracias por ayudar a mejorar el controlador. Los cambios deben conservar el
funcionamiento autónomo, el uso no bloqueante y la compatibilidad con placas de
recursos limitados.

## Antes de empezar

1. Buscá si ya existe un reporte relacionado.
2. Para fallas de seguridad, seguí [SECURITY.md](SECURITY.md) y no abras un
   reporte público.
3. Para cambios grandes, describí primero el caso de uso y el hardware
   involucrado.

## Desarrollo

- Evitá memoria dinámica en el ciclo de control.
- No uses `delay()` en la lógica que se ejecuta continuamente.
- Mantené las protecciones y límites por encima de las órdenes de seguimiento.
- Documentá pines, unidades, rangos y supuestos de hardware.
- Añadí o actualizá pruebas cuando cambie la máquina de estados o el cálculo.

## Verificación

Con Arduino CLI disponible en `PATH`, ejecutá en PowerShell:

```powershell
./scripts/verify.ps1
```

El script instala el core AVR y compila todos los ejemplos para Arduino Uno y
Mega 2560. La integración continua también ejecuta las pruebas de lógica y
Arduino Lint en modo estricto.

## Envío de cambios

1. Creá una rama descriptiva.
2. Mantené los commits enfocados y con mensajes claros.
3. Confirmá que las verificaciones locales pasen.
4. Abrí un pull request explicando el problema, la solución y el hardware
   probado.

Al contribuir aceptás que tu trabajo se distribuya bajo la Licencia MIT del
proyecto.
