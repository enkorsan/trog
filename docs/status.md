# Estado

Version: 13-oct-2026, revision 16 (sustituye a cualquier status.md anterior)

## Hecho
- Pasos 1c a 3g y 2d. Carlos ha probado el 2d en emulador: funciona. Ajustes posteriores, probados por Carlos (detalle en decisions.md):
  recogida de huevos a EGG_TOL = 2 px (antes TURN_TOL); reaparicion con resp_eat (el huevo del nodo se recoge solo si la muerte fue a TURN_TOL px o menos del nodo; las caidas siempre);
  find_closest ignora el huevo a distancia 0 y choose_dir pasa a best_by_rating si no hay candidato (best_d = 255); TURN_TOL = 2 probado por Carlos (el salto al girar mejora), provisional.
- Reaparicion: la logica del port coincide con GET_BEST_DIRECTION, GET_DIRECTIONS, DINO_FIND_BEST y DINO_SEARCH_SAFE leidos del fuente (docs/analisis_reaparicion_giro.md). Con un huevo y el dino justo debajo el port va a la derecha
  (DINITDIR = abajo descarta la inversa, arriba); en MAME con DINITDIR distinto va hacia el huevo. Se deja asi (decision de Carlos): el comportamiento es parecido y no afecta a la jugabilidad.
- Codigo cortado en modulos .inc (main.asm, config.inc, vars.inc, ai.inc, eggs_logic.inc, inv.inc, debug.inc, dino.inc, sprite.inc, input_hud.inc, video.inc, house.inc) mas los datos (sprite_dino.inc, stones.inc, eggs.inc, house_gfx.inc).
  Comprobado por Carlos: mismo hash SHA-256 del .sna antes y despues del corte.
- Dino provisional de 16x24 (4 direcciones x 4 fases, 2.304 bytes, mismo formato que la flecha) generado con tools/make_dino_placeholder.py. Los graficos definitivos son de Carlos.
- Paso 2e-1 y 2e-2 (probados por Carlos): house.inc con la tabla exit_tab (28 salidas, provisional) y house_select (elige la salida al empezar; marcador de depuracion con XOR).
- Paso 2e-3 (probado por Carlos: funciona segun lo esperado): house_build (al recoger el ultimo huevo o con la tecla B), espera de 51 fotogramas, extension de isla estatica (placeholders: 32x20 arriba y abajo, 16x32 a los lados; la derecha es la izquierda reflejada
  por la herramienta) y letrero 16x8 HOME/REX alternado (house_gfx.inc, tools/make_house_placeholder.py). El dino sigue cayendo por esa salida (todavia no hay victoria).
- Fuente leido y analizado (docs/analisis_*.md): DINO_STICK, DINO_CENTER, DINO_NEW_PNT, GET_DINO_CPNT/MPNT, PNT_IN_RANGE, FIND_CLOSE_MPNT/EPNT, DINOTURN, GET_BEST_DIRECTION, DINO_FIND_BEST, DINO_SEARCH_SAFE, CHECK_DIRECTION, DINO_LOOK_HOME,
  EGGDINO, EGG_OBJ, FIRST_HOUSE/HOUSEB, PLAYER_HOUSE, FIND_OPEN_EXIT, ISHOME, HOUSE_BLOCK_CHECK, CELEBRATE, CALCA1GT/CALCA1LT, LEGAL_EXIT, INITEMOVE/FINDEMOVE y la estructura EMOVE (TROGEQU.ASM).
  TROGHAUS.ASM son las casas de transporte (teletransportadores), rocas y generadores de Trog: no es la casa del dino.

## Pendiente de aplicar por Carlos
- Parpadeo del letrero a velocidad constante (medido en MAME: 0,83 s el ciclo HOME+REX, sin aceleracion): HOUSE_FLASH_FRAMES = 21; quitar HOUSE_FLASH_START/MIN y house_interval (cambios indicados en el chat del 13-oct).

## Siguiente tarea (propuesta)
1. Paso 2e-4a: llegada a casa y victoria provisional. check_fall: si hay casa (house_state >= 1; en el arcade el objeto casa existe desde que se construye) y el dino va hacia la salida de la casa por su calle, no cae ni se detiene ante el borde: avanza hasta la
   celda adicional de la salida y ahi empieza la victoria. Victoria provisional: quieto mirando abajo, uno de los cinco textos (Congratulations, Good Job, Excellent, Way To Go, DINO-MITE) unos 2 s y reinicio del nivel. Prueba con la tecla B.
2. Paso 2e-4b: reorientacion hacia la casa (find_closest/choose_dir con casa, dir_valid acepta la salida con casa, best_by_rating +5).
3. Giros con fotogramas (despues del 2e): alinear con DINO_CENTER y luego animar; 1 diagonal intermedia por giro (4 diagonales, solo fase 0). Carlos cuenta los fotogramas de NES (2 diagonales) y arcade (3).
4. Paso 3 (Trog basico): andar y perseguir, sin variantes.
5. Aleatoriedad de la primera partida (semilla con el tiempo hasta pulsar una tecla): con el menu del hito 3. De momento el primer arranque es reproducible.
6. Pruebas independientes: 4a detector de bancos contendidos, 4b coste del dibujado por banco, 4c medicion de ZX0.
7. Decisiones de memoria: espejado de sprites en la herramienta (mas ROM) o en ejecucion (tabla de 256 bytes; el espejado de la fase p da la fase 8-p). Decidir con el presupuesto de memoria.
8. Pulido (mas adelante): animacion de crecimiento de la extension, fade-in muy rapido del arcade (por concretar), anchura de la plataforma arriba y abajo (hasta 24 px por lado), borde de huesos y rocas, un dibujo propio por lado.

## Pendiente de analisis del fuente
- No leido: EXITPOINTS (llamador de CALCA1GT/CALCA1LT; confirma el numero y la posicion de las salidas del nivel 1), GET_EXIT_ANI, WAIT_WAVE_END y WAVE_END, OUTEGGS y CNT_CMOVEEGG_*, guion REXDOORMAT_x (TROGDDAT.ASM: tiempos del letrero),
  DINOCOLL y DINO_TROG_OOF (DINITDIR en muertes por enemigo), SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT (radio), TROG_START, DINO_ADD_VEL, DINO_VEL, TROGBAR.ASM y WAVEHAND.
  Texto disponible, analisis pendiente: PUNCHED, DINO_PUNCHED, SPIT_FIRE.
- Revision: confirmar que el fuente archivado es la compilacion LA5 (el repositorio tiene el fuente en la raiz y carpetas LA3 y LA5). POSTMAP1 leido (60 postes, 10 x 6, paso 32); ROCKPOST1 a ROCKPOST4 sin verificar.

## Repositorio
- El fork github.com/enkorsan/trog (rama main) aun no contiene el codigo del port: src_z80/ y tools/ solo tienen README.md, y docs/status.md del repositorio es una version antigua y corta. La copia de trabajo es el proyecto de Perplexity.
  Cuando Carlos lo decida se sube todo a src_z80/, tools/ y docs/ en un solo commit, con confirmacion previa.

## Archivos del proyecto de Perplexity
- docs: decisions.md (rev. 15), status.md (rev. 16), testing.md (rev. 12), calibracion.md, memoria.md, catalogo_frames.md (rev. 1), layout_c.md, layout_d.md, analisis_*.md (nuevos: analisis_reaparicion_giro.md, analisis_casa_salidas.md).
  testing_paso_2d.md queda superado por testing.md rev. 12 y decisions_rev14_cambios.md queda aplicado en decisions.md rev. 15.
- src: main.asm y los .inc citados arriba. tools: make_sprites.py, make_stones.py, frame_report.py, make_dino_placeholder.py, make_house_placeholder.py.
