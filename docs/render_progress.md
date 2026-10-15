# Analisis y resultados de render R1-R6

Version: 15-oct-2026. Fuentes: archivos del port aportados, codigo de benches generado en la conversacion y pruebas informadas por Carlos. Distincion obligatoria: analisis del codigo, modelo Python y ejecucion real no son equivalentes.

## Mecanismo
R1 restaura fondo individual y pinta. Con solapes guarda/restaura pixels de otros sprites y deja suciedad.
R2 compone ventanas con fondo y solo entidad propia: al volcar un rectangulo borra a los vecinos, aunque la mascara propia funcione.
R3 compone todos los vecinos relevantes por prioridad.
R4 optimiza ruta lenta y convierte el fondo a layout lineal.
R5 separa preparacion de fondo y sprites, prueba bancos y bordes.
R6 distingue dino/Trogs y permite cambios de imagen/presencia.
Mascaras: pantalla/composicion = (fondo AND mascara) OR grafico. Mascara 1 conserva el fondo; grafico puede ser cero opaco donde mascara es cero.

## Resultados R1 (Carlos)
| N | E | Maquina | LIBRE | MIN | RETR | PERD |
|---|---|---|---:|---:|---:|---:|
| 1 | 1 | Pentagon, Spectaculator | 910 | 1503 | 0 | 0 |
| 3 | 1 | Pentagon, Spectaculator | 700 | 560 | 0 | 0 |
| 5 | 1 | Pentagon, Spectaculator | 498 | 423 | 0 | 0 |
| 5 | 0 | Pentagon, Spectaculator | 490 | 0 | 28 | 28 |
| 8 | 1 | Pentagon, Spectaculator | 160 | 0 | 14 | 14 |
| 1 | 1 | RVM 128K | 896 | 1472 | 0 | 0 |
| 5 | 1 | RVM 128K | 460 | 424 | 0 | 0 |
| 5 | 0 | RVM 128K | 450 | 0 | 28 | 28 |

## Resultados R2 (RVM 128K)
| N | E | LIBRE | MIN | RETR | PERD | SOLP max |
|---|---|---:|---|---|---|---:|
| 1 | 1 | 924 | 1405 | 0 | 0 | 0 |
| 5 | 1 | 585 | 547 | 0 | 0 | 64 |
| 5 | 0 | 585 | 530-640 | 0 | 0 | 77 |
| 8 | 1 | 310 | 80 | 0 | ocasionalmente 1 | 150 |
| 8 | 0 | 298 | 0 | 28 | 28 | 150 |

## Resultados R3 (RVM 128K)
| N | E | S | LIBRE | MIN | RETR | PERD | SOLP max |
|---|---|---|---|---|---|---|---:|
| 1 | 1 | 1 | 925 | 1744 | 0 | 0 | 0 |
| 5 | 1 | 0 | 613 | 985-1053 | 0 | 0 | 0 |
| 5 | 1 | 1 | 468-595 | 338-949 | 0 | 0 | 74 |
| 5 | 0 | 1 | 466-590 | 0 | 9 | 9 | 53 |
| 8 | 1 | 1 | 90-279 | 0 | 0-43 | 0-43 | 84 |
| 8 | 0 | 1 | 133-315 | 0 | 28-56 | 28-56 | 91 |

## Resultados R4 (RVM 128K)
| N | E | S | P | LIBRE | MIN | RETR/PERD | SOLP max |
|---|---|---|---|---|---|---|---:|
| 1 | 1 | 1 | 0 | 937 | 1768-1789 | 0/0 | 0 |
| 5 | 1 | 0 | 0 | 676 | 1136-1196 | 0/0 | 0 |
| 5 | 1 | 1 | 0 | 573-657 | 758-1140 | 0/0 | 62 |
| 5 | 2 | 1 | 0 | 570-658 | 624-1135 | 0/0 | 67 |
| 5 | 2 | 1 | 1 | 572-754 | 550-1123 | 0/0 | 70 |
| 5 | 0 | 1 | 0 | 557-656 | 59-804 | 0/0 | 59 |
| 8 | 1 | 1 | 0 | 232-431 | 0-199 | casi siempre 0, max 15 | 72 |
Carlos confirma prioridades correctas y ausencia de parpadeos incluso al variar velocidad y perder fotogramas puntualmente.

## Resultados R5 (RVM 128K)
Tomados ANTES de corregir DEC E en el volcado derecho.
| N | E | S | P | G | B | MEDIA | MINA | SOLX | PERD |
|---|---|---|---|---|---|---:|---:|---:|---:|
| 5 | 1 | 1 | 0 | 0 | 2 | 619 | 533 | 91 | 0 |
| 5 | 1 | 1 | 0 | 1 | 2 | 631 | 0 | 82 | 0 |
| 5 | 2 | 1 | 1 | 1 | 2 | 677 | 2 | 76 | 0 |
| 5 | 1 | 1 | 0 | 0 | 4 | 625 | 0 | 91 | 0 |
| 5 | 1 | 1 | 0 | 0 | 6 | 617 | 527 | 90 | 0 |
| 5 | 1 | 1 | 0 | 0 | 1 | 633 | 527 | 91 | 0 |
| 5 | 1 | 1 | 0 | 0 | 3 | 613 | 0 | 91 | 0 |
| 5 | 1 | 1 | 0 | 0 | 7 | 624 | 0 | 91 | 0 |
| 8 | 1 | 1 | 0 | 1 | 2 | 340 | 0 | 168 | 39 |

## Resultados R6 (RVM 128K)
Tomados ANTES de corregir DEC E en el volcado derecho.
| N | E | S | P | G | A | F | B | MEDIA | MINA | SOLX | PERD |
|---|---|---|---|---|---|---|---|---:|---:|---:|---:|
| 5 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | 615 | 526 | 91 | 0 |
| 5 | 1 | 1 | 0 | 0 | 1 | 0 | 2 | 610 | 199 | 110 | 0 |
| 5 | 1 | 1 | 0 | 0 | 0 | 1 | 2 | 711 | 0 | 100 | 0 |
| 5 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 750 | 104 | 64 | 0 |
| 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 512 | 0 | 121 | 0 |

## Correcciones del codigo y evidencia
### init_ents
ent_set_qpos paso a alterar BC: conservar BC y HL en la llamada de inicializacion. Sin ello el contador e indice se corrompian y el programa iba a negro. Carlos confirmo el arreglo.
### DJNZ largo
R5 linea 455: desplazamiento -129. R6 linea 470: -141. Sustituir el DJNZ del bucle de inicializacion por DEC B / JP NZ,.l. Carlos confirmo ensamblado/ejecucion despues.
### Borde derecho
ent_blit copia tres bytes con LDI. Si empieza en $xxFD acaba en $(xx+1)00; tres DEC E dejan el byte alto incrementado. Tres DEC DE restauran el puntero completo antes de avanzar linea.
Comprobacion por calculo en Python: 23 combinaciones incorrectas con DEC E entre lineas 24..190 y columnas 0..29; ninguna con DEC DE. No fue emulacion Z80.
Carlos confirma que ya funciona tras corregirlo. No se han recibido tablas nuevas tras la correccion.

## Limites de los modelos y mediciones
- Las comprobaciones Python compararon composicion ideal y algoritmo; no ejecutaron los registros ni instrucciones del Z80. Un lint de etiquetas tampoco verifica rangos de saltos ni conservacion de registros.
- PERD omite iteraciones con HUD mediante skip_acc. Corregir antes de afirmar total de perdidas.
- MIN cero puede ser cuantizacion; no implica por si solo fotograma perdido.
- MEDIA es promedio movil de 16 ventanas, no media historica total.
- SOLX cuenta ventanas lentas, no parejas de colision ni numero de entidades solapadas distinto.
- Diferencias entre bancos de R5 no concluyentes; medir configuraciones de igual duracion/semilla y con contadores corregidos.
- No derivar un coste exacto por movimiento usando solo extremos de LIBRE/SOLP de ventanas distintas.

## Margen observado y lectura prudente
R6 base: MEDIA 615 -> ~43.050 T libres medios; MINA 526 -> ~18.410 T libres minimos.
R6 acciones: MEDIA 610 -> ~42.700 T libres medios; MINA 199 -> ~6.965 T minimos.
R6 completo 5: MEDIA 750 -> ~52.500 T medios; MINA 104 -> ~3.640 T minimos.
Las caidas reducen entidades visibles durante la espera y elevan la media libre; no prueban que todos los fotogramas de caida sean baratos. Falta coste real de IA, sonido, colisiones, animaciones y cambios de fondo.
