# Suplemento 2 de decisiones: integracion del renderer

Version: 16-oct-2026 (revision 2, 04:00), suplemento 2 a decisions.md revision 15. Se suma al suplemento 1 (decisions_render_update.md); no sustituye ninguno de los dos ni decisions.md. Integrar en decisions.md cuando se disponga de la ultima version local completa.

## Decisiones y preferencias confirmadas por Carlos (15-oct, 23:33 y 16-oct)
- Adaptar el juego al renderer actual. El codigo de src_z80/ es la version previa (dibujo directo, sin optimizacion): se reutiliza su logica, no su dibujo. Mantener en lo posible la logica del juego original y el renderer. En la practica se programa el juego de nuevo.
- Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero mejor jugable en 48K si se puede. No cambia el orden de los hitos.
- La muerte por caida en los contornos es una animacion sin parpadeo; el parpadeo era de una version anterior.
- HUD de depuracion en la mitad derecha (columnas 16-31, filas 0-2); la mitad izquierda queda libre para puntos, huevos, vidas, etc. Etiquetas: MG, WN (fila 0), MN, SX (fila 1), PC, LT (fila 2).
- Carpetas: src/ en local y src_z80/ en GitHub.

## Decisiones confirmadas el 16-oct (02:30 a 04:00)
1. Sincronizacion y logica: un paso de logica por iteracion, con espera por contador y sin recuperar pasos perdidos (sin catch-up). La logica no se entera de un fotograma perdido; los temporizadores cuentan pasos. Un fotograma perdido retrasa el juego 20 ms; solo una perdida sostenida (PC > 0 de forma continua) seria ralentizacion. Carlos acepta perdidas puntuales cuando coinciden con solapes de dos sprites, sin parpadeo.
2. Huevos: son decorado del fondo limpio del banco 0, no entidades. En el arcade son objetos (TROGEGGS.ASM: EGGHATCH/QUICK_EGGHATCH con GPALOBJSTF y INSERT_OBJ_SUPP en la lista EGGS; EGGDINO los retira con OBJPROC_KILL y limpia CMOVEEGG); en el port se mantiene egg_map como equivalente de CMOVEEGG. El suelo conserva textura y dibujos: al comenzar el nivel se guarda el suelo bajo cada nodo (2 bytes x 16 lineas = 32 bytes por nodo, unos 1,4 KB para 43 huevos) en el banco 0 y la retirada de un huevo restaura ese patron en el fondo y vuelca un rectangulo sucio componiendo las entidades que lo toquen (el dino esta encima en ese momento). Sustituye a erase_egg_area, que ponia ceros. EGG_GROW (dos jugadores o mas) queda para mucho mas adelante.
3. Escalonado de movimientos: fases iniciales repartidas a partes iguales, fase_i = floor(64 * i / N) (con el modulo 64 de la velocidad). Con velocidades iguales y N >= 5 hay siempre algun sprite moviendose en cada fotograma y el pico baja respecto a E1 (N = 8 y 9: 3-4 movimientos por fotograma frente a 2-5). Paradas, acciones y cambios de velocidad desplazan las fases con el tiempo; no es un planificador.
4. Velocidad base: acumulador de modulo 64 con base 26 (equivalente a 13/32, pasos de 2 px). Dino y Trogs usan la misma base. Carlos podra ajustar la base mas adelante.

### Evidencia de la velocidad (analisis del fuente + medidas de Carlos)
- Distancia entre nodos en el arcade: POSTMAP1 (TROGMAPS.ASM) coloca postes cada 32 px en X y en Y (10 x 6 postes, 9 x 5 intersecciones). Nuestra rejilla es de 24 px (piedra 8 + calle 16): escala 0,75 en ambos ejes.
- DINOSPEED y TROGSPEED salen de WAVE_DIFF_TAB (TROGWAVE.ASM) con los mismos valores en las 50 oleadas (oleada 1 = 1000H; 1200H oleadas 5-6; 1400H desde la 7; hasta 1700H). TROG_PROCESS y DINOWALK hacen SLL 4 y llaman a DINO_VEL (TROGDINO.ASM): 1000H << 4 = 0x10000, formato 16.16 (la nota de DINO_INIT_VEL indica bits 12-15 enteros y 0-11 fraccion): 1,0 px por tick. UPDATE_VEL copia +-DVEL a OXVEL y OYVEL (mismo valor en ambos ejes). Es una inferencia que la posicion se integre una vez por tick; la medida de MAME la apoya.
- Cuadre: 8 intervalos = 256 px del arcade. Teoria a 55 fps: 4,66 s (a 54,71 Hz: 4,68 s). Lectura visual de Carlos en MAME: 4,78 s o algo mas (+2,5 % sobre la teoria). Lectura en el Spectrum con velocidad 7/16: 4,51 s frente a 4,39 s teoricos (+2,7 %). Ambas lecturas comparten sesgo, por lo que se comparan directamente.
- Comparacion con MAME (4,78 s): 7/16 queda un 5,7 % mas rapido; 6/16 un 10 % mas lento (teoria 5,12 s); 26/64 un 1,6 % mas lento (teoria 4,73 s, lectura estimada 4,85 s).
- El cambio de modulo 16 a 64 es de constantes (cp y sub en ent_update; fases del escalonado en init_ents) y no cuesta T-states. Con base 26 la velocidad maxima del arcade (2 x base) es 52/64, un paso de 2 px por fotograma como maximo.
- Menos movimientos (13/32 frente a 7/16): -7 %, unos 2.700 T por fotograma con 8 entidades (estimacion, sin medir).

### Para hitos posteriores (no decidido)
- Multiplicadores del arcade en unidades de 64: floor(26 x multiplicador). Dino con items: DINO_SPEEDUP suma 4000H (flor, 12 x 60 ticks) y DINO_SLOWDOWN lo resta (seta, 5 x 60 ticks); DINO_ADD_VEL limita entre 0,56 x y 2 x la base (en unidades de 64: 15 y 52); DINO_VEL_NORMAL devuelve a la base de 0x100 en 0x100. Trog con muelles: tabla SPRGSPEED 1800H o superior (1,5 x la base = 39/64).
- Pendiente de leer: TROGSPEEDPID, TSPEEDUPS e INITSPEEDUPS (TROGTROG.ASM) para saber si la velocidad del Trog cambia durante una oleada en LA5; la linea .DEF TROG_SLOWDOWN, TROG_SPEEDUP esta comentada.

## Propuestas del asistente aun no aprobadas
- Contrato del renderer (render.inc) antes de codificar: entidades, rectangulo de decorado compuesto por prioridad, direcciones por EQU y macros de paginacion que no generan codigo en 48K.
- Plan por pasos pequenos con prueba de Carlos: esqueleto con dino como entidad 0; huevos como decorado; casa y victoria como rectangulos; Trogs placeholder y HUD de depuracion; medida del fotograma completo.

## Metricas del banco (decididas con Carlos)
- MG, WN, MINA, SOLX, PC y LT se leen con el HUD congelado en VENT = 64. Un valor MEDIA tomado en un instante no se usa como rendimiento.
- Margen del peor caso: BENCH_BURN sube la carga en pasos de 24 T por vuelta; se anota la carga con la que PC deja de ser 0.
- PC mide deficit real de fotogramas (n-1); LT cuenta iteraciones tardias. PERD queda sustituido y no es comparable.

## Abiertas
- Pantalla sombra (no descartada) frente al esquema B+.
- Pila fuera del banco 5 (contendido): hipotesis sin medir.
- Ordenacion por claves de un byte: ahorro estimado, sin aplicar.
- Si un huevo puede tapar parcialmente al dino por Y, como en el arcade (SET_MZPOS): a observar en MAME.
- Velocidad del Trog constante o con aceleraciones periodicas (ver arriba).

## Correcciones de afirmaciones anteriores
- MEDIA en un instante no es un valor de rendimiento.
- MINA = 0 con PC = 0 indica iteraciones tardias absorbidas, no ausencia de picos.
- La ablacion de la primera tanda (SKIP_BLIT, SKIP_SORT) no es valida; BENCH_SKIP_BLIT solo envolvia el volcado inicial.
- El instrumento es lineal (BENCH_BURN): una sospecha de error sistematico de 18 unidades fue un malentendido de configuracion.
- Los resultados con PC = 0 y LT > 0 solo valen con espera por contador.
- El 7/16 del banco y del juego previo no es la velocidad del arcade (un 8 % mas rapida segun la teoria; 5,7 % segun las lecturas de Carlos).
