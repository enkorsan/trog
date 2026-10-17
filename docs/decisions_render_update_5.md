# Suplemento 5 de decisiones: invencibilidad, muerte en sitio, casa y victoria

Version: 17-oct-2026, revision 0. Suplemento 5 a decisions.md revision 15; se suma a decisions_render_update.md, _2 (rev. 2), _3 (rev. 1) y _4 (rev. 0). No sustituye ninguno. Las decisiones marcadas "Decidido por Carlos" son suyas; lo demas son adaptaciones o interpretaciones del asistente. El codigo asociado (src_z80/, rev8) esta ensamblado y probado por Carlos en 128K y 48K; el asistente no ejecuta sjasmplus ni emulador.

## 1. Invencibilidad y parada ante el borde (rev5)
- Decidido por Carlos: el dino es invencible al arrancar la fase y al reaparecer (fidelidad con el arcade; igual que dino_arm del motor anterior): INV_FRAMES = 238 (60 + 200 ticks), START_WAIT = 55. La casilla 6 del HUD la muestra y parpadea al final (INV_BLINK).
- Parada: STOP_QUIRK, edge_stop, DWAIT_FRAMES = 111 y EDGE_TURN como en decisions.md rev15 ("Caida y parada ante el borde"), portados a dino_r7.inc:check_fall y dino_update.
- Decidido por Carlos (17-oct, al compararlo con el arcade): STOP_TOP_Y +4 (40 -> 44) y STOP_BOTTOM_Y -4 (160 -> 156). STOP_LEFT_X = 12 y STOP_RIGHT_X = 228 no cambian. Constantes STOP_TOP_SHIFT y STOP_BOTTOM_SHIFT en config.inc.
- Decidido por Carlos: FALL_* no cambian hasta tener el sprite definitivo (FALL_TOP_Y 26, FALL_BOTTOM_Y 164, FALL_LEFT_X 4, FALL_RIGHT_X 236). Motivo y estimaciones en analysis/parada_caida_arcade.md.
- Correccion a decisions.md rev15: la frase "entre la parada y la caida quedan 8 px" salia de las constantes del port, no del arcade. Con los valores actuales los huecos son 8 (izq.), 8 (der.), 18 (arriba) y 8 (abajo) px.

## 2. Muerte en sitio (rev6)
- respawn_mode: 0 = caida (reaparece en el extremo de la misma calle, rumbo desde el auxiliar del borde), 1 = muerte en sitio (interseccion mas cercana en cada eje con snap_axis; el origen de la busqueda es la propia interseccion). rsp_arm lo pone a 0.
- resp_eat (como en el motor anterior): 1 = egg_pickup activo. En caidas siempre 1. En muerte en sitio vale 1 si node_offset indica que murio a TURN_TOL px o menos de un nodo real; si no, 0 y el huevo del nodo no se recoge hasta el primer paso (dino_update lo vuelve a 1 al dar un paso). En r7 egg_pickup se llama cada fotograma desde game_update, por eso se comprueba resp_eat dentro de egg_pickup.
- Adaptacion provisional: el arcade usa GETANIXY o FIND_CLOSE_CPNT en muertes sin caida; el punto de reaparicion en muertes por enemigo sigue pendiente de verificar (decisions.md rev15).
- Teclas de depuracion (DEBUG_CHEAT = 1, debug_r7.inc): H alterna cheat_god (invencibilidad permanente); K mata sin caida con el dino vivo (E_STATE = 0); B construye la casa; J pasa a la siguiente salida (0..27, solo sin casa; el HUD muestra EX). R (redistribuir huevos) no esta portada.

## 3. Casa (rev7)
- Decorado: la extension (32x20 arriba y abajo, 16x32 izquierda y derecha) y el letrero (16x8, HOME/REX cada HOUSE_FLASH_FRAMES = 21) se funden con mascara en BG_BASE y se piden al renderer con decor_push3 (teselas de 3 bytes; la ultima columna se desplaza a la 29). Siempre por debajo de los sprites (decisions_render_update_4.md, seccion 1). No hay decor_rect general: house_stamp lo sustituye para la casa.
- house_build se llama al recoger el ultimo huevo (egg_pickup) o con la tecla B; la casa aparece tras HOUSE_APPEAR_FRAMES = 51 fotogramas ocultos. house_select elige la salida (28 entradas en exit_tab, interpretacion pendiente de EXITPOINTS); HOUSE_FORCE_EXIT >= 0 la fuerza en pruebas.
- Graficos: house_gfx.inc (placeholders de tools/make_house_placeholder.py, 640 bytes).

## 4. Llegada y victoria provisional (rev8)
- house_on_street (house_state >= 1, misma direccion y misma columna o fila que el nodo de retorno): check_fall no frena ni cae; avanza hasta la celda adicional (decision de Carlos 13-oct): HOUSE_ARR_TOP_Y 28, HOUSE_ARR_BOTTOM_Y 172, HOUSE_ARR_LEFT_X 0, HOUSE_ARR_RIGHT_X 240. Ahi E_STATE = 3.
- win_update (E_STATE = 3): el dino mira abajo, uno de cinco textos centrado (linea 96, directo en pantalla), WIN_WAIT_FRAMES = 100 (provisional) y jp start. El renderer solo distingue E_STATE = 2; 3 es del juego.
- Pendiente: la secuencia real de CELEBRATE (SKYDOWN, WIPE_FROM_MID, baile, mensajes, 25.000 puntos por rival), el rumbo hacia la casa al reaparecer y la aleatoriedad.

## 5. Aleatorio (pendiente, decide Carlos)
eggs_setup reinicia rng_state con EGG_SEED en cada arranque: huevos, celda libre (nodo 0 y otra), salida de la casa y texto de victoria son siempre los mismos y el reinicio tras ganar repite el nivel. Carlos: "mas adelante o cuando tu me digas". Opciones: no reiniciar la semilla al reiniciar; mezclar frame_counter; semilla con el tiempo hasta pulsar una tecla (menu, hito 3).

## 6. Memoria (medida)
rev6 +184 bytes sobre rev5; rev8 +1503 bytes sobre rev6. Fin de codigo y datos $AB54 (128K) y $AB4D (48K); libres hasta $B100: 1452 y 1459 bytes. No se declara margen cerrado.
