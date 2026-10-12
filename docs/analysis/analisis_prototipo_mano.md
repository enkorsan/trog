# Prototipo de la mano y restos en el fuente

Estado: informacion de Carlos mas observaciones del fuente. Version: 12-oct-2026, revision 3.

## Informacion de Carlos
- En el prototipo, el jugador controla una mano que pone y quita huesos formando barreras en los pasillos.
- El dino tenia movimiento lineal automatico y no lo controlaba el jugador.
- El jugador debia lograr que el dino cogiera todos los huevos poniendo y quitando barreras.
- LA5 es la ultima version desarrollada como Pac-Man (el jugador controla al dino). La mano no aparece en el juego final.
- El prototipo se podria agregar opcionalmente al final del desarrollo. Es el hito 4 de la hoja de ruta.

## Observaciones directas del fuente
1. TROGIO.DOC llama DRAW al boton de accion de cada jugador.
2. TROG.MAP (enlace del 31-ago-1990) incluye TROGBAR.OBJ en el programa principal y simbolos como P1BARS a P4BARS, PXBARS, MAXBARS y WAVEHAND.
3. TROGRAM.ASM reserva las listas P1BARS, P2BARS, P3BARS, P4BARS y PXBARS (huesos de cada jugador e indestructibles) y la variable MAXBARS (maximo de huesos activos por jugador).
4. TROGWAVE.ASM define MAXBARS en la tabla de dificultad de partida (GAME_DIFF_TAB, valor por defecto 12) y patrones de huesos por oleada (WTBONES, WC1BONES, WF1BONES, etc.).
5. TROGDINO.ASM, DINO_STICK (parte 2): consulta el campo MOVEPLYR del movimiento adyacente en la direccion pedida. Si hay hueso, el dino sigue recto.
6. TROGDINO.ASM, tabla DINOCOLL (parte 7): colisiones con HBAROID y VBAROID mediante DINOHHIT y DINOVHIT (parte 3 y 4). DINODEFL (parte 5) trata la deflexion.
7. TROGDINO.ASM, DINOWXFER (parte 1): usa BONED_IN y BONE_OBJ en el movimiento normal.
8. TROGCOL.ASM, CDDETECT: escanea al dino contra P1BARS a P4BARS y PXBARS.

## Interpretaciones pendientes de comprobar
- Los huesos y barras participan en la logica de colision del dino en este fuente. Falta comprobar si el codigo de mano y dibujo (boton DRAW, TROGBAR) esta activo en LA5.
- Carlos no ha visto la mano en el juego final. WAVEHAND puede referirse a otra cosa. Pendiente de leer el codigo que usa WAVEHAND.
- Las barras estarian en los segmentos entre postes (tiles de pasillo entre rocas en nuestro mapa).

## Preparacion en el port
- No se implementa nada del prototipo en los hitos 1 a 3.
- Reservar un bit de barrera en los datos de cada tile de segmento. El movimiento ya consulta el vecino en la direccion pedida, igual que DINO_STICK.

## Desconocido
- Codigo de TROGBAR.ASM y de las rutinas de huesos (BONE_OBLITERATE, BONE_SMASH, BONED_IN, BONE_OBJ).
- Significado exacto de WAVEHAND.
- Reglas del prototipo.
