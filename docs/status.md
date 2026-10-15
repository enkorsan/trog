# Estado del proyecto Trog ZX

Version: 16-oct-2026, revision 19.
Base documental: status.md revision 18 (15-oct-2026, 23:55), decisions.md revision 15, testing.md revision 12 y memoria.md revision 0; actualizado con la conversacion de desarrollo y pruebas hasta el 16-oct, 01:24.
Este documento sustituye al status.md revision 18. Distingue codigo revisado por el asistente (sin ensamblar), pruebas informadas por Carlos y estimaciones.

## Plataforma y herramientas
- Port logico de Trog LA5 para ZX Spectrum 128K convencional, Z80 y sjasmplus. Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero se prefiere mantener jugable en 48K si se puede. Sin cambiar la hoja de ruta.
- sjasmplus 1.24.0 confirmado por Carlos.
- Referencia de rendimiento: RetroVirtualMachine, ZX Spectrum 128K original toastrack. Spectaculator abre los SNA como Pentagon; Fuse intenta Pentagon y termina en 48K si falta su ROM; RVM mantiene la maquina seleccionada. No mezclar mediciones de Pentagon con las del 128K.
- Carlos ensambla y prueba localmente. El asistente no ha ejecutado sjasmplus ni un emulador.
- Carpetas: en local el codigo esta en src/ y en GitHub en src_z80/ (confirmado por Carlos el 16-oct). Los .bat locales usan src/; el asistente lee src_z80/ en el repositorio. No renombrar sin acuerdo.
- Scripts de Carlos: build.bat (juego): sjasmplus --nologo --msg=err --lst=build/trogzx.lst --sym=build/trogzx.sym -Isrc src/main.asm, y lanza build/trogzx_paso1.sna. build_bench.bat (bancos): sjasmplus NAME.asm; el nombre del .sna lo fija el SAVESNA de cada fichero. Para el listado y los simbolos de un banco se uso una variante con --lst y --sym (build/bench_r6_integracion-2.lst y -3.sym, locales). El juego anterior (trogzx.sym) ya no se usa como referencia.

## Juego: version previa (otra conversacion)
- El codigo de src_z80/ (main.asm y modulos .inc) es la version previa del juego: dibujo directo en pantalla con guardar/borrar/repintar, bucle con HALT, sin optimizacion. Carlos confirmo en su momento: movimiento hasta 3g, huevos 2d, casa y letrero (HOUSE_FLASH_FRAMES = 21), salidas funcionales (13-oct) y victoria provisional.
- Decision de Carlos (15-oct): adaptar el juego al renderer actual, manteniendo en lo posible la logica del juego original y el renderer; en la practica se reescribe el juego reutilizando la logica ya probada. Inventario y plan en analysis/integracion_inventario.md.
- La muerte por caida en los contornos es una animacion, sin parpadeo; el parpadeo de fall_update es de la version anterior.
- Dino provisional de 16x24, cuatro orientaciones y cuatro fases horizontales; formato compatible con el renderer R6.

## Renderer: bancos de pruebas independientes
- R1 a R6 como en las revisiones anteriores (R1 referencia de coste; R2-R4 composicion en ventanas y prioridad; R5 recorte y bancos; R6 dino distinto de Trogs, E_DIRTY, acciones en el sitio, caida con animacion provisional).
- R6.1 (asistente, a partir del R6 de Carlos): instrumentacion corregida. Se lee frame_counter una vez; se acumulan por separado iteraciones tardias y deficit de fotogramas (n-1) con y sin HUD; PC (deficit sin HUD) y LT (iteraciones tardias sin HUD); velocidad en el HUD.
- R6.2: anade MG (media de las 64 primeras ventanas) y WN (peor media de ventana), BENCH_STOPVENT (congela el HUD al llegar a VENT), BENCH_BURN (carga artificial de 24 T por vuelta), BENCH_SKIP_BLIT y BENCH_SKIP_SORT, y el arreglo que deja draw_clipped fuera del IF BENCH_SLOW. Ensamblado y ejecutado por Carlos.
- R6.3: HUD de depuracion en la mitad derecha (columnas 16-31: MG y WN en la fila 0; MN (MINA) y SX (SOLX) en la fila 1; PC y LT en la fila 2). La mitad izquierda lleva solo, en el banco, la configuracion N E S P G A F B (fila 0) y V y VENT (fila 1), para que quede libre en el juego (puntos, huevos, vidas). Ensamblado por Carlos y mitad derecha del HUD confirmada visualmente el 16-oct (captura). Copia local: src/bench_r6_integracion.asm (R6.3 renombrado); no esta en el repositorio. MEDIA ya no se muestra (se sigue calculando).
- El renderer experimental todavia NO esta integrado en el juego.

## Mediciones y memoria
Detalle en analysis/render_mediciones_r6_2.md (tablas informadas por Carlos, lectura prudente, captura del R6.3, estimaciones y mapa de memoria medido). Resumen:
- N8 (E2 P1 A1 F1), RVM 128K, V7, VENT = 64: MG 510 (unos 35.700 T libres de media), WN 228, LT 21, PC 0. Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T, PC = 2.
- N5: MG 613-725, WN 552-592, LT 0-1, PC 0.
- Mapa de memoria medido (listado y simbolos del R6.3 con BENCH_BURN = 292 y BENCH_SKIP_SORT = 1): codigo y tablas $8000-$8A58 (2.649 bytes), variables $8A59-$8ACD, entidades $8ACE (8 x 32 bytes), buffers de ventana $8C00-$8FFF (1.024 bytes), video.inc $9000, sprite_dino $90CA-$99C9 y sprite_trog $99CA-$A2C9 (2.304 bytes cada uno), piedras y fuente hasta $A50D. Fin de datos $A50E; margen hasta IM2_TABLE ($B800): 4.850 bytes. Fondo limpio: banco 0, $C000, 6.144 bytes. Pila en $7FFF.
- 48K: el conjunto residente del banco (codigo, buffers y sprites provisionales) ocupa unos 9,5 KB y el fondo limpio 6 KB; la RAM libre de un 48K ($5B00-$FFFF) es de unos 42,5 KB. Parece caber para el hito 1; sin comprobar en un modelo 48K.

## Pendiente inmediato
1. Paso 0 del plan de integracion: contrato del renderer (entidades, rectangulo de decorado, sincronizacion por contador, macros de paginacion no-op en 48K, direcciones por EQU). Documentarlo en decisions.md o su suplemento antes de codificar.
2. Confirmar si la tabla BENCH_BURN se tomo con BENCH_SKIP_SORT = 0 (el .sym del R6.3 indica SKIP_SORT = 1 y BURN = 292 en esa compilacion).
3. Variante 48K del banco: ensamblar con DEVICE ZXSPECTRUM48 y probar en RVM modelo 48K.
4. Probar la pila fuera de la zona contendida ($7FFF esta en el banco 5): comparar MG.
5. Decisiones abiertas: sincronizacion con espera por contador frente a HALT y tiempo de juego frente a fotogramas perdidos; huevos como fondo o entidad; pantalla sombra (no descartada); pila en el banco 2.
6. decisions.md y testing.md siguen en sus revisiones; los cambios nuevos estan en los suplementos (decisions_render_update.md, decisions_render_update_2.md, testing_render.md). Integrarlos cuando se disponga de la ultima version local completa.
7. memoria.md no esta en el repositorio: Carlos debe subir el original (rev. 0) para anotarle las medidas reales.

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
- decisions_render_update.md y decisions_render_update_2.md, y testing_render.md, son suplementos: NO sustituyen decisions.md revision 15 ni testing.md revision 12.
- analysis/render_progress.md conserva las tablas anteriores (R1-R6, antes de la instrumentacion corregida); las mediciones nuevas estan en analysis/render_mediciones_r6_2.md.
- analysis/integracion_inventario.md: inventario de la version previa del juego, puntos de contacto con el renderer y plan revisado.
- continuation.md es el mensaje de traspaso para otra conversacion (actualizado el 16-oct).
- memoria.md revision 0 es borrador, no presupuesto cerrado.
- El repositorio contiene el codigo del port en src_z80/; la copia local de Carlos (src/) es la copia de trabajo y puede ir por delante.
