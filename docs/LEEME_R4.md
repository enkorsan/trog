# R4 - ruta lenta mas barata, fondo lineal, volcado en linea y escalonado aleatorio

Estado: PROPUESTO (sin ensamblar ni probar). El algoritmo se verifico con un modelo en Python (0 diferencias con la imagen ideal en 2.500 fotogramas con 2, 3, 5 y 8 entidades, con solapes, acumuladores aleatorios y paradas). Las rutinas en Z80 no se han ejecutado.

## Resultados de R3 (medidos por Carlos, RetroVirtualMachine 128K toastrack)
| N | ESC | SLOW | LIBRE | MIN | RETR | PERD | SOLP (max.) | Azul |
|---|---|---|---:|---:|---:|---:|---:|---|
| 1 | 1 | 1 | 925 | 1744 | 0 | 0 | 0 | no toca la zona de juego |
| 5 | 1 | 0 | 613 | 985-1053 | 0 | 0 | 0 | primer tercio |
| 5 | 1 | 1 | 468-595 | 338-949 | 0 | 0 | 74 | entre primer y segundo tercio |
| 5 | 0 | 1 | 466-590 | 0 | 9 | 9 | 53 | entre segundo tercio y toda la pantalla |
| 8 | 1 | 1 | 90-279 | 0 | 0-43 | 0-43 | 84 | casi toda la pantalla |
| 8 | 0 | 1 | 133-315 | 0 | 28-56 | 28-56 | 91 | toda la pantalla |

Lectura: el test de solape barato ahorra ~2.000 T por fotograma (SLOW = 0: 613 frente a 585 de R2); la ruta lenta cuesta de media ~5.700 T por fotograma con N = 5 (unos 5.000-7.000 T por ventana lenta). Con N = 5 y escalonado no se pierden fotogramas, pero el peor fotograma deja solo ~11.800 T libres. Prioridades correctas; no hay parpadeos aunque se pierdan fotogramas.

## Cambios de R4
1. Ruta lenta: la entidad propia se pinta por la ruta rapida; antes se pintan, recortados, los vecinos de menor prioridad y despues los de mayor. Ahorro previsto: ~3.000 T por ventana lenta.
2. draw_clipped con tres bucles (1, 2 y 3 bytes por linea) sin variables en el bucle. Ancho, puntero de datos, columna y linea superior de cada entidad se precalculan al moverse (E_QW, E_DP, E_QC, E_QT).
3. Fondo limpio lineal en el banco 0 ($C000 + linea * 32 + byte): la copia a la ventana avanza 29 bytes por linea; el volcado baja de linea sin llamar a down_hl. Ahorro previsto: ~0,8 k T en la copia y ~0,5 k T en el volcado por ventana.
4. BENCH_STAGGER = 2 (acumuladores iniciales aleatorios) y BENCH_STOP = 1 (las entidades se paran a veces en las intersecciones: 1 de cada 8, 16 a 47 fotogramas).

## Uso
1. Copiar `bench_r4.asm` junto a `main.asm`. `build_bench.bat bench_r4`. Abrir `build\bench_r4.sna` en RetroVirtualMachine con 128K.
2. Parametros al principio del .asm: BENCH_N, BENCH_STAGGER (0, 1, 2), BENCH_SPEED, BENCH_PROF, BENCH_SLOW, BENCH_STOP.
3. Franjas del borde: verde = volcado, rojo = logica, azul = preparacion, negro = libre.

## Pruebas (rellenar)
| N | ESC | SLOW | STOP | LIBRE | MIN | RETR | PERD | SOLP | Azul |
|---|---|---|---|---|---|---|---|---|---|
| 1 | 1 | 1 | 0 | | | | | | |
| 5 | 1 | 0 | 0 | | | | | | |
| 5 | 1 | 1 | 0 | | | | | | |
| 5 | 2 | 1 | 0 | | | | | | |
| 5 | 2 | 1 | 1 | | | | | | |
| 5 | 0 | 1 | 0 | | | | | | |
| 8 | 1 | 1 | 0 | | | | | | |

Con ESC = 2 el resultado cambia en cada arranque (el generador aleatorio tiene semilla fija, pero la secuencia de llamadas depende de los parametros): anotar dos o tres arranques. Comprobar a ojo que las prioridades siguen bien y que no queda ningun rectangulo.

## Prevision (estimacion; a contrastar)
- Coste por movimiento: ~8.600 T (copia del fondo ~2.400, sprite ~3.500, volcado ~2.700), frente a ~10.000 T en R3.
- Coste extra de una ventana lenta: ~3.000-3.500 T (frente a ~6.000 T).
- N = 5, ESC = 1, SLOW = 1: LIBRE ~600-650 (frente a ~530) y MIN por encima de 600 (frente a 338).

## Pendiente para R5 (si hace falta)
Recorte por arriba y por abajo (celda adicional de la casa), datos de sprites en los bancos 4 y 6 (contencion), rutinas desenrolladas y decision sobre el tope de ventanas por fotograma (requiere ventana de 4 bytes si se aplaza un movimiento).
