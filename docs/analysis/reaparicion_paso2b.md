# Reaparicion y auditoria de rumbo: paso2b

17-oct-2026,revision1. Carlos comunica02:14 que todo funciona segun esperado:PROBADO Y VALIDADO dentro del alcance entregado. El asistente no ejecuta sjasmplus/emulador. No equivale a reaparicion arcade completa.

## Procedencia
Z80 anterior src_z80/ai.inc blob f1df8a3: dino_respawn,choose_dir,find_closest,best_direction,dir_valid,best_by_rating. Adaptado a ENT0/egg_map en respawn_r7.inc. game_r7.asm rev4 incluye ese modulo;dino_r7.inc separa inicio de muerte. Renderer sin cambios.
Adjuntos game_r7-3.asm,respawn_r7-4.inc,decor_r7-2.inc y game_r7.lst203410bytes. Fuentes del listado coinciden con adjuntos normalizando blancos. dino_r7.inc recuperado completo del listado. Comentarios/cabeceras compactados sin modificar instrucciones/datos. No listado48K nuevo2b;prueba funcional informada por Carlos.

## Contrato actual
Caida:vuelve al nodo extremo de la MISMA calle conservando eje transversal (x24 izquierda,x216 derecha,y52 arriba,y148 abajo). No teletransportar al huevo mas cercano. Elegir rumbo antes de que game_update recalcule qpos y recoja huevo del nodo. rsp_arm:E_IMG=E_DIR,E_STATE=0,E_VALID=0,temporizador/pasos/pausa/acumulador0,peticion vacia,START_WAIT. Ventana vieja restaurada al ocultarse;nueva al reaparecer.
Solo caidas;SIN muerte en sitio/snap_axis/resp_eat condicional,invencibilidad/parada,casa ni rumbo hacia ella.

## Auditoria del motor anterior
ai.inc:find_closest usa Manhattan desde auxiliar de borde,empate por filas,ignora distancia0. best_direction:eje mayor,vertical en empate,segundo eje,descarta inversa de dinitdir. dir_valid solo limites de rejilla. choose_dir recurre a best_by_rating si no hay objetivo/direccion:2por huevo en recta,mayor longitud(2*n+1),ultimo U,D,L,R en empate. rsp_* conserva estos criterios;no se perdieron al migrar renderer.
ai.inc:dino_arm,inv.inc:tick_inv/is_inv/show_inv y dino.inc:check_fall/update_dino prueban que invencibilidad/parada existian antes. Muerte en sitio usa snap_axis y resp_eat condicional. Aun no migradas. house.inc/victory.inc implementan llegada a casa,pero no amplian choose_dir/best_by_rating. Casa,pina,paredes/peligros no estaban en el rumbo del ai.inc anterior;son ampliaciones,no regresiones de2b.

## Contraste directo con arcade
TROGDINO.ASM:DINO_SKIPANIM busca DINO_FIND_CLOSEST y GET_BEST_DIRECTION (huevo O casa);fallo a DINO_START_SAFE/DINO_FIND_BEST. Primer inicio y modo instrucciones omiten busqueda. Bloques alternativos de posicion/FIND_SAFEST con **** comentados,no activos.
TROGDINO.ASM:DINO_FIND_BEST/DINO_SEARCH_SAFE (tambien archivo de proyecto TROGDINO.ASM.p07_L4264-4743.txt):4direcciones,valoracion,pasos,ultimo en empate. EGG_RATING2,PINEAPPLE_RATING3,WALL_RATING1,HOUSE_RATING5. Huevos propios/comunitarios; casa solo salida propia/comunitaria con EMOVEOBJ existente. TARID/FIREID/PITID cortan exploracion. No ruta global ni evidencia de evitacion de Trogs.

## Validacion, memoria y regresion
Interprete Python reducido de fuente frente a modelo independiente:1876caidas+inicio sin diferencias;no binarioZ80 ni perfilCPU. Carlos valida comportamiento esperado. Listado2b128K con EGG_TEST_FLOOR1:fin$A410,3312bytes hasta$B100. No medida48K2b ni presupuesto cerrado.
Prueba futura:inicio arriba izquierda mirando abajo;caidas4bordes/calles distintas;rumbo segun mapa;huevo bajo reaparicion se recoge despues de rumbo,una vez;sin restos;espera/OPQA posterior/trama/PC-LT. Scripts existentes128K/48K. Proxima tarea:proteccion/parada por separado;despues casa como objetivo/salida valida. Colision logica y dibujo decorado separados.
