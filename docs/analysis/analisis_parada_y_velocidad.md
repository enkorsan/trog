# Parada del dino invencible, orientacion al reaparecer y velocidad

Estado: lectura del fuente (TROGDINO.ASM.p01 y rutinas pegadas por Carlos). Version: 8-oct-2026.
Ubicacion prevista en el proyecto: docs/analysis/

## Observaciones directas del codigo

Parada en el borde y media vuelta (TROGDINO.ASM.p01):
1. DINO_SAFETY_LOCAL: si el punto de movimiento actual tiene un puntero de salida (MOVEEPTR) y la direccion de la salida (EMOVEDIR) coincide con la
   direccion en que anda el dino, salta a DINO_NEAR_EXIT. DINO_NOT_NEAR_EXIT tambien llega ahi si no hay otro punto de movimiento en esa direccion.
2. DINO_NEAR_EXIT: si DINVIN es 0 (y es jugador y no es "smart") sigue andando (DINOWLP) y caera. Si DINVIN es distinto de 0 va a
   DINO_SAVE_FROM_DEATH, que carga el fotograma de pie (STANDU + direccion), lo anima y vuelve a DINOWLP.
3. DINOWLP_MOVE: si STOPPED indica que esta parado, incrementa DWAIT en cada pasada. Mientras DWAIT <= DWAIT_TIME (120) sigue esperando
   (DINOWLP_STOPPED: guarda DWAIT, SLEEP 1, DINOWLP_CHECK). Cuando DWAIT supera 120 comprueba con FINDAMOVE si esta sobre un punto de movimiento
   AUXILIAR: si lo esta, hace NOTDIR + DINOTURN (media vuelta), DINO_WALK_LOAD y anda en sentido contrario (DINOWLP_WALK). Si no, reinicia DWAIT
   (DINOWLP_NOREV) y sigue parado.
4. Mientras esta parado, DINOWLP_CHECK sigue ejecutandose cada tick: DINO_STICK acepta una media vuelta pedida por el jugador en cualquier momento.
5. Los puntos de salida cuelgan de los puntos medios entre postes (puntos auxiliares), no de los centros. La parada ocurre cuando el dino registra ese
   punto medio (DINO_NEXT_MOVE, a menos de [4,4] de el): media casilla antes de la salida.

Orientacion al reaparecer (rutinas pegadas por Carlos):
6. DINO_FIND_CLOSEST: primero una casa global (HOUSELST); si no, la casa de salida del jugador si ya existe; si no, el huevo mas cercano de su especie
   o compartido (EGGOID|PXPLAYER). Distancia: |dy| + |dx| desde TCURMPOS hasta el punto de animacion del huevo.
7. GET_BEST_DIRECTION: obtiene una primera y una segunda direccion (GET_DIRECTIONS, no pegada). Descarta la que sea la contraria de la direccion actual
   (A3 = NOTDIR(A11)) y acepta la primera que pase CHECK_DIRECTION (no pegada). Si ninguna vale, devuelve Z.
8. Al reaparecer, DDIR y A11 valen DINITDIR, que es la contraria de la direccion de la salida (DINODTH5). Por tanto la contraria de la actual es la
   direccion hacia el borde por el que cayo, y GET_BEST_DIRECTION nunca la elige. Si GET_BEST_DIRECTION falla, DINO_FIND_BEST prueba las cuatro
   direcciones con DINO_SEARCH_SAFE y elige la de mejor puntuacion (WALL_RATING 1, EGG_RATING 2, PINEAPPLE_RATING 3, HOUSE_RATING 5) y mas pasos.
9. DINOTURN: elige la tabla por la direccion actual (TURNTAB). 0FEH = misma direccion, 0FFH = media vuelta (animacion elegida al azar entre dos),
   otro valor = animacion de giro. Salta casi toda la animacion (SKIP_ANIMATION) y fija DDIR.

Velocidad (rutinas pegadas por Carlos):
10. DINO_VEL: velocidad en formato 16.16 (pixeles por tick), maxima 18000H (1,5 px por tick). La velocidad base sale de la entrada RIGHT de la tabla
    de animacion del dino y DSMULT = (base << 8) / velocidad ajusta la velocidad de la animacion.
11. DSPEED esta empaquetado: bits 0-11 fraccion y 12-15 enteros. La velocidad inicial es DSPEED << 4; por ejemplo 0800H equivale a 0,5 px por tick.
12. DINO_ADD_VEL suma o resta una cantidad a DVEL. El valor absoluto no puede bajar de DSPEED/2 + 1000H ni subir de DSPEED*2 (en esas unidades).
13. DINO_VEL_NORMAL (llamada en cada DINOWLP_CHECK) acerca DVEL a DSPEED (o a DSPEED2 si no es 0) en pasos de 100H, salvo que DVNFLAG sea distinto
    de 0. Mientras actua una flor o una seta, DVNFLAG vale 1 y la velocidad no se normaliza.
14. Con DINO_SPEEDUP (flor): se matan los procesos de velocidad anteriores del dino (KILALLA8), DVNFLAG = 1, DINO_ADD_VEL con +4000H y espera 12*60
    ticks. Cada flor suma otros 4000H hasta el tope de 2x DSPEED y reinicia el temporizador: el efecto es acumulativo, como dice Carlos. La seta resta 4000H
    hasta el minimo.

## Interpretaciones pendientes de comprobar

- La parada ocurre cerca del punto medio del borde. En nuestro mapa seria el centro del tile de borde: sprite en y = 16 (arriba), y = 176 (abajo),
  x = 8 (izquierda) y x = 232 (derecha). Pendiente de comprobar en MAME donde se queda un dino invencible.
- 121 ticks de espera = 110,6 fotogramas de 50 Hz (factor 0,914), unos 2,2 s.
- Con DSPEED = 0800H el dino andaria a 0,5 px por tick, unos 27 px/s a 54,71 Hz. No se sabe el valor de DINOSPEED en la oleada 1.

## Desconocido

- GET_DIRECTIONS, CHECK_DIRECTION, DINO_SEARCH_SAFE, STOPPED y FINDAMOVE (no pegadas).
- Tabla de dificultad por oleada (WAVE_DIFF_TAB): DINOSPEED y su unidad real.
