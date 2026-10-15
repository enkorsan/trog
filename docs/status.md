# Estado del proyecto Trog ZX

Version: 15-oct-2026, revision 17.
Base documental: status.md revision 16 (13-oct-2026), decisions.md revision 15, testing.md revision 12 y memoria.md revision 0; actualizado con la conversacion de desarrollo y pruebas hasta el 15-oct, 00:44.
Este documento sustituye al status.md revision 16 del proyecto. No actualiza automaticamente el fork de GitHub ni los archivos locales de Carlos.

## Plataforma y herramientas
- Port logico de Trog LA5 para ZX Spectrum 128K convencional, Z80 y sjasmplus.
- sjasmplus 1.24.0 confirmado por Carlos.
- Referencia de rendimiento: RetroVirtualMachine, ZX Spectrum 128K original toastrack.
- Spectaculator abre los SNA generados como Pentagon. Fuse intenta Pentagon y termina en 48K si falta su ROM. RVM mantiene la maquina seleccionada.
- No mezclar mediciones de Pentagon con las del 128K.
- Carlos ensambla y prueba localmente. El asistente no ha ejecutado sjasmplus ni un emulador.

## Juego: hecho y probado por Carlos
- Base de movimiento y pantalla hasta 3g, huevos 2d y ajustes de recogida/reaparicion descritos en los documentos anteriores.
- Codigo dividido en modulos .inc. Carlos confirmo igual SHA-256 del snapshot antes y despues del corte puro.
- Dino provisional de 16x24, cuatro orientaciones y cuatro fases horizontales.
- Casa: seleccion de salida, construccion al recoger el ultimo huevo o pulsar B, extension provisional y letrero HOME/REX.
- Parpadeo del letrero a velocidad constante aplicado: HOUSE_FLASH_FRAMES = 21; retirados HOUSE_FLASH_START/MIN y house_interval.
- Salidas funcionales confirmadas por Carlos el 13-oct. victory.inc implementa llegada, mensaje y reinicio provisional. No se ha documentado una matriz exhaustiva de prueba de los cuatro lados, cinco mensajes y regresiones.
- Optimizacion del letrero aplicada: no borrar/repintar el dino cuando no se solapa con el letrero. La primera aparicion y los solapes todavia pueden producir picos de dibujo en el renderer del juego.

## Renderer: bancos de pruebas independientes
- R1: guardar fondo individual, borrar y pintar; referencia de coste. Suciedad con solapes.
- R2: composicion en ventanas y volcado diferido; solapes aun incorrectos.
- R3: composicion de vecinos con prioridad y medidas escalonadas. Carlos confirma prioridades correctas.
- R4: entidad propia por ruta rapida; vecinos recortados antes/despues; fondo lineal; bucles de 1/2/3 bytes; acumuladores pseudoaleatorios y paradas.
- R5: recorte vertical, celdas adicionales, sprites en bancos seleccionables y HUD estadistico.
- R6: dino distinto de Trogs; E_DIRTY por posicion, imagen o presencia; acciones en el sitio; caida, desaparicion y reaparicion lejos. Acciones y caidas usan imagenes provisionales, no las animaciones definitivas.
- Todos ensamblados y ejecutados por Carlos tras las correcciones descritas en analysis/render_progress.md.
- Ultima correccion: tres DEC DE, no DEC E, en ent_blit de R5/R6. Carlos confirma que ya funciona; las tablas registradas se tomaron antes de este arreglo.
- El renderer experimental todavia NO esta integrado en el juego.

## Arquitectura experimental vigente
- Fondo limpio lineal: banco 0, $C000 + Y*32 + columna_byte, 6144 bytes de pixeles.
- Buffers de ventanas residentes en banco 2 y alineados a 128 bytes.
- Ventana habitual de 3 bytes por 24/26 lineas para movimientos de 2 px.
- Preparacion: fondo + sprites con mascara, respetando prioridad. Volcado al comienzo de la siguiente iteracion/fotograma.
- R5/R6: dos pasadas para permitir paginar fondo y datos de sprites por separado.
- No confundir con el fondo anterior en layout de bitmap Spectrum.

## Pendiente inmediato
1. Leer el bench_r6.asm vigente de Carlos, sin reconstruirlo desde mensajes antiguos.
2. Corregir instrumentacion: skip_acc excluye tambien perdidas de las iteraciones con HUD; PERD no es un total absoluto de todas las iteraciones.
3. Separar MEDIA/MINA sin coste del HUD de perdidas totales. MIN=0 puede proceder de redondeo de una espera muy corta.
4. Comparar pruebas de igual duracion y configuracion, incluida velocidad; repetir comprobacion visual del borde derecho corregido.
5. Evaluar margen con acciones/solapes y futuras cargas; decidir despues la integracion incremental.

## Otras tareas pendientes del juego (sin cambiar la hoja de ruta)
- 2e-4b: reorientacion hacia la casa.
- Animaciones de giro tras el bloque de casa/salidas; luego Trog basico.
- Velocidad del dino: Carlos cree que es algo mas lento en arcade; medir mas adelante.
- Verificar velocidades de Trogs y variaciones posibles en el fuente original. No asumir velocidad constante.
- Colisiones/ataques: punetazos del dino y del Trog; Carlos indica que tocarse sin atacar no produce dano por si solo.
- Fuente definitiva y migracion del HUD: no dar por aplicada toda la migracion.
- Sonido AY y menus corresponden al hito 3. No hay reproductor elegido ni integrado.
- Aleatoriedad inicial con menu; pruebas de bancos/ZX0, catalogo grafico y presupuesto completo pendientes.

## Analisis del fuente
- EXITPOINTS leido en TROGPOST.ASM: usar la segunda version activa, no la comentada con ****. CALCA1GT/CALCA1LT registran las salidas.
- Geometria de 28 salidas deducida por lectura para la rejilla rectangular; no se ejecuto la construccion arcade para contarla. ADDEMOVE/deduplicacion sigue pendiente de verificacion especifica.
- El nivel 1 no requiere filtrar tipos de salida por lago/pozo/volcan, segun analisis y observacion de Carlos.
- Siguen pendientes, segun el documento anterior: GET_EXIT_ANI, WAIT_WAVE_END/WAVE_END, OUTEGGS/CNT_CMOVEEGG_*, guion REXDOORMAT_x, DINOCOLL/DINO_TROG_OOF, SKIP_ANIMATION, STOPPED, CENTER_CHECK, DINOS_NEAR_PNT, TROG_START, velocidades, TROGBAR.ASM y WAVEHAND. PUNCHED/DINO_PUNCHED/SPIT_FIRE disponibles pero sin analisis completo autorizado.
- Confirmar revision LA5 del fuente archivado y ROCKPOST1..4.

## Documentacion y continuidad
- decisions_render_update.md y testing_render.md son suplementos: NO sustituyen decisions.md revision 15 ni testing.md revision 12.
- analysis/render_progress.md conserva tablas, correcciones y limites de evidencia.
- continuation.md es el mensaje de traspaso para otra conversacion.
- memoria.md revision 0 es borrador, no presupuesto cerrado; Carlos lo aporto y fue leido.
- Consultar la ultima copia de trabajo aportada por Carlos. El repositorio sigue siendo referencia del arcade; comprobar su estado si se necesita una version nueva.
