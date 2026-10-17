# Estado del proyecto Trog ZX

17-oct-2026, revision23. Sustituye rev22; no cambia hitos ni decisions.md rev15/testing.md rev12. Pruebas de Carlos; asistente sin ejecucion de sjasmplus/emulador.

## Base sincronizada
main: src_z80/game_r7.asm revision4, dino_r7.inc y respawn_r7.inc del2b; renderer/huevos existentes sin cambio de logica. Fuentes cotejados con adjuntos/listado2b128K. dino_r7.inc recuperado completo del listado. Comentarios compactados; instrucciones/datos conservados. Ya no usar revision3 ni pedir otra vez los fuentes subidos.
Spectrum128K convencional, sjasmplus1.24.0, LA5/MAME. Local src/, GitHub src_z80/. Scripts raiz build_game_r7.bat y build_game_r7_48.bat generan SNA/LST/SYM en build/; -DTARGET_48K sin paginacion. RVM128Ktoastrack/48K; no mezclar Pentagon; version RVM pendiente. Fuente arcade intacto. main.asm y modulos antiguos son referencia de logica.

## Probado
Paso1: OPQA,giros directos provisionales,caida animada sin parpadeo,desaparicion/reaparicion,sin restos/perdidas observadas128K/48K; ocho intervalos4,75-4,80s frente a4,78s MAME. Velocidad26/64,pasos2px,espera por contador sin catch-up.
Paso2a (17-oct00:45):43huevos,recogida con suelo liso/patron,restauracion correcta,sin basura/perdidas observadas. Ultimo43/43/all_eggs=1,sin casa aun. decor_push3:tesela3bytes x1..42lineas,FIFO8,una preparada/iteracion;subconjunto de decor_rect,no ancho variable. EGG_TEST_FLOOR=1 como probado;0 para suelo normal.
Paso2b (17-oct02:14):Carlos confirma todo segun lo esperado,probado/validado dentro de alcance. Nodo extremo de MISMA calle de caida,rumbo antes de recoger huevo del nodo,no teletransporte al huevo. Inicio separado,rearme de espera/acumulador/peticion. SIN invencibilidad,parada ante borde,muerte en sitio ni rumbo hacia casa.
ENT_COUNT=1: no extender validacion a vecinos,prioridad,SPR_BANK!=2 ni diagonales4..7.

## Medidas
Listado2b128K recibido (203410bytes),EGG_TEST_FLOOR=1:fin exclusivo$A410,3312bytes hasta$B100. No listado48K nuevo2b; prueba funcional informada,no medida memoria48K. Cifras2a$A1F4/$A1ED no son del2b. Suelo guardado$D800-$DD9F,45x32=1440bytes,43nodos inicializados;solo pixeles,actualizar patron si cambia suelo.
Mapa:buffers$B100,STAT_BUF$B500,ENTS$B580,QPOS$B680,ZLIST$B690,IM2$B800,fondo$C000(6144bytes),pila$7FFF contendida. No margen cerrado ni perfil completo. Referencia benchR6.x en analysis/render_mediciones_r6_2.md,no trasladar margenes al juego.

## Continuidad y decisiones
Leer continuation.md rev2 y analysis/reaparicion_paso2b.md (incluye auditoria y pruebas). Conservados suplementos decisions_render_update.md,_2rev2,_3rev1,_4rev0; testing_render.md rev3 y analisis de inventario/extraccion/huevos/tamanos/T-Rex/casa. Algunas cifras/interpretaciones de esos analisis siguen provisionales; no confundir transporte TROGHAUS con salida de nivel.
ClaseA:dino/Trog16px ancho,24alto,ventana3bytes;4bytes posible no adoptado,medir CPU. Suelo con dibujos deseado. Casa/letrero decorado bajo sprites;apariciones Trogs/pozas/fosos igual SI no colisionan (flags pendientes). Giro90 una diagonal;media vuelta inicial-diagonal-perpendicular-diagonal-final,sentido aleatorio4direcciones. Punetazo1cuadro/direccion;tiempos/hitboxes pendientes. T-Rex sustituye dino con pina,objetivo60%,unos32px alto;ancho/buffer/ruta/animaciones pendientes. Generar fases al cargar por estudiar,no medido. CPU prioritaria.128K todo si cabe+AY;48K base+cargas posteriores/recortes,sin adelantar hito5. AY/menus/puntuacion completa hito3.

## Siguiente tarea y pendientes
Primero recuperar invencibilidad/parada desde ai.inc:dino_arm,inv.inc,dino.inc:check_fall,config.inc;mantener rumbo probado y probar cambio pequeno128K/48K. Luego casa/salidas/victoria/reinicio y rumbo hacia casa(2e-4b);Trogs/vecinos/medidas completas. Completar decor_rect segun necesidad,sin nuevo motor.
Resto:consumidor de giros/punetazos (no fijar tiempos sumando guiones),TROG_PIT/TINIT_TAB;mascaras/tamanos especiales;pantalla sombra/pila banco2/cuenta dinamica/sort1byte/M1no adoptados;TROGSPEEDPID/TSPEEDUPS/INITSPEEDUPS,DSPEED2,POWERUP_DURATION,limites T-Rex e items/ataques pendientes. Tocarse sin atacar no causa dano por si solo(Carlos).
Confirmar revisionLA5,ROCKPOST1..4,ADDEMOVE/deduplicacion;28salidas deducidas no contadas ejecutando arcade,usar EXITPOINTS activo. GET_EXIT_ANI,WAIT_WAVE_END/WAVE_END,OUTEGGS/CNT_CMOVEEGG_*,DINOCOLL/DINO_TROG_OOF,SKIP_ANIMATION,STOPPED,CENTER_CHECK,DINOS_NEAR_PNT,TROGBAR,WAVEHAND sin analisis completo. Fuente/HUD,catalogo,bancos/ZX0,aleatoriedad/presupuesto pendientes. memoria.md ya existe:leer antes de actualizar. Consolidar decisions/testing sin perder revisiones.
