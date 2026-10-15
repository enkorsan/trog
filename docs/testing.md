# Pruebas

Version: 13-oct-2026, revision 12 (sustituye a testing.md rev. 11 y a testing_paso_2d.md)

| Paso | Ensambla | Arranca | Observaciones |
|---|---|---|---|
| 1c, 2a, 2b | si | si | Informados por Carlos |
| 2c | si | si | Funciona. La velocidad es algo mayor que la de MAME (despues Carlos informa de que parece correcta) |
| 3a | si | si | Layout C. Funciona perfectamente |
| 3b a 3e | si | si | 3e: el giro se parece al de MAME; con el dino de 16x24 el salto de alineacion se nota algo (el arcade tambien salta, pero lo disimula con el fotograma de giro) |
| 3f | si | si | Error: borde derecho, el dino vuelve a detenerse al pulsar de nuevo (corregido en el 3g) |
| 3g | si | si | Base del 2d; Carlos no ha informado de fallos |
| 2d | si | si | Funciona. Ajustes: recogida a 2 px, resp_eat, find_closest sin el huevo a distancia 0 (probados) |
| Corte en .inc | si | si | Mismo hash SHA-256 del .sna antes y despues |
| Dino provisional | si | si | Se ve mejor que la flecha. TURN_TOL = 2 mejora el salto al girar |
| 2e-1, 2e-2 | si | si | 28 salidas; el marcador sale en un lado distinto con R; sin restos |
| 2e-3 | si | si | Funciona segun lo esperado. Pendiente: parpadeo a velocidad constante (HOUSE_FLASH_FRAMES = 21) |
| 2e-4a | sin hacer | sin hacer | Llegada a casa y victoria provisional |

## Procedimiento comun
1. Ejecutar build.bat. Anotar cualquier aviso o error de sjasmplus (los ASSERT deben pasar).
2. Abrir build/trogzx_paso1.sna en Spectaculator, Fuse y RetroVirtualMachine (128K).
3. Comparar con lo esperado de cada paso y anotar diferencias entre emuladores. Si solo se corta el codigo en modulos, comparar el hash del .sna (certutil -hashfile build\trogzx_paso1.sna SHA256) antes y despues.
4. Todos los .inc y los de datos (sprite_dino.inc, stones.inc, eggs.inc, house_gfx.inc) deben estar guardados y junto a main.asm. Un error "Label not found" suele indicar un INCLUDE que falta o un main.asm sin guardar.

## Regresiones fijas (cada paso)
Giro con tolerancia (sin girar fuera de las intersecciones reales), parada ante el borde y caida, apagado de 55 fotogramas, teclas H, K, R y B, sin restos del sprite al moverse, recorte arriba y abajo, huevos y contador.

## Pruebas del paso 2e-3 (hechas)
1. Con R varias veces y luego B: tras unos 51 fotogramas aparece la extension en el lado del marcador, con el letrero HOME/REX alternando. Repetir en los cuatro lados (R se bloquea con la casa construida: reiniciar el emulador).
2. Caminar sobre la extension: sin restos del sprite y con el letrero parpadeando por debajo.
3. Recoger el ultimo huevo (con R y el contador): la casa se construye sola.
4. El dino sigue cayendo por esa salida como siempre.

## Pruebas previstas del paso 2e-4a (llegada y victoria)
Escenario: construir la casa con B (o recogiendo todos los huevos) y llevar al dino a mano por la calle de la salida.
1. Invencible o no, el dino no cae ni se detiene ante el borde en la calle de la salida de la casa: sigue hasta la celda adicional (arriba y = 28, abajo y = 172, izquierda x = 0, derecha x = 240) y ahi empieza la victoria.
2. En el resto de bordes todo sigue igual (parada, espera, media vuelta y caida).
3. Victoria provisional: dino quieto mirando abajo, uno de los cinco textos unos 2 s y reinicio del nivel con huevos nuevos.
4. Comprobar los cuatro lados: arriba, la cabeza queda 4 lineas bajo el recorte del HUD.
5. Con el dino llegando por la calle de la casa pero en sentido contrario (hacia dentro) o por otra calle: sin victoria.
6. La casa en estado "apareciendo" (oculta unos 51 fotogramas) ya cuenta: si el dino llega entonces, tambien se produce la victoria (en el arcade el objeto casa existe desde que se construye).

## Mediciones pendientes en MAME (docs/calibracion.md)
- Velocidad del dino y tiempo entre intersecciones; distancia de parada ante el borde; hueco de 4 px.
- Valores nuevos de calibracion: EGG_TOL = 2 (recogida), TURN_TOL = 2 (giro), HOUSE_APPEAR_FRAMES = 51, HOUSE_FLASH_FRAMES = 21 (0,83 s el ciclo), posicion de parada en la celda adicional de la salida.
