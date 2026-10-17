# Estado del proyecto Trog ZX

17-oct-2026, revision24. Sustituye rev23; no cambia hitos ni decisions.md rev15/testing.md rev12 (las correcciones a decisions.md estan en decisions_render_update_5.md). Pruebas de Carlos en 128K y 48K; asistente sin ejecucion de sjasmplus/emulador.

## Base probada
Spectrum128K convencional, sjasmplus1.24.0, LA5/MAME. Local src/, GitHub src_z80/. Scripts raiz build_game_r7.bat y build_game_r7_48.bat (-DTARGET_48K). RVM128Ktoastrack/48K. Fuente arcade intacto.
Modulos (todos ensamblados y probados por Carlos el 17-oct): game_r7.asm rev8, dino_r7.inc rev7, respawn_r7.inc rev6, eggs_r7.inc rev7, debug_r7.inc rev7 (nuevo), house_r7.inc rev0 (nuevo), victory_r7.inc rev0 (nuevo), config.inc (STOP_TOP_SHIFT/STOP_BOTTOM_SHIFT). render.inc y render_core.inc sin cambios de logica; house_gfx.inc e inv.inc sin cambios.
Hecho y validado: pasos1, 2a y 2b (anteriores); rev5 invencibilidad al arrancar y reaparecer (INV_FRAMES238) y parada ante el borde (STOP_QUIRK, edge_stop, DWAIT, EDGE_TURN); tanda1 (rev6) teclas H y K, muerte en sitio (respawn_mode, snap_axis, resp_eat); tanda2a (rev7) casa estatica con letrero HOME/REX, teclas B y J, HUD EX; tanda2b (rev8) llegada a casa y victoria provisional (E_STATE=3, jp start). Sin borde rojo (cola de decorado) en las pruebas.
Limites actuales (Carlos): STOP_LEFT_X12, STOP_RIGHT_X228, STOP_TOP_Y44 (+4 sobre el calculo inicial), STOP_BOTTOM_Y156 (-4); FALL_TOP_Y26, FALL_BOTTOM_Y164, FALL_LEFT_X4, FALL_RIGHT_X236 sin cambio hasta tener el sprite definitivo. Huecos parada-caida: 8 (izq.), 8 (der.), 18 (arriba), 8 (abajo) px.

## Medidas (listados de Carlos; fin exclusivo del codigo y datos, 128K/48K)
rev5 $A4BD/$A4B6; rev6 $A575/$A56E (+184 bytes); rev8 $AB54/$AB4D (+1503 sobre rev6: casa, victoria y teclas B/J). Libres hasta $B100 (buffers): 1452 bytes en 128K, 1459 en 48K. Sprites residentes 4608 bytes (dino y Trog placeholders, SPR_BANK2); graficos de la casa 640 bytes. No hay margen cerrado ni perfil de CPU (solo contadores PC/LT sin cambio). SPR_BANK distinto de 2 sigue sin probar.
Mapa: buffers$B100, STAT_BUF$B500, ENTS$B580, QPOS$B680, ZLIST$B690, IM2$B800, fondo$C000(6144), suelo guardado$D800-$DD9F, pila$7FFF.

## Cuestiones abiertas
Aleatorio: la semilla EGG_SEED se reinicia en cada arranque, asi que huevos, celda libre, salida de la casa y texto de victoria son siempre los mismos; el reinicio tras ganar repite el nivel. Carlos lo dejo para mas adelante o cuando lo indique el asistente.
Limites de caida: el arcade cae cuando todo el sprite sale de la caja BOUNDRY1 (analysis/parada_caida_arcade.md); los FALL_* dependen del sprite definitivo. Estimacion derivada del fuente (provisional): izq.6, der.236, arriba30, abajo164; estimacion visual de Carlos: 8, 232, 26, 160.
Casa: 28 salidas deducidas, no contadas ejecutando el arcade (EXITPOINTS); EMOVEPOS/EMOVERET/EMOVETYPE sin analisis completo. Texto de victoria y duracion (WIN_WAIT_FRAMES100) provisionales. Tocarse sin atacar no causa dano por si solo (Carlos).

## Siguiente tarea (propuesta; decide Carlos)
1) Probar SPR_BANK distinto de 2 (sprites a otro banco) antes de seguir creciendo. 2) Rumbo hacia la casa al reaparecer (arcade: DINO_FIND_CLOSEST busca huevo o casa; leer su fuente primero) y dejar eggs_left=0 como objetivo la casa. 3) Aleatorio (semilla, salida, texto, celda libre). 4) Tecla R sobre eggs_r7.inc. 5) Trogs/vecinos/medidas completas. No cambiar hitos.
Resto pendiente: consumidor de giros/punetazos (no fijar tiempos sumando guiones), TROG_PIT/TINIT_TAB; mascaras/tamanos especiales; pantalla sombra/pila banco2/cuenta dinamica/sort1byte/M1 no adoptados; TROGSPEEDPID/TSPEEDUPS/INITSPEEDUPS, DSPEED2, POWERUP_DURATION, limites T-Rex e items/ataques; ROCKPOST1..4, ADDEMOVE/deduplicacion; GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROGBAR, WAVEHAND sin analisis completo. Fuente/HUD, catalogo, bancos/ZX0, aleatoriedad/presupuesto pendientes. Consolidar decisions/testing sin perder revisiones.
