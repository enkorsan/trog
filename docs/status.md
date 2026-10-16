# Estado del proyecto Trog ZX

Version: 16-oct-2026, revision 20.
Base documental: status.md revision 19 (16-oct, 01:30), decisions.md revision 15 (sin tocar), decisions_render_update.md y decisions_render_update_2.md (revision 2), testing.md revision 12 (sin tocar), testing_render.md revision 3 y memoria.md revision 0; actualizado con la conversacion de desarrollo y pruebas hasta el 16-oct, 04:07.
Este documento sustituye al status.md revision 19. Distingue codigo revisado por el asistente (sin ensamblar), pruebas informadas por Carlos y estimaciones.

## Plataforma y herramientas
- Port logico de Trog LA5 para ZX Spectrum 128K convencional, Z80 y sjasmplus. Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero se prefiere mantener jugable en 48K si se puede. Sin cambiar la hoja de ruta.
- sjasmplus 1.24.0 confirmado por Carlos.
- Referencia de rendimiento: RetroVirtualMachine, ZX Spectrum 128K original toastrack. Spectaculator abre los SNA como Pentagon; Fuse intenta Pentagon y termina en 48K si falta su ROM; RVM mantiene la maquina seleccionada. No mezclar mediciones de Pentagon con las del 128K.
- Carlos ensambla y prueba localmente. El asistente no ha ejecutado sjasmplus ni un emulador.
- Carpetas: en local el codigo esta en src/ y en GitHub en src_z80/ (confirmado por Carlos). Los .bat locales usan src/; el asistente lee src_z80/ en el repositorio.
- Scripts de Carlos: build.bat (juego): sjasmplus --nologo --msg=err --lst=build/trogzx.lst --sym=build/trogzx.sym -Isrc src/main.asm, y lanza build/trogzx_paso1.sna. build_bench.bat (bancos): sjasmplus NAME.asm; el nombre del .sna lo fija el SAVESNA de cada fichero. Para listado y simbolos de un banco se uso una variante con --lst y --sym. El juego anterior (trogzx.sym) ya no se usa como referencia.

## Juego: version previa (otra conversacion)
- El codigo de src_z80/ (main.asm y modulos .inc) es la version previa del juego: dibujo directo en pantalla con guardar/borrar/repintar, bucle con HALT, sin optimizacion. Carlos confirmo en su momento: movimiento hasta 3g, huevos 2d, casa y letrero (HOUSE_FLASH_FRAMES = 21), salidas funcionales (13-oct) y victoria provisional.
- Decision de Carlos (15-oct): adaptar el juego al renderer actual, manteniendo en lo posible la logica del juego original y el renderer; en la practica se reescribe el juego reutilizando la logica ya probada. Inventario y plan en analysis/integracion_inventario.md.
- La muerte por caida en los contornos es una animacion, sin parpadeo; el parpadeo de fall_update es de la version anterior.
- Dino provisional de 16x24, cuatro orientaciones y cuatro fases horizontales; formato compatible con el renderer R6.

## Renderer: bancos de pruebas independientes
- R1 a R6 como en las revisiones anteriores (R1 referencia de coste; R2-R4 composicion en ventanas y prioridad; R5 recorte y bancos; R6 dino distinto de Trogs, E_DIRTY, acciones en el sitio, caida con animacion provisional).
- R6.1 (asistente, a partir del R6 de Carlos): instrumentacion corregida. Se lee frame_counter una vez; se acumulan por separado iteraciones tardias y deficit de fotogramas (n-1) con y sin HUD; PC (deficit sin HUD) y LT (iteraciones tardias sin HUD); velocidad en el HUD.
- R6.2: anade MG (media de las 64 primeras ventanas) y WN (peor media de ventana), BENCH_STOPVENT, BENCH_BURN (carga artificial de 24 T por vuelta), BENCH_SKIP_BLIT y BENCH_SKIP_SORT, y el arreglo que deja draw_clipped fuera del IF BENCH_SLOW. Ensamblado y ejecutado por Carlos.
- R6.3: HUD de depuracion en la mitad derecha (columnas 16-31: MG y WN en la fila 0; MN (MINA) y SX (SOLX) en la fila 1; PC y LT en la fila 2). La mitad izquierda lleva solo, en el banco, la configuracion N E S P G A F B (fila 0) y V y VENT (fila 1). Ensamblado por Carlos y HUD de la mitad derecha confirmado visualmente (captura del 16-oct). Subido al repositorio como src_z80/bench_r6.asm (commit 08d29fd, SHA de blob ab2d42e..., 34.233 bytes, fines de linea LF) con los valores tal como se probaron: BENCH_BURN = 292 y BENCH_SKIP_SORT = 1 (para medir una base hay que ponerlos a 0). Copia local: src/bench_r6_integracion.asm (R6.3 renombrado).
- El renderer experimental todavia NO esta integrado en el juego. Para esa integracion se abre una conversacion nueva (ver continuation.md).

## Decisiones del 16-oct (detalle en decisions_render_update_2.md, revision 2)
- Un paso de logica por iteracion, con espera por contador y sin recuperar pasos perdidos.
- Huevos como decorado del fondo limpio (banco 0) con guardado del suelo bajo cada nodo (unos 1,4 KB); la retirada restaura ese patron y vuelca un rectangulo sucio. Sustituye a erase_egg_area (ceros). EGG_GROW queda para dos jugadores o mas.
- Escalonado con fases repartidas a partes iguales: fase_i = floor(64 * i / N).
- Velocidad base: acumulador de modulo 64 con base 26 (= 13/32 de 2 px por fotograma), igual para dino y Trogs, ajustable despues. Motivo: 8 intervalos entre nodos = 256 px del arcade (postes cada 32 px, POSTMAP1) a 1,0 px/tick (DINOSPEED 1000H << 4 en formato 16.16) y 55 fps dan 4,66 s; Carlos mide 4,78 s en MAME; en el Spectrum con 7/16 mide 4,51 s (teoria 4,39 s). 7/16 queda un 5,7 % mas rapido que MAME, 6/16 un 10 % mas lento y 26/64 un 1,6 % mas lento.

## Mediciones y memoria
Detalle en analysis/render_mediciones_r6_2.md. Resumen:
- N8 (E2 P1 A1 F1), RVM 128K, V7, VENT = 64: MG 510 (unos 35.700 T libres de media), WN 228, LT 21, PC 0. Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T, PC = 2. La tabla de BENCH_BURN se tomo con BENCH_SKIP_SORT = 0 (confirmado por Carlos).
- N5: MG 613-725, WN 552-592, LT 0-1, PC 0.
- Ordenacion: la captura del R6.3 con SKIP_SORT = 1 y BURN = 292 da MG 424, WN 203, LT 65, frente a MG 410, WN 124, LT 99 con SKIP_SORT = 0. La ordenacion cuesta unos 1.000 T de media por fotograma y unos 5.500 T en la peor ventana en ese escenario. Cautela: SX difiere entre compilaciones (121 frente a 153) y sigue sin explicacion.
- Mapa de memoria medido (listado y simbolos del R6.3): codigo y tablas $8000-$8A58 (2.649 bytes), variables $8A59-$8ACD, entidades $8ACE (8 x 32 bytes), buffers de ventana $8C00-$8FFF, video.inc $9000, sprite_dino $90CA-$99C9 y sprite_trog $99CA-$A2C9 (2.304 bytes cada uno), piedras y fuente hasta $A50D. Fin de datos $A50E; margen hasta IM2_TABLE ($B800): 4.850 bytes. Fondo limpio: banco 0, $C000, 6.144 bytes. Pila en $7FFF.
- 48K: el conjunto residente del banco ocupa unos 9,5 KB y el fondo limpio 6 KB; la RAM libre de un 48K ($5B00-$FFFF) es de unos 42,5 KB. Parece caber para el hito 1; sin comprobar en un modelo 48K.

## Pendiente inmediato
1. Paso 0 del plan de integracion en la conversacion nueva: contrato del renderer (entidades, rectangulo de decorado, espera por contador, macros de paginacion no-op en 48K, direcciones por EQU, velocidad modulo 64). Documentarlo en decisions_render_update_3.md o decisions.md antes de codificar.
2. Esqueleto nuevo: dino como entidad 0 con la logica reutilizada, velocidad base 26/64 y el HUD de depuracion en la mitad derecha. Prueba de Carlos: movimiento, giros y caida con animacion frente a MAME.
3. Variante 48K del banco (DEVICE ZXSPECTRUM48) y prueba en RVM modelo 48K.
4. Prueba de la pila fuera de la zona contendida ($7FFF esta en el banco 5): comparar MG.
5. Decisiones abiertas: pantalla sombra; pila en el banco 2; si un huevo puede tapar al dino por Y.
6. Leer TROGSPEEDPID, TSPEEDUPS e INITSPEEDUPS (TROGTROG.ASM) para saber si la velocidad del Trog cambia durante una oleada.
7. decisions.md y testing.md siguen en sus revisiones; los cambios nuevos estan en los suplementos. Integrarlos cuando se disponga de la ultima version local completa.
8. memoria.md no esta en el repositorio: Carlos debe subir el original (rev. 0) para anotarle las medidas reales.

## Otras tareas pendientes del juego (sin cambiar la hoja de ruta)
- 2e-4b: reorientacion hacia la casa.
- Animaciones de giro y de caida definitivas; luego Trog basico.
- Items y velocidades de hitos posteriores: flor y seta del dino (DINO_SPEEDUP y DINO_SLOWDOWN), Trogs con muelles (SPRGSPEED) y velocidades por oleada (WAVE_DIFF_TAB); ver decisions_render_update_2.md.
- Colisiones/ataques: punetazos del dino y del Trog; tocarse sin atacar no produce dano por si solo (Carlos).
- Fuente definitiva y migracion del HUD: no dar por aplicada toda la migracion.
- Sonido AY y menus corresponden al hito 3. No hay reproductor elegido ni integrado.
- Aleatoriedad inicial con menu; pruebas de bancos/ZX0, catalogo grafico y presupuesto completo pendientes.

## Analisis del fuente
- EXITPOINTS leido en TROGPOST.ASM: usar la segunda version activa, no la comentada con ****. CALCA1GT/CALCA1LT registran las salidas.
- Geometria de 28 salidas deducida por lectura para la rejilla rectangular; no se ejecuto la construccion arcade para contarla. ADDEMOVE/deduplicacion sigue pendiente de verificacion especifica.
- El nivel 1 no requiere filtrar tipos de salida por lago/pozo/volcan, segun analisis y observacion de Carlos.
- Leido el 16-oct: TROGEGGS.ASM (huevos como objetos, EGGHATCH, EGGDINO, EGG_GROW), TROGWAVE.ASM (WAVE_DIFF_TAB, WAVE_TABLE), TROGMAPS.ASM (POSTMAP1) y, en TROGDINO.ASM, DINOWALK, DINO_VEL, UPDATE_VEL, DINO_ADD_VEL, DINO_VEL_NORMAL, DINO_SPEEDUP y DINO_SLOWDOWN.
- Siguen pendientes: GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, guion REXDOORMAT_x, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROG_START, TROGSPEEDPID, TROGBAR.ASM y WAVEHAND. PUNCHED/DINO_PUNCHED/SPIT_FIRE disponibles pero sin analisis completo autorizado.
- Confirmar revision LA5 del fuente archivado y ROCKPOST1..4.

## Documentacion y continuidad
- decisions_render_update.md y decisions_render_update_2.md, y testing_render.md, son suplementos: NO sustituyen decisions.md revision 15 ni testing.md revision 12.
- analysis/render_progress.md conserva las tablas anteriores (R1-R6, antes de la instrumentacion corregida); las mediciones nuevas estan en analysis/render_mediciones_r6_2.md (revision del 16-oct, 04:10).
- analysis/integracion_inventario.md: inventario de la version previa del juego, puntos de contacto con el renderer y plan revisado.
- continuation.md es el mensaje de traspaso para la conversacion nueva (actualizado el 16-oct, 04:10).
- memoria.md revision 0 es borrador, no presupuesto cerrado.
- El repositorio contiene el codigo del port en src_z80/; la copia local de Carlos (src/) es la copia de trabajo y puede ir por delante.
