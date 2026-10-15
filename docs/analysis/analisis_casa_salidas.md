# Analisis: casa, salidas y victoria

Version: 13-oct-2026. Fuente: TROGEQU.ASM, TROGMAPS.ASM, TROGHAUS.ASM y TROGDIN2.ASM (leidos del repositorio historicalsource/trog, raiz; revision LA5 sin confirmar) y rutinas pegadas por Carlos (TROGEGGS.ASM: EGGDINO, EGG_OBJ, FIRST_HOUSE,
PLAYER_HOUSE, FIND_OPEN_EXIT, ISHOME; TROGPOST.ASM: CALCA1GT, CALCA1LT, LEGAL_EXIT, INITEMOVE, GETEMOVE, FINDEMOVE, FIND_CLOSE_EPNT, MARKEMOVE; TROGDINO.ASM: CELEBRATE, HOUSE_BLOCK_CHECK, DINOTURN). Cada punto indica la etiqueta.

## 1. Observaciones directas del codigo

### Estructuras (TROGEQU.ASM)
- EMOVE (salida), EMOVESIZE = 0B0H: EMOVEPOS (posicion), EMOVEPLYR (FF = se puede construir, FE = sin casas, numero de jugador = ocupada), EMOVEDIR (0 arriba, 1 abajo, 2 izquierda, 3 derecha), EMOVEOBJ (objeto casa; 0 = ninguno),
  EMOVERET (indice del movimiento de retorno), EMOVEMPTR (puntero al movimiento), EMOVETYPE (0 acantilado, 1 pozo, 2 lago, 3 volcan), EMOVEPIT. NEMOVES = NMOVES/9 = 56 salidas como maximo (NMOVES = 512).
- Mapas: MOVEMAP (todos los movimientos, con vecinos arriba, abajo, izquierda, derecha y diagonales), CMOVEMAP (centros; CMOVEEGG = numero de jugador del huevo, FF libre), AMOVEMAP (auxiliares), EMOVEMAP (salidas). PXPLAYER = 4 (marca de la casa comunitaria y de los huesos indestructibles).
- TROGMAPS.ASM: POSTMAP1 = 60 postes, 6 filas (Y 72..232) x 10 columnas (X 50..338), paso 32; centros = 45. BOUNDRY1: recuadro de acantilado [70,53]-[215,336] y nada mas. Otras fases anaden lagos, pozos o volcan.
- Tipos de zona (TROGEQU.ASM): TYPE_CLIFF = 0 (fuera cae al agua), TYPE_PIT = 1, TYPE_LAKE = 2, TYPE_VOLCANO = 3.

### Creacion de salidas (TROGPOST.ASM)
- CALCA1GT (arriba e izquierda) y CALCA1LT (derecha y abajo): si el movimiento no tiene vecino (A0 negativo), calculan una posicion = posicion del movimiento + (poste - movimiento) / 4 con las mitades de X e Y intercambiadas (RL 16), llaman a ADDEMOVE,
  LEGAL_EXIT y guardan: MOVEEPTR del movimiento = la salida, EMOVEDIR = A2 (lo pasa el llamador), EMOVERET = A4 (centro de retorno), EMOVEMPTR = A6 (el propio movimiento auxiliar).
- LEGAL_EXIT: marca la salida libre (EMOVEPLYR = FF); recorre BOUNDRIES; si la posicion cae dentro de un recuadro de pozo, lago o volcan guarda su tipo (y el punto medio del pozo); si no, TYPE_CLIFF. El recuadro de acantilado exterior se salta.
- INITEMOVE llena EMOVEMAP con FF y pone EMOVECNT a 0. FINDEMOVE busca una posicion o devuelve la primera entrada libre. FIND_CLOSE_EPNT: la salida mas cercana (comparacion por componentes). GETEMOVE: indice -> direccion. MARKEMOVE: llama a MARK_MOVES.

### Eleccion de la salida
- PLAYER_HOUSE (al empezar la fase, por el savestate de Carlos): si TUTOR, tabla fija PH_TABLE ([232,66], [232,130], [232,258], [232,322] por jugador, con FIND_CLOSE_EPNT); si no, FIND_OPEN_EXIT. Guarda el puntero en PEXITPNT y marca EMOVEPLYR con el numero del jugador.
- FIND_OPEN_EXIT: hasta 10 intentos: indice al azar (RANDU), salida libre (EMOVEPLYR negativo; FF y FE cuentan como libres), con retorno (EMOVERET) y sin bloqueo (HOUSE_BLOCK_CHECK), y EMOVETYPE = TYPE_CLIFF. Si fallan, recorre EMOVEMAP en orden con las mismas condiciones.
  Devuelve Z si no hay ninguna.
- HOUSE_BLOCK_CHECK: bloqueado si el centro de retorno tiene (CMOVEEGG) HAUSID, ROCKID, CATID o TGENID; los huevos normales no bloquean.

### Construccion de la casa
- EGGDINO (huevo choca con dino): si el huevo es del mismo jugador (o comunitario) lo borra, suma PEGGSTHISLIFE y PEGGS, da EGG_SCORE (MOVI 50H, EGGP1_SND, SENDOBJ). Sin casas en bonus. Si no hay lista comun de huevos: PEGGACTIVE - 1, OUTEGGS, y si PEGGOUT < WAVEEGGS no construye.
  Luego: sin mas huevos en la lista (ISSUPPID) ni en los mapas (CNT_CMOVEEGG_CMOVE/AMOVE) -> EGGDINO_BUILD -> EGGDINO_HOUSE: crea el proceso FIRST_HOUSE (HOUSEPID | jugador) copiando DATABLE y DPLAYER. (Antes hay un secreto, TUNNEL_PIT, si se deja para el final el huevo mas lejano y no se ha muerto.)
- FIRST_HOUSE: A10 = PEXITPNT; si 0 (dron), BOZO_HOUSE usa FIND_OPEN_EXIT. HOUSEB: marca EMOVEPLYR, lanza ISLE_EXTEND (RECLAIM_LAND: desliza un trozo de isla y construye el borde, con sus piedras y huesos), crea el objeto casa (ALLHOUSE_INIT si comunitaria; HOUSE_TAB por jugador),
  guarda EMOVEOBJ de inmediato, lo coloca con TOMOVE sobre EMOVEMPTR, elige el guion en HOUSE_ANIS (REXDOORMAT_U/D/L/R, TROGDDAT.ASM) por direccion de salida (y jugador), anima el primer fotograma, OBJ_OFF, insercion en HOUSELST, SLEEP 38H (56 ticks), OBJ_ON, sonido
  (HOUSEB_SND si es la primera y no suena la musica thrash; si no HOUSEB_SND2), @HOUSES = 1, @MAINTUNE = 1, y HOUSE_FLASH_LP (dos fotogramas por vuelta con multiplicador de espera que baja de 100H a 40H). Quita el enlace al terminar el guion.
- DELETE_HOUSE: borra la casa de un jugador (procesos HOUSEPID y objetos HOUSEOID).

### Llegada y victoria
- ISHOME: salida mas cercana al dino (FIND_CLOSE_EPNT sobre su centro); su EMOVEDIR debe ser la direccion del dino o la contraria; EMOVEPLYR >= 0 y (PXPLAYER o el jugador); EMOVEOBJ distinto de 0 -> en casa.
- DINO_OUT_OF_BOUNDS (TROGDINO.ASM): ISHOME en Z -> DINODTH5 (muerte); si no, CELEBRATE (si hay jugadores o en modo attract).
- DINO_LOOK_HOME (TROGDIN2.ASM): recorre la calle en una direccion; sin movimiento siguiente, hay "casa delante" si la salida (MOVEEPTR) tiene EMOVEPLYR valido (PXPLAYER o el jugador) y EMOVEOBJ; en el camino no puede haber paredes ni centros que no pasen CENTER_CHECK. DINO_SAFETY lo usa para no parar al dino ante el borde.
- CELEBRATE: KILL_INVINCIBILITY; retira un posible pozo de casa (HOUSE_PITS); si GAME_STATE no es INGAME lo pone (primero en llegar), si ya lo es salta a DINOMADXFER; WAVE_OVER = 1; mata procesos de colision; DINOMAD; TREX_KILL; bonus de ventaja (START_SCORE) si PWINS = 0; PWINS + 1; PLASTWIN = WAVE;
  DINOTURN a DOWN y STANDU; SLEEP 10; en modo attract, CELEBRATE_AMODE_DIE; WAIT_WAVE_END; SLEEP 30; SKYDOWN; SOUND ALLOFF; SLEEP 4; WIN_SND; KILALL; WIPE_FROM_MID (30); el dino vuela a [180,185] (DINOFLY_ANIM, SLINEVEL 40 ticks); pantalla negra; PARTY_STUFF, SLEEP 40, DANCE_STUFF; "LEVEL BONUS AWARDED"
  si hubo bonus; DINOCHEER_ANIM y CELEBRATEAMATE; sonido de COMPSNDTAB; mensaje: un jugador, uno de COMP_MESSAGES (NUM_WIN_MESSAGES = 5: Congratulations, Good Job, Excellent, Way To Go, DINO-MITE), varios jugadores, WIN_MESSAGES y 25000*(n-1) puntos; SLEEP 10, SLEEP 85, SOUND YAMOFF, JAUC WAVE_END.
- TROGHAUS.ASM no es la casa del dino: HOUSE_START/HOUSE_PROCESS son casas de transporte (teletransporte entre dos puntos), ROCK_START y TGEN_START (generadores de Trog) y las colisiones HOUSE_DINO/TROG/WHEEL.

## 2. Observaciones de Carlos (MAME y otras versiones)
- La salida de la casa se decide antes de coger el ultimo huevo (savestate con pocos huevos: siempre en el mismo sitio).
- La extension de isla crece con una animacion; luego aparece una galleta con forma de hueso con "HOME" y "REX" (nombre del dino que ha acabado). La extension tiene sus propias piedras y huesos en el contorno.
- El letrero alterna HOME y REX a velocidad estable: 0,83 s el ciclo (unos 63 bpm); no acelera. En NES solo pone HOME; en MS-DOS alterna. El texto esta en los bitmaps.
- La salida puede estar en cualquier lado de la isla, al parecer al azar.
- El dino se detiene en la posicion equivalente a una celda adicional en la salida.
- El arcade tiene un fade-in minimo y muy rapido (por concretar donde).

## 3. Interpretaciones (no confirmadas)
- Los 28 puntos de salida del nivel 1: una por cada centro del perimetro y direccion hacia fuera (9 + 9 + 5 + 5); dos en cada esquina. Se apoya en PH_TABLE (Y = 232 = ultima fila de postes, X = centros de columna) y en que EMOVEMPTR es un auxiliar del borde. Falta EXITPOINTS.
- El dino pisa la extension al salir del recuadro del acantilado (DINO_OUT_OF_BOUNDS), y ahi ISHOME decide casa o caida. La casa existe para ISHOME desde que HOUSEB guarda EMOVEOBJ (antes de los 56 ticks ocultos).
- EGG_SCORE = 50H probablemente son 50 puntos en BCD. El multiplicador de HOUSE_FLASH_LP no acelera el parpadeo: la primera lectura (aceleracion de 100H a 40H) no cuadra con la medicion de Carlos.
- FE en EMOVEPLYR ("sin casas") no se usa en FIND_OPEN_EXIT.

## 4. Desconocidos
- EXITPOINTS (que movimientos reciben salida y que A2/A4 pasa), GET_EXIT_ANI, WAIT_WAVE_END y WAVE_END (siguiente fase), OUTEGGS, CNT_CMOVEEGG_*, SKIP_ANIMATION, HOUSE_PITS/HOUSE_PIT.
- Guion REXDOORMAT_x (TROGDDAT.ASM; empieza en HOMEU1): numero de fotogramas y retardos del letrero.
- Dimensiones exactas de la casa y de la extension en pantalla del arcade.

## 5. Port (house.inc, house_gfx.inc, tools/make_house_placeholder.py)
- exit_tab: 28 entradas (columna, fila, direccion), house_select (rand16 mod 28), house_build (egg_pickup con el ultimo huevo o tecla B), house_flash_update (espera de 51 fotogramas, extension y letrero alternado), stamp_draw con mascara.
- Pendiente: parpadeo constante (HOUSE_FLASH_FRAMES = 21), 2e-4a (llegada, sin caida, a la celda adicional: arriba y = 28, abajo y = 172, izquierda x = 0, derecha x = 240; victoria provisional) y 2e-4b (reorientacion hacia la casa).
