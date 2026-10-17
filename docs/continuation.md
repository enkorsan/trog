# Continuacion de Trog ZX

17-oct-2026, revision2. Sustituye traspaso16-oct04:10 y borrador localrev1.

## Mensaje para otra conversacion
Continuamos Trog ZX. Lee primero docs/continuation.md y docs/status.md desde enkorsan/trog (rama main), y sigue las lecturas y la siguiente tarea alli indicadas. Codigo en src_z80/ (local: src/). No empezar otro motor ni declarar cerrado el margen de memoria o CPU.

## Lecturas obligatorias
1. docs/status.md rev23.
2. docs/analysis/reaparicion_paso2b.md:contrato,auditoria,validacion y limites.
3. src_z80/game_r7.asm revision4,dino_r7.inc,respawn_r7.inc;seguir INCLUDE para renderer,huevos y recursos. Todos estan en main,no pedir adjuntos de nuevo.
4. Para programar ahora:src_z80/ai.inc:dino_arm,inv.inc:tick_inv/is_inv/show_inv,dino.inc:update_dino/check_fall,config.inc (START_WAIT,INV_FRAMES,INV_BLINK,DWAIT_FRAMES,STOP_*,FALL_*,EDGE_TURN,STOP_QUIRK).
5. Contratos:docs/decisions_render_update_2.md rev2,_3.md rev1,_4.md rev0 y testing_render.md rev3;suplementos,no sustituyen decisions.md rev15/testing.md rev12.
Segun necesidad:analysis/huevos_paso2a.md,integracion_inventario.md,extraccion_render_core.md,render_mediciones_r6_2.md,tamanos_sprites.md,trex_casa_animaciones.md,docs/memoria.md.

## Base probada sincronizada
Pasos1,2a y2b validados por Carlos,asistente sin ejecutar sjasmplus/emulador. Huevos43,suelo liso/patron,restauracion sin ceros,sin basura/perdidas observadas128K/48K. Reaparicion2b confirmada17-oct02:14:misma calle de caida,rumbo antes de recoger huevo del nodo;inicio separado,rearme de espera/acumulador/peticion. SIN invencibilidad/parada,muerte en sitio ni rumbo hacia casa. No volver a revision3 ni rehacer otra reaparicion.
Fuentes game_r7-3.asm y respawn_r7-4.inc recibidos llevan sufijos de descarga,no revisiones;game_r7.asm es rev4. dino_r7.inc recuperado del listado2b128K;otros fuentes cotejados con adjuntos. Solo comentarios/cabeceras compactados,logica intacta. No necesitan volver a adjuntarse.
ENT_COUNT=1,no validar por extension vecinos/prioridad/SPR_BANK!=2/diagonales. Pipeline blit_all+blit_decor -> logica -> prepare_all+prepare_decor -> espera,sin catch-up. Velocidad26/64,pasos2px;8intervalos4,75-4,80s frente a4,78s MAME. EGG_TEST_FLOOR=1 diagnostico,0 suelo normal. Ultimo43/43/all_eggs=1,sin casa aun. decor_push3 no es decor_rect general.

## Primera tarea
Recuperar invencibilidad de reaparicion y parada ante borde del Z80 anterior,no su dibujo. Leer funciones indicadas,documentar interfaz y preparar cambio pequeno con prueba128K/48K;mantener posicion/rumbo ya probados. Luego casa/salidas/victoria y rumbo hacia casa;Trogs/solapes y medidas completas. No cambiar hitos.

## Reglas de rumbo y decisiones
2b conserva Manhattan desde auxiliar del borde,eje mayor/vertical en empate,alternativa,inversa de dinitdir descartada,2por huevo en recta,longitud y ultimo U,D,L,R en empate. No solo huevo mas cercano. Motor anterior tenia proteccion/parada y muerte en sitio,aun no migradas. Casa/pina/paredes/peligros no estaban en el rumbo de ai.inc;ver auditoria en reaparicion_paso2b.md.
ClaseA16px ancho,24alto,ventana3bytes;ampliar4bytes no decidido,CPU prioritaria. Casa/letrero decorado bajo sprites;apariciones Trogs/pozas/fosos igual SI sin colision (verificar). Giro90 una diagonal;180 inicial/diagonal/perpendicular/diagonal/final,sentido aleatorio4direcciones. Punetazo1cuadro/direccion,tiempos/hitboxes pendientes. T-Rex reemplaza dino con pina,objetivo60%,unos32px alto;ancho/buffer/animaciones por definir. Preshifts al cargar por estudiar,no medidos.128K todo si cabe+AY;48K base+cargas/recortes,sin adelantar hito5. AY/menus hito3.

## Herramientas y memoria
sjasmplus1.24.0,RVM128Ktoastrack/48K,no mezclarPentagon. Scripts raiz build_game_r7.bat/build_game_r7_48.bat usan src/ y -DTARGET_48K.
Listado2b128K con trama:fin exclusivo$A410,3312bytes hasta$B100. No listado48K nuevo2b,no aplicar cifras2a$A1F4/$A1ED. Suelo guardado$D800-$DD9F(1440bytes). Mapa:buffers$B100,STAT_BUF$B500,ENTS$B580,QPOS$B680,ZLIST$B690,IM2$B800,fondo$C000,pila$7FFF. No margen cerrado ni extrapolar medidas del bench. Pedir listado nuevo solo para medidas/discrepancias,no para leer codigo ya subido.
