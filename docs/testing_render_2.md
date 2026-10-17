# Pruebas del juego r7: tandas 1, 2a y 2b

17-oct-2026, revision 0 del suplemento a testing.md rev12 y testing_render.md rev3. Pruebas realizadas por Carlos en 128K y 48K (RVM); resultado: todas correctas. El asistente no ejecuta emulador.

## Preparacion
build_game_r7.bat y build_game_r7_48.bat (sjasmplus1.24.0, -DTARGET_48K). DEBUG_CHEAT = 1 y DBG_HUD = 1. Teclas: Q A O P, H, K, B, J. HOUSE_FORCE_EXIT = -1 (al azar con semilla fija) o 0..27.

## Tanda 1 (H, K, muerte en sitio)
1. Regresion: arranque, caida, parada con invencibilidad, 43 huevos, PC/LT sin cambio.
2. H enciende y apaga la casilla del HUD; con H el dino se detiene en los bordes.
3. K a 2 px o menos de un nodo con huevo: reaparece en el nodo y recoge el huevo al instante.
4. K a mitad de calle: nodo mas cercano en cada eje; no recoge durante la espera y lo recoge en el primer paso.
5. K en nodo sin huevo o con huevo a distancia 0: sin recogida; rumbo desde la interseccion.
6. K y despues una caida por un borde: reaparece en el extremo de su calle (rama de caida intacta).
7. K durante la espera inicial o con el dino parado en un borde: sin errores.

## Tanda 2a (casa estatica)
1. Regresion. 2. HUD EX nn; J avanza 0..27 y vuelve a 0. 3. B: a ~1 s aparecen extension y letrero y alterna cada ~0,4 s; salidas 0, 8, 9, 17, 18, 22, 23 y 27, extension centrada y dentro de pantalla (derecha en X 240..255). 4. Dino sobre la casa (con H): por encima del decorado, sin restos ni parpadeo. 5. Recoger los 43 huevos: la casa aparece sola. 6. B de nuevo o J con la casa: sin efecto. 7. Sin borde rojo (cola de decorado desbordada).

## Tanda 2b (llegada y victoria)
1. Regresion. 2. Salida 0 (arriba, columna 0): subir con Q; pasa los puntos de parada y caida y llega a Y=28; el dino mira abajo, aparece uno de cinco textos y a los ~2 s reinicia el nivel. 3. Igual con una salida abajo, una izquierda y una derecha. 4. Otra calle con la casa construida: parada y caida como antes. 5. Mismo eje en sentido contrario: parada y caida como antes. 6. Sin casa: cae. 7. Reinicio completo: contador 0/43, dino en el arranque con invencibilidad, sin restos de extension ni texto, B funciona de nuevo. 8. Llegada por arriba (Y=28): cabeza recortada bajo el HUD sin ensuciarlo. 9. K sin efecto durante la victoria, el letrero sigue alternando, sin borde rojo.

## Limitaciones conocidas
Semilla fija: la salida, la celda libre y el texto salen siempre iguales y el reinicio repite el nivel. Ninguna prueba mide CPU mas alla de PC/LT. SPR_BANK distinto de 2 y ENT_COUNT mayor que 1 siguen sin probar.
