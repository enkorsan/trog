# Extraccion del nucleo del renderer (render_core.inc) y prueba del paso 1

Version: 16-oct-2026, 15:0x, revision 1. Con los resultados de la prueba de Carlos. El asistente no ha ejecutado sjasmplus ni emulador: lo marcado "informado" lo comunica Carlos; lo marcado "listado" sale de sus listados (game_r7.lst y game_r7_48-2.lst).
Origen: bench_r6_integracion.asm (R6.3, copia local de Carlos, 36.123 bytes con CRLF). Se leyo entero.

## Que se ha extraido y que no
| Rutina del banco | Destino |
|---|---|
| build_bg | render_core.inc, sin cambios |
| prepare_all, blit_all | render_core.inc; paginacion por macros PAGE_BG / PAGE_SPR; BENCH_N -> ENT_COUNT |
| ent_set_qpos, ent_prep1, ent_prep2 | render_core.inc; se quitan los interruptores BENCH_FALL y BENCH_SLOW (siempre activos) |
| ent_hits_other, ent_q_intersects, ent_pre, ent_post, nb_range | render_core.inc; ovl_cnt detras de RENDER_STATS |
| draw_clipped, ent_blit | render_core.inc, sin cambios |
| sort_z | render_core.inc; AHORA SE EJECUTA (el R6.3 tenia BENCH_SKIP_SORT = 1) |
| b_put_glyph, b_print_text, b_print_dec4 | render_core.inc, sin cambios (b_print_dec4: HL = valor < 10000, B = linea, C = columna en bytes) |
| init_ents | sustituido por render_init (en render_core.inc): solo buffers, E_IDX, E_TIE y ZLIST; la posicion la fija la logica |
| ent_update, ent_arrive, ent_choose, ent_respawn, get_bit, bit_tab, b_rand | NO se extraen: logica de prueba del banco |
| main_loop, acc16, cap9999, end_window, report_step, rep_tab, hud_labels y variables del HUD del banco | NO se extraen: instrumentacion del banco (la espera por contador se reescribe en game_r7.asm: frame_wait) |

Comprobacion posterior sobre el listado de Carlos (listado): el texto de render_core.inc que se ensamblo coincide, linea a linea y en orden, con el que genera la extraccion (775 de 782 lineas con contenido comprobadas de forma automatica; las 7 restantes eran de build_bg, que se colo antes de lo previsto en la busqueda y se revisaron a mano).

## Resultados de la prueba del paso 1 (16-oct)
- Ensamblado: informado por Carlos para 128K y para 48K. Los listados (game_r7.lst y game_r7_48-2.lst) muestran los dos ensamblados completos hasta SAVESNA, con la rama correcta de cada IFDEF.
- Ejecucion (informado): "todo funciona bien", tambien en 48K. PC y LT se mantienen en 0 con un solo dino. No se ha detallado la prueba de giros, caidas y reaparicion; no se dan por validados mas alla de lo informado.
- Velocidad (informado): cronometrado 4,75 a 4,80 s para 8 intervalos. Referencias: MAME 4,78 s; teoria 26/64 4,73 s; lectura esperada 4,85 s. La base 26/64 queda dentro de la lectura de MAME; no hace falta ajustarla por ahora.
- Macros (listado): en 48K las direcciones de codigo se desplazan 7 bytes (por ejemplo, call draw_field en $802E apunta a $891D en 128K y a $8916 en 48K). Son los 7 bytes de la unica PAGE_BG (LD BC, LD A, OUT (C),A). Confirma que la macro no genera codigo en 48K.
- Aviso de sjasmplus (listado): "File name contains \\, use / instead" por la ruta src\game_r7.asm. Los .bat de esta revision usan / en todas las rutas; no probados tras ese cambio.

## Memoria medida (listado)
| Bloque | Direcciones 128K | Bytes |
|---|---|---:|
| game_r7.asm (codigo, HUD de depuracion y variables) | $8000-$814E | 335 |
| dino_r7.inc | $814F-$836E | 544 |
| render_core.inc (con ENT_COUNT = 1; sort_z es un RET) | $836F-$880A | 1.180 |
| input_hud.inc | $880B-$88F7 | 237 |
| video.inc | $88F8-$89C1 | 202 |
| sprite_dino | $89C2-$92C1 | 2.304 |
| sprite_trog | $92C2-$9BC1 | 2.304 |
| stones.inc | $9BC2-$9C2D | 108 |
| font_msg.inc | $9C2E-$9E05 | 472 |

Fin de datos: $9E06 en 128K y $9DFF en 48K. Hasta SCR_BUFS ($B100): 4.858 bytes en 128K y 4.865 en 48K. NO es un margen cerrado del juego: el esqueleto no tiene todavia huevos, casa, IA, colisiones, decor_rect ni ordenacion real (sort_z crecera con ENT_COUNT > 1), y los sprites son placeholders de 4 imagenes (con diagonales: +2.304 bytes por juego). La cifra del R6.3 (4.850 hasta IM2_TABLE) no es comparable: contaba otras regiones.

## Cambio de comportamiento que afecta a las mediciones
Las tablas del R6.2/R6.3 se tomaron en parte con BENCH_SKIP_SORT = 1 (sin ordenar). El juego ordena. La estimacion de render_mediciones_r6_2.md es de unos 1.000 T por fotograma de media y unos 5.500 T en la peor ventana en N8. Con un solo dino no hay solapes y sort_z no se llama. Hay que medir de nuevo cuando haya Trogs. No se declara margen cerrado.

## Notas de Carlos (16-oct, 14:26)
- Altura de los sprites: a revisar cuando se estudie el T-Rex; con puñetazos cree que cabe.
- 48K: se buscaran recursos para que quepa (quitar animaciones, suelo sin dibujos, etc.); se ira viendo.
- El arcade tiene muchos fotogramas de animacion; el port los reduce mucho, pero debe sentirse igual al jugar.
- Casa: hoy se pinta como entidad (por comprobar en house.inc / house_gfx.inc; el inventario de integracion la describe como pintado directo en pantalla con borrado del dino). Evaluar pintarla como decorado en el paso 3.

## Abiertas
1. Detalle de la prueba: giros en nodo y media vuelta, caida con animacion, reaparicion y restos en pantalla (informado solo como "todo funciona bien").
2. Lectura del fuente arcade de las animaciones (giro, puñetazos, golpes recibidos, muertes) y de sus tamaños, antes de fijar el formato de animacion y ENT_SIZE.
3. Casa como decorado o como entidad (paso 3).
4. Formato de animacion y tamaño de sprite maximo (SPR_H, ancho de ventana de 3 bytes) con el T-Rex.
5. render.inc y dino_r7.inc siguen con la cabecera "PROPUESTO (sin ensamblar ni probar)" en el repositorio; ya estan ensamblados y probados dentro de esta prueba. Se actualizara la cabecera en el proximo cambio de esos ficheros.
