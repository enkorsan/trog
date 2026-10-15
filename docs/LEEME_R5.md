# R5 - recorte, celdas adicionales, datos de sprites en otro banco y HUD con estadisticas

Estado: PROPUESTO (sin ensamblar ni probar). El algoritmo se verifico con un modelo en Python (0 diferencias con la imagen ideal en 3.000 fotogramas con 1, 3, 5 y 8 entidades, con solapes, recorte arriba y entidades llegando a x = 0..240 e y = 28..172). Las rutinas en Z80 no se han ejecutado.

## Resultados de R4 (medidos por Carlos, RetroVirtualMachine 128K toastrack; extremos de cada rango)
| N | ESC | SLOW | STOP | LIBRE | MIN | RETR / PERD | SOLP (max.) | Azul |
|---|---|---|---|---|---|---|---|---|
| 1 | 1 | 1 | 0 | 937 | 1768-1789 | 0 / 0 | 0 | muy por encima de la pantalla |
| 5 | 1 | 0 | 0 | 676 | 1136-1196 | 0 / 0 | 0 | primer tercio |
| 5 | 1 | 1 | 0 | 573-657 | 758-1140 | 0 / 0 | 62 | del primer tercio a la mitad |
| 5 | 2 | 1 | 0 | 570-658 | 624-1135 | 0 / 0 | 67 | del primer tercio a la mitad |
| 5 | 2 | 1 | 1 | 572-754 | 550-1123 | 0 / 0 | 70 | desde el primer tercio hasta casi toda la pantalla |
| 5 | 0 | 1 | 0 | 557-656 | 59-804 | 0 / 0 | 59 | mitad de la pantalla, a veces el tercer tercio |
| 8 | 1 | 1 | 0 | 232-431 | 0-199 | casi siempre 0 (max. 15) | 72 | del tercer tercio a casi toda la pantalla |

Lectura: coste por movimiento ~8.700 T; la ruta lenta cuesta ~4.300 T por fotograma con N = 5; en el peor caso con 5 entidades quedan unos 19.000-26.000 T libres. N = 8 solo pierde fotogramas puntualmente.

## Cambios de R5
1. BENCH_EDGE = 1: las entidades pueden entrar en las celdas adicionales del borde (y = 28 y 172, x = 0 y 240; de ellas solo se sale volviendo). La ventana se recorta a CLIP_TOP..CLIP_BOTTOM (la entidad se pinta entonces con draw_clipped) y la primera columna se limita a 29. Con esto se prueba el recorte superior (cabeza bajo el HUD en y = 28).
2. BENCH_SPRBANK: banco donde viven los datos de sprites (2 = residente como en R4; 1, 3, 4, 6 o 7 = en ese banco, en $C000). El fondo limpio sigue en el banco 0; prepare_all se divide en dos pasadas (copia del fondo con el banco 0, pintado con el banco de sprites). En la RVM toastrack los bancos 1, 3, 5 y 7 son contendidos; el 4 y el 6 no.
3. HUD con estadisticas acumuladas.

## Como leer el HUD (3 filas)
| Fila | Campo | Significado |
|---|---|---|
| 0 | NnEeSsPpGgBb | Configuracion: N entidades, E escalonado, S ruta lenta, P paradas, G celdas adicionales, B banco de sprites |
| 0 | VENT | Ventanas medidas desde el arranque (cada una son 64 fotogramas medidos, unos 1,3 s) |
| 1 | MEDIA | Media de LIBRE de las ultimas 16 ventanas (valida con VENT >= 16) |
| 1 | MINA | Menor MIN desde el arranque (peor fotograma) |
| 1 | SOLX | Mayor SOLP de una ventana desde el arranque |
| 2 | LIBRE | Tiempo libre medio de la ultima ventana |
| 2 | MIN | Menor tiempo libre de un fotograma de la ultima ventana |
| 2 | PERD | Fotogramas perdidos desde el arranque (maximo 9999) |

LIBRE y MEDIA estan en unidades de 70 T-states (un fotograma de 128K = unas 1013); MIN y MINA en unidades de 35 T-states (unas 2026 = fotograma entero libre). Los numeros se escriben de uno en uno en los fotogramas que siguen al cierre de cada ventana y esos fotogramas no se miden.

## Protocolo de medida
Dejar correr la prueba hasta VENT >= 32 (unos 40 s) y apuntar: MEDIA, MINA, SOLX, PERD y la franja azul/verde. Si hay mucha variacion, anotar tambien el rango de LIBRE y MIN que se ve en la fila 2.

## Pruebas (rellenar con lo que muestra el HUD)
| N | E | S | P | G | B | MEDIA | MINA | SOLX | PERD | Azul |
|---|---|---|---|---|---|---|---|---|---|---|
| 5 | 1 | 1 | 0 | 0 | 2 | | | | | |
| 5 | 1 | 1 | 0 | 1 | 2 | | | | | |
| 5 | 2 | 1 | 1 | 1 | 2 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 4 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 6 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 1 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 3 | | | | | |
| 5 | 1 | 1 | 0 | 0 | 7 | | | | | |
| 8 | 1 | 1 | 0 | 1 | 2 | | | | | |

Con G = 1 comprobar a ojo que cuando un sprite llega a y = 28 se ve recortado bajo el HUD y que en x = 0 y x = 240 no queda ninguna basura en el borde de pantalla.

## Prevision (estimacion; a contrastar)
- Con G = 1 los resultados deben ser casi iguales a G = 0 (el recorte afecta a muy pocas ventanas).
- B = 4 y B = 6 (no contendidos) deben dar lo mismo que B = 2.
- B = 1, 3 y 7 (contendidos) deben bajar la MEDIA algo (estimo unos 10-20 puntos con N = 5: leer los datos de sprites en un banco contendido cuesta unos 400 T mas por ventana).
