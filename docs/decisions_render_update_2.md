# Suplemento 2 de decisiones: integracion del renderer

Version: 16-oct-2026, suplemento 2 a decisions.md revision 15. Se suma al suplemento 1 (decisions_render_update.md); no sustituye ninguno de los dos ni decisions.md. Integrar en decisions.md cuando se disponga de la ultima version local completa.

## Decisiones y preferencias confirmadas por Carlos (15-oct, 23:33 y 16-oct)
- Adaptar el juego al renderer actual. El codigo de src_z80/ es la version previa (dibujo directo, sin optimizacion): se reutiliza su logica, no su dibujo. Mantener en lo posible la logica del juego original y el renderer. En la practica se programa el juego de nuevo.
- Compatibilidad 48K deseable (hito 5): si hace falta un banco extra de 128K es aceptable, pero mejor jugable en 48K si se puede. No cambia el orden de los hitos.
- La muerte por caida en los contornos es una animacion sin parpadeo; el parpadeo era de una version anterior.
- HUD de depuracion en la mitad derecha (columnas 16-31, filas 0-2); la mitad izquierda queda libre para puntos, huevos, vidas, etc. Etiquetas: MG, WN (fila 0), MN, SX (fila 1), PC, LT (fila 2).
- Carpetas: src/ en local y src_z80/ en GitHub.

## Propuestas del asistente aun no aprobadas
- Contrato del renderer (render.inc) antes de codificar: entidades, rectangulo de decorado compuesto por prioridad, sincronizacion por contador, direcciones por EQU y macros de paginacion que no generan codigo en 48K.
- Todo lo que cambia el decorado desde el juego (huevos, casa, texto, marcadores) pasa por el fondo limpio del banco 0 y por un rectangulo sucio.
- Plan por pasos pequenos con prueba de Carlos: esqueleto con dino como entidad 0; huevos como decorado; casa y victoria como rectangulos; Trogs placeholder y HUD de depuracion; medida del fotograma completo.

## Metricas del banco (decididas con Carlos)
- MG, WN, MINA, SOLX, PC y LT se leen con el HUD congelado en VENT = 64. Un valor MEDIA tomado en un instante no se usa como rendimiento.
- Margen del peor caso: BENCH_BURN sube la carga en pasos de 24 T por vuelta; se anota la carga con la que PC deja de ser 0.
- PC mide deficit real de fotogramas (n-1); LT cuenta iteraciones tardias. PERD queda sustituido y no es comparable.

## Abiertas
- Sincronizacion: espera por contador (absorbe la iteracion tardia) frente a HALT del juego anterior (pierde un paso de logica). Tiempo de juego por iteracion o por tiempo real.
- Huevos como fondo o como entidad (los documentos apuntan a fondo).
- Pantalla sombra (no descartada) frente al esquema B+.
- Pila fuera del banco 5 (contendido): hipotesis sin medir.
- Ordenacion por claves de un byte: ahorro estimado, sin aplicar.

## Correcciones de afirmaciones anteriores
- MEDIA en un instante no es un valor de rendimiento.
- MINA = 0 con PC = 0 indica iteraciones tardias absorbidas, no ausencia de picos.
- La ablacion de la primera tanda (SKIP_BLIT, SKIP_SORT) no es valida; BENCH_SKIP_BLIT solo envolvia el volcado inicial.
- El instrumento es lineal (BENCH_BURN): una sospecha de error sistematico de 18 unidades fue un malentendido de configuracion.
- Los resultados con PC = 0 y LT > 0 solo valen con espera por contador.
