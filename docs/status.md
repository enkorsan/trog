# Estado del proyecto Trog ZX

Version: 16-oct-2026, revision 21.
Base documental: status.md revision 20, decisions.md revision 15 (sin tocar), decisions_render_update.md, decisions_render_update_2.md (revision 2) y decisions_render_update_3.md (revision 1), testing.md revision 12 (sin tocar), testing_render.md revision 3, analysis/extraccion_render_core.md (revision 1) y memoria.md revision 0; actualizado con la conversacion de desarrollo y pruebas hasta el 16-oct, 15:44.
Este documento sustituye al status.md revision 20. Distingue codigo revisado por el asistente, pruebas informadas por Carlos y estimaciones. El asistente no ha ejecutado sjasmplus ni emulador: todo lo "probado" es informado por Carlos.

## Plataforma y herramientas
- Port logico de Trog LA5 para ZX Spectrum 128K convencional, Z80 y sjasmplus. Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero se prefiere mantener jugable en 48K si se puede. Sin cambiar la hoja de ruta.
- sjasmplus 1.24.0 confirmado por Carlos. Compatible con -DTARGET_48K (documentacion de sjasmplus 1.24.0 y prueba de Carlos): IFDEF/IFNDEF solo ven DEFINE y -D, no las EQU.
- Referencia de rendimiento: RetroVirtualMachine, ZX Spectrum 128K original toastrack. Spectaculator abre los SNA como Pentagon; Fuse intenta Pentagon y termina en 48K si falta su ROM; RVM mantiene la maquina seleccionada. No mezclar mediciones de Pentagon con las del 128K. La prueba 48K se hizo en RVM modelo 48K (informado).
- Carlos ensambla y prueba localmente.
- Carpetas: en local el codigo esta en src/ y en GitHub en src_z80/ (confirmado por Carlos). El asistente lee src_z80/ en el repositorio.
- Scripts: build.bat (juego anterior): sjasmplus --nologo --msg=err --lst=build/trogzx.lst --sym=build/trogzx.sym -Isrc src/main.asm. build_bench.bat (bancos). build_game_r7.bat y build_game_r7_48.bat (esqueleto nuevo, 128K y -DTARGET_48K), en la raiz; generan build/game_r7.sna y build/game_r7_48.sna con listado y simbolos. Los de esta revision usan / en las rutas (sjasmplus avisa de la barra invertida); tras ese cambio no se han probado.

## Juego: version previa y esqueleto nuevo
- El codigo de src_z80/ main.asm y modulos (dino.inc, ai.inc, eggs_logic.inc, house.inc, victory.inc, sprite.inc...) es la version previa del juego: dibujo directo en pantalla, bucle con HALT. Carlos confirmo en su momento: movimiento hasta 3g, huevos 2d, casa y letrero (HOUSE_FLASH_FRAMES = 21), salidas funcionales (13-oct) y victoria provisional. No se modifica; es la fuente de logica reutilizable.
- Decision de Carlos (15-oct): adaptar el juego al renderer actual manteniendo la logica del juego original; en la practica se reescribe el juego reutilizando la logica ya probada (analysis/integracion_inventario.md).
- Esqueleto nuevo (paso 1 del plan, hecho): src_z80/game_r7.asm (rev. 2) + render.inc (contrato) + render_core.inc (nucleo del renderer extraido del R6.3) + dino_r7.inc (dino como entidad 0). Un paso de logica por iteracion, espera por contador, velocidad 26/64, HUD de teclas a la izquierda y PC/LT a la derecha.
- La muerte por caida en los contornos es una animacion sin parpadeo; el parpadeo de fall_update es de la version anterior.
- Dino provisional de 16x24, cuatro orientaciones y cuatro fases horizontales.

## Integracion del renderer: paso 1 (16-oct)
Detalle en analysis/extraccion_render_core.md.
- Ensamblado: 128K y 48K (informado y visto en los listados game_r7.lst y game_r7_48-2.lst). En 48K las macros PAGE_* no generan codigo: el codigo se desplaza 7 bytes (listado).
- Prueba de juego (informado por Carlos, 16-oct, 15:44): "todo lo esperable funciona"; control OPQA correcto; giros; caidas y animacion de caida provisional correctas; sin restos en pantalla; sin caida de fotogramas (PC y LT en 0); tambien en 48K. Carlos pide anotarlo todo como validado salvo que encuentre algo. Alcance: ENT_COUNT = 1 (solo el dino).
- Velocidad: cronometrada 4,75 a 4,80 s para 8 intervalos, frente a 4,78 s en MAME. La base 26/64 se mantiene.
- Reaparicion: siempre en el nodo de salida (arriba a la izquierda). Es provisional; la real (DINO_FIND_CLOSEST + GET_BEST_DIRECTION, ai.inc) depende de los huevos.
- No probado todavia en esta integracion: ENT_COUNT > 1 (sort_z y la ruta de solapes con varias entidades), decor_rect, imagenes 4..7, SPR_BANK distinto de 2.
- Cabeceras de render.inc, dino_r7.inc, render_core.inc y game_r7.asm actualizadas con este estado.

## Renderer: bancos de pruebas independientes
- R1 a R6 como en las revisiones anteriores (R1 referencia de coste; R2-R4 composicion en ventanas y prioridad; R5 recorte y bancos; R6 dino distinto de Trogs, E_DIRTY, acciones en el sitio, caida con animacion provisional).
- R6.1: instrumentacion corregida (frame_counter leido una vez; iteraciones tardias y deficit de fotogramas n-1; PC y LT). R6.2: MG, WN, BENCH_STOPVENT, BENCH_BURN, BENCH_SKIP_BLIT y BENCH_SKIP_SORT. R6.3: HUD de depuracion en la mitad derecha. Ensamblados y ejecutados por Carlos. Repositorio: src_z80/bench_r6.asm (commit 08d29fd, BENCH_BURN = 292 y BENCH_SKIP_SORT = 1 tal como se probo; para medir una base ponerlos a 0). Copia local: src/bench_r6_integracion.asm.

## Decisiones del 16-oct
Manana (decisions_render_update_2.md, rev. 2):
- Un paso de logica por iteracion, espera por contador, sin recuperar pasos perdidos.
- Huevos como decorado del fondo limpio con guardado del suelo bajo cada nodo (unos 1,4 KB); la retirada restaura ese patron y vuelca un rectangulo sucio. EGG_GROW para dos jugadores o mas.
- Escalonado floor(64 * i / N). Velocidad base 26/64 igual para dino y Trogs, ajustable.
Tarde (decisions_render_update_3.md, rev. 1):
- Mapa de memoria por EQU aprobado por Carlos (SCR_BUFS $B100, STAT_BUF $B500, ENTS $B580, QPOS $B680, ZLIST $B690, IM2_TABLE $B800, BG_BASE $C000).
- Contrato del renderer (entidades, propiedad de campos, imagen ideal, rectangulo de decorado, macros de paginacion, velocidad modulo 64) como PROPUESTA; la parte probada es la del paso 1.
- El suelo del campo debe tener dibujos (decorado no plano). El renderer lo admite sin coste en ejecucion; la restauracion bajo los huevos usa el patron guardado.
- Los huevos se pintan siempre POR DEBAJO del dino, sin respetar Y (el arcade parece hacerlo asi). El decorado nunca tapa a las entidades. El letrero de la casa sigue sin decidir.
- Giro con imagenes intermedias en diagonal (placeholder): media vuelta derecha, derecha-abajo, abajo, izquierda-abajo, izquierda; sentido al azar como en el arcade. Duracion y avance durante el giro: por leer en el fuente. IMG_COUNT pasara de 4 a 8 (+2.304 bytes por juego de sprites).
- Estrategia de carga: 128K con todo cargado y AY; 48K con base jugable y niveles en cargas posteriores, sin AY. Se buscaran recursos para que quepa (quitar animaciones, suelo sin dibujos...). Las animaciones del arcade se reduciran mucho, pero el juego debe sentirse igual.
- Altura de los sprites a revisar con el T-Rex; con puñetazos se cree que cabe.
- Mejora M1 (datos frios en $5B00-$7EFF, 9.216 bytes) propuesta, NO adoptada.
- Casa: evaluar pintarla como decorado en el paso 3 (hoy: entidad o pintado directo, por comprobar).

## Mediciones y memoria
Detalle en analysis/render_mediciones_r6_2.md (banco R6.x) y analysis/extraccion_render_core.md (esqueleto).
- Banco, N8 (E2 P1 A1 F1), RVM 128K, V7, VENT = 64: MG 510 (unos 35.700 T libres de media), WN 228, LT 21, PC 0. Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T, PC = 2 (tabla tomada con SKIP_SORT = 0). N5: MG 613-725, WN 552-592. La ordenacion cuesta unos 1.000 T de media por fotograma y unos 5.500 T en la peor ventana en N8 (estimado de la comparacion SKIP_SORT 0/1; SX difiere entre compilaciones sin explicar).
- Esqueleto (listado de Carlos): fin de datos $9E06 en 128K y $9DFF en 48K; hasta SCR_BUFS ($B100) quedan 4.858 y 4.865 bytes. Bloques: game_r7.asm 335 B, dino_r7.inc 544, render_core.inc 1.180 (con ENT_COUNT = 1), input_hud.inc 237, video.inc 202, sprite_dino 2.304, sprite_trog 2.304, stones 108, font_msg 472. No es margen cerrado del juego: faltan huevos, casa, IA, colisiones, decor_rect y la ordenacion con varias entidades. El mapa del R6.3 (4.850 bytes hasta IM2_TABLE) ya no es la referencia.
- No se mide todavia el tiempo libre por fotograma del esqueleto con varias entidades (PC y LT en 0 con una).
- 48K: RAM libre de un 48K ($5B00-$FFFF) unos 42,5 KB; el esqueleto funciona en 48K con ENT_COUNT = 1.

## Pendiente inmediato
1. Decidir el orden de las siguientes tareas (ver conversacion): estudio de tamanos de sprites (T-Rex, puñetazos, muertes) antes de los pasos 2 y 3 si afecta a SPR_H o al ancho de ventana de 3 bytes; luego huevos y salidas; luego formato de animacion.
2. Paso 2 del plan: huevos como decorado (decor_rect, guardado del suelo bajo cada nodo, recogida con rectangulo sucio, egg_map) y reaparicion real (ai.inc).
3. Paso 3: casa, letrero y victoria como rectangulos de decorado (valorar casa como decorado), salidas.
4. Paso 4: Trogs placeholder (ENT_COUNT > 1), prueba de sort_z, y medida del fotograma completo.
5. Lectura del fuente arcade de las animaciones (giro, puñetazos, golpes recibidos, muertes) y de sus tamanos; formato de animacion y ENT_SIZE.
6. Prueba de la pila fuera de la zona contendida ($7FFF esta en el banco 5): comparar tiempo libre.
7. Decisiones abiertas: pantalla sombra; letrero de la casa por encima o por debajo del dino; mejora M1; cuenta de entidades dinamica.
8. Leer TROGSPEEDPID, TSPEEDUPS e INITSPEEDUPS (TROGTROG.ASM) para saber si la velocidad del Trog cambia durante una oleada.
9. decisions.md y testing.md siguen en sus revisiones; los cambios nuevos estan en los suplementos. Integrarlos cuando se disponga de la ultima version local completa.
10. memoria.md no esta en el repositorio: Carlos debe subir el original (rev. 0) para anotarle las medidas reales.

## Otras tareas pendientes del juego (sin cambiar la hoja de ruta)
- 2e-4b: reorientacion hacia la casa.
- Animaciones de giro (con diagonales) y de caida definitivas; luego Trog basico.
- Invencibilidad y parada ante el borde en el esqueleto nuevo (is_inv, edge_stop, dwait), hoy solo en la version previa.
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
- Siguen pendientes: GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, guion REXDOORMAT_x, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROG_START, TROGSPEEDPID, TROGBAR.ASM y WAVEHAND, y las rutinas de giro y de animaciones de TROGDINO.ASM. PUNCHED/DINO_PUNCHED/SPIT_FIRE disponibles pero sin analisis completo autorizado.
- Confirmar revision LA5 del fuente archivado y ROCKPOST1..4.

## Documentacion y continuidad
- decisions_render_update.md, decisions_render_update_2.md, decisions_render_update_3.md y testing_render.md son suplementos: NO sustituyen decisions.md revision 15 ni testing.md revision 12.
- analysis/render_progress.md conserva las tablas anteriores (R1-R6); las mediciones del banco estan en analysis/render_mediciones_r6_2.md; el paso 1 de la integracion, en analysis/extraccion_render_core.md.
- analysis/integracion_inventario.md: inventario de la version previa del juego, puntos de contacto con el renderer y plan revisado.
- continuation.md es el mensaje de traspaso (revision del 16-oct, 04:10; no actualizado en esta revision).
- memoria.md revision 0 es borrador, no presupuesto cerrado.
- El repositorio contiene el codigo del port en src_z80/; la copia local de Carlos (src/) es la copia de trabajo y puede ir por delante.
