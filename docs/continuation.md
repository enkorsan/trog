# Continuacion en otra conversacion: Trog ZX, integracion del renderer en el juego

Actualizado: 16-oct-2026, 04:10.

## Primer mensaje sugerido
Continuamos Trog ZX: integrar el renderer (R6.3) en el juego. Lee primero, desde el repositorio enkorsan/trog (rama main): docs/status.md (rev. 20), docs/decisions_render_update_2.md (rev. 2, decisiones del 16-oct), docs/analysis/integracion_inventario.md, docs/analysis/render_mediciones_r6_2.md y docs/testing_render.md (rev. 3). El codigo esta en src_z80/ (local: src/). src_z80/bench_r6.asm es el R6.3; la copia local es src/bench_r6_integracion.asm.
Primera tarea: paso 0 del plan, el contrato del renderer (render.inc): entidades, rectangulo de decorado compuesto por prioridad, espera por contador, macros de paginacion que no generan codigo en 48K, direcciones por EQU y velocidad con modulo 64. Documentarlo antes de codificar y despues construir el esqueleto con el dino como entidad 0. No empezar otro motor ni declarar margen cerrado del juego.

## Decisiones ya cerradas (16-oct)
- Se adapta el juego al renderer; el codigo anterior de src_z80/ es la version previa (dibujo directo, HALT) y se reutiliza solo su logica (node_offset, update_request, ai.inc, place_eggs, egg_pickup, house_select, exit_tab, house_on_street, read_keys, inv.inc), no su dibujo.
- Un paso de logica por iteracion, con espera por contador y sin catch-up.
- Huevos como decorado del fondo limpio (banco 0, $C000, 6.144 bytes) con guardado del suelo bajo cada nodo (32 bytes por nodo, unos 1,4 KB); la retirada restaura ese patron y vuelca un rectangulo sucio. EGG_GROW queda para mucho mas adelante.
- Escalonado: fase_i = floor(64 * i / N).
- Velocidad base: acumulador de modulo 64, base 26 (13/32 de 2 px por fotograma), igual para dino y Trogs; ajustable despues. Evidencia en decisions_render_update_2.md.
- Caida por el borde: animacion sin parpadeo. Prioridad: mayor Y encima; dino encima con igual Y; empate de Trogs provisional.
- HUD de depuracion en la mitad derecha (columnas 16-31, MG, WN, MN, SX, PC, LT); la izquierda queda libre para puntos, huevos, vidas.
- 48K deseable: si hace falta un banco extra de 128K es aceptable.
- Sprites 16x24 sin reducir, cuatro fases horizontales (0, 2, 4, 6 px), 144 bytes por imagen y fase. La H de la fuente de Carlos no se corrige.

## Datos medidos de referencia (RVM 128K toastrack, V7, HUD congelado en VENT = 64)
- N8 E2 P1 A1 F1: MG 510 (unos 35.700 T libres de media), WN 228, LT 21, PC 0. Sin deficit sostenido hasta +7.000 T por fotograma; con +14.000 T, PC = 2. N5: MG 613-725, PC 0.
- Memoria del R6.3: datos hasta $A50E, margen de 4.850 bytes hasta IM2_TABLE ($B800). Pila en $7FFF (banco 5, contendido; hipotesis de coste sin medir).
- Estimacion estatica (asistente): una ventana de movimiento cuesta unos 11.000 T y cada entidad unos 500 T fijos por fotograma.
- Velocidad: teoria del arcade 4,66 s para 8 intervalos; lectura en MAME 4,78 s; Spectrum con 7/16, 4,51 s.

## Pendiente
- Leer TROGSPEEDPID, TSPEEDUPS e INITSPEEDUPS (TROGTROG.ASM): si la velocidad del Trog cambia durante una oleada en LA5.
- Si un huevo puede tapar al dino por Y (observar en MAME).
- Variante 48K del banco; prueba de la pila en zona no contendida; pantalla sombra (no descartada).
- Subir memoria.md (rev. 0 original) al repositorio; decisions.md y testing.md no se han tocado (los cambios estan en los suplementos).

## Reglas de trabajo
- Distinguir siempre codigo propuesto, revisado y probado. El asistente no ha ejecutado sjasmplus ni un emulador; las pruebas reales son las de Carlos. Los modelos Python y los scripts de etiquetas no sustituyen a ensamblar.
- Tocar el repositorio solo con confirmacion explicita de Carlos y en commits pequenos.
- No modificar el fuente original del arcade (raiz y carpetas LA3, LA5, NES1, NES2, IMG, SOUND).

## Archivos que Carlos debe adjuntar si el asistente no puede leerlos del repositorio
src/bench_r6_integracion.asm, los .bat locales, memoria.md (original rev. 0) y el listado y los simbolos si se vuelve a ensamblar.
