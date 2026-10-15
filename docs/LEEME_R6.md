# R6 - Trog distinto del dino, acciones en el sitio y caida por el borde

Estado: PROPUESTO (sin ensamblar ni probar). El algoritmo se verifico con un modelo en Python (0 diferencias con la imagen ideal en 4.000 fotogramas con 1, 3, 5 y 8 entidades, con cambios de imagen en el sitio y unas 70 caidas y reapariciones por prueba). Las rutinas en Z80 no se han ejecutado.

## Archivos
- `bench_r6.asm`: el banco (junto a `main.asm`).
- `sprite_trog.inc`: Trog PROVISIONAL de 16x24 con mascara, 4 direcciones x 4 fases (mismo formato que el dino). Generado por `tools/make_trog_placeholder.py` (`--preview` muestra los dibujos en texto). Los graficos definitivos son de Carlos.
- La entidad 0 usa `sprite_dino`; las demas, `sprite_trog`.

## Cambios respecto a R5
1. Dino y Trogs distintos (cada entidad guarda su puntero de datos).
2. E_DIRTY: la entidad se redibuja cuando cambia la posicion o la imagen. La imagen (E_IMG) puede ser distinta de la direccion de marcha.
3. BENCH_ACT = 1: acciones en el sitio. En 1 de cada 8 llegadas a una interseccion la entidad se queda quieta 12 fotogramas y cambia de imagen cada 4 (placeholder de giro o punetazo).
4. BENCH_FALL = 1: caida por el borde. Salen en linea recta hasta los umbrales (y <= 26, y >= 164, x <= 4, x >= 236), hacen una animacion de caida de 24 fotogramas (cambio de imagen en el sitio), desaparecen 55 fotogramas (se compone la ventana sin el sprite y no cuentan como vecino) y reaparecen en un nodo al azar. No hay parpadeo. Anula BENCH_EDGE.

## HUD (igual que R5, con mas configuracion)
Fila 0: NnEeSsPpGgAaFfBb (N entidades, E escalonado, S ruta lenta, P paradas, G celdas adicionales, A acciones, F caidas, B banco de sprites) y VENT. Fila 1: MEDIA, MINA, SOLX. Fila 2: LIBRE, MIN, PERD. Esperar a VENT >= 32.

## Pruebas (rellenar)
| N | E | S | P | G | A | F | B | MEDIA | MINA | SOLX | PERD | Azul |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 5 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 1 | 0 | 2 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 0 | 1 | 2 | | | | | |
| 5 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | | | | | |
| 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | | | | | |

Comprobar a ojo que el Trog y el dino se distinguen, que al cruzarse respetan la prioridad (mayor Y encima, dino en la misma fila), que durante la caida la imagen cambia en el sitio, que al desaparecer no queda rastro del sprite y que al reaparecer solo se ve el sprite nuevo (sin restos en la posicion de la caida).

## Prevision (estimacion; a contrastar)
- Las acciones en el sitio cuestan una ventana de 24 lineas cada 4 fotogramas mientras duran (unos 8.500 T por ventana): comparable a una entidad que se mueve; con A = 1 la MEDIA debe bajar poco.
- Las caidas añaden ventanas puntuales (animacion, ocultacion y reaparicion) pero reducen el numero de entidades activas durante los 55 fotogramas de desaparicion: con F = 1 la MEDIA puede incluso subir.
