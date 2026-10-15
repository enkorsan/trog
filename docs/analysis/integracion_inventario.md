# Inventario previo a la integracion del renderer (paso 1)

Fecha: 15-oct-2026. Estado: analisis por lectura del codigo del repositorio. El asistente no ha ensamblado ni ejecutado nada; las pruebas reales son las que informa Carlos.

## Alcance y aviso de version
El codigo leido en src_z80/ (main.asm, dino.inc, eggs_logic.inc, house.inc, victory.inc, ai.inc, sprite.inc, input_hud.inc, inv.inc, debug.inc, vars.inc) es la VERSION PREVIA del juego, hecha en otra conversacion: dibujo directo en pantalla, sin optimizacion y con pocas consideraciones de rendimiento. Carlos decide adaptar el juego al renderer actual (bancos R1-R6, ultimo R6.3) manteniendo en lo posible la logica original del arcade y el renderer. En la practica se programa el juego de nuevo reutilizando la logica ya probada. No se modifica el orden de los hitos ni el reparto de trabajo.

Leido: docs/status.md (rev. 17), continuation.md, render_progress.md, decisions_render_update.md, memoria-9.md (copia local de Carlos) y los modulos citados. NO leido en esta pasada: video.inc y config.inc del repositorio, eggs.inc, house_gfx.inc, level1_map.inc, decisions.md, testing.md, project_brief.md.

## Decisiones de Carlos (15-oct-2026, 23:33)
- Adaptar el juego al renderer actual; mantener la logica del juego original en lo posible.
- Compatibilidad 48K deseable (paso 5 de la hoja de ruta): si hace falta un banco extra de 128K no es un problema, pero es mejor que sea jugable en 48K si se puede.
- La muerte por caida en los contornos es una ANIMACION (sin parpadeo). El parpadeo de fall_update es de la version anterior.
- HUD de depuracion en la mitad derecha (columnas 16-31, filas 0-2); la mitad izquierda queda libre para puntos, huevos, vidas, etc.
- status.md se actualiza en un paso siguiente.

## Observaciones directas del codigo (version previa)
- Formato de sprite compatible con R6: fase (x & 7)/2, ancho 2 si fase 0 y 3 si no, 144 bytes por imagen y fase, 6 bytes por linea, linea superior = dino_y - SPR_OFF_Y, recorte CLIP_TOP/CLIP_BOTTOM (sprite.inc, sprite_draw).
- Direcciones D_UP, D_DOWN, D_LEFT, D_RIGHT = 0..3, igual que E_IMG en R6.
- Renderer actual: guardar fondo en save_buf (72 bytes) y restaurar con sprite_erase (tipo R1). Variables saved_valid, save_first, save_col, save_count, save_width compartidas con otros modulos (vars.inc, house.inc).
- Bucle principal con HALT: una iteracion de logica por interrupcion; frame_isr solo incrementa frame_counter (main.asm).
- Reinicio de nivel con jp start (victory.inc, win_update).
- Pila en $7FFF (banco 5, contendido); memoria-9.md propone la pila en el banco 2.

## Puntos de contacto con el renderer
| Donde | Que hace con la pantalla | Que exige al nuevo renderer |
|---|---|---|
| update_dino (.redraw), turn_node, fall_update, dino_respawn, win_update, update_reseed | sprite_erase + sprite_draw del dino, saved_valid | Entidad 0 con E_DIRTY, E_STATE, E_VALID |
| egg_pickup / erase_egg_area | Se llama con el sprite borrado; pone ceros 16x16 | Actualizar el fondo limpio (banco 0) y recomponer la ventana, incluida la parte del huevo que el dino no cubre |
| draw_egg, draw_all_eggs | Pintan con mascara directo en pantalla | Pintar tambien en el fondo del banco 0 |
| house_flash_update, stamp_draw, house_plq_overlap | Borra el dino, pinta extension y letrero, repinta el dino; el test de solape lee save_* | Rectangulo estatico compuesto por prioridad; reescribir el test de solape sin save_* |
| win_message / print_text | Texto en la linea 96 con el sprite fuera | Texto en el fondo + rectangulo sucio |
| house_marker_toggle, update_reseed (depuracion) | XOR sobre pantalla (lineas 28 y 180, dentro del campo) y borrado de huevos | Pasar tambien por el fondo |
| HUD (draw_hud_glyphs, draw_egg_counter, show_keys) | Lineas 0-23 y atributos | Sin conflicto: el renderer recorta en CLIP_TOP (24) |

## Diferencias de modelo
1. Sincronizacion: con HALT una iteracion que acaba pasada la interrupcion pierde un paso de logica; el banco usa espera por contador y absorbe la iteracion tardia. Los resultados del banco (PC = 0 con LT > 0) solo valen para ese esquema.
2. Latencia: el volcado se hace al comienzo de la iteracion siguiente (un fotograma mas de retraso).
3. Decoracion: todo lo que cambia el decorado desde el juego debe pasar por el fondo limpio (decisions_render_update.md ya lo indica para erase_egg_area).
4. Estado de la entidad: dino_state del juego (0 andando, 1 muriendo con parpadeo, 2 apagado, 3 victoria) frente a E_STATE del banco (0 andando, 1 animacion de caida, 2 oculta). La animacion de caida del banco, sin parpadeo, es el modelo definitivo.

## Reutilizable frente a rehacer (propuesta, por lectura)
Reutilizable (logica): node_offset, mod_step, update_request, dir_from_mask, parte logica de turn_node y check_fall, ai.inc (dino_reset, dino_respawn, dino_arm, snap_axis, div24, choose_dir, find_closest, dir_valid, best_direction, best_by_rating), rand16, place_eggs, node_line_col, logica de egg_pickup, house_select, exit_tab, geometria de house_build, house_on_street, house_arrive_check, read_keys, inv.inc.
A rehacer: sprite.inc, draw_egg y erase_egg_area, stamp_draw y house_flash_update, house_plq_overlap, win_message sobre el fondo, fall_update, el bucle principal y la parte de dibujo de update_dino.

## Compatibilidad 48K (consideracion, sin medir)
- Con BENCH_SPRBANK = 2 el banco solo pagina el banco 0 en $C000 para el fondo limpio; en un 48K esa zona es RAM plana. Las unicas dependencias 128K del banco residente son las escrituras al puerto $7FFD y, en el juego completo, los conjuntos de sprites en otros bancos.
- Estimacion de memoria residente del renderer: fondo limpio 6.144 bytes, buffers de ventana 1.024, sprites dino 2.304 y Trog 2.304 (provisionales), mas codigo; la RAM libre de un 48K ($5B00-$FFFF) es de unos 42,5 KB. Para el hito 1 parece caber; para hitos posteriores hara falta paginar o recortar.
- Propuesta para no cerrar la puerta: direcciones del fondo, buffers y sprites como EQU y macros de paginacion que no generan codigo en 48K. Hay que comprobarlo ensamblando una variante con DEVICE ZXSPECTRUM48 y probandola en RVM modelo 48K.

## Interpretaciones pendientes y desconocidos
- Tamano compilado del juego y margen hasta IM2_TABLE ($B800): falta el listado de sjasmplus.
- El script de compilacion del juego no esta en el repositorio (Carlos usa src/, el repositorio src_z80/); no se fijan rutas sin verlo.
- Huevos como fondo o como entidad: los documentos apuntan a fondo, no esta cerrado.
- La pantalla sombra no esta descartada (decisions_render_update.md); no se ha comparado con el esquema B+.
- Tiempo de juego frente a fotogramas perdidos: si la logica avanza por iteracion o por tiempo real.

## Plan propuesto (revisado; pasos pequenos con prueba de Carlos)
0. Contrato del renderer (render.inc): entidades, rectangulo de decorado, sincronizacion por contador, macros de paginacion, direcciones por EQU. Documentar en decisions.md antes de codificar.
1. Esqueleto nuevo: bucle principal + renderer + dino como entidad 0 con la logica reutilizada. Prueba: movimiento, giros y caida con animacion provisional frente a MAME.
2. Fondo con huevos como decorado (banco 0) y recogida con rectangulo sucio. Prueba: sin restos al recoger con el dino encima.
3. Casa, letrero y victoria como rectangulos estaticos. Prueba: sin restos y prioridad correcta.
4. Trogs placeholder y HUD de depuracion (PC, LT, MG, WN) en la mitad derecha.
5. Medir el fotograma completo y decidir optimizaciones con los numeros.

## Documentos a actualizar en pasos siguientes
- status.md (rev. 18): mediciones R6.2 y R6.3, reserva medida, decisiones de este dia.
- testing.md / testing_render.md: protocolo con STOPVENT, MG, WN, BURN y SKIP.
- decisions.md: contrato del renderer, sincronizacion, 48K.
