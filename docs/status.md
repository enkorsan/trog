# Estado del proyecto Trog ZX

Version: 17-oct-2026, revision 22. Sustituye rev.21. El asistente no ejecuta sjasmplus ni emulador: las pruebas son de Carlos. No sustituye decisions.md rev.15 ni testing.md rev.12 ni cambia los hitos.

## Entorno
Spectrum 128K convencional, sjasmplus1.24.0, LA5 en MAME. Carlos mantiene graficos y pruebas; local src/, repositorio src_z80/. Fuente original intacto. main.asm y modulos antiguos se conservan como referencia de logica.
RVM128K toastrack y modelo48K; no mezclar Pentagon. Version de RVM pendiente. Scripts build_game_r7.bat y build_game_r7_48.bat en raiz, usan src/ y generan SNA/LST/SYM en build/. -DTARGET_48K selecciona48K y macros sin paginacion.

## Probado en main
- game_r7.asm rev.3, paso2a: render.inc, render_core.inc, dino_r7.inc, decor_r7.inc y eggs_r7.inc. ENT_COUNT=1, espera por contador sin catch-up, velocidad26/64 y pasos2px.
- Paso1 (Carlos16-oct): OPQA, giros directos, caida animada sin parpadeo, desaparicion/reaparicion, sin restos ni perdidas observadas en128K/48K. Ocho intervalos en4,75-4,80s frente a4,78s de MAME.
- Paso2a (Carlos17-oct00:45): ensamblado,43 huevos, recogida con fondo liso y patron, restauracion correcta, sin basura ni perdidas observadas en ambos modelos. Listados leidos por el asistente.
- Huevos en fondo limpio bajo sprites, restaurar patron no ceros. decor_push3: teselas3bytes x1..42lineas, FIFO8, una preparada por iteracion; cada huevo modifica2x16 y recompone3x16. Subconjunto de decor_rect; no ancho variable/teselado general.
- Ultimo huevo43/43, all_eggs=1; sin casa/salidas/victoria. Reaparicion de main aun arriba izquierda.
- EGG_TEST_FLOOR=1 como listados probados; poner0 y recompilar para suelo normal. Comentarios compactados sin modificar instrucciones/datos.

## Propuesta local 2b
Paquete entregado: game_r7.asm rev.4, dino_r7.inc modificado y respawn_r7.inc. SIN ensamblar ni probar en Z80; no sustituye main hasta prueba de Carlos. Reutiliza ai.inc blob f1df8a3: nodo extremo de la MISMA calle de caida, rumbo segun huevos, recogida del huevo del nodo DESPUES. No teletransporta al huevo mas cercano. Inicio dino_reset separado; rearma espera/acumulador/peticion. Solo caidas, sin muerte en el sitio, invencibilidad, parada ante borde ni direccion hacia casa.
Interprete Python reducido del fuente frente a modelo independiente:1876 caidas y un inicio sin diferencias. No es binario Z80 ni perfil CPU ni validacion LA5. Protocolo en paquete local. Subir2b tras prueba.

## Memoria y decisiones
Paso2a con trama: fin exclusivo $A1F4/$A1ED en128K/48K; hasta SCR_BUFS=$B100 quedan3852/3859bytes. No margen cerrado ni medida de2b. FLOOR_SAVE=$D800-$DD9F,45x32=1440bytes, solo43 nodos inicializados; banco0/RAMplana48K; solo pixeles, sin atributos.
Mapa: buffers$B100, STAT_BUF$B500, ENTS$B580, QPOS$B680, ZLIST$B690, IM2$B800, fondo$C000(6144bytes), pila$7FFF contendida. ENT_COUNT>1, prioridad con vecinos y SPR_BANK!=2 pendientes en integracion. R6.x independientes: analysis/render_mediciones_r6_2.md; sus margenes no se trasladan al juego.
Suplementos decisions_render_update.md y _2rev.2,_3rev.1,_4rev.0; testing_render.md rev.3. Analisis integracion_inventario.md, extraccion_render_core.md, tamanos_sprites.md, trex_casa_animaciones.md y huevos_paso2a.md. No confundir transportes de TROGHAUS con salida del nivel; cifras provisionales por revisar.
ClaseA hito1: dino/Trog16px ancho, alto24, ventana3bytes. Garrotes ajustar dibujo;4bytes posible no decidido, medir CPU. T-Rex reemplaza dino con pina; objetivo60%,unos32px alto; ruta/clase especial propuesta, ancho/buffer/animaciones sin cerrar. Suelo con dibujos deseado. Casa y letrero decorado bajo sprites aprobado; apariciones Trogs/pozas/fosos igual SI no colisionan (verificar flags). Giro90 una diagonal; media vuelta inicial/diagonal/perpendicular/diagonal/final con sentido aleatorio en cuatro direcciones. Punetazo un cuadro/direccion, tiempos e hitboxes pendientes. Generar fases al cargar: estudiar, ahorra carga no RAM final, sin medir.
128K cargar todo si cabe y AY;48K base+cargas posteriores y posibles recortes, sin adelantar hito5. AY/menus/puntuacion completa en hito3.

## Pendiente
1. Probar2b y recibir listados; proteccion al reaparecer/parada ante borde con prueba separada.
2. Casa/letrero, salidas, victoria/reinicio y reorientacion hacia casa(2e-4b); Trogs placeholder, vecinos y medidas completas. Completar decor_rect segun necesidades, sin nuevo motor. Actualizar suelo guardado si cambia durante nivel.
3. DINOTURN/DINOLOOK_TURN/ANIMATE_TURN y consumidor de punetazos: no fijar tiempos sumando guiones. TROG_PIT/TINIT_TAB: colision de apariciones. Diagonales4..7 y sprites especiales sin probar.
4. Pantalla sombra, pila banco2, cuenta dinamica, sort de un byte y M1 datos frios: no adoptados. TROGSPEEDPID/TSPEEDUPS/INITSPEEDUPS, DSPEED2, POWERUP_DURATION, limites T-Rex, muelles/flor/seta/fuego/ataques pendientes. Tocarse sin atacar no causa dano por si solo(Carlos).
5. Confirmar revisionLA5, ROCKPOST1..4, ADDEMOVE/deduplicacion. EXITPOINTS: version activa no comentada,28 salidas deducidas para rejilla no contadas ejecutando arcade. GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROGBAR y WAVEHAND sin analisis completo.
6. Fuente/HUD definitivos, catalogo, bancos/ZX0, aleatoriedad inicial, presupuesto y memoria.md originalrev.0 pendientes. Consolidar decisions/testing sin perder revisiones. continuation.md anticuado(16-oct04:10); manda este estado y conversacion reciente.
