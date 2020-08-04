$ErrorActionPreference = 'Stop'

$cli = Get-Command arduino-cli -ErrorAction Stop
$root = Split-Path -Parent $PSScriptRoot

& $cli.Source core update-index
& $cli.Source core install arduino:avr
& $cli.Source lib install Servo

$targets = @(
    'examples/BasicDualAxis',
    'examples/SerialController',
    'examples/SolarPosition',
    'examples/CompleteController'
)

foreach ($board in @('arduino:avr:uno', 'arduino:avr:mega')) {
    foreach ($target in $targets) {
        & $cli.Source compile --fqbn $board --library $root (Join-Path $root $target)
        if ($LASTEXITCODE -ne 0) {
            throw "Falló la compilación de $target para $board"
        }
    }
}
