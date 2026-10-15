# Decisiones tecnicas

Version: 13-oct-2026, revision 15 (sustituye a cualquier decisions.md anterior; incorpora decisions_rev14_cambios.md)

Directiva principal (Carlos, 10-oct-2026): conversion lo mas fiel posible al arcade, dentro de las limitaciones del Spectrum. Cuando se encuentra una regla del arcade en el fuente, se implementa
sin preguntar; se pregunta solo si hay varias interpretaciones razonables o una decision de jugabilidad que contradice una decision anterior. Las observaciones de Carlos en MAME mandan
sobre las interpretaciones del codigo y se anotan como tales. Los valores que dependen de medir (distancias, tiempos) estan en constantes y se listan en docs/calibracion.md.

## Geometria de pantalla: layout D (pasos 3b, 3f y 3g; detalle en docs/layout_d.md)
- El layout C (docs/layout_c.md, paso 3a, piedras de 12x12 y paso de 28 px) queda guardado por Carlos como alternativa validada.
- Piedras de 8x8, alineadas a byte, 10 columnas x 6 filas: X = 16 + 24*c (c 0..9), Y = GRID_TOP + 24*r (r 0..5) con GRID_TOP = 44. Paso 24 en X e Y. Calles de 16 px.
- Intersecciones: X = 24 + 24*c (c 0..8), Y = GRID_TOP + 8 + 24*r (r 0..4). Total 45.
- HUD de 24 px (HUD_H = 24, tres filas de atributos; los simbolos siguen en la fila 0; Carlos lo ajustara).
- Hueco entre las piedras exteriores y el agua (observado por Carlos en el arcade): EDGE_GAP = 4 px (paso 3g; en el 3f eran 8). Suelo X 12..243, Y 40..175; agua lateral X 0..11 y 244..255;
  agua superior Y 24..39; agua inferior Y 176..191. El agua lateral deja de ser multiplo de 8: byte parcial con mascara ($F0 en el byte 1, $0F en el 30).
- Intersecciones siempre X mod 8 = 0. El desplazamiento de 2 px recorre las cuatro fases del sprite.
- El layout depende de HUD_H, GRID_TOP y EDGE_GAP (constantes con ASSERT; EDGE_GAP par entre 2 y 8).

## Organizacion del codigo (13-oct-2026)
- main.asm reducido (cabecera, tabla IM2, start, main_loop, frame_isr, INCLUDE y SAVESNA) y modulos .inc incluidos en el mismo orden que tenia el archivo unico: config.inc (EQUs, opciones, ASSERT), vars.inc, ai.inc (reaparicion y
  reorientacion), eggs_logic.inc, inv.inc, debug.inc, dino.inc, sprite.inc, input_hud.inc, video.inc y house.inc; datos en sprite_dino.inc, stones.inc, eggs.inc y house_gfx.inc. sjasmplus no enlaza: todo se ensambla desde main.asm con INCLUDE.
- Comprobado por Carlos: el corte puro dio el mismo hash SHA-256 del .sna. Los .inc de datos van junto a main.asm (los INCLUDE son relativos).
- Si se cambia el orden de los modulos cambian las direcciones: la prueba del hash solo vale para cortes sin reordenar.

## Piedras con mascara y variantes (paso 3c)
- Cada piedra de 8x8 se dibuja con mascara: (pantalla AND mascara) OR grafico. Mascara 1 conserva el fondo; mascara 0 escribe el grafico (grafico 0 con mascara 0 es opaco).
- stones.inc: stone_gfx (STONE_VARIANTS variantes de 16 bytes) y stone_map (60 bytes, indice 10*fila + columna). Hoy 3 variantes provisionales. Compresion ZX0 mas adelante (Carlos).

## Dino de 16x24 (paso 3c)
- Origen logico (dino_x, dino_y) = esquina superior izquierda de la calle. El sprite se dibuja SPR_OFF_Y = 8 lineas mas arriba, con la base alineada con el fondo de la calle.
- Punto de pies y umbrales de caida respecto al origen logico sin cambios (Carlos): (x + 8, y + 13). 4 direcciones x 4 fases x 144 bytes = 2.304 bytes. Recorte superior en CLIP_TOP = HUD_H.
- Sprite provisional (13-oct-2026): dino erguido con contorno, cuerpo con trama de damero, ojos, hocico y cola (tools/make_dino_placeholder.py, mismo formato que la flecha). Los graficos definitivos son de Carlos. La trama del cuerpo es estable al moverse
  porque las posiciones son pares y la linea superior del sprite (dino_y - 8) es par.

## Giro (paso 3e, revisado)
- Arcade (leido): DINO_STICK pide la direccion cada tick. La media vuelta se acepta en cualquier punto (DS_CK_NEW compara con NOTDIR). El giro perpendicular exige que el punto de movimiento fisico mas cercano sea un centro y sea el ultimo registrado
  por DINO_NEXT_MOVE (rango [4,4] con PNT_IN_RANGE = MAKEBOX + PNTINBOX, borde incluido); entonces DS_ND_CENTER llama a DINO_CENTER, que alinea al dino de golpe (ALIGNOBJ, sin deslizamiento) y sigue con DINOTURN. En un punto auxiliar no hay giro perpendicular.
  Dos teclas a la vez se ignoran (DINO_MOVE_TAB). Girar hacia el borde esta permitido. Un hueso en la direccion pedida la ignora.
- DINO_NEW_PNT alinea el centro del dino 3 px por encima del nodo (SUBI [3,0]); GET_DINO_MPNT mide con el centro + 5 px en Y. Interpretacion (no confirmada del todo): la ventana de giro en vertical es asimetrica (bajando de 6 px antes a 2 px despues, subiendo de 2 px antes
  a 6 px despues, en pixeles del arcade con paso 32); en horizontal es simetrica (+/-4). Escalada a 24 px: aproximadamente -4..+2 bajando, -2..+4 subiendo y +/-2 en horizontal. No se implementa: se mantiene la ventana simetrica.
- Port: giro perpendicular a TURN_TOL px o menos de una interseccion real, con alineacion inmediata (turn_node). TURN_TOL = 4 (arcade, escala 32) dio un salto visible con el dino de 16x24; Carlos probo TURN_TOL = 2 (proporcional: 4 * 24/32 = 3, con pasos de 2 px) y mejora.
  TURN_TOL = 2 provisional, a confirmar cuando existan los fotogramas de giro. node_offset tambien lo usa check_fall (STOP_QUIRK) y resp_eat.
- Observacion de Carlos (MAME, 13-oct-2026): el arcade tambien salta al alinear, pero primero alinea y luego gira con todos sus fotogramas, y eso lo disimula.
- Retencion de la direccion pedida: KEY_HOLD_FRAMES = 2 fotogramas tras soltar la tecla (arcade: 2 ticks, PJOYX). Varias direcciones a la vez: se ignoran.

## Caida y parada ante el borde (pasos 3e a 3g)
- Punto de pies (x + 8, y + 13). Caida: arriba e izquierda, pie en la primera linea o columna de suelo o antes; abajo y derecha, pie en la primera linea o columna de agua o despues. Posiciones pares.
  Con EDGE_GAP = 4: arriba y <= 26, abajo y >= 164, izquierda x <= 4, derecha x >= 236 (se derivan de FLOOR_*; con 8 eran 22, 168, 0 y 240; sin hueco, 30, 160, 8 y 232).
- La invencibilidad no evita la caida (arcade: BOUNDRY_CHECK no mira DINVIN; confirmado por Carlos en MAME). Invencible = anillo, tecla H o herradura (CAN_INV): mismo mecanismo (is_inv).
- Parada: el invencible se detiene 4 px antes del punto auxiliar del borde (media calle, 12 px, de la ultima interseccion), desplazado EDGE_GAP hacia fuera: STOP_LEFT_X = 12, STOP_RIGHT_X = 228,
  STOP_TOP_Y = 40, STOP_BOTTOM_Y = 160. Entre la parada y la caida quedan 8 px. Hipotesis: el centro del sprite (origen + 8) equivale al punto de movimiento. Pendiente de medir en MAME.
- Quirk del arcade (STOP_QUIRK = 1): DINO_NEAR_EXIT solo se evalua al registrar un punto nuevo. Mientras el auxiliar del borde siga siendo el ultimo punto registrado (TCURMPOS), no se evalua otra vez.
  Observado por Carlos en MAME: parado ante el borde, pulsar la misma direccion no hace nada; con la tecla mantenida, el dino espera, gira al lado opuesto, vuelve a girar de inmediato y cae.
  Igualmente cae si se gira con el control y se vuelve. Para volver a detenerse hay que alejarse a 4 px o menos de una interseccion real (edge_reg se borra en node_offset). (EDGE_PUSH, del 3f, se ha quitado.)
- Espera y media vuelta: sin tocar el control, tras DWAIT_FRAMES = 111 fotogramas (DWAIT_TIME = 120 ticks) da media vuelta (EDGE_TURN = 1). Una direccion contraria la cancela. Si la invencibilidad acaba
  con el dino parado, no cae. El arcade solo gira en un punto auxiliar (FINDAMOVE); el punto de parada del port equivale a uno.
- Error corregido en 3g: node_offset aceptaba una interseccion inexistente pasada la ultima (x = 240), borraba edge_reg y el dino se volvia a detener en el borde derecho. Ahora solo cuentan las reales.
- Recorte vertical obligatorio: dibujar mas alla de la linea 191 escribiria sobre los atributos ($5800). Se recorta tambien por arriba en CLIP_TOP.
- Animacion de caida: parpadeo de FALL_FRAMES = 50 fotogramas (el arcade usa FRANIMATE_DIR con DEATH_UP).

## Muerte y reaparicion
- Secuencia (paso 3d): parpadeo, apagado DEAD_WAIT_FRAMES = 55 fotogramas (DINO_ETERNAL: OBJ_OFF + SLEEP_START 60 ticks), reaparicion, START_WAIT = 55 fotogramas quieto e INV_FRAMES = 238 de invencibilidad
  (60 + 200 ticks; el indicador parpadea los ultimos INV_BLINK = 73). dino_state: 0 andando, 1 muriendo, 2 apagado. No implementado: interrupcion con START, vidas (SUBLIFE), puntos por morir (SCORE 10H).
- Tras caer reaparece en la interseccion de salida (EMOVERET, el centro de retorno de la salida). Tras una muerte sin caida el arcade usa la posicion de la animacion (GETANIXY) o el centro mas cercano (FIND_CLOSE_CPNT): el port usa la
  interseccion mas cercana (respawn_mode = 1). Pendiente de verificar: DINITPOS en muertes por enemigo y si FIND_CLOSE_MPNT (que incluye puntos auxiliares y compara X e Y por separado) puede dejar al dino en mitad de una calle.
- Direccion al reaparecer (paso 2d): DINO_FIND_CLOSEST (casa; si no, huevo con menor |dy| + |dx| desde TCURMPOS; tras una caida es el punto auxiliar del borde, una unidad de 12 px mas alla de la interseccion; con empate gana el primero, en el port por filas)
  y GET_BEST_DIRECTION con GET_DIRECTIONS: eje de mayor distancia primero (con empate, el vertical), direccion por la diferencia buscador - objetivo (con diferencia 0 el arcade deja ABAJO y DERECHA), se descarta la inversa de A11 (en DINO_SKIPANIM, A11 = DINITDIR)
  y se valida con CHECK_DIRECTION (hoy: hay interseccion vecina; faltan huesos, alquitran, fuego y fosos). Si falla, DINO_FIND_BEST con DINO_SEARCH_SAFE (2 puntos por huevo en la linea, 2n+1 pasos; mayor valoracion, luego mas pasos, con empate la ultima).
  DINITDIR vale DOWN al empezar, se pone hacia dentro en cada caida (DINODTH5) y se actualiza al usar DINO_FIND_BEST. No se ha encontrado otra escritura de DINITDIR en TROGDINO.ASM (queda por leer la parte con las muertes por enemigo).
- El huevo de la interseccion de reaparicion se recoge despues de elegir la direccion, como en el arcade, pero solo si la muerte fue a TURN_TOL px o menos del nodo (resp_eat; las caidas siempre). Decision del port (13-oct-2026): find_closest ignora el huevo a distancia 0
  (en el arcade TCURMPOS es un punto ya registrado, con su huevo comido) y, si no queda otro candidato (best_d = 255), se usa DINO_FIND_BEST.
- Contraste con MAME (Carlos, 12-13-oct-2026): con un huevo y el dino justo debajo, el port va a la derecha (DINITDIR = abajo descarta arriba) y MAME va hacia el huevo; justo encima, MAME va a la izquierda (el arcade compara centros en pixeles, el port en nodos, y con dx = 0 el port elige derecha).
  Se deja asi: comportamiento similar que no afecta a la jugabilidad.
- La reaparicion pierde los aumentos de velocidad (DSPEED2 = 0). Conversion de tiempos: factor 0,914 (ticks de 54,71 Hz a fotogramas de 50 Hz).
- EGG_AI = 0 deja solo DINO_FIND_BEST (comportamiento del 3g).

## Huevos (paso 2d)
- 45 intersecciones - 2 = 43 (ALLOT_EGGS, TROGEGGS.ASM: SUBK 2,A3 deja dos libres para Trog). Con un jugador, EGGS_AT_ONCE = WAVEEGGS = 43: se colocan todos al empezar la fase (EGGHATCH). Colocacion: al azar entre los centros libres, evitando los cercanos al dino
  (EGGHATCH usa DINOS_NEAR_PNT, radio sin leer). En el port: todas las intersecciones salvo la salida y otra al azar (xorshift de 16 bits, semilla EGG_SEED). Los huevos que se reponen durante la fase (QUICK_EGGHATCH, EGG_GROW: salen del suelo) no se implementan.
- Recogida: EGG_OBJ la ejecuta DINO_NEXT_MOVE al registrar el centro (rango [4,4], medido con una sonda 5 px por debajo del centro); ademas EGGCOLL colisiona con los dinos (EGGDINO). En el arcade hay que tapar casi todo el huevo para recogerlo (observacion de Carlos).
  El port recoge a EGG_TOL = 2 px o menos del centro (ventana de 3 posiciones con pasos de 2 px); ASSERT EGG_TOL <= TURN_TOL. Un giro con alineacion (turn_node) puede recoger el huevo a mas distancia.
- Puntuacion del arcade: EGG_SCORE da 50H por huevo (probablemente 50 puntos en BCD, interpretacion) y EGGSCRTAB sube con los huevos recogidos en una vida (500H, 1000H, 2000H ... 10000H): hito 3.
- Aleatoriedad: la semilla es fija, asi que el primer arranque es reproducible (misma distribucion de huevos y misma salida de la casa); R mezcla el contador de fotogramas. Se aleatorizara con el menu del hito 3 (semilla con el tiempo hasta pulsar una tecla).
- HUD: contador en la fila 1 (icono, recogidos, barra y total).
- Items: centro libre sin huevo, sin Trog reservado y sin dino ni enemigo cerca (FIND_OPEN_CPNT). Flor: acelera. Seta: frena. Herradura: invencibilidad (CAN_INV: (POWERUP_DURATION >> 2) + 50 ticks, START_INVINCIBILITY). Fuego, hielo y pina: mas adelante.

## Casa, salidas y victoria (pasos 2e)
- Fin de fase (arcade, leido): EGGDINO suma PEGGS; si no hay casas en bonus y no quedan huevos de ese dino (PEGGOUT >= WAVEEGGS, ISSUPPID, CNT_CMOVEEGG_CMOVE/AMOVE), EGGDINO_BUILD lanza FIRST_HOUSE. Con un jugador equivale a eggs_left = 0.
  La salida se reserva antes, al empezar la fase (PLAYER_HOUSE -> FIND_OPEN_EXIT, PEXITPNT); Carlos lo comprobo en MAME con un savestate: la casa sale siempre en el mismo sitio.
- Salidas (EMOVE): una por punto auxiliar del borde que no tiene vecino hacia fuera (CALCA1GT/CALCA1LT + ADDEMOVE + LEGAL_EXIT). Campos: EMOVEPOS (unos 4 px fuera del auxiliar, interpretacion), EMOVEDIR (la que pasa el llamador), EMOVERET (centro de retorno: donde se reaparece tras caer por ahi y
  que se comprueba con HOUSE_BLOCK_CHECK), EMOVEMPTR (el propio movimiento auxiliar; la casa se ancla a el), EMOVETYPE (CLIFF por defecto; lago, pozo o volcan si la salida cae dentro de un recuadro de BOUNDRIES). BOUNDRY1 (fase 1) solo tiene el acantilado exterior: todas las salidas son de tipo acantilado.
  FIND_OPEN_EXIT: 10 intentos al azar (salida libre, retorno no bloqueado, tipo CLIFF) y, si fallan, recorre EMOVEMAP en orden. HOUSE_BLOCK_CHECK bloquea si el centro de retorno tiene casa de transporte, roca, gato o generador de Trog.
- Port (interpretacion pendiente de confirmar con EXITPOINTS): 28 salidas, una por cada centro del perimetro y direccion hacia fuera (9 arriba, 9 abajo, 5 izquierda, 5 derecha; dos en cada esquina). house.inc: exit_tab (columna, fila del nodo de retorno, direccion) y house_select
  (rand16 modulo 28; sin filtros en la fase 1). house_exit = indice elegido. Marcador de depuracion con XOR (house_marker_toggle) dentro de DEBUG_CHEAT.
- Construccion (2e-3): house_build se llama al recoger el ultimo huevo (egg_pickup) o con la tecla B. La casa aparece oculta HOUSE_APPEAR_FRAMES = 51 fotogramas (SLEEP 38H = 56 ticks) y luego house_flash_update pinta la extension y el letrero y los alterna.
  Extension estatica: arriba 32x20 (lineas 24..43), abajo 32x20 (172..191), izquierda 16x32 (X 0..15) y derecha 16x32 (X 240..255), centrada en la calle; letrero 16x8 centrado, igual en los cuatro lados. Al repintar se protege el sprite (se borra, se pinta la casa y se repinta).
  Arriba y abajo son dibujos distintos; la derecha es la izquierda reflejada (la herramienta lo hace al generar). Se puede ensanchar arriba y abajo en multiplos de 8 px (6 bytes = 48 px) o con margenes transparentes; hasta 24 px por lado.
- Letrero (Carlos, MAME): alterna HOME y REX (en NES solo HOME; en MS-DOS HOME y REX); el texto va en los bitmaps; con dos jugadores cambiaria el nombre. Velocidad estable: 0,83 s el ciclo completo, sin aceleracion. La lectura de HOUSE_FLASH_LP como aceleracion era erronea.
  Correccion pendiente de aplicar: HOUSE_FLASH_FRAMES = 21 (cada letrero 21 fotogramas; 24 si se prefiere 63 bpm).
- Arcade (observacion de Carlos): la extension crece con una animacion, despues sale una galleta con forma de hueso con HOME y REX; la extension tiene sus propias piedras y huesos en el contorno. Decision: sin animacion de crecimiento por ahora (posible reveal por filas o columnas del sello final, sin arte nuevo, mas adelante).
- Llegada (arcade): ISHOME se consulta en DINO_OUT_OF_BOUNDS (el dino sale del recuadro del acantilado): si hay casa propia o comunitaria y objeto casa, CELEBRATE; si no, DINODTH5. DINO_SAFETY no frena al dino si DINO_LOOK_HOME ve la casa de frente (calle sin paredes ni CENTER_CHECK peligroso).
- Decision de Carlos (13-oct-2026): el dino se detiene en la posicion equivalente a una celda adicional en la salida (como el arcade): el nodo del borde mas 24 px hacia fuera. Con el layout D: arriba y = 28, abajo y = 172, izquierda x = 0, derecha x = 240. Consecuencias (propuesta, por validar):
  los umbrales de caida (y <= 26, y >= 164, x <= 4, x >= 236) se alcanzan antes que la celda adicional abajo y en los lados, asi que en la calle de la casa la caida se sustituye por esta llegada; arriba la cabeza del sprite (8 lineas por encima del origen) queda 4 lineas bajo el recorte del HUD.
- Victoria (CELEBRATE, leido): quita la invencibilidad, retira un posible pozo de casa, WAVE_OVER = 1, desactiva colisiones, bonus de ventaja (START_SCORE) en la primera victoria, DINOTURN a DOWN y STANDU, espera 10 ticks, WAIT_WAVE_END y 30 ticks, baja el cielo (SKYDOWN), WIN_SND, WIPE_FROM_MID, el dino vuela a [180,185],
  pantalla negra con fiesta y baile, mensaje (con un jugador, uno de cinco: Congratulations, Good Job, Excellent, Way To Go, DINO-MITE; con varios, "The Winner/The Best/The Victor/Top Dino/DINO-MITE" y 25.000 puntos por rival), 10 y 85 ticks y WAVE_END. Carlos puede detallar la secuencia mas adelante.
  Port, provisional (2e-4a): dino quieto mirando abajo, uno de los cinco textos unos 2 s y reinicio del nivel con huevos nuevos.

## Animaciones de giro (decision de Carlos, 13-oct-2026)
- Arcade (leido): DINOTURN elige la animacion en TURNTAB segun la direccion actual y la nueva: 8 giros de 90 grados (TURNLU, TURNRU, TURNLD, TURNRD, TURNUL, TURNDL, TURNUR, TURNDR); media vuelta: animacion ABOUTxx con un bit aleatorio (hacia un lado u otro). Llama a SKIP_ANIMATION (sin analizar)
  y a DINO_FRANIM con A1 = 1, y deja DDIR en la nueva direccion. Interpretacion: el dino queda quieto en el nodo durante la animacion.
- Conteo (Carlos): NES usa 2 fotogramas intermedios diagonales; el arcade creo que 3; el port usara 1.
- Port (se implementa despues del 2e): alinear con DINO_CENTER y luego animar (como el arcade). 1 fotograma intermedio por giro, 4 diagonales (arriba-izquierda, arriba-derecha, abajo-izquierda, abajo-derecha) compartidas por los 8 giros. Como tras alinear el dino esta en X mod 8 = 0, bastan las fase 0: 4 x 144 = 576 bytes en el formato actual. Media vuelta propuesta: dos giros de 90 grados por un lado al azar (direccion vieja, diagonal, perfil, diagonal, nueva), 3 intermedios; por confirmar.
- Presupuesto aproximado (estimaciones): una pose de 16x24 con mascara y 4 fases = 576 bytes. Dino con 4 fotogramas de andar por direccion = 9.216 bytes; 4 poses de giro = 2.304; 8 poses = 4.608.
- Andar: ciclo de 4 pasos [quieto, paso A, quieto, paso B] con 3 imagenes distintas por direccion; el fotograma quieto es el de reposo. Los dos lados se dibujan una vez y se espejan. Punetazo: 1 fotograma por direccion (el horizontal se espeja).
  Estos numeros se fijan sin contrastar con NES; para el resto de animaciones se contara el spritesheet de NES al implementarlas.
- Espejado: la casa (estatica) se espeja en la herramienta. Para los sprites moviles hay dos opciones: generar ambas orientaciones en la herramienta (mas ROM, sin coste de CPU) o espejar en ejecucion (tabla de 256 bytes para invertir bits, invertir el orden de los 3 bytes por linea, y la fase p
  pasa a 8 - p: las fases 2, 4 y 6 se corresponden y la 0 equivale a la 0 desplazada un byte). Se decide con el presupuesto de memoria (version 48K incluida).

## Atributos (provisional)
- Un unico atributo para todo el campo (ATTR_FIELD). Se revisara mas adelante (Carlos): monocromo o solo el dino en color. Banco 0 como fondo base (layout_c.md, seccion 7): NO implementado.

## Velocidad
- Acumulador: DINO_SPEED pasos de 2 px cada 16 fotogramas. Valor actual 7 = 43,75 px/s. Layout D: tramo de 24 px = 12 pasos = unos 0,55 s. Pendiente: medir en MAME el tiempo entre intersecciones (Carlos).

## Teclas de depuracion (DEBUG_CHEAT = 1)
- H: invencibilidad permanente (como el anillo o la herradura). K: muerte sin caida (parpadeo, apagado, reaparicion en la interseccion mas cercana). R: nueva distribucion de huevos y de la salida de la casa (no funciona con la casa construida).
  B: construye la casa en la salida elegida. El marcador de la salida elegida tambien es de depuracion (se puede separar en una opcion propia, DEBUG_HOUSE_MARK).

## Control (Carlos)
- Teclado primero: Q arriba, A abajo, O izquierda, P derecha, ESPACIO accion, H, K, R y B depuracion. Luego Sinclair y despues Kempston. Un solo boton de accion por dino. Mascara de bits: arriba 1, abajo 2,
  izquierda 4, derecha 8, accion 16.

## Jugadores y prototipo de la mano
- Un jugador en los hitos actuales; dos jugadores NO; cuatro descartado. Prototipo de la mano: hito 4, opcional; reservar un bit de barrera en los datos de los tiles de segmento.

## Planificacion de memoria
- Reparto de datos: se empieza con la opcion C (sin reubicar; codigo en el banco 2, tabla set_bank) manteniendo A y B abiertas.
- Opcion A con cinta: el cargador puede cargar cada conjunto directamente en el banco que toque si detecta antes la mascara de contencion (rutina de deteccion en el cargador). No sirve para snapshots (.sna), donde el reparto es fijo.
- La deteccion por medida y la deduccion por modelo no se excluyen; ver memoria.md.

## Metodo de trabajo
- Los errores que se encuentran al probar se corrigen en el mismo paso. La calibracion fina (distancias, tiempos, tolerancias) se agrupa en una pasada con mediciones en MAME despues de cada bloque
  (huevos 2d, items, Trog), usando docs/calibracion.md.
- Pasos pequenos con una prueba concreta; entregar archivos completos cuando es practico. Al cortar el codigo en modulos, comprobar el hash del .sna antes y despues.
- Antes de decidir un comportamiento, leer la rutina del fuente (Carlos pega el texto cuando el conector lo trunca). Separar siempre observaciones directas, interpretaciones y desconocidos (docs/analisis_*.md).

## Pendiente
- Leer del fuente: ver "Pendiente de analisis del fuente" en status.md (EXITPOINTS, WAVE_END, DINOCOLL, SKIP_ANIMATION, STOPPED, CENTER_CHECK, TROG_START, DINO_ADD_VEL, DINO_VEL, TROGBAR.ASM, WAVEHAND, PUNCHED, DINO_PUNCHED, SPIT_FIRE).
- Verificar ROCKPOST1 a ROCKPOST4 (IMGTBL.ASM) y que el fuente archivado es la compilacion LA5.
- Fade-in muy rapido del arcade: dejado para mas adelante (por concretar donde: inicio de fase, aparicion de la casa u otro).
