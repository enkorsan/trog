# Analisis: reaparicion y giro del dino

Version: 13-oct-2026. Fuente: texto de TROGDINO.ASM (leido del repositorio historicalsource/trog, raiz; la revision LA5 no esta confirmada) y rutinas pegadas por Carlos (GET_BEST_DIRECTION y compania en el bloque que
TROGTROG.ASM define con .DEF; PNT_IN_RANGE y FIND_CLOSE_* de un archivo de utilidades sin identificar). Cada punto indica la etiqueta que lo respalda. Todo es del arcade salvo lo marcado "Port".

## 1. Observaciones directas del codigo

### Reaparicion tras una muerte (TROGDINO.ASM)
- DINODEATH_XFER pone PTEMP1 = 0 ("FLAG SAFE SEARCH"). En DINO_SKIPANIM, si PTEMP1 es distinto de 0 (primer arranque de la fase) o el jugador esta en modo instrucciones, se salta la busqueda; si no, llama a DINO_FIND_CLOSEST ("el huevo o la casa mas cercanos"),
  y con el resultado en A4 y el movimiento actual (PTEMP3) llama a GET_BEST_DIRECTION. Si no hay objetivo (Z) o no hay direccion (Z), salta a DINO_START_SAFE.
- DINO_START_SAFE: carga PTEMP3 en A1, llama a DINO_FIND_BEST y guarda el resultado (A2) en DINITDIR y en A11.
- A11 al llegar a DINO_SKIPANIM: se carga en DINOINIT_DRONE con MOVE *A13(DINITDIR),A11,W y nada de lo que sigue hasta DINOPLAY lo sobrescribe (lectura del texto). Es decir, A11 = DINITDIR.
- Escrituras de DINITDIR en TROGDINO.ASM: inicio de la fase (DINO_WAVE_1ST, A11 = direccion de arranque de la tabla), DINODTH5 (A11 = NOTDIR(EMOVEDIR): hacia dentro) y DINO_START_SAFE. En otros archivos solo aparecen las de Trog, rueda y muelle. Queda por leer la parte de TROGDINO.ASM
  con las muertes por enemigo.
- DINO_FIND_CLOSEST, DINO_FIND_FURTHEST: referenciados, no vistos. Usados tambien en DINO_END_INIT (huevo mas lejano, DFURTHEST) y, para dinos con DSMART, en DINO_CK_LOOK en cada punto.

### GET_BEST_DIRECTION, GET_DIRECTIONS, CHECK_DIRECTION
- GET_BEST_GEN: A3 = NOTDIR(A11). Si la primera opcion (A1) es A3 se salta a la segunda sin comprobar; si no, CHECK_DIRECTION; si falla pasa a la segunda; si la segunda es A3 sale con Z; si CHECK_DIRECTION falla, limpia A2 y sale con Z.
- GET_DIRECTIONS: A2 = centro del buscador - centro del buscado (SUBXY, con signo). Y > 0: arriba; si no, X > 0: izquierda; si no, abajo y derecha respectivamente (diferencia 0: abajo y derecha). A1 y A2 = |dy| y |dx|; si |dx| > |dy| la primera opcion es la de X y la segunda la de Y;
  si no (incluye el empate), la primera es la de Y. Suma las velocidades del objeto buscado (en un huevo, 0).
- GET_DIRECTIONS_TO_MOVE (no usada en este flujo): como la anterior pero con un margen de 5 px por eje ("X on target") que hace que la unica opcion sea la del otro eje.
- CHECK_DIRECTION: desde un centro, ONE_MOVE en la direccion; sin movimiento (borde): para un dino, valida solo si hay casa propia o comunitaria en esa salida (CHECK_DIR_EXIT_MOVE); si hay movimiento, MOVEPLYR >= 0 (pared o hueso) falla y, si no, se mira el siguiente centro con CENTER_CHECK.
  Desde un punto auxiliar (CHECK_AUX) la logica es parecida con dos movimientos.

### DINO_FIND_BEST y DINO_SEARCH_SAFE
- DINO_FIND_BEST recorre UP, DOWN, LEFT, RIGHT (A0 = 0..3), llama a DINO_SEARCH_SAFE y se queda con la mayor valoracion (A6); con empate, el que tiene mas pasos (A5); con empate total, el posterior.
- DINO_SEARCH_SAFE camina por MOVEREL: cada movimiento suma 1 paso. Valoraciones: huevo propio o comunitario +2 (EGG_RATING), pina +3 (PINEAPPLE_RATING), pared al final +1 (WALL_RATING), casa propia o comunitaria al final de una salida +5 (HOUSE_RATING). Alquitran, fuego y pozo local terminan la busqueda.
  Una salida sin casa propia no suma nada. Los huevos de otros jugadores no cuentan.

### Giro del jugador (TROGDINO.ASM)
- DINO_STICK: GET_DINO_MPNT (punto de movimiento mas cercano al centro del dino + 5 px en Y); lee PJOYSTCK y lo traduce con DINO_MOVE_TAB (varias teclas a la vez = 0FFH, ilegal: sigue igual). Misma direccion: sigue. Hay movimiento en la direccion pedida y MOVEPLYR >= 0 (hueso):
  sigue en la actual. Direccion contraria (DS_CK_NEW, NOTDIR): DS_NEW_DIRECTION de inmediato. Perpendicular: si el movimiento fisico es el ultimo registrado (TCURMPOS) y es un centro (MOVECPTR valido) -> DS_ND_CENTER: DINO_CENTER y giro; si es un auxiliar, no gira.
- DINO_NEXT_MOVE registra el punto con PNT_IN_RANGE [4,4] (A1 = centro + 5 px, A3 = posicion del movimiento) y llama a EGG_OBJ.
- PNT_IN_RANGE = MAKEBOX (caja centrada en A3 con el rango A2) + PNTINBOX (borde incluido).
- DINO_CENTER -> DINO_NEW_PNT: resta [3,0] al punto, calcula el centro actual (GET_DINO_CPNT) y ALIGNOBJ: alineacion instantanea; el comentario dice "with animation" pero el codigo no anima.
- DINOTURN: TURNTAB (por direccion actual): byte por direccion nueva: 0FEH misma, 0FFH media vuelta, o el indice de la animacion de giro (TURNLU, TURNRU, TURNLD, TURNRD, TURNUL, TURNDL, TURNUR, TURNDR). Media vuelta: ABOUTUL + 2*direccion + bit de RANDOM. Luego DINOLOAD, SKIP_ANIMATION, DINO_FRANIM (A1 = 1) y DDIR = nueva.
- FIND_CLOSE_MPNT / FIND_CLOSE_EPNT: recorren todos los movimientos (centros y auxiliares) o salidas; el candidato sustituye si su distancia en X no es mayor y la de Y tampoco (comparacion por componentes, no por suma).

## 2. Interpretaciones (no confirmadas)
- La ventana de giro en vertical es asimetrica por la sonda (+5) frente a la alineacion (-3): sonda a +2 px del nodo con el dino alineado. Bajando: de 6 px antes a 2 px despues; subiendo: de 2 antes a 6 despues; horizontal +/-4 (en pixeles del arcade, paso 32).
  Depende de que A1 conserve el +5 entre GET_DINO_MPNT y PNT_IN_RANGE (asi se lee en el texto).
- Con DINITDIR = abajo (valor inicial) el arcade descarta "arriba" como primera opcion; con DINITDIR = arriba (tras una caida por abajo) descarta "abajo". Cuadra con lo observado por Carlos en MAME (12-13-oct-2026): con el huevo debajo va hacia el huevo, con el huevo encima va a la izquierda.
- La diferencia entre arcade y port con dx = 0 (el port elige derecha, MAME izquierda) viene de comparar centros en pixeles frente a nodos: unos pixeles de desfase bastan para cambiar el signo.
- La media vuelta del arcade por un lado al azar es lo que hace que no gire siempre hacia el mismo lado.

## 3. Desconocidos
- Valor de DINITDIR tras una muerte por enemigo (DINOCOLL, DINO_TROG_OOF) y DINITPOS en esos casos; si FIND_CLOSE_MPNT puede dejar al dino en un punto auxiliar.
- Contenido de SKIP_ANIMATION, STOPPED, CENTER_CHECK y la duracion de las animaciones de giro (guiones en las tablas de animacion).
- DINO_FIND_CLOSEST y DINO_FIND_FURTHEST (se usan como en el port, de las notas de Carlos de sesiones previas; no releidos hoy).

## 4. Port
- ai.inc: choose_dir, find_closest, best_direction, dir_valid y best_by_rating reproducen DINO_SKIPANIM/DINO_START_SAFE, DINO_FIND_CLOSEST, GET_BEST_DIRECTION/GET_DIRECTIONS, CHECK_DIRECTION (simplificada) y DINO_FIND_BEST/DINO_SEARCH_SAFE (solo huevos).
- Lo que falta: casa (+5 y salida valida), huesos, alquitran, fuego, pozos, pina, ventana de giro asimetrica, fotogramas de giro.
