# Planificacion de memoria (borrador, revision 0)

Version: 10-oct-2026. Estado: BORRADOR para discutir con Carlos. Las cifras marcadas como estimacion son hipotesis de trabajo, no mediciones del Z80.
Reproducible: python tools/frame_report.py original/TROGDDAT.TBL original/TROGTDAT.TBL original/TROGTREX.TBL original/TROGWHL.TBL original/TROGSPRG.TBL original/TROGENEM.TBL

## 1. Que contiene el arcade (recuento de las tablas de imagenes)

Recuento manual de las entradas de cada .TBL (el script anterior lo calcula con exactitud; margen de error de unas pocas entradas). Tamanos: primeras dos palabras de cada entrada (ancho, alto; suposicion a comprobar en IMGTBL.ASM).

| Conjunto | Fuente | Fotogramas | Tamano medio | Observaciones |
|---|---|---:|---|---|
| Dino | TROGDDAT.TBL | ~426 | 22 x 26 | Los cuatro dinos (Rex, Bloop, Spike, Gwen) comparten las mismas imagenes y solo cambia la paleta (por ejemplo ELSTP0, BLSTP0 y RESTP0 apuntan a la misma direccion que SPSTP0) |
| Trog (mano y palo) | TROGTDAT.TBL | ~422 | 28 x 32 | ~233 con las manos y ~189 con el palo |
| T-Rex del jugador | TROGTREX.TBL | ~118 | 33 x 41 | Imagenes de hasta 59 px de ancho |
| Rueda | TROGWHL.TBL | ~80 | 22 x 24 | Incluye caida y subida |
| Trog con muelles | TROGSPRG.TBL | ~52 | grande | Algunas piezas de hasta 133 x 76 |
| Llamas, alquitran, foso | TROGENEM.TBL | ~50 | 26 x 26 | |
| Total | | ~1.170 | | |

Coste a tamano arcade en 1 bit por pixel con mascara (estimacion con las medias de arriba): unos 257 KB sin desplazar y unos 1.110 KB con 4 fases de desplazamiento. No cabe en un Spectrum 128K ni comprimido: hay que decidir un catalogo.

Detalle del dino (familias por prefijo; P0 = abajo, P4 = lado, P8 = arriba): caminar 23, quieto y giro (SPSTP) 10, mirar (SPLK) 26, cola (SPTL) 21, animos varios (animar, sonreir, enfadarse, preguntar, saludar) 27, SPBP 53, golpeado (OOF) 10, puñetazo 16, K.O. 19, caida al agua (TRP) 27, caida al foso (FLP) 59, derrape 20, grito 15, esqueleto 15, cadaver 18, aplastado por la rueda 33, caminar con fuego 18 y puño 16.
El arcade guarda el lado una sola vez y lo voltea con FLIPBITS/M_FLIPH (SPIKESTAND_L y SPIKESTAND_R usan SPSTP4); SPIKEPIT_LEFT y SPIKEPIT_RIGHT comparten el mismo guion.

## 2. Principios propuestos

1. Catalogo por niveles de uso, no por completitud (ver seccion 3).
2. Fidelidad de tiempo, no de numero de fotogramas: una animacion de 59 fotogramas del arcade se puede reproducir con 12 en la misma duracion total.
3. Preoperar solo lo frecuente: los 4 desplazamientos horizontales (0, 2, 4, 6 px) solo para andar, quieto, giro y objetos moviles. Cuesta 528 bytes por pose de 16x24 con 4 fases (576 con el formato actual de 3 bytes fijos) frente a 96 bytes sin desplazar.
4. Las animaciones de evento (caer, foso, aplastado, comido, golpeado) se pueden hacer en posicion alineada a byte: se alinea el dino al byte mas cercano al empezar (como DINO_CENTER en el arcade) y solo hace falta la fase 0. Los nodos ya estan alineados (X mod 8 = 0).
5. Guardar todo lo que no sea del bucle caliente comprimido con ZX0 y expandirlo a un area de trabajo al empezar la fase o el evento. Los lados izquierdo y derecho se guardan una vez y el espejo se genera al expandir (tabla de 256 bytes para invertir bits).
6. Conjuntos intercambiables por fase: un tipo de Trog (manos, palo, rueda, muelles) y el T-Rex se expanden solo cuando hacen falta.
7. Los fondos y pantallas ya van comprimidos y se vuelcan a un buffer (acordado en layout_c.md).

## 3. Catalogo por niveles (propuesta)

| Nivel | Contenido | Formato | Estimacion |
|---|---|---|---:|
| A. Residente caliente | Dino: andar 4 fotogramas por direccion (16 poses), quieto 4, giro 1 fotograma por pareja de direcciones (4 poses) = 24 poses. Trog: andar, quieto/mirar y atacar (~32 poses). Huevos, items, fuego, casa (~24 pequenos) | 4 fases, sin comprimir | ~13 + 17 + 5 = ~35 KB |
| B. Eventos | Caida al agua (~9), foso (~12), aplastado (~9), comido (~9), golpeado y K.O. (~8), esqueleto y cadaver (~6) para el dino; los de Trog (~60) | Fase 0, ZX0, area de trabajo | ~6 KB (dino) + ~6 KB (Trog) comprimido |
| C. Conjuntos por fase | Trog con palo, rueda, muelles; T-Rex | ZX0, se expanden al empezar la fase | Trog: ~9 KB comprimido por variante; T-Rex: 13 KB (24 poses de 16x24) a 31 KB (32 poses de 24x32) expandido |
| D. Raros | Animos del dino (celebrar, sonreir...), cola, derrape, pantallas de bonus | Decidir mas adelante | |

## 4. Presupuesto aproximado (estimacion)

Memoria utilizable en 128K: 16 KB del banco 2 + 9,4 KB libres del banco 5 + 6 bancos de 16 KB (0, 1, 3, 4, 6, 7) = unos 121 KB, descontando la pantalla (6,9 KB).

| Partida | KB | Notas |
|---|---:|---|
| Codigo, IA, fases, menus | 24-32 | Suposicion: no hay medida todavia (el motor actual ocupa menos de 8 KB) |
| Nivel A residente | ~35 | Seccion 3 |
| Nivel B comprimido + area de trabajo | ~12 + 6 | |
| Nivel C (un Trog alternativo + T-Rex comprimidos) | 22-40 | Depende del tamano del T-Rex y de cuantas variantes de Trog |
| Fondos y tiles | 8-12 | 8 tipos de terreno comprimidos |
| Pantallas (titulo, menus, final) | 6-12 | ZX0 |
| Sonido AY (musica y efectos) | 12-20 | Hito 3 |
| Buffers: fondo base (banco 0) + pantalla sombra (banco 7) | ~14 | layout_c.md |
| Total | ~140-175 | Supera los ~121 KB |

Lectura por hitos: hito 1 (un nivel) cabe sobradamente (~75 KB); hito 2 (varias fases, IA completa, items) ~100 KB; hito 3 (sonido, menus, puntuacion) roza el limite; el T-Rex, las cuatro variantes de Trog y la mano del hito 4 solo caben si se recortan.

## 5. Mapa de bancos (propuesta)

| Banco | Direccion | Uso |
|---|---|---|
| 5 | $4000-$7FFF | Pantalla ($4000-$5AFF) y datos no criticos en $5B00-$7FFF (zona contendida) |
| 2 | $8000-$BFFF | Codigo caliente, pila, tabla IM2. No contendido |
| 0 | $C000 | Fondo base (6.912 bytes) y datos calientes. El unico banco no contendido en todos los modelos |
| 4, 6 | $C000 | Conjuntos de sprites (no contendidos en el 128K y +2; contendidos en +2A y +3) |
| 1, 3 | $C000 | Datos comprimidos y frios (contendidos en el 128K y +2) |
| 7 | $C000 | Pantalla sombra opcional y datos |

Cada entidad (dino, Trog) tendra su conjunto en un banco; el dibujo pagina al banco del conjunto antes de pintar. El IM2 y la pila no deben estar nunca en el banco paginado.

## 6. Decisiones pendientes (Carlos)

1. Tamano del lienzo de Trog y T-Rex (16x24 como el dino, 24x32 u otro). Determina casi toda la memoria de los niveles A y C.
2. Un fotograma intermedio de giro (propuesta) o dos.
3. Que eventos entran en el hito 1 y 2 (caida, foso, aplastado, comido) y con cuantos fotogramas.
4. Que variantes de Trog (palo, rueda, muelles) y si el T-Rex entra antes del hito 4.
5. Plataforma de referencia para la contencion: 128K/+2 o tambien +2A/+3.

## 7. Mediciones previstas

- Ejecutar tools/frame_report.py con las seis tablas y guardar el resultado (--csv) en docs/.
- Medir el tamano real del motor al ensamblar (listado de sjasmplus).
- Probar la compresion ZX0 con los sprites provisionales del dino y de las piedras (compresor de Einar Saukas; descompresores de 68 a 673 bytes).
- Medir el coste de descomprimir y espejar un conjunto de eventos al empezar un evento.
