# Banco de pruebas R1 - linea base del motor actual (borrar y pintar) con N entidades

Estado: PROPUESTO. Sin ensamblar ni probar (el asistente no tiene sjasmplus). Los numeros "previstos" son estimaciones del asistente por conteo de instrucciones, no mediciones.

## Archivos
- `bench_r1.asm`: el banco. Colocarlo junto a `main.asm` (usa `config.inc`, `video.inc`, `stones.inc`, `sprite_dino.inc` y `font_msg.inc` con INCLUDE relativo).
- `build_bench.bat`: ensambla y deja `build\bench_r1.sna`. No toca nada del juego.

## Parametros (al principio de bench_r1.asm)
| EQU | Valor | Significado |
|---|---|---|
| BENCH_N | 5 | Entidades (1..8). 5 = 1 dino + 4 Trogs |
| BENCH_STAGGER | 1 | 1 = acumuladores iniciales 3*i mod 16 (escalonado); 0 = todos en fase |
| BENCH_SPEED | 7 | Pasos de 2 px cada 16 fotogramas (7 = DINO_SPEED actual) |
| BENCH_PROF | 1 | Franjas de color en el borde: rojo = logica, azul = borrado, verde = pintado, negro = libre |

## Como leer el HUD (ventana de 64 fotogramas, unos 1,3 s)
- LIBRE: tiempo libre medio por fotograma, en unidades de 70 T-states. Un fotograma de 128K = 70.908 T = unas 1013 unidades (LIBRE / 10 = % libre aproximado).
- MIN: menor tiempo libre de un fotograma de la ventana, en unidades de 35 T-states (unas 2026 = fotograma entero libre). 0 = hubo retraso.
- RETR: iteraciones que acabaron despues de la siguiente interrupcion (el juego va mas lento).
- PERD: fotogramas perdidos en la ventana.
- SPR y ESC repiten BENCH_N y BENCH_STAGGER.
- Las entidades no se evitan entre si: si se solapan puede quedar algun resto de sprite (no afecta a la medida).

## Pruebas a hacer (rellenar con lo que se vea; esperar dos o tres ventanas)
| N | ESC | LIBRE | MIN | RETR | PERD | Franja azul + verde (lineas de borde) |
|---|---|---|---|---|---|---|
| 1 | 1 | | | | | |
| 3 | 1 | | | | | |
| 5 | 1 | | | | | |
| 5 | 0 | | | | | |
| 8 | 1 | | | | | |

Para cada fila, cambiar BENCH_N y BENCH_STAGGER, ejecutar build_bench.bat y abrir build\bench_r1.sna (128K). Con BENCH_PROF = 1 anotar tambien cuantas lineas de borde ocupan el rojo, el azul y el verde (1 linea = 228 T-states).

## Resultado previsto (estimacion; a contrastar)
- Coste por entidad que se mueve (borrar + pintar): unos 17.000-18.000 T-states.
- N = 5 con escalonado: como maximo 3 movimientos por fotograma y 2,19 de media -> unos 54.000 T en el peor fotograma: MIN cerca de 400-450 y RETR = 0.
- N = 5 sin escalonado (ESC = 0): el 44 % de los fotogramas tienen 5 movimientos (unos 90.000 T): RETR de unos 25-30 por ventana.
- N = 8 con escalonado: hasta 5 movimientos en algun fotograma: se esperan retrasos.

## Siguiente paso (R2)
Mismo escenario con la variante B+: fondo base en el banco 0 (pantalla completa, 6.912 bytes, direccion = pantalla + $8000), buffers de preparacion en el banco 2 y volcado ordenado de arriba abajo. Se compara con esta tabla.
