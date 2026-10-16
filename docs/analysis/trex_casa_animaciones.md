# T-Rex, giro y puñetazo del dino, casa y apariciones (lectura del fuente arcade)

Version: 16-oct-2026, 19:3x, revision 0. Estado: ANALISIS POR LECTURA. Separado en observaciones directas (con fichero y etiqueta), interpretaciones y desconocido. El asistente no ha ejecutado nada; ningun numero de CPU o memoria de este documento esta medido.
Ficheros leidos (rama main, raiz): TROGTREX.ASM (hasta TREXSTOMP_U; el resto se cortó en la lectura), TROGTREX.TBL, TROGDINO.ASM (DINOPAPPLE, DINO_PLANT_HIT, DINODINO, DINOTAR, DINOFIRE, DINOPIT, DINO_SPEEDUP), TROGDDAT.ASM (parte 2 a 4: SPIKEUP, SPIKEUL..SPIKELD, SPIKEABOUT_*, SPIKEPUNCH*, SPIKEOOF*, REXDOORMAT_*), TROGHAUS.ASM, TROGTROG.ASM (hasta TROG_WHEEL; el resto se cortó).
Unidad de tiempo: ticks del arcade (54,71 Hz); en el port, x 0,914 (50 Hz).

## 1. T-Rex (observaciones directas)
- Activacion: DINOPAPPLE (TROGDINO.ASM) llama a DINO_PLANT_HIT con A1 = PTREXPID y A7 = DINOXFORM. Antes comprueba CK_PIT_CENT (centrado en la planta), que el dino no este muerto y que tenga proceso. Da SCORE 775H, llama a FREE_MOVE_EGG, borra la planta con DELETE_OBJ_SUPP (lista ENEMIES) y transfiere el proceso de la planta a PLANT_STALL, que duerme RANGRAND entre 5*60 y 8*60 ticks y muere.
- DINOXFORM (TROGTREX.ASM): PSTOP; KILL_INVINCIBILITY; duracion = POWERUP_DURATION + 180 ticks de gracia, y START_INVINCIBILITY con esa duracion; DATABLE = TREXATAB; DTREXFLG = 1; PAC_SCORE = PACSTART (10000H); EAT_COUNT = 0; WOW_COUNT = max(TROG_CNT, MINWOWCNT = 7); DSPEED2 = DSPEED + 400H; animacion de metamorfosis METAU_TAB (3 cuadros de 6, 8 y 10 ticks) y espera POWERUP_DURATION - 112.
- Fin: DINOXBACK espera mientras DPUNCHFLG indica que esta comiendo (salvo en BONUSW_1), lanza TREX_FLASH (alterna el flag de color cada 4 ticks), duerme 60 ticks y DTREXRESET: restaura la tabla de animacion, DTREXFLG = 0, DSPEED2 = 0 y metamorfosis inversa METAD_TAB (3 cuadros de 6, 8 y 6 ticks).
- Comer: DINO_MUNCH (cuando el T-Rex toca un Trog). Animaciones TREXEAT_U/D/L/R: 8 cuadros RXMUNC* (4, 6, 2, 4 y 6, 6, 4, 4, 4 ticks) y 3 RXCHEW* (4, 8, 8 y 6): unos 66 ticks. Pone al Trog detras del T-Rex en Z (OZVAL - 1). Puntuacion: PAC_SCORE se suma a si mismo con BCDADD hasta PACTOP = 80000H; un Trog de muelles (TTYPE_SPRING) puntua el doble (SCORE2XOBJ); si EAT_COUNT llega a WOW_COUNT sale un WOW diferido (DELAYED_WOW). En BONUSW_1 cada Trog vale PACSTART y no hay mascado. Lanza el proceso EARTHSHAKER en el cuadro 6.
- Pisoton: DINODINO (TROGDINO.ASM) con DTREXFLG en el atacante lanza TREX_STOMP sobre otro dino que no sea T-Rex, ni invencible, ni mareado (DDIZZYFLG). Es para dos jugadores o mas.
- Inmunidades: DINOTAR, DINOPIT y DINOTUNNEL no lo afectan si DTREXFLG != 0 (DINOTAR_BAIL, DINOPIT_BAIL); DINOFIRE crea WHLFIRE_EXT y apaga el fuego. DTREX_LEAP salta desde TCURMPOS hasta el punto central pedido (SLINEVEL con 20 de velocidad, ORIM M_NOCOLL, DPUNCHFLG = 1).
- Trogs: en TROGTROG.ASM (TROGWLP), un Trog que persigue a un dino que se vuelve T-Rex cancela la persecucion (CLEAR_PURSUIT) y se va (CLUBOUTU, SLEEP 30, TROG_MACHA_MACHA: da la vuelta).
- Animaciones del T-Rex (TREXATAB): caminar TREX_U/D/L/R con 7 cuadros RXWAP (7, 9, 7, 7, 9, 7, 7 ticks); giros por las 9 orientaciones RXSTP0..8 (4 ticks cada una); estar parado TREXSTAND_*; rugir con andar REXGROWL_*; enfado REXMAD_* (RXSNP o RXPOUN); comer TREXEAT_*; metamorfosis. Todo lo demas (muertes, puñetazos, golpes, pozo, brea, fuego, KO, aplastamiento, lava) es DUMSCRIPT: el T-Rex no tiene esas animaciones.
- Giros del T-Rex: 90 grados = RXSTP4 (o la base) 16 ticks, marcador 0 "HEAD TURN SPLIT", y 4 pasos de 4 ticks por 3 imagenes intermedias (por ejemplo RXSTP3, RXSTP2, RXSTP1). Media vuelta (TREXABOUT_*): 12 ticks, marcador 0 y 8 pasos de 4 ticks por las 9 orientaciones.
- Frames de la tabla (TROGTREX.TBL): RXSTP 9; RXWAP 21 (7 por vista); RXWAGRP 14; RXCHEWP 9; RXMETAP 9; RXMUNCP 24; RXSNP 14; RXPOUNP 6; RXSTOMP 12; en total unos 118.

## 2. Giro y puñetazo del dino (observaciones directas, TROGDDAT.ASM)
- Giro de 90 grados (SPIKEUL, SPIKERU, SPIKELU, SPIKEDL, SPIKELD, SPIKERD...): una fase de "mirar" con los cuadros SPLK* (por ejemplo 5, 5, 10, 5, 5 ticks = 30), el marcador 0 "HEAD TURN SPLIT" y la rotacion del cuerpo por SPSTP (5 imagenes de 3 ticks = 15 ticks): SPSTP4, 5, 6, 7, 8 hacia arriba y SPSTP4, 3, 2, 1, 0 hacia abajo. En total unos 45 ticks, con 3 imagenes intermedias.
- Media vuelta (SPIKEABOUT_UL/UR/DL/DR/LU/LD/RU/RD): 100 a 110 ticks de "mirar" en el caso que se ha sumado (ABOUT_UL = 110) mas una rotacion por las 9 imagenes SPSTP de 3 ticks (24 a 27 ticks): del orden de 130 ticks.
- La asignacion de estos guiones a las pulsaciones del jugador no esta leida (DINOTURN, DINOLOOK_TURN): no se sabe si el dino se detiene durante el giro ni si el jugador los ve en el giro de un nodo o solo cuando mira.
- Puñetazo (SPIKEPUNCHU, SPIKEPUNCHD, SPIKEPUNCHLR): dos variantes por direccion de 3 cuadros SPFIST de 1 tick cada una (por ejemplo SPFIST81, 82, 83 y SPFIST81, 84, 85 arriba; SPFIST01, 02, 03 y 04, 03, 05 abajo; SPFIST41, 42, 43 y 44, 45, 46 lateral), con TROGSWOOSH_SND y un marcador 0 entre cuadros que avisa al golpeado para que reaccione ("A DOUBLE NULL DENOTES END OF SEQUENCE"). Recibir un golpe (SPIKEOOFU/D/LR): 3 o 4 cuadros de 4, 2, 2 y 2 ticks.
- Trog: el puñetazo tiene mas cuadros (TRPUNP0/P4/P8 con 6 cada uno); los guiones de TROGATAB no se han leido.

## 3. Casa y letrero
- REXDOORMAT_U/D/L/R (TROGDDAT.ASM) y BLOOP/SPIKE/GWENDOORMAT_x: imagen HOMEU1/HOMEL1/HOMEC1/HOMEC2 (26 x 16 en el arcade, anclas distintas segun el lado) 60 ticks, y despues bucle de 30 ticks con la imagen del jugador (REXHU1 etc.) y 30 ticks con la imagen neutra. HOUSE_ANIS (TROGHAUS.ASM) indexa los guiones por jugador y direccion.
- La casa (transporte) es otro objeto: HOUSE_INIT con la imagen trancave1 (30 x 50) y colision HOUSE_COLL; HOUSE_PROCESS crea ademas la gema indicadora y un fondo (TRANBACK).
- 30 ticks son unos 27 fotogramas del port. Hay una discrepancia por comprobar con HOUSE_FLASH_FRAMES = 21 del estado del proyecto.

## 4. Aparicion de Trogs (observacion directa, TROGTROG.ASM)
TROG_PROCESS crea el pozo con CREATE TROGSUBPID, TROG_PIT en la posicion inicial, duerme 61 ticks y llama a TROG_EMERGE: carga la animacion INTRODUCE, 4 cuadros de FRANIM (los TRUP*), INSERT_OBJ_SUPP en la lista TROGS, una espera, 1 cuadro mas y SLEEP 10, y entonces DINOTURN. Los flags de colision durante la emergencia no se han verificado (TROGWXFER hace ANDNIM M_NOCOLL mas tarde).

## 5. Interpretaciones
- Las 9 orientaciones SPSTP0..8 son la animacion de giro: el dino del arcade pasa por 3 imagenes intermedias por cuarto de vuelta. Carlos decide reducirlo a 1 diagonal por giro, lo que es una simplificacion visual consciente.
- El puñetazo del arcade dura unos 3 ticks por variante; un cuadro por direccion que se mantenga unos 4 a 6 fotogramas no cambia la sensacion de juego, siempre que el golpe (el marcador 0) se aplique en el fotograma correcto. El lateral (SPFIST4x) mide unos 20 a 27 px a escala 0,75 (tamanos_sprites.md): hay que recortar el dibujo a 18 px o ampliar la ventana.
- El T-Rex al 60 % ocuparia 4 bytes x 30 a 32 lineas (tamanos_sprites.md, seccion 6). Los cuadros de comer laterales (RXMUNCP4F4-F7) superan los 26 px y requieren recorte o una ventana de 5 bytes.
- Casa y letrero: dos imagenes fijas que alternan; encajan como decorado (rectangulo con dos mapas de bits que se vuelcan al cambiar), sin limite de tamano de sprite y sin coste por fotograma. La casa de 30 x 50 (22 x 37 a 0,75) tambien es decorado, con colision tratada por la logica del juego.
- Apariciones de Trogs, pozas y fosos: si no tienen colision durante la animacion pueden ser teselas de decorado con una secuencia de cuadros, sin ocupar una entidad ni entrar en la ordenacion; un Trog que emerge pasaria a entidad solo al terminar. Pendiente de verificar los flags de colision (TINIT_TAB, TROG_PIT).
- Preshift: ver tamanos_sprites.md, seccion 8. Generar las fases al iniciar el nivel no reduce la RAM ni la CPU por fotograma; reduce el tamano de carga y la herramienta; desplazar en el momento del giro reduce la RAM a costa de un pico de CPU.

## 6. Desconocido
- POWERUP_DURATION y quien consume DSPEED2 (la velocidad real del T-Rex); BOUNDRY_CHECK con DTREXFLG (que no se cae por el borde) y el uso de DTREX_LEAP.
- DINOTURN, DINOLOOK_TURN y el vinculo entre pulsacion y guion de giro; duracion del giro percibida por el jugador.
- Resto de TROGTREX.ASM (TREXSTOMP_U/LR) y de TROGTROG.ASM (TROG_WHEEL..), TROGATAB y TROG_CLUB_ATAB.
- Flags de colision de la emergencia de Trogs, pozas y fosos.
- Numero maximo de T-Rex a la vez (un DTREXFLG por dino).

## 7. Abiertas para Carlos
1. Aprobar casa, letrero y apariciones sin colision como decorado.
2. Tamano del T-Rex (60 %): confirmar 26 px de ancho y 30 a 32 lineas como maximos de dibujo y que los cuadros de comer laterales se recorten.
3. Si la media vuelta del dino usa la diagonal y se resuelve la fase (tamanos_sprites.md, seccion 8).
4. Cuando se leyo DINOTURN, ajustar la duracion de giro y puñetazo del port.
