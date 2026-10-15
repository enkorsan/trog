# R3 - solapes con prioridad, test de solape barato y medida sin picos

Estado: PROPUESTO (sin ensamblar ni probar). El algoritmo se verifico con un modelo en Python (0 diferencias con la imagen ideal en 2.500 fotogramas con 2, 3, 5 y 8 entidades, con solapes).

## Resultados de R2 (medidos por Carlos, RetroVirtualMachine 128K)
| N | ESC | LIBRE | MIN | RETR | PERD | SOLP (max.) | Verde (hasta donde llega) |
|---|---|---:|---:|---:|---:|---:|---|
| 1 | 1 | 924 | 1405 | 0 | 0 | 0 | muy por encima de la pantalla |
| 5 | 1 | 585 | 547 | 0 | 0 | 64 | muy por encima de la pantalla |
| 5 | 0 | 585 | 530-640 | 0 | 0 | 77 | tapa el HUD |
| 8 | 1 | 310 | 80 | 0 | 0 (alguna vez 1) | 150 | primera fila del HUD |
| 8 | 0 | 298 | 0 | 28 | 28 | 150 | mitad de la pantalla (sin parpadeos) |

Observacion: al solaparse dos sprites se veia el rectangulo completo. No es que falle la mascara: R2 vuelca ventanas rectangulares de 3 bytes (24 px) con el fondo y solo el sprite de esa entidad, asi que el rectangulo borra la parte del otro sprite que cae dentro. R3 lo resuelve componiendo en la ventana todos los sprites que la tocan.

## Prioridad de pintado (observaciones de Carlos en MAME)
1. Misma columna (misma X, distinta Y): encima el que esta mas abajo (mayor Y).
2. Misma fila (misma Y): el dino siempre encima.
3. Trogs en la misma fila: sin patron claro; R3 usa un desempate fijo por indice. Las acciones (un Trog come al dino) parecen dar prioridad al que actua y se estudiaran mas adelante (se desarrollan en las 4 direcciones).
Implementacion: orden ascendente por (Y, E_TIE), con E_TIE = 255 para el dino y el indice para los Trogs. La lista se reordena (burbuja, casi ordenada) solo cuando hace falta, una vez por fotograma.

## Cambios de R3 respecto a R2
- Ruta lenta (BENCH_SLOW = 1): si la ventana de una entidad toca a otra, se pintan en la ventana todos los sprites que la cortan, recortados y de menor a mayor prioridad. Sin solape se usa la ruta rapida de R2. BENCH_SLOW = 0 reproduce R2.
- Test de solape barato: tabla qpos (columna y linea superior) y bucle corto sin PUSH/POP ni IY; sustituye a ent_overlap de R2.
- Medida sin picos: los cinco numeros del HUD se escriben uno por fotograma tras cerrar la ventana; esos fotogramas no cuentan para LIBRE, MIN ni RETR. SOLP pasa a ser el numero de ventanas compuestas por la ruta lenta.

## Uso
1. Copiar `bench_r3.asm` junto a `main.asm`. `build_bench.bat bench_r3`. Abrir `build\bench_r3.sna` en RetroVirtualMachine con 128K.
2. Parametros al principio del .asm: BENCH_N, BENCH_STAGGER, BENCH_SPEED, BENCH_PROF, BENCH_SLOW.
3. Franjas del borde: verde = volcado, rojo = logica, azul = preparacion, negro = libre.

## Pruebas (rellenar)
| N | ESC | SLOW | LIBRE | MIN | RETR | PERD | SOLP | Azul (hasta donde llega) |
|---|---|---|---|---|---|---|---|---|
| 1 | 1 | 1 | | | | | | |
| 5 | 1 | 0 | | | | | | |
| 5 | 1 | 1 | | | | | | |
| 5 | 0 | 1 | | | | | | |
| 8 | 1 | 1 | | | | | | |
| 8 | 0 | 1 | | | | | | |

Con SLOW = 0 se mide el coste del test de solape barato frente a R2; con SLOW = 1, el de la ruta lenta. Comprobar a ojo que dos sprites que se cruzan respetan la prioridad y que no queda ningun rectangulo visible.

## Prevision (estimacion; a contrastar)
- Segun el modelo, la ruta lenta se usa en el 15-20 % de las ventanas con N = 5 (unas 25 por cada 64 fotogramas) y en el 30 % con N = 8 (unas 70).
- Coste de una ventana lenta: unos 7.000-9.000 T mas que la rapida (la propia entidad tambien pasa por la rutina recortada, mas la ordenacion y los sprites vecinos).
- El test de solape barato deberia ahorrar del orden de 1.000 T por fotograma con N = 5.
- MIN deberia mejorar respecto a R2 porque ya no incluye el HUD.

## Pendiente para R4 (si hace falta)
Tabla de direcciones de linea en lugar de down_hl, rutinas desenrolladas, recorte por arriba y por abajo (celda adicional de la casa) y datos de sprites en los bancos 4 y 6 con contencion.
