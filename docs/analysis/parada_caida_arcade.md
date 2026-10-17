# Parada ante el borde y caida: lectura del fuente arcade

17-oct-2026, revision 0. Lectura por el asistente (TROGDINO.ASM, TROGPOST.ASM, TROGMAPS.ASM); nada ejecutado en el arcade. Separa observaciones, interpretaciones y desconocidos.

## Observaciones directas
- TROGDINO.ASM:DINO_NEXT_MOVE registra un punto de movimiento cuando el dino esta dentro de [4,4] (PNT_IN_RANGE). DINO_SAFETY_LOCAL: si el punto actual tiene salida (MOVEEPTR) y EMOVEDIR coincide con la direccion del dino, pasa a DINO_NEAR_EXIT.
- DINO_NEAR_EXIT: con DSMART distinto de 0, o sin jugador, salta a DINO_SAVE_FROM_DEATH; si no, DINVIN = 0 vuelve a DINOWLP (sigue andando) y DINVIN distinto de 0 continua en DINO_SAVE_FROM_DEATH (animacion de quieto y DINOWLP). La linea que activaba el salvado por BOZO esta comentada (****).
- DINOWLP_MOVE: parado (STOPPED) y DWAIT supera DWAIT_TIME (120 ticks): FINDAMOVE sobre TCURMPOS; si es auxiliar, NOTDIR + DINOTURN; si no, reinicia la espera.
- DINOWLP_CHECK llama a BOUNDRY_CHECK (no lee DINVIN); NZ -> DINO_OUT_OF_BOUNDS: ISHOME -> CELEBRATE (con jugadores) o DINODTH5 (caida).
- TROGPOST.ASM:BOUNDRY_CHECK recorre BOUNDRIES; para una caja de tipo acantilado (0) llama a QBOXOUT y para pozo, lago y volcan a QBOXIN. QBOXOUT: toma OXPOS/OYPOS menos WORLDTL como esquina superior izquierda y suma OSIZE para la inferior derecha; el objeto esta FUERA solo si esta completamente fuera: esquina inferior derecha por encima o a la izquierda de la caja (Y < ULY o X < ULX), o esquina superior izquierda por debajo (Y > LRY) o a la derecha (X >= LRX).
- TROGMAPS.ASM:BOUNDRY1 (fase 1) = [70,53],[215,336] en [Y,X], tipo CLIFF. POSTMAP1: piedras en Y = 72..232 y X = 50..338, paso 32 (formato [Y,X]).
- TROGPOST.ASM:MOVEPNT crea el punto auxiliar en el MIDPOINT entre dos piedras vecinas. CALCA1GT/CALCA1LT calculan la posicion de la salida (EMOVEPOS) a partir del auxiliar con dos HALFXY (cuarto de la diferencia piedra-auxiliar) y un RL 16 intermedio.

## Interpretaciones (no comprobadas ejecutando el arcade)
- La salida (EMOVEPOS) queda unos 4 px fuera del auxiliar (cuarto de 16 px); el RL 16 intermedio de CALCA1GT/CALCA1LT no se ha analizado.
- Los auxiliares de los bordes caen en X = 50 y 338 e Y = 72 y 232 (a 16 px de la ultima interseccion). La parada ocurre cuando el centro de movimiento del dino entra en [4,4] del auxiliar, es decir a unos 4 px antes.
- La caida depende del tamano del sprite (OSIZE del fotograma), no de un punto de pies; el punto de pies (x+8, y+13) del port es una calibracion visual de Carlos.
- Con escala 24/32 = 0,75, centro de la primera interseccion arcade [88,66] = centro de la calle del port (y=60, x=32) y el sprite provisional 16x24 con origen 8 lineas por encima: paradas derivadas Y = 43 (arriba) y 157 (abajo), muy cerca de las de Carlos (44 y 156); laterales 15 y 225 (Carlos: 12 y 228). Caidas derivadas: izquierda 6, derecha 236, arriba 30, abajo 164. Estimacion visual de Carlos: 8, 232, 26, 160.

## Desconocido
Tamano real de los fotogramas del dino en el arcade (OSIZE), valor de WORLDTL en fase 1, si los puntos de las piedras y la caja BOUNDRY1 estan en el mismo sistema de coordenadas, y el desplazamiento del ancla OXPOS/OYPOS respecto al centro de movimiento. Hasta tener el sprite definitivo, FALL_* queda como esta (decision de Carlos, 17-oct).
