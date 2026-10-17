# Huevos como decorado: paso 2a

17-oct-2026, revision 1. ENSAMBLADO Y VALIDADO POR CARLOS en128K/48K (00:45). El asistente no ejecuta sjasmplus ni emulador. Base: sus listados game_r7.lst y game_r7_48-2.lst; comentarios compactados sin cambiar instrucciones/datos.

## Interfaces
Base: game_r7.asm rev.3, dino_r7.inc paso1, render_core.inc sin cambios, decor_r7.inc y eggs_r7.inc nuevos. No incluir eggs_logic.inc antiguo.
eggs_setup guarda suelo sin huevo en FLOOR_SAVE, pinta huevos/contador; build_bg copia pantalla con huevos sin sprites. place_eggs reutiliza distribucion anterior:43 de45 nodos, salida y otro libres. egg_pickup lee ENT0, valida limites/alineacion/tolerancia, elimina en egg_map, restaura suelo y encola recomposicion; no ceros. Repetir recogida no suma. Ultimo: all_eggs=1,43/43,sin casa aun.
decor_push3: B=linea,C=columna byte,D=alto. Tesela3bytes x1..42lineas, columna<=29,dentro de CLIP_TOP..CLIP_BOTTOM. FIFO8, una preparada por iteracion. SUBCONJUNTO de decor_rect, no ancho variable. Huevo modifica2x16/recompone3x16. Descriptor privado ENT_SIZE fuera de ENTS/ZLIST; nb_range/sort_z/ent_blit pintan sprites encima. Entrada invalida/cola llena/ventana pendiente: borde rojo y parada, no descartar cambios.
Orden: blit_all+blit_decor, logica, prepare_all+prepare_decor, espera por contador. Ventanas de entidad/decorado con mismo estado, volcadas siguiente iteracion. Retirada y contador cuentan como coste de juego; exclusiones del HUD de depuracion sin cambios.

## Resultado y memoria
Carlos verifica43 huevos,fondo liso/patron,suelo restaurado,sin basura ni perdidas observadas,128K/48K. Alcance ENT_COUNT=1; multiples vecinos pendientes.
Listados EGG_TEST_FLOOR=1: fin exclusivo$A1F4/$A1ED, hasta SCR_BUFS=$B100 quedan3852/3859bytes. No margen cerrado. FLOOR_SAVE=$D800,45huecosx32=1440bytes,fin exclusivo$DDA0; solo43 ocupados inicializados. Banco0(128K)/RAMplana48K; reserva ejecucion,no datos carga. Solo pixeles,no atributos. Actualizar patron si cambia suelo despues.

## Prueba
build_game_r7.bat/build_game_r7_48.bat,rutas locales src/; RVM128Ktoastrack/modelo48K. Recoger desde cuatro lados,girar sobre huevo,volver por nodo vacio,terminar43/43 sin casa. EGG_TEST_FLOOR=1 y recompilar para trama;0 para suelo normal. Main conserva1 como probado y reaparicion arriba izquierda.
Paso2b entregado como propuesta local con game_r7.asm rev.4,dino_r7.inc modificado,respawn_r7.inc; su estado no se mezcla con esta validacion. Contrato/protocolo en paquete local.
