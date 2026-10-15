# Bancos de pruebas de render: R1 (linea base) y R2 (B+)

Estado de R2: PROPUESTO (sin ensamblar ni probar). La logica de ventanas se verifico con un modelo en Python.

## Resultados de R1 (medidos por Carlos)
Referencia: RetroVirtualMachine, ZX Spectrum 128K (primer modelo). Spectaculator abre el .sna como Pentagon (el formato .sna de 128K no indica el modelo; Fuse cae a 48K porque no tiene la ROM del Pentagon).

| N | ESC | Maquina | LIBRE | MIN | RETR | PERD | Franja azul + verde |
|---|---|---|---:|---:|---:|---:|---|
| 1 | 1 | Pentagon | 910 | 1503 | 0 | 0 | primer tile del HUD |
| 3 | 1 | Pentagon | 700 | 560 | 0 | 0 | primer tercio (algun fotograma el segundo) |
| 5 | 1 | Pentagon | 498 | 423 | 0 | 0 | primer tercio (algun fotograma el segundo) |
| 5 | 0 | Pentagon | 490 | 0 | 28 | 28 | toda la pantalla |
| 8 | 1 | Pentagon | 160 | 0 | 14 | 14 | toda la pantalla |
| 1 | 1 | RVM 128K | 896 | 1472 | 0 | 0 | primer tile del HUD |
| 5 | 1 | RVM 128K | 460 | 424 | 0 | 0 | primer tercio (algun fotograma el segundo) |
| 5 | 0 | RVM 128K | 450 | 0 | 28 | 28 | toda la pantalla |

Lectura (128K, 70.908 T-states por fotograma; Pentagon 71.680):
- Coste por entidad que se mueve (borrar + pintar + logica): unos 18.000 T-states de media (N = 1: 18.700; N = 5: 17.700); en el peor fotograma con 3 movimientos unos 56.000 T.
- La contencion de la RAM de pantalla cuesta unos 3 % sobre el Pentagon.
- Con 6 entidades escalonadas el maximo es de 3 movimientos por fotograma (cabe); con 7 es de 4 (se pierden fotogramas); con 8 de 5. Sin escalonar (ESC = 0) con 5 entidades, el 44 % de los fotogramas tienen 5 movimientos: RETR 28 de cada 64, como predice el modelo.
- Limite practico del motor actual: 3 movimientos por fotograma con margen; 4 solo sin nada mas (logica, sonido, HUD).

## R2: que cambia
- Fondo limpio en el banco 0 ($C000, copia de los 6.144 bytes de pixeles de la pantalla sin sprites; direccion = pantalla + $8000). En un 48K ese area es RAM normal y el banco de pruebas tambien funciona.
- Preparacion en buffers del banco 2 (uno por entidad, 128 bytes alineados): ventana de 3 bytes x (24 + |dy|) lineas = union de la posicion vieja y la nueva, compuesta como fondo + sprite con mascara. No toca la pantalla.
- Volcado al principio del fotograma siguiente: 3 LDI por linea. Es lo unico que escribe en la pantalla; si termina antes de que el haz llegue al campo (unos 19.800 T desde la interrupcion para la linea 24) no hay parpadeo ni cizalla.
- Medida nueva SOLP: ventanas que se solapan con otra entidad (cota conservadora). R2 no resuelve los solapes; sirve para saber con que frecuencia habra que usar un camino lento.

## Uso
1. Copiar `bench_r2.asm` y `build_bench.bat` junto a `main.asm`.
2. `build_bench.bat bench_r2` (o `build_bench.bat bench_r1`). Salida: `build\bench_r2.sna`. Abrirlo en RetroVirtualMachine con 128K seleccionado.
3. Parametros al principio del .asm: BENCH_N, BENCH_STAGGER, BENCH_SPEED, BENCH_PROF, BENCH_OVL.
4. Franjas del borde en R2: verde = volcado, rojo = logica, azul = preparacion, negro = libre.

## Tabla de pruebas de R2 (rellenar)
| N | ESC | LIBRE | MIN | RETR | PERD | SOLP | Verde (hasta donde llega) |
|---|---|---|---|---|---|---|---|
| 1 | 1 | | | | | | |
| 5 | 1 | | | | | | |
| 5 | 0 | | | | | | |
| 8 | 1 | | | | | | |

## Prevision de R2 (estimacion; a contrastar)
- Coste por movimiento: unos 10.000 T-states (copia del fondo ~3.200, sprite ~3.500, volcado ~3.400), es decir, algo menos de la mitad que R1.
- N = 5 con escalonado: MIN cerca de 650; N = 8 con escalonado (hasta 5 movimientos): MIN cerca de 450 y sin retrasos.
- El volcado de 5 ventanas debe acabar en unos 17.000 T, antes de que el haz llegue a la linea 24.
- Siguientes optimizaciones previstas para R3 (si hacen falta): tabla de direcciones de linea en lugar de down_hl, rutinas desenrolladas, camino lento para solapes.
