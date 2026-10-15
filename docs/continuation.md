# Continuacion en otra conversacion: Trog ZX renderer

Actualizado: 15-oct-2026, 00:44, tras confirmacion de Carlos de que DEC DE corrige el borde derecho en R5/R6.

## Primer mensaje sugerido
Continuamos Trog ZX desde las pruebas de render R1-R6. Lee primero los archivos adjuntos y estos documentos. No reconstruyas los benches desde mensajes antiguos. La copia vigente de Carlos tiene las correcciones de BC en init_ents, DEC B / JP NZ en el bucle de inicializacion largo y tres DEC DE en ent_blit. Carlos confirma que funcionan.
Siguiente tarea: corregir instrumentacion para separar muestras sin HUD y perdidas totales; luego comparar configuraciones de igual duracion y evaluar integracion incremental. No empezar otro motor ni declarar margen cerrado del juego.

## Documentos adjuntos
- status.md rev.17: estado del juego y renderer, siguiente tarea.
- decisions_render_update.md: suplemento, NO reemplaza decisions.md rev.15.
- testing_render.md: suplemento, NO reemplaza testing.md rev.12.
- analysis/render_progress.md: todas las tablas y correcciones.
- memoria.md rev.0: adjuntar el original de Carlos; es un borrador.

## Archivos que Carlos debe adjuntar
Para corregir R6: bench_r6.asm vigente, config.inc, video.inc, sprite_dino.inc, sprite_trog.inc, stones.inc, font_msg.inc y BAT real de compilacion; recomendable el script make_trog_placeholder.py y bench_r5.asm corregido si se repetira la comparacion.
Para integrar despues: ZIP de todos los modulos actuales del port, herramientas y documentos locales vigentes, sin mezclar con el arcade original.

## Resumen de referencia
- sjasmplus 1.24.0; RVM 128K toastrack; resultados iniciales R1 en Spectaculator fueron Pentagon.
- Fondo limpio lineal banco 0, ventanas banco 2, composicion con mascara y prioridades, volcado al comienzo de siguiente iteracion/fotograma.
- Dino entidad 0; Trogs placeholder 16x24; prioridad por Y, dino encima con igual Y; empate de Trogs provisional.
- E_DIRTY: posicion/imagen/presencia; E_IMG separado de E_DIR.
- SLOW si, escalonado si; perdidas puntuales sin parpadeo toleradas, no lentitud sostenida.
- Renderer no integrado aun; caidas y acciones del bench son pruebas sinteticas, no comportamiento arcade final.
- La caida definitiva NO parpadea. Retirar la entidad despues de la animacion no es un estado de parpadeo.
- No reducir sprites por ahora. No alterar la H de Carlos. Velocidades por calibrar; velocidades de Trogs por leer en fuente.
- PERD actualmente omite iteraciones excluidas por HUD. MINA=0 con PERD=0 no es evidencia de ausencia absoluta de perdidas.
- La tabla R5/R6 fue medida antes del ultimo arreglo del borde; no hay mediciones posteriores todavia.
- IM2 actual solo cuenta frames; AY sin elegir ni implementar. Preservar registros y paginacion; revisar atomicidad del espejo $7FFD/OUT.
- Original arcade por conector GitHub; port por los ultimos archivos aportados. No escribir ni modificar el arcade.
