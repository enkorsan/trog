# Mediciones R6.2 y R6.3 del renderer

Version: 16-oct-2026. Fuentes: tablas informadas por Carlos (15-oct), captura de pantalla del R6.3 (16-oct), listado y simbolos del R6.3 (bench_r6_integracion-2.lst y -3.sym) y calculos del asistente. Distincion obligatoria: medida de Carlos, calculo estatico del asistente y estimacion no son equivalentes. El asistente no ha ejecutado sjasmplus ni emulador.

## Condiciones
RVM 128K toastrack, sjasmplus 1.24.0, velocidad V7 (7 de 16 por fotograma, pasos de 2 px), HUD congelado en VENT = 64 (BENCH_STOPVENT), unas 4.100 iteraciones medidas (unos 90 s). La misma configuracion repetida da el mismo MG: el banco es reproducible dentro de una compilacion.

## Definiciones del HUD
- MG: media del tiempo libre de las 64 primeras ventanas (de 64 iteraciones medidas). MG x 70 = T-states libres medios por fotograma. Valida con VENT >= 64.
- WN: peor media de ventana (la mas baja). WN x 70 = T-states libres de la ventana mas cargada.
- MINA (MN en el R6.3): menor MIN desde el arranque, en unidades de 35 T. Llega a 0 con una sola iteracion tardia.
- SOLX (SX): mayor numero de ventanas con solape en una ventana estadistica.
- PC: deficit de fotogramas sin HUD, suma de (n-1) por iteracion tardia (n = interrupciones cruzadas).
- LT: iteraciones tardias sin HUD (acabaron tras una interrupcion; la siguiente las absorbe si PC no crece).
- Un fotograma de 128K = unos 70.900 T. Son medidas aproximadas, no perfiles exactos.
- MEDIA (media movil de 16 ventanas) ya no se usa como valor de rendimiento: un instante no es representativo.

## Tabla 1: carga base (R6.2, informada por Carlos)
| N | E | S | P | G | A | F | B | MG | WN | MINA | SOLX | PC | LT |
|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---:|---:|
| 5 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | 613 | 552 | 514 | 90 | 0 | 0 |
| 5 | 1 | 1 | 0 | 0 | 1 | 0 | 2 | 614 | 558 | 0 | 87 | 0 | 1 |
| 5 | 1 | 1 | 0 | 0 | 0 | 1 | 2 | 695 | 577 | 399 | 101 | 0 | 0 |
| 5 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 725 | 592 | 549 | 58 | 0 | 1 |
| 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 510 | 228 | 0 | 153 | 0 | 21 |

Equivalencias (MG x 70): N5 base 42.900 T libres (60 %), N5 con F 48.650 T (69 %), N5 E2 P1 A1 F1 50.750 T (72 %), N8 35.700 T (50 %). WN de N8: 16.000 T libres (22,5 %).

## Tabla 2: carga artificial (N8 E2 S1 P1 A1 F1; BENCH_BURN x 24 T por fotograma)
| BENCH_BURN | T extra | MG | WN | MINA | PC | LT |
|---:|---:|---:|---:|---:|---:|---:|
| 0 | 0 | 510 | 228 | 0 | 0 | 21 |
| 146 | 3.500 | 460 | 176 | 0 | 0 | 59 |
| 292 | 7.000 | 410 | 124 | 0 | 0 | 99 |
| 584 | 14.000 | 310 | 39 | 0 | 2 | 309 |

## Lectura prudente
- MG baja exactamente lo que cuesta la carga (50, 100 y 200 unidades): el instrumento es lineal.
- Con N8 la media libre es del 50 % pero la peor ventana deja el 22,5 %. Hay que mirar WN y LT, no solo MG.
- Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T aparecen 2 fotogramas perdidos en unos 90 s. Valido para esa trayectoria (primeras 64 ventanas con esa semilla).
- LT crece mas que proporcionalmente entre +7.000 T (99) y +14.000 T (309): se acerca al limite.
- Con HALT (juego anterior) una iteracion tardia pierde un paso de logica; los resultados de este banco valen para espera por contador, que la absorbe.
- Confirmar si la tabla 2 se tomo con BENCH_SKIP_SORT = 0.
- No se declara margen cerrado del juego: faltan IA, colisiones, sonido, animaciones, cambios de fondo y contencion real.

## Ablacion de la primera tanda (no valida)
BENCH_SKIP_BLIT solo envolvia el volcado inicial de start, no el del bucle, y la diferencia con BENCH_SKIP_SORT (MEDIA 552 frente a 534) no esta explicada. No se usan para repartir costes.

## Captura del R6.3 (16-oct)
Configuracion en pantalla N8 E2 S1 P1 G0 A1 F1 B2, V7, VENT 0064. Mitad derecha: MG 0424, WN 0203, MN 0000, SX 0121, PC 0000, LT 0065. El .sym de esa compilacion indica BENCH_BURN = 292 y BENCH_SKIP_SORT = 1 (BENCH_SKIP_BLIT = 0).
Lectura condicionada (si captura y .sym corresponden a la misma ejecucion y la tabla 2 se tomo con SKIP_SORT = 0): frente a la fila BURN 292 (MG 410, WN 124, LT 99), sin ordenacion MG sube 14 unidades (unos 1.000 T por fotograma), WN sube 79 (unos 5.500 T en la peor ventana) y LT baja de 99 a 65. Seria compatible con una ordenacion cara en las ventanas mas cargadas.
Cautela: SX es 121 frente a 153 de la tabla 1, cuando las trayectorias identicas darian el mismo valor; no esta explicado, asi que la comparacion entre compilaciones es debil. Repetir con SKIP_SORT = 0 y 1 en la misma sesion y anotar BURN.

## Calculo estatico de T-states (asistente, sobre el fuente, sin contencion)
| Fase de una ventana de 25 lineas | Coste |
|---|---|
| Volcado (ent_blit): 103 T por linea (147 al cambiar de fila de caracteres) mas unos 300 fijos | 3.000 T |
| Copia de fondo (ent_prep1): 80 T por linea mas unos 830 fijos | 2.800 T |
| Composicion (ent_prep2): 145 T por linea con ancho 3, mas ent_hits_other (unos 96 T por entidad) y fijos | 4.400 T (N5), 4.700 T (N8) |
| Logica (ent_update, ent_set_qpos) | 750 T |
| Total por movimiento | 11.000 T |
Mas unos 500 T fijos por entidad y fotograma y unos 600 T del bucle. Para N5 base (2,19 movimientos por fotograma) da unos 27.200 T usados frente a 28.000 T medidos (3 %). Solo esta validado con esa fila.

Candidatos de optimizacion (estimaciones, no aplicados):
| Cambio | Ahorro |
|---|---|
| Bajar la velocidad de V7 a V6 | 14 % de ventanas (unos 4.400 T por fotograma en N8) |
| Ordenar con claves de un byte en vez de IX/IY | unos 1.200 T por fotograma en N8 |
| Volcado por tramos de fila de caracteres | unos 450 T por ventana |
| Desenrollar los bucles de composicion | unos 312 T por ventana (1,1 KB mas de codigo) |
| Recortar el preambulo de ent_prep1 | unos 250 T por ventana |
Costo marginal por entidad andando a V7: unos 5.300 T por fotograma (7,5 %); a V6 unos 4.700 T.

## Mapa de memoria medido (R6.3, BENCH_BURN = 292, BENCH_SKIP_SORT = 1)
| Region | Direcciones | Bytes |
|---|---|---:|
| Codigo y tablas | $8000-$8A58 | 2.649 |
| Variables | $8A59-$8ACD | 117 |
| Entidades (8 x 32) | $8ACE-$8BCD | 256 (reserva hasta $8C00) |
| Buffers de ventana | $8C00-$8FFF | 1.024 |
| video.inc | $9000-$90C9 | 202 |
| sprite_dino | $90CA-$99C9 | 2.304 |
| sprite_trog | $99CA-$A2C9 | 2.304 |
| Piedras | $A2CA-$A335 | 108 |
| Fuente font_msg | $A336-$A50D | 472 |
Fin de datos $A50E; margen hasta IM2_TABLE ($B800): 4.850 bytes. Fondo limpio en banco 0 ($C000, 6.144 bytes). Pila en $7FFF (banco 5, contendido; hipotesis de coste sin medir). El tamano incluye la instrumentacion del banco.

## Pruebas pendientes
- Repetir SKIP_SORT 0/1 en la misma sesion con BURN anotado.
- Probar la pila en la zona no contendida y comparar MG.
- Variante 48K (DEVICE ZXSPECTRUM48) y prueba en RVM modelo 48K.
- Medir el fotograma completo al integrar el renderer en el juego.
