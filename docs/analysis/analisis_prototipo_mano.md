# Prototipo de la mano y restos en el fuente

Estado: informacion de Carlos mas observaciones del fuente. Version: 8-oct-2026, revision 2.
Ubicacion prevista en el proyecto: docs/analysis/

## Informacion de Carlos

- En el prototipo, el jugador controla una mano que pone y quita huesos formando barreras en los pasillos.
- El dino tenia movimiento lineal automatico y no lo controlaba el jugador.
- El jugador debia lograr que el dino cogiera todos los huevos poniendo y quitando barreras.
- LA5 es la ultima version desarrollada como Pac-Man (el jugador controla al dino). La mano no aparece en el juego final.
- El prototipo se podria agregar opcionalmente al final del desarrollo. Es el hito 4 de la hoja de ruta, condicionado a memoria, rendimiento e informacion.

## Observaciones directas del fuente

1. TROGIO.DOC llama DRAW al boton de accion de cada jugador y TROGPROJ.DOC describe el concepto original: dibujar muros entre postes.
2. TROG.MAP (enlace de LA5, 31-ago-1990) incluye TROGBAR.OBJ en el programa principal (.text ffe2e110, 0x2b40 bytes) y simbolos de ese modo:
   BLOOHHAND1, BLOOHHAND2, BLOOVHAND1, BLOOVHAND2, BOXARH1, BOXARH2, BOXARV1, BOXARV2, P1BARS a P4BARS, PXBARS, MAXBARS y WAVEHAND.
3. Las rutinas BONE_OBLITERATE, BONE_SMASH, BONED_IN y BONE_OBJ caen dentro del rango de TROGBAR.OBJ en el mapa de enlace.
4. WAVEHAND esta dentro del bloque de parametros de dificultad por oleada (entre WDIFF_RAM_ST y WDIFF_RAM_END).
5. DINO_STICK comprueba si hay un hueso en la direccion pedida (MOVEPLYR) y, si lo hay, el dino sigue recto. DINOVHIT trata la colision con
   una barra vertical (TROGDINO.ASM.p04).

## Interpretaciones pendientes de comprobar

- Que un simbolo este enlazado no significa que este activo en LA5. El codigo de barras y mano puede ser un resto del prototipo.
- Carlos no ha visto la mano en el juego final, asi que WAVEHAND probablemente se refiere a otra cosa (HAND tambien puede ser otra cosa distinta
  de la mano del jugador). Pendiente de leer el codigo que usa WAVEHAND.
- Las barras del prototipo estarian en los segmentos entre postes, que en nuestro mapa son los tiles de pasillo entre rocas.

## Preparacion en el port

- No se implementa nada del prototipo en los hitos 1 a 3.
- Reservar un bit de barrera en los datos de cada tile de segmento, por si el hito 4 lo necesita. El movimiento ya consulta el vecino en la
  direccion pedida, igual que DINO_STICK, asi que una barrera se comprobaria en el mismo punto.

## Desconocido

- Codigo de TROGBAR.ASM y de las rutinas de huesos.
- Significado exacto de WAVEHAND y de MAXBARS.
- Reglas del prototipo: que hace el dino al chocar con una barrera, cuantas barreras se permiten y como se puntua.
