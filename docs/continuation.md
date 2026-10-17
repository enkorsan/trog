# Continuacion de Trog ZX

17-oct-2026, revision3. Sustituye continuation.md rev2.

## Mensaje para otra conversacion
Continuamos Trog ZX. Lee primero docs/continuation.md y docs/status.md desde enkorsan/trog (rama main), y sigue las lecturas y la siguiente tarea alli indicadas. Codigo en src_z80/ (local: src/). No empezar otro motor ni declarar cerrado el margen de memoria o CPU.

## Lecturas obligatorias
1. docs/status.md rev24.
2. docs/decisions_render_update_5.md rev0 (decisiones y adaptaciones de las tandas 1, 2a y 2b) y docs/analysis/parada_caida_arcade.md rev0.
3. src_z80/game_r7.asm rev8; seguir INCLUDE: dino_r7.inc, respawn_r7.inc, inv.inc, debug_r7.inc, render_core.inc, decor_r7.inc, eggs_r7.inc, house_r7.inc, victory_r7.inc, input_hud.inc, video.inc y datos. Todos estan en main; no pedir adjuntos de nuevo.
4. docs/analysis/reaparicion_paso2b.md (contrato y auditoria de rumbo); config.inc (STOP_*, FALL_*, INV_*, DWAIT_FRAMES, TURN_TOL, EGG_TOL).
5. Contratos: decisions_render_update_2.md rev2, _3.md rev1, _4.md rev0, _5.md rev0 y testing_render.md rev3, testing_render_2.md rev0; suplementos, no sustituyen decisions.md rev15/testing.md rev12.
Segun necesidad: analysis/huevos_paso2a.md, integracion_inventario.md, extraccion_render_core.md, render_mediciones_r6_2.md, tamanos_sprites.md, trex_casa_animaciones.md, docs/memoria.md.

## Base probada sincronizada
Todo lo de status.md rev24 esta ensamblado y probado por Carlos en 128K y 48K (el asistente no ejecuta sjasmplus ni emulador). No volver a revisiones anteriores ni rehacer reaparicion, muerte en sitio, casa o victoria.
Pipeline: blit_all+blit_decor -> logica -> prepare_all+prepare_decor -> espera, sin catch-up. ENT_COUNT=1. Decorado (huevos, casa, letrero) = escritura en BG_BASE + decor_push3 (teselas de 3 bytes, cola de 8, una por fotograma); siempre por debajo de los sprites. E_STATE: 0 andando, 1 caida/muerte animada, 2 oculto, 3 victoria. El texto de victoria se escribe directo en pantalla (centro del campo).
EGG_TEST_FLOOR=1 diagnostico (suelo con trama); 0 suelo normal. DBG_HUD=1 y DEBUG_CHEAT=1: teclas H (invencibilidad), K (muerte en sitio), B (construir casa), J (siguiente salida, HUD EX). HOUSE_FORCE_EXIT>=0 fuerza la salida.
Semilla fija: huevos, celda libre, salida y texto siempre iguales; el reinicio repite el nivel (pendiente, decide Carlos).

## Primera tarea
Ver status.md rev24 (siguiente tarea): probar SPR_BANK distinto de 2; rumbo hacia la casa leyendo antes DINO_FIND_CLOSEST/GET_BEST_DIRECTION del fuente arcade; aleatorio. Cambios pequenos con prueba concreta 128K/48K. No cambiar hitos.

## Reglas de rumbo y limites
Rumbo al reaparecer: Manhattan desde el auxiliar del borde en caidas y desde la interseccion en muerte en sitio, eje mayor/vertical en empate, alternativa, descarta inversa de dinitdir, best_by_rating (2 por huevo, longitud, ultimo U,D,L,R en empate). Casa/pina/paredes/peligros todavia no entran en el rumbo.
Limites: STOP_*/FALL_* en config.inc; Carlos ajusto STOP_TOP +4 y STOP_BOTTOM -4; FALL_* se revisaran con el sprite definitivo.
ClaseA16px ancho,24alto,ventana3bytes; T-Rex, giros 90/180, punetazo, preshifts, 128K/48K y AY/menus: como en decisions.md y suplementos (sin cambios).

## Herramientas y memoria
sjasmplus1.24.0, RVM128Ktoastrack/48K, no mezclar Pentagon. Fin del codigo rev8: $AB54 (128K) y $AB4D (48K); libres hasta $B100: 1452 y 1459 bytes. No margen cerrado. Pedir listados nuevos solo para medidas o discrepancias.
