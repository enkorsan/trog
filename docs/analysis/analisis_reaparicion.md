# Reaparicion, retencion de tecla y velocidad

Estado: lectura del fuente (TROGDINO.ASM.p01 y p02, TROGPOST.ASM.p01 y p02). Version: 8-oct-2026.
Ubicacion prevista en el proyecto: docs/analysis/

## Observaciones directas del codigo

Retencion de la direccion (TROGPOST.ASM.p01, PJOY):
1. PJOY vigila el interruptor en un bucle (PJOY_WATCH_LP). Al soltarlo (PJOYX) duerme 2 ticks ("BOUNCE PROTECT") antes de borrar el bit
   de PJOYSTCK. Por tanto la direccion se conserva unos 2 ticks tras soltar la tecla. Con la pantalla a 54,71 Hz son unos 37 ms.

Reaparicion tras caer (TROGDINO.ASM.p02, DINO_DEATH_DONE y DINODEATH_OLD_RET; TROGPOST.ASM.p02, CALCEXITADD):
2. Al caer, DINITPOS se carga con la posicion del punto de movimiento EMOVERET de la salida por la que cayo. EMOVERET es el "return center point"
   que se guarda al crear cada punto de salida. El dino reaparece, por tanto, en el centro (interseccion) de la que salio.
3. DINODEATH_XFER limpia el golpe, el aturdimiento, la invencibilidad (DINVIN) y el aumento de velocidad (DSPEED2) y pone PTEMP1 a 0
   (comentario del fuente: "FLAG SAFE SEARCH").

Orientacion al reaparecer (TROGDINO.ASM.p01, DINO_SKIPANIM):
4. Si PTEMP1 es 0 (no es el primer inicio de la fase), llama a DINO_FIND_CLOSEST (huevo o casa mas cercanos) y despues a GET_BEST_DIRECTION
   para elegir la direccion hacia ellos. Si no hay ninguno o no hay camino, va a DINO_START_SAFE, que llama a DINO_FIND_BEST ("find us the best
   move from here"). La direccion elegida se pasa a DINOTURN.
5. El fuente conserva, comentado con ****, un bloque que usaba FIND_SAFEST para detectar si la mejor direccion llevaba a la muerte y poner al
   dino a rascarse la cabeza. Esta desactivado.

Esperas e invencibilidad (TROGDINO.ASM.p01):
6. DINOPLAY lanza START_INVINCIBILITY con 500. DINO_START_MOVING espera 5 ticks, gira (DINOTURN) y espera 35. DINO_END_INIT espera 20,
   llama a KILL_INVINCIBILITY y en oleadas normales lanza START_INVINCIBILITY con 200 antes de pasar a DINOWXFER (andar).
   Son unos 60 ticks quieto con el anillo de 500 y 200 ticks mas andando con el de 200. Se aplica tambien al primer inicio de la fase.
7. DINOWXFER consulta DINO_STICK antes de andar: si el jugador pide una direccion valida, sustituye a la elegida.
8. DINO_NEAR_EXIT: un dino con DINVIN distinto de 0 no sigue hacia la salida; carga el fotograma de pie (STANDU) y vuelve a DINOWLP.

## Interpretaciones pendientes de comprobar

- Lo que Carlos ve en MAME ("se reorienta para no caerse") se explica por dos efectos: la direccion hacia el huevo o la casa mas cercanos y la
  invencibilidad de unos 260 ticks, durante la cual el dino no puede caer.
- Un tick equivale a un fotograma de pantalla de 54,71 Hz (MAME). Por eso 5*60 ticks son unos 5,5 s y 12*60 ticks unos 13,2 s, no 5 y 12 s.
  Pendiente de confirmar que SLEEP cuenta fotogramas.
- La retencion de 2 ticks es solo la parte de teclado. La tolerancia de posicion del arcade depende de FIND_CLOSE_MPNT y PNT_IN_RANGE (no leidos).

## Desconocido

- Codigo de DINO_FIND_CLOSEST, GET_BEST_DIRECTION, DINO_FIND_BEST, FIND_SAFEST, DINO_VEL y DINO_VEL_NORMAL (las busquedas no los devuelven).
- Valor de DINOSPEED en la oleada 1 y su unidad.
- Duracion de DINOTURN.

## Conversion de tiempos al Spectrum (50 Hz)

Factor 50 / 54,71 = 0,914. START_WAIT = 60 ticks = 55 fotogramas. INV_FRAMES = 260 ticks = 238. INV_BLINK = 80 ticks = 73. KEY_HOLD = 2 ticks = 2.
