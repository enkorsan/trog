# R6 (R6.1 a R6.3) - Trog distinto del dino, acciones en el sitio, caida por el borde e instrumentacion

Estado: R6 ensamblado y ejecutado por Carlos; R6.1 y R6.2 ejecutados por Carlos (tablas en analysis/render_mediciones_r6_2.md); R6.3 ensamblado por Carlos con el HUD de la mitad derecha confirmado visualmente (16-oct). El algoritmo se verifico ademas con un modelo en Python; las rutinas Z80 no las ha ejecutado el asistente.

## Archivos
- Copia local: src/bench_r6_integracion.asm (R6.3 renombrado). En el repositorio solo esta bench_r6.asm de src_z80/ (R6.2).
- sprite_trog.inc: Trog PROVISIONAL de 16x24 con mascara, 4 direcciones x 4 fases (mismo formato que el dino), generado por tools/make_trog_placeholder.py. Los graficos definitivos son de Carlos.
- La entidad 0 usa sprite_dino; las demas, sprite_trog.

## Cambios respecto a R5
1. Dino y Trogs distintos (cada entidad guarda su puntero de datos).
2. E_DIRTY: la entidad se redibuja cuando cambia la posicion o la imagen (E_IMG distinto de E_DIR).
3. BENCH_ACT = 1: acciones en el sitio (1 de cada 8 llegadas a una interseccion; 12 fotogramas quieta, cambio de imagen cada 4; placeholder).
4. BENCH_FALL = 1: caida por el borde hasta los umbrales (y <= 26, y >= 164, x <= 4, x >= 236), animacion de caida de 24 fotogramas (cambio de imagen en el sitio), 55 fotogramas desaparecida y reaparicion en un nodo al azar. Sin parpadeo. Anula BENCH_EDGE.

## Cambios de R6.1 a R6.3
- R6.1: instrumentacion corregida (PC y LT; frame_counter leido una vez); velocidad V en el HUD.
- R6.2: MG y WN, BENCH_STOPVENT, BENCH_BURN, BENCH_SKIP_BLIT, BENCH_SKIP_SORT; draw_clipped fuera del IF BENCH_SLOW.
- R6.3: HUD de depuracion a la derecha.

## HUD (R6.3)
Mitad derecha (columnas 16-31): fila 0: MG, WN; fila 1: MN (MINA), SX (SOLX); fila 2: PC, LT.
Mitad izquierda (solo en el banco): fila 0 configuracion NnEeSsPpGgAaFfBb; fila 1 V (velocidad) y VENT.
Leer con el HUD congelado en VENT = 64 (BENCH_STOPVENT). Poner BENCH_BURN, BENCH_SKIP_BLIT y BENCH_SKIP_SORT a 0 para medir una base.

## Tabla a rellenar
| N | E | S | P | G | A | F | B | V | VENT | BURN | MG | WN | MINA | SOLX | PC | LT |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 5 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | 7 | 64 | 0 | | | | | | |
| 5 | 1 | 1 | 0 | 0 | 1 | 0 | 2 | 7 | 64 | 0 | | | | | | |
| 5 | 1 | 1 | 0 | 0 | 0 | 1 | 2 | 7 | 64 | 0 | | | | | | |
| 5 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 7 | 64 | 0 | | | | | | |
| 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 7 | 64 | 0 | | | | | | |

Comprobar a ojo que el Trog y el dino se distinguen, que al cruzarse respetan la prioridad (mayor Y encima, dino en la misma fila), que durante la caida la imagen cambia en el sitio, que al desaparecer no queda rastro y que al reaparecer solo se ve el sprite nuevo.
