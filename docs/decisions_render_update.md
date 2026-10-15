# Suplemento de decisiones: renderer y continuidad

Version: 15-oct-2026, suplemento 1 a decisions.md revision 15.
No sustituye ni elimina las decisiones generales, hoja de ruta o acuerdos anteriores. Integrar sus cambios en decisions.md cuando se disponga de la ultima version local completa.

## Decisiones y preferencias confirmadas por Carlos
- Mantener sprites de aproximadamente 16x24 px; no reducirlos como primera medida de rendimiento.
- Resolver solapes (SLOW=1) y escalonar movimientos. Perder fotogramas de forma puntual sin parpadeo es tolerable; no se ha aprobado ralentizacion sostenida.
- Pruebas de rendimiento en RVM 128K toastrack; sjasmplus 1.24.0.
- La caida definitiva sera una animacion al agua, sin parpadeo. El parpadeo de muerte del juego actual es provisional.
- La H de la fuente de Carlos es deliberada y coincide con el arcade: no modificarla.
- Velocidades y colisiones se calibraran/implementaran mas adelante; no asumir que la velocidad de Trogs sea constante.

## Prioridad: observacion de Carlos frente a regla experimental
Observacion en MAME:
- Misma X y distinta Y: el situado mas abajo queda encima.
- Misma Y: dino encima.
- Entre Trogs en la misma fila no encuentra patron.
- Acciones como comer parecen conceder prioridad al actor; analizar mas adelante.
Regla experimental: orden ascendente por (Y, desempate), mayor prioridad pintada al final. Dino entidad 0 con desempate 255; Trogs con desempate fijo por indice. La generalizacion a todo solape es provisional; Carlos confirma que visualmente funciona en los benches.

## Arquitectura evaluada, aun no integrada
- Fondo limpio lineal en banco 0; ventanas en banco 2; preparacion y volcado separado.
- Ruta rapida para entidad propia; vecinos de menor prioridad antes y de mayor despues, recortados a la ventana.
- Datos de sprites paginables en un banco seleccionado por prueba. No se ha validado aun el reparto final de todos los conjuntos del juego.
- E_DIRTY abarca cambio de posicion, imagen y presencia. E_IMG independiente de E_DIR.
- Desaparicion tras caer significa retirar la entidad y dejar de incluirla como vecina, no parpadear. Reaparicion lejana genera una ventana nueva, no una union gigantesca.
- El suelo puede incorporar decoracion al fondo limpio. erase_egg_area del juego, que pone ceros, habra que sustituirlo por restauracion correcta de terreno al integrar.

## Lo que NO queda decidido
- La pantalla sombra no esta descartada. No se acepta la afirmacion general de que el doble buffer duplique siempre el coste: depende de la politica de actualizacion, estado persistente de cada pantalla, animaciones y numero de zonas cambiadas. Comparar con presupuesto real si resulta util.
- No declarar rendimiento suficiente para el juego completo solo por MEDIA alta: las acciones tienen picos y falta IA, sonido, colisiones, HUD y cambios de fondo.
- Las diferencias de bancos de R5 no son una medicion concluyente de contencion.
- El hito 48K no condiciona automaticamente la arquitectura inicial de 128K; sigue siendo posterior y sujeto a recortes aceptables.

## IM2 y AY: propuesta, no implementacion aceptada
- IM2 de benches solo actualiza frame_counter.
- Propuesta previa: reproductor AY a ritmo de interrupcion, teclado en bucle principal. Elegir reproductor y medir antes de fijar su ubicacion.
- El trabajo en IM2 consume CPU y desplaza el inicio del volcado; no es gratis.
- Preservar registros y paginacion; pila/ISR fuera del banco paginado.
- Revisar atomicidad del espejo de $7FFD y OUT: una interrupcion entre ambos puede ver estado incoherente. No implementar solo con una variable escrita antes del OUT sin proteccion/diseno.

## Correcciones de afirmaciones anteriores
- DJNZ NO modifica banderas; DEC B si. El reemplazo DEC B / JP NZ es adecuado en el bucle de inicializacion concreto, no identico en general.
- Acumuladores pseudoaleatorios con semilla fija no producen necesariamente pruebas distintas al reiniciar con identica configuracion; no afirmar lo contrario.
- Modelos Python validan algoritmos, no instrucciones Z80 ni su temporizacion.
- Las estimaciones de ciclos publicadas durante la exploracion no son medidas exactas y algunas se corrigieron al identificar Pentagon.
