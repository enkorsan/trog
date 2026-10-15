# Continuacion en otra conversacion: Trog ZX

Actualizado: 16-oct-2026, 01:30.

## Primer mensaje sugerido
Continuamos Trog ZX desde la integracion del renderer. Lee primero docs/status.md (rev. 19), docs/analysis/integracion_inventario.md, docs/analysis/render_mediciones_r6_2.md, docs/decisions_render_update.md y docs/decisions_render_update_2.md, y docs/testing_render.md. El codigo esta en src_z80/ del repositorio (local: src/). La copia local src/bench_r6_integracion.asm es el R6.3 renombrado y no esta en el repositorio; pidela si hace falta.
Siguiente tarea: paso 0 del plan de integracion (contrato del renderer: entidades, rectangulo de decorado, sincronizacion por contador, macros de paginacion no-op en 48K, direcciones por EQU). Despues, un esqueleto nuevo con el dino como entidad 0 reutilizando la logica del juego anterior. No empezar otro motor ni declarar margen cerrado del juego.

## Resumen de referencia
- sjasmplus 1.24.0; RVM 128K toastrack. Carlos ensambla y prueba; el asistente no ha ejecutado sjasmplus ni emulador.
- Fondo limpio lineal banco 0 ($C000, 6.144 bytes), buffers de ventana en $8C00 (banco 2, alineados a 128), composicion con mascara y prioridad, volcado al comienzo de la siguiente iteracion.
- Prioridad: mayor Y encima; dino encima con igual Y; empate de Trogs provisional.
- Sprites 16x24 (no reducir), 4 fases (0, 2, 4, 6 px), 144 bytes por imagen y fase.
- Caida definitiva: animacion al agua, sin parpadeo. La H de la fuente de Carlos no se corrige.
- Metricas del banco: MG, WN, MINA, SOLX, PC, LT con HUD congelado en VENT = 64. N8: MG 510, WN 228, LT 21, PC 0; sin deficit sostenido hasta +7.000 T por fotograma; +14.000 T: PC 2.
- Mapa de memoria medido del R6.3: datos hasta $A50E, margen de 4.850 bytes hasta IM2_TABLE ($B800). Pila en $7FFF (banco 5, contendido).
- 48K deseable: si hace falta un banco extra de 128K es aceptable.
- HUD de depuracion en la mitad derecha (columnas 16-31); mitad izquierda libre para puntos, huevos y vidas.
- El juego de src_z80/ es la version previa (dibujo directo, HALT); se reutiliza su logica, no su dibujo.
- decisions.md (rev. 15) y testing.md (rev. 12) no se han tocado: los cambios estan en los suplementos. memoria.md rev. 0 no esta en el repositorio.

## Archivos que Carlos debe adjuntar si el asistente no puede leerlos del repositorio
src/bench_r6_integracion.asm, los .bat locales, memoria.md (original rev. 0) y el listado y los simbolos si se vuelve a ensamblar.
