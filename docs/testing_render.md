# Pruebas del renderer R1-R6.3

Version: 16-oct-2026, revision 2 del suplemento a testing.md revision 12.
No sustituye los procedimientos ni regresiones del juego del documento anterior.

## Entorno
- sjasmplus 1.24.0.
- RVM, ZX Spectrum 128K original toastrack. Version del emulador aun sin registrar.
- Carlos compila con build_bench.bat (ejemplo: build_bench.bat src/bench_r6_integracion). El nombre del .sna lo fija el SAVESNA del fichero. Para tamano y direcciones se ensambla con --lst y --sym.
- Carpeta local src/, GitHub src_z80/.
- El asistente no ha ensamblado ni ejecutado emulacion; los resultados reales son los informados por Carlos.

## Estado de validacion
| Banco | Ensambla / ejecuta | Observaciones |
|---|---|---|
| R1 a R5 | confirmado por Carlos | como en la revision 1 |
| R6 | confirmado por Carlos | Trogs distintos, acciones y caidas |
| R6.1 y R6.2 | confirmado por Carlos | instrumentacion corregida, MG, WN, STOPVENT, BURN, SKIP; tablas en analysis/render_mediciones_r6_2.md |
| R6.3 | ensamblado por Carlos; HUD de la mitad derecha confirmado visualmente (16-oct) | copia local src/bench_r6_integracion.asm |

## Correcciones que debe contener la copia vigente
1. init_ents: PUSH BC / PUSH HL / CALL ent_set_qpos / POP HL / POP BC.
2. Bucle largo de init_ents: DEC B / JP NZ,.l en lugar de DJNZ .l.
3. ent_blit: tres DEC DE en lugar de tres DEC E tras los tres LDI y JP PO,.done.
4. draw_clipped fuera de IF BENCH_SLOW (con BENCH_SLOW = 0 daba Label not found); sort_z sigue dentro de IF BENCH_SLOW.
5. Las constantes BENCH_STOPVENT, BENCH_SKIP_BLIT, BENCH_SKIP_SORT y BENCH_BURN deben estar definidas en el bloque BENCH_* antes de usarse. BENCH_SKIP_BLIT debe envolver el volcado del bucle principal, no solo el de start.

## Instrumentacion vigente (R6.2 y R6.3)
- Ventanas de 64 iteraciones medidas; HUD escalonado y exclusion de las iteraciones con HUD mediante skip_acc para MG, WN y MINA.
- MG: media de las 64 primeras ventanas. WN: peor media de ventana. MINA: menor MIN desde el arranque. SOLX: maximo de ventanas con solape. PC: deficit de fotogramas sin HUD. LT: iteraciones tardias sin HUD.
- MG y WN en unidades de 70 T; MINA en 35 T. Son aproximaciones.
- BENCH_STOPVENT congela el HUD al llegar a VENT = N (0 = no parar).
- BENCH_BURN anade 24 T por vuelta (292 vueltas = unos 7.000 T). BENCH_SKIP_BLIT y BENCH_SKIP_SORT quitan una fase y son solo de medida.
- Ponga BENCH_BURN, BENCH_SKIP_BLIT y BENCH_SKIP_SORT a 0 para medir una base.
- cnt_hud y def_hud se acumulan en memoria y no se muestran.

## Protocolo de medida
1. Misma configuracion y mismo BENCH_STOPVENT (64).
2. Anotar N, E, S, P, G, A, F, B, V, VENT, BURN, SKIP_BLIT, SKIP_SORT, MG, WN, MINA, SOLX, PC y LT.
3. Una ejecucion base y las de carga artificial en la misma sesion.
4. No mezclar mediciones de maquinas distintas ni de compilaciones distintas sin anotarlo (SX distinto entre compilaciones: ver analysis/render_mediciones_r6_2.md).
5. Tabla base: N5 E1 S1 P0 G0 A0 F0; N5 con A1; N5 con F1; N5 E2 P1 A1 F1; N8 E2 P1 A1 F1.
6. Margen del peor caso: N8 con BURN 0, 146, 292 y 584 (3.500, 7.000 y 14.000 T extra).

## Regresiones visuales del renderer
- Mascaras correctas sobre piedras, agua y vecinos.
- Prioridad estable: mayor Y y dino con igual Y.
- No rectangulos ni restos al cruzarse.
- Derecha: x cercano a 236/240, ventanas en columna 29; comprobar varias filas de pantalla.
- Arriba: y=28 y umbral de caida y=26, sin tocar HUD.
- Acciones en el sitio redibujan aunque X/Y no cambien.
- Al desaparecer, restaurar fondo y vecinos; no dejar imagen antigua.
- Al reaparecer lejos, solo ventana nueva; no limpiar un rectangulo enorme entre ambas posiciones.
- Verificar solapes con entidades quietas.
- HUD de depuracion: legible en la mitad derecha, sin invadir la izquierda.

## Pruebas pendientes
- SKIP_SORT 0/1 repetido en la misma sesion con BURN anotado.
- Pila fuera del banco 5 y comparacion de MG.
- Variante 48K (DEVICE ZXSPECTRUM48) y RVM modelo 48K.

## Al integrar en el juego
Ademas de lo anterior: teclado, giros, caidas (animacion sin parpadeo), paradas e invencibilidad, recogida de huevos y contador, casa y letrero, llegada, victoria y reinicio. Las animaciones del bench no validan las reglas arcade ni sustituyen pruebas de juego.

Tablas completas en analysis/render_progress.md (anteriores) y analysis/render_mediciones_r6_2.md (R6.2 y R6.3).
