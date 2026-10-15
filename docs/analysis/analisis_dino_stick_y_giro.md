# Giro del dino, caida y aparicion de objetos (nivel 1)

Estado: analisis del fuente, hito 1. Referencias: archivo + etiqueta (los archivos estan troceados, sin numeracion de linea fiable).
Ubicacion prevista en el proyecto: docs/analysis/

## Observaciones directas del codigo

Giro (TROGPOST.ASM.p01 PJOY; TROGDINO.ASM DINO_STICK, DINOWLP_SLEEP, DINOWLP_CHECK):
1. PJOY mantiene PJOYSTCK con un unico bit mientras el interruptor esta pulsado; al soltar se borra. No hay cola de giros.
2. DINO_STICK traduce PJOYSTCK con DINO_MOVE_TAB. Solo una direccion simple vale; combinaciones o 0 = "seguir igual".
3. DS_CK_NEW: la direccion contraria a la actual se acepta siempre (media vuelta en cualquier punto del pasillo).
4. DS_CENT_CK / DS_ND_CENTER: un giro perpendicular solo se acepta si el punto de movimiento fisico del dino es un centro y
   coincide con TCURMPOS. Entonces DINO_CENTER alinea al dino con el centro y se acepta la direccion.
5. DINO_NEXT_MOVE solo registra un punto de movimiento (TCURMPOS) si el dino esta a menos de [4,4] (PNT_IN_RANGE).
6. DINO_STICK se consulta en cada tick y en cada paso de animacion.

Punto de referencia del dino (TROGDINO.ASM, pegado por Carlos):
7. GET_DINO_MPNT: centro del dino (GET_DINO_CPNT) + [5,0] ([Y,X]) y busca el punto de movimiento mas cercano (FIND_CLOSE_MPNT).
8. DINO_CENTER -> DINO_NEW_PNT: alinea el centro del dino con el punto de movimiento (resta [3,0]).

Caida (TROGPOST.ASM, pegado por Carlos):
9. BOUNDRY_CHECK recorre @BOUNDRIES: [ULY,ULX], [LRY,LRX], tipo. Tipo 0 (cliff): hay violacion si QBOXOUT da NZ.
   Resto de tipos: LAKE y VOLCANO con QBOXIN; el demas se trata como PIT (lee tambien su centro). Devuelve Z = sin violacion,
   NZ = violacion, A5 = tipo, A6 = centro del pozo. Recibe A1 = direccion actual y A8 = objeto.
10. QBOXOUT (cabecera): "Z = total o parcialmente dentro de la caja; NZ = completamente fuera". Usa OXPOS/OYPOS (esquina
    superior izquierda de la imagen) y OSIZE (esquina inferior derecha), restando @WORLDTL. La direccion no se usa en esta rutina.
11. QBOXIN (cabecera): "NZ = completamente dentro de la caja". El texto pegado esta cortado tras las primeras instrucciones.
12. BOUNDRY1 (nivel 1, TROGMAPS.ASM): [70,53],[215,336] ([Y,X]), tipo TYPE_CLIFF. Comentario del fuente: tipo 0 = seguro
    permanecer dentro de la caja (limitada por acantilados); tipo 1 = seguro permanecer fuera (pozo).
    BOUNDRY2 anade un pozo [87,141]-[202,247] con centro [159,196]; BOUNDRYV2 un volcan [87,135]-[207,254];
    BOUNDRYX5, X6, X2 y X3 anaden lagos. BOUNDRYX1 y X7 estan dentro de .IF 0.

Objetos (TROGDINO.ASM, FIND_OPEN_CPNT, pegado por Carlos):
13. FIND_OPEN_CPNT elige un centro al azar (hasta 10 intentos) con CMOVEEGG libre, CMOVETROG libre y DINO_PNT_TST con Z
    (sin dinos ni enemigos cerca). Si falla recorre los centros en orden. Z = ninguno libre.
14. WAVE1DSP y WAVE1TIME coinciden con lo leido antes.

## Interpretaciones pendientes de comprobar

- La caida del arcade se dispara cuando el rectangulo de imagen del objeto esta completamente fuera de BOUNDRY1; no hay punto de pies.
  Con ese criterio el dino camina fuera del borde una distancia parecida a su tamano antes de caer. No coincide del todo con
  "en cuanto los pies tocan el contorno" (Carlos); falta saber el tamano de imagen del dino arcade.
- El +5 en Y de GET_DINO_MPNT es coherente con un punto de pies unos px por debajo del centro (8 + 5 = 13 en nuestro sprite).
- Giro: el arcade acepta la direccion cuando el dino ha registrado el centro y luego lo alinea con DINO_CENTER, de modo que el giro
  acaba en la misma columna o fila. En MAME no se aprecia ningun salto (Carlos). Pendiente: FIND_CLOSE_MPNT y PNT_IN_RANGE.
- La aparicion de Trog probablemente usa un criterio parecido al de FIND_OPEN_CPNT. Pendiente: TROG_START.

## Desconocido

- Tamano de imagen (OSIZE) del dino arcade y de la imagen de caida.
- FIND_CLOSE_MPNT, PNT_IN_RANGE, radio de DINOS_NEAR_PNT y ENEMIES_NEAR_PNT, TROG_START.
- Velocidad real del dino (DINOSPEED, DINO_VEL).
