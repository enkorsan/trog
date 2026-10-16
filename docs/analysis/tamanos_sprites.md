# Estudio de tamanos de sprites (arcade frente al renderer del port)

Version: 16-oct-2026, 19:3x, revision 1. Estado: ANALISIS POR LECTURA. El asistente no ha ejecutado nada; las cifras de la tabla 1 son una lectura manual de los ficheros .TBL (rangos aproximados, no un calculo por programa). Las estimaciones de CPU y memoria estan marcadas como tales.
Fuentes leidas (rama main de enkorsan/trog, raiz): TROGDDAT.TBL (dino), TROGTDAT.TBL (Trog), TROGTREX.TBL (T-Rex), TROGSPRG.TBL (Trog con muelles y rebote), TROGENEM.TBL (llamas, brea), IMGTBL.ASM e IMGSRC.ASM. Las imagenes (IMG/*.IMG) son binarias y no se han abierto.
Ver tambien: docs/analysis/trex_casa_animaciones.md (T-Rex, giro, puñetazo, casa).

## 1. Observaciones directas
Formato de cada entrada (IMGTBL.ASM, TROG*.TBL): `.word ancho, alto, ancho_ancla, alto_ancla` y `.long direccion`, a veces `.long paleta`. Medidas en pixeles del arcade.

Rangos por grupo de nombres (ancho x alto; la funcion de cada grupo NO se deduce del nombre):
| Fichero / grupo | Ancho | Alto | Nota |
|---|---:|---:|---|
| TROGDDAT: SPWAP0 y SPWAP8 | 17-19 | 27-30 | |
| TROGDDAT: SPWAP4 | 23-29 | 27-30 | |
| TROGDDAT: SPSTP (parado) | 16-29 | 27-30 | 9 orientaciones SPSTP0..8 |
| TROGDDAT: SPPUNP0 / P4 / P8 | 19-24 / 25-27 / 20-22 | 20-22 / 26-29 / 28-31 | |
| TROGDDAT: SPFIST (puñetazo) | 19-36 | 21-31 | los usan los guiones SPIKEPUNCH* |
| TROGDDAT: SPKOP0 / P4 / P8 | 18-23 / 30-40 / 16-21 | 15-26 / 17-26 / 17-25 | |
| TROGDDAT: SPTRP, SPBPP, SPYLP, SPOOFP, SPSKP, SPFLP | 3-41 | 2-34 | muchos cuadros |
| TROGDDAT: SPSQ | 23-45 | 14-34 | |
| TROGDDAT: SPSKELP | 26-39 | 32-43 | el mas alto del dino |
| Huevo | 10 | 13 | |
| TROGTDAT: TRSTP, TRWAP, TRLK, TRPUN | 19-39 | 28-35 | Trog base |
| TROGTDAT: TRTR, TREAT, TRSK, TRPSQ | 20-40 | 26-37 | |
| TROGTDAT: TRDIG, TRCL, TRCH, TRY | 20-47 | 18-41 | el mas ancho: TRYLP0F3 47x36 |
| TROGTDAT: TRFL y TRWEUP / TRUP | 3-36 | 3-38 | |
| TROGSPRG: TRBO | 20-42 | 30-43 | |
| TROGTREX: RXSTP | 25-38 | 39-48 | |
| TROGTREX: RXWAP, RXWAGRP, RXCHEWP | 23-43 | 33-50 | |
| TROGTREX: RXMUNCP, RXSNP, RXPOUNP, RXMETAP | 19-55 | 22-50 | |
| TROGTREX: RXSTOMP | 24-59 | 23-56 | maximo: 59 x 56 |
| IMGTBL: BTRHED, BTRBEARD (presentacion) | 164-189 | 126-177 | fuera de la partida |

Cuadros: TROGDDAT.TBL (22,6 KB) y TROGTDAT.TBL (20,9 KB) tienen del orden de 400-500 cuadros cada uno (estimado por tamano de fichero).

## 2. Lo que asume hoy el renderer (render_core.inc y config.inc)
- Ventana de 3 bytes por linea: ent_prep1 y ent_blit con tres LDI; ent_prep2 solo tiene los bucles .d2 y .d3; draw_clipped trabaja con anchos 1 a 3 (`ld a,3 / sub c`); ent_hits_other y ent_q_intersects usan `add a,2 / cp 5` (cinco columnas) y `add a,23`; ent_prep1 limita la primera columna a 29 (`cp 30 / ld a,29`).
- Alto fijo SPR_H = 24 (config.inc) en ent_prep1 y ent_prep2; draw_clipped usa `add a,24`.
- 4 fases por imagen (desplazamiento 0, 2, 4, 6 px); ancho 2 bytes con fase 0 y 3 con las demas; 144 bytes por imagen y fase (24 lineas x 3 bytes x mascara y grafico), 576 bytes por imagen. El atajo de fase 0 supone dibujo de 16 px como maximo; con hasta 18 px cabrian 3 bytes en cualquier fase.
- Buffer de ventana de 128 bytes alineado a 128 (ent_prep2 y ent_blit incrementan solo E).
- Coste (R6.2, ventana de 25 lineas y ancho 3): unos 11.000 T por movimiento (calculo estatico del asistente validado con una fila).

## 3. Proyeccion a escala 0,75 (interpretacion)
La rejilla del arcade es de 32 px y la del port de 24 px (0,75; decisions_render_update_2.md). La escala del dibujo la decide Carlos; esto es solo una referencia. Las etiquetas funcionales son hipotesis a partir de los prefijos.
| Grupo | Ancho | Alto | Cabe en 3 bytes x 24 lineas |
|---|---:|---:|---|
| Dino andar P0 y P8 | 13-14 | 20-23 | si |
| Dino andar P4 | 17-22 | 20-23 | solo hasta 18 px |
| Dino puñetazo (SPFIST): arriba y abajo | 14-19 | 16-18 | si, justo |
| Dino puñetazo (SPFIST) lateral | 19-27 | 20-21 | no (2 a 9 px de mas; el cuadro SPFIST41 da unos 20) |
| Dino KO P4 | 23-30 | 13-20 | no |
| Dino SPSQ / SPSKELP | hasta 34 / 29 | hasta 26 / 32 | no |
| Trog base | 14-29 | 21-26 | parcial |
| Trog casos anchos (TRY, TRBO, TRDIG) | hasta 35 | hasta 32 | no |

## 4. Decisiones de Carlos (16-oct, 18:07 y 19:2x)
1. El Trog mide de ancho lo mismo que el dino (16 px). Algunos Trogs con garrote pueden ser algo mas anchos; si es poco, se ajusta la animacion. Los Trogs que salen del suelo y lanzan ruedas son entidades aparte.
2. Clase A (3 bytes x 24 lineas) para el hito 1, teniendo siempre presente el coste de CPU.
3. Todo lo demas que no sean las entidades jugables se trata como decorado. Posibilidad: tratar como decorado las animaciones de aparicion de Trogs, pozas y fosos mientras no tengan colision.
4. El T-Rex al 60 % (unos 32 px de alto); las animaciones se revisaran.
5. Puñetazo: 1 frame por direccion. Giro: 1 frame diagonal intermedio por giro. A estudiar si los frames de giro pre-desplazados se generan al iniciar el nivel.

## 5. Se podria pasar a 4 bytes x 24 lineas? (pregunta de Carlos)
Si. No es un cambio de datos, es un cambio del renderer, y se puede dejar como opcion de ensamblado (ancho de ventana WIN_W = 3 o 4 como EQU) sin tocar la logica del juego.
Que hay que tocar (observacion en render_core.inc): los tres LDI de ent_prep1 y de ent_blit y el salto de linea de ent_prep1 (`add a,29`); el bucle .d3 de ent_prep2 (hace falta un .d4); draw_clipped (`ld a,3 / sub c`, los multiplos por 3 de dc_wr y dc_dr, el bucle .l3); el limite de la primera columna (29 pasaria a 28); las cotas de solape de ent_hits_other y ent_q_intersects (`add a,2 / cp 5` pasaria a seis columnas); el calculo del puntero en ent_set_qpos (144 pasaria a 192 bytes por imagen y fase).
Memoria (calculo): 192 bytes por imagen y fase, 768 por imagen (+33 % frente a 576). Un dibujo de hasta 26 px cabe en 4 bytes en cualquier fase; hasta 24 px con la fase 0 en 3 bytes.
Buffer: 4 x 26 = 104 bytes con 24 lineas, cabe en los 128 actuales (hasta 32 lineas de ventana).
CPU (ESTIMACION, sin medir): el coste por linea crece en la copia, la composicion y el volcado; el movimiento de 11.000 T pasaria a unos 14.000 T (+25 a 30 %). Con 8 entidades, unos 3.000 T mas por movimiento, es decir, mucho si se acerca al limite medido.
Variante mixta (dino en 3 bytes y Trogs en 4, o al reves): exige ancho por entidad y bucles variables en todas las rutinas; es la opcion B de la seccion 7.

## 6. T-Rex al 60 % (resumen; detalle en trex_casa_animaciones.md)
Proyeccion al 60 % de los grupos del T-Rex: RXSTP 15-23 x 23-29; RXWAP 14-26 x 22-28; RXCHEWP 16-26 x 20-29; RXMETAP 11-18 x 15-28; RXSNP 14-32 x 20-30; RXMUNCP 16-33 x 13-28; RXSTOMP 14-35 x 14-34 (solo dos jugadores). Con el T-Rex como entidad 0 (sustituye al dino) hace falta una ventana de 4 bytes x 32 lineas para esa entidad (dibujo de hasta 26 px y 30 lineas; con 32 lineas de dibujo el buffer de 128 bytes se queda corto y haria falta uno de 256). Los cuadros laterales de comer (RXMUNCP4F4-F7, hasta 33 px) no caben en 4 bytes: hay que recortarlos o tratarlos aparte.
Memoria (calculo): 4 bytes x 32 lineas x 2 = 256 bytes por fase, 1.024 por imagen con 4 fases; 15 imagenes ocuparian unos 15 KB. Es una cifra solo valida para 128K.

## 7. Opciones de arquitectura de sprites
- A (acordada para el hito 1): 3 bytes x 24 lineas para dino y Trogs.
- B (hito 2, para el T-Rex): ancho y alto por entidad con una tabla de clases; la entidad 0 cambia de clase al transformarse. Cambia ent_prep1, ent_prep2, ent_blit, draw_clipped y las cotas de solape; el T-Rex costaria unas 1,5 a 1,8 veces el movimiento normal (estimacion).
- C (2 fases por imagen): no recomendada sin hablarlo, cambia el paso de 2 px.

## 8. Preshift: cuando y como (a estudiar)
- Los puñetazos y los demas cuadros que solo se dibujan con la entidad en una calle vertical tienen x multiplo de 8 (NODE_X0 = 24, NODE_STEP = 24) y solo necesitan la fase 0. Los laterales necesitan las 4 fases. Para un personaje: arriba y abajo 2 x 144 = 288 bytes, izquierda y derecha 2 x 576 = 1.152 bytes, en total 1.440 bytes (3 bytes de ancho).
- Los giros de 90 grados ocurren en nodos (turn_node alinea la entidad): x multiplo de 8, una fase. La media vuelta puede ocurrir en cualquier punto de la calle y sus imagenes necesitan las 4 fases.
- Una imagen solo de calle vertical con 1 fase en lugar de 4 ahorra el 75 % de su memoria; exige un puntero por imagen en lugar de la formula imagen x 4 + fase.
- Tres formas de obtener las fases (a decidir): (a) generarlas en la herramienta, como ahora (memoria completa en el fichero); (b) guardar solo la fase 0 y desplazar al iniciar el nivel (misma RAM; menos tamano de carga; cero coste por fotograma; coste de carga estimado de 3.000 a 7.000 T por imagen, es decir, unos 0,1 a 0,2 s para unas 100 imagenes, sin medir); (c) desplazar en el momento del giro con un buffer temporal (RAM minima; un pico de unos miles de T en el fotograma del giro, que puede provocar un fotograma tardio).
- Los frames izquierdos se pueden generar tambien por espejo al iniciar el nivel (ahorra el tamano de carga, no la RAM).

## 9. Abiertas
1. Tamanos definitivos del Trog con garrote y de los cuadros laterales de puñetazo (unos 20 px frente a los 16-18 del dibujo).
2. Si la media vuelta usa la diagonal y como se resuelve su fase (seccion 8).
3. Cuando y cuantas veces sale el T-Rex a la vez: se supone uno por jugador (DTREXFLG por dino); sin comprobar.
4. Medir el coste real de una ventana de 4 bytes (seccion 5) antes de decidir nada.
5. Imagenes reales: solo se han leido tablas.
