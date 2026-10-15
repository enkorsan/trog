# Golpe, items y colision dino-dino

Estado: analisis del fuente (TROGDINO.ASM.p04_L2222-2951.txt y p01). Version: 8-oct-2026, revision 2.
Ubicacion prevista en el proyecto: docs/analysis/

## Observaciones directas del codigo

1. DINODINO (dino contra otro dino): no hace nada si alguno de los dos esta muerto, ni si el otro es T-rex, esta invencible
   (DINVIN) o aturdido (DDIZZYFLG). Si el que choca es T-rex, pisotea al otro (TREX_STOMP / DINO_STOMPED).
   Si no, DD_PUNCH_CK llama a PUNCHED; si hay golpe, el golpeado se detiene (PSTOP), queda aturdido (DDIZZYFLG = 1),
   no devuelve el golpe (DPUNCHFLG = 0) y pasa a DINOOOF.
2. HITS_TO_KO = 5: numero de golpes para dejar K.O. a un dino (TROGDINO.ASM.p01).
3. No he visto en este fragmento ningun golpe de dino a Trog. La rutina PUNCHED y la lectura del boton no estan leidas.
4. Efectos de los items al tocar al dino (DINO_PLANT_HIT, solo si el dino esta vivo y centrado sobre el item, CK_PIT_CENT):
   - Flor: DINO_SPEEDUP. Suma 4000H a la velocidad durante 12*60 ticks.
   - Seta: DINO_SLOWDOWN. Resta 4000H durante 5*60 ticks.
   - Herradura: CAN_INV. Invencibilidad de (POWERUP_DURATION >> 2) + 50 ticks.
   - Pina: DINOXFORM (T-rex).
   - Hielo: FREEZE_EM, creado al recoger el item.
   - Chile: SPIT_FIRE (proceso PFIREPID).
   Todos dan SCORE con A1 = 775H, liberan el centro (FREE_MOVE_EGG) y el item queda en PLANT_STALL de 5 a 8 s antes de morir.
5. DINOTAR y DINOFIRE: el dino muere salvo que sea T-rex o este invencible; DINOTAR exige estar centrado (CK_OBJ_CENT).

## Interpretaciones pendientes de comprobar

- El golpe solo afecta a otros dinos, asi que con un jugador no tendria efecto. Pendiente de ver en MAME si golpea a Trog.
- El hielo actua al recogerlo, sin boton. El chile probablemente usa el boton para escupir fuego: falta leer SPIT_FIRE.
- 775H seria 775 puntos en BCD. Pendiente de confirmar la unidad de SCORE.
- Las duraciones estan en ticks. Si un tick es un fotograma de 54,71 Hz, 12*60 ticks son unos 13,2 s y 5*60 unos 5,5 s (correccion: antes se
  escribio 12 s y 5 s suponiendo 60 ticks por segundo).
- Carlos indica que el efecto de la flor es acumulativo. DINO_SPEEDUP reinicia el temporizador (KILALLA8) y suma con DINO_ADD_VEL; falta leer
  DINO_ADD_VEL para ver si acumula en DSPEED2.

## Desconocido

- Codigo de PUNCHED, de la lectura del boton, de SPIT_FIRE, de FREEZE_EM y de DINO_ADD_VEL.
- Valor de POWERUP_DURATION.
- Alcance y duracion del golpe y tiempo de aturdimiento (DINOOOF).
