# Estado del proyecto Trog ZX

Version: 15-oct-2026, revision 18.
Base documental: status.md revision 17 (15-oct-2026, 00:44), decisions.md revision 15, testing.md revision 12 y memoria.md revision 0; actualizado con la conversacion de desarrollo y pruebas hasta el 15-oct, 23:51.
Este documento sustituye al status.md revision 17. Distingue codigo revisado por el asistente (sin ensamblar), pruebas informadas por Carlos y estimaciones.

## Plataforma y herramientas
- Port logico de Trog LA5 para ZX Spectrum 128K convencional, Z80 y sjasmplus. Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero se prefiere mantener jugable en 48K si se puede. Sin cambiar la hoja de ruta.
- sjasmplus 1.24.0 confirmado por Carlos.
- Referencia de rendimiento: RetroVirtualMachine, ZX Spectrum 128K original toastrack. Spectaculator abre los SNA como Pentagon; Fuse intenta Pentagon y termina en 48K si falta su ROM; RVM mantiene la maquina seleccionada. No mezclar mediciones de Pentagon con las del 128K.
- Carlos ensambla y prueba localmente. El asistente no ha ejecutado sjasmplus ni un emulador.
- Scripts de Carlos: build.bat (juego: sjasmplus --nologo --msg=err --lst=build\trogzx.lst --sym=build\trogzx.sym -Isrc src\main.asm; lanza build\trogzx_paso1.sna) y build_bench.bat (sjasmplus %NAME%.asm; el nombre del .sna lo fija el SAVESNA de cada fichero). La copia local usa src/ y el repositorio src_z80/: pendiente decidir una de las dos (ver Pendiente inmediato).
- src/bench_r6_integracion.asm es el R6.3 renombrado (copia local de Carlos; no esta en el repositorio).

## Juego: version previa (otra conversacion)
- El codigo de src_z80/ (main.asm y modulos .inc) es la version previa del juego: dibujo directo en pantalla con guardar/borrar/repintar, bucle con HALT, sin optimizacion. Carlos confirmo en su momento: movimiento hasta 3g, huevos 2d, casa y letrero (HOUSE_FLASH_FRAMES = 21), salidas funcionales (13-oct) y victoria provisional.
- Decision de Carlos (15-oct): adaptar el juego al renderer actual, manteniendo en lo posible la logica del juego original y el renderer; en la practica se reescribe el juego reutilizando la logica ya probada. Inventario y plan en analysis/integracion_inventario.md.
- La muerte por caida en los contornos es una animacion, sin parpadeo; el parpadeo de fall_update es de la version anterior.
- Dino provisional de 16x24, cuatro orientaciones y cuatro fases horizontales; formato compatible con el renderer R6.

## Renderer: bancos de pruebas independientes
- R1 a R6 como en la revision 17 (R1 referencia de coste; R2-R4 composicion en ventanas y prioridad; R5 recorte y bancos; R6 dino distinto de Trogs, E_DIRTY, acciones en el sitio, caida con animacion provisional).
- R6.1 (asistente, a partir del R6 de Carlos): instrumentacion corregida. Se lee frame_counter una vez; se acumulan por separado iteraciones tardias y deficit de fotogramas (n-1) con y sin HUD; HUD con PC (deficit sin HUD) y LT (iteraciones tardias sin HUD); la velocidad aparece en el HUD.
- R6.2: anade MG (media de las 64 primeras ventanas) y WN (peor media de ventana), BENCH_STOPVENT (congela el HUD al llegar a VENT), BENCH_BURN (carga artificial de 24 T por vuelta), BENCH_SKIP_BLIT y BENCH_SKIP_SORT, y el arreglo que deja draw_clipped fuera del IF BENCH_SLOW. Ensamblado y ejecutado por Carlos con estos resultados.
- R6.3 (propuesto, sin ensamblar ni probar; compilacion por confirmar): HUD de depuracion en la mitad derecha (columnas 16-31, etiquetas MG, WN, MN, SX, PC, LT); la izquierda queda libre. Configuracion, V y VENT en la mitad izquierda solo en el banco.
- El renderer experimental todavia NO esta integrado en el juego.

## Mediciones R6.2 (informadas por Carlos; RVM 128K toastrack; V7; HUD congelado en VENT = 64)
Conversion aproximada: MG x 70 T, WN x 70 T, MINA x 35 T; fotograma de 128K = unos 70.900 T. Son medidas aproximadas, no perfiles exactos.

| N | E | S | P | G | A | F | B | MG | WN | MINA | SOLX | PC | LT |
|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---:|---:|
| 5 | 1 | 1 | 0 | 0 | 0 | 0 | 2 | 613 | 552 | 514 | 90 | 0 | 0 |
| 5 | 1 | 1 | 0 | 0 | 1 | 0 | 2 | 614 | 558 | 0 | 87 | 0 | 1 |
| 5 | 1 | 1 | 0 | 0 | 0 | 1 | 2 | 695 | 577 | 399 | 101 | 0 | 0 |
| 5 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 725 | 592 | 549 | 58 | 0 | 1 |
| 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | 510 | 228 | 0 | 153 | 0 | 21 |

Carga artificial (N8, E2, S1, P1, A1, F1):

| BENCH_BURN | T extra | MG | WN | MINA | PC | LT |
|---:|---:|---:|---:|---:|---:|---:|
| 0 | 0 | 510 | 228 | 0 | 0 | 21 |
| 146 | 3.500 | 460 | 176 | 0 | 0 | 59 |
| 292 | 7.000 | 410 | 124 | 0 | 0 | 99 |
| 584 | 14.000 | 310 | 39 | 0 | 2 | 309 |

Lectura prudente:
- MG baja exactamente lo que cuesta la carga (50, 100 y 200 unidades): el instrumento es lineal.
- Con N8 la media libre es del 50 % (unos 35.700 T) pero la peor ventana deja el 22,5 % (unos 16.000 T). Un MEDIA tomado en un instante no es representativo.
- Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T aparecen 2 fotogramas perdidos en unos 90 s. Valido para esa trayectoria (primeras 64 ventanas con esa semilla).
- El banco es reproducible: misma configuracion, mismo MG a VENT = 64.
- La ablacion de la primera tanda no es valida: BENCH_SKIP_BLIT solo envolvia el volcado inicial de start, no el del bucle, y la diferencia con SKIP_SORT (-18 unidades) no esta explicada. No se usa para repartir costes.
- Estimacion estatica de T-states (calculo del asistente sobre el fuente, sin contencion): una ventana de movimiento cuesta unos 11.000 T (volcado ~3.000, copia de fondo ~2.800, composicion ~4.400-4.700, logica ~750) mas ~500 T fijos por entidad. Coincide con la fila N5 base dentro de un 3 % (27.200 T estimados frente a 28.000 T medidos). Es una estimacion, no una medida; solo esta validada con esa fila.
- No se declara margen cerrado del juego: faltan IA, colisiones, sonido, animaciones, cambios de fondo y la contencion real (la pila esta en $7FFF, banco 5, contendido; hipotesis sin medir).

## Arquitectura experimental vigente
- Fondo limpio lineal: banco 0, $C000 + Y*32 + columna_byte, 6144 bytes de pixeles.
- Buffers de ventanas residentes en banco 2 y alineados a 128 bytes.
- Ventana habitual de 3 bytes por 24/26 lineas para movimientos de 2 px.
- Preparacion: fondo + sprites con mascara, respetando prioridad. Volcado al comienzo de la siguiente iteracion/fotograma.
- R5/R6: dos pasadas para permitir paginar fondo y datos de sprites por separado.
- Con BENCH_SPRBANK = 2 el banco solo pagina el banco 0 en $C000; en un 48K esa zona es RAM plana (por comprobar con DEVICE ZXSPECTRUM48 y RVM modelo 48K).

## Pendiente inmediato
1. Confirmar que R6.3 (src/bench_r6_integracion.asm) ensambla y que el HUD de la mitad derecha se ve bien.
2. Ensamblar el banco con --lst y --sym para obtener tamano real y margen hasta IM2_TABLE ($B800). Propuesta de cambio de build_bench.bat entregada, sin probar.
3. Decidir la carpeta de codigo (src/ local frente a src_z80/ del repositorio) y anotarlo en testing.md.
4. Paso 0 del plan de integracion: contrato del renderer (entidades, rectangulo de decorado, sincronizacion por contador, macros de paginacion no-op en 48K, direcciones por EQU). Documentar en decisions.md antes de codificar.
5. Decisiones abiertas: sincronizacion con espera por contador frente a HALT y tiempo de juego frente a fotogramas perdidos; huevos como fondo o entidad; pila en el banco 2; pantalla sombra (no descartada); prueba de la pila fuera del banco contendido.

## Otras tareas pendientes del juego (sin cambiar la hoja de ruta)
- 2e-4b: reorientacion hacia la casa.
- Animaciones de giro y de caida definitivas; luego Trog basico.
- Velocidad del dino: Carlos cree que es algo mas lenta en arcade; calibrar contra MAME (bajar la velocidad reduce la carga de ventanas).
- Verificar velocidades de Trogs y variaciones posibles en el fuente original. No asumir velocidad constante.
- Colisiones/ataques: punetazos del dino y del Trog; tocarse sin atacar no produce dano por si solo (Carlos).
- Fuente definitiva y migracion del HUD: no dar por aplicada toda la migracion.
- Sonido AY y menus corresponden al hito 3. No hay reproductor elegido ni integrado.
- Aleatoriedad inicial con menu; pruebas de bancos/ZX0, catalogo grafico y presupuesto completo pendientes.

## Analisis del fuente
- EXITPOINTS leido en TROGPOST.ASM: usar la segunda version activa, no la comentada con ****. CALCA1GT/CALCA1LT registran las salidas.
- Geometria de 28 salidas deducida por lectura para la rejilla rectangular; no se ejecuto la construccion arcade para contarla. ADDEMOVE/deduplicacion sigue pendiente de verificacion especifica.
- El nivel 1 no requiere filtrar tipos de salida por lago/pozo/volcan, segun analisis y observacion de Carlos.
- Siguen pendientes: GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, guion REXDOORMAT_x, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROG_START, velocidades, TROGBAR.ASM y WAVEHAND. PUNCHED/DINO_PUNCHED/SPIT_FIRE disponibles pero sin analisis completo autorizado.
- Confirmar revision LA5 del fuente archivado y ROCKPOST1..4.

## Documentacion y continuidad
- decisions_render_update.md y testing_render.md son suplementos: NO sustituyen decisions.md revision 15 ni testing.md revision 12. Pendiente integrar en ellos el protocolo con STOPVENT, MG, WN, BURN y SKIP y las decisiones de este dia.
- analysis/render_progress.md conserva tablas anteriores (R1-R6 antes de la instrumentacion corregida); las mediciones nuevas estan en este documento.
- analysis/integracion_inventario.md: inventario de la version previa del juego, puntos de contacto con el renderer y plan revisado.
- continuation.md es el mensaje de traspaso; hay que actualizarlo con esta revision.
- memoria.md revision 0 es borrador, no presupuesto cerrado.
- El repositorio contiene ahora el codigo del port en src_z80/; la copia local de Carlos es la copia de trabajo y puede ir por delante.
