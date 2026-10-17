; config.inc - constantes y opciones. STOP_TOP_Y y STOP_BOTTOM_Y ajustados por Carlos (17-oct) tras comparar con el arcade: ver STOP_TOP_SHIFT y STOP_BOTTOM_SHIFT.
; ---- Opciones de compilacion -------------------------------------------------
DEBUG_CHEAT     EQU 1           ; 1 = teclas de depuracion: H invencibilidad permanente, K muerte sin caida, R nueva distribucion de huevos
EDGE_TURN       EQU 1           ; 1 = el invencible parado ante el borde da media vuelta pasado DWAIT_FRAMES (0 = se queda parado)
STOP_QUIRK      EQU 1           ; 1 = el punto auxiliar del borde se evalua una sola vez mientras sea el ultimo registrado (como el arcade)
EGG_AI          EQU 1           ; 1 = al reaparecer busca el huevo mas cercano (DINO_FIND_CLOSEST); 0 = solo la valoracion de DINO_FIND_BEST

; ---- Constantes de pantalla ------------------------------------------------
SCR_BASE        EQU $4000
ATTR_BASE       EQU $5800
HUD_H           EQU 24          ; alto del HUD en lineas (multiplo de 8)
GRID_TOP        EQU 44          ; primera linea de las piedras (par)
EDGE_GAP        EQU 4           ; suelo entre las piedras exteriores y el agua (par, 2..8)
FLOOR_TOP       EQU GRID_TOP-EDGE_GAP           ; primera linea de suelo: 40
FLOOR_BOTTOM    EQU GRID_TOP+127+EDGE_GAP       ; ultima linea de suelo: 175 (piedras: 5*24 + 8 = 128 lineas)
FLOOR_LEFT      EQU 16-EDGE_GAP                 ; primera columna de suelo: 12 (piedras en X 16..239)
FLOOR_RIGHT     EQU 239+EDGE_GAP                ; ultima columna de suelo: 243
WATER_PX        EQU 16-EDGE_GAP                 ; columnas de agua a cada lado: 12
WATER_FULL      EQU WATER_PX/8                  ; bytes de agua completos a cada lado
WATER_REM       EQU WATER_PX&7                  ; columnas de agua del byte parcial
WATER_PART      EQU (WATER_REM+7)/8             ; 1 si hay byte parcial
WATER_MASK_L    EQU ($FF<<(8-WATER_REM))&$FF    ; parte de agua del byte parcial izquierdo (bits altos)
WATER_MASK_R    EQU $FF>>(8-WATER_REM)          ; parte de agua del byte parcial derecho (bits bajos)
FIELD_TOP       EQU HUD_H       ; primera linea del campo (agua superior)
CLIP_TOP        EQU HUD_H       ; primera linea donde se puede pintar un sprite (bajo el HUD)
CLIP_BOTTOM     EQU 192         ; primera linea que NO se puede pintar (fuera de pantalla)
FIELD_ATTR_ROW  EQU HUD_H/8     ; primera fila de atributos del campo
FIELD_ATTR_BASE EQU ATTR_BASE+FIELD_ATTR_ROW*32
FIELD_ATTR_LEN  EQU (24-FIELD_ATTR_ROW)*32-1

STONE_COLS      EQU 10
STONE_ROWS      EQU 6
STONE_H         EQU 8           ; piedra de 8x8
WATER_EVEN      EQU $AA         ; trama del agua en lineas pares
WATER_ODD       EQU $55         ; trama del agua en lineas impares

SPR_H           EQU 24          ; alto del sprite del dino en lineas
SPR_OFF_Y       EQU 8           ; lineas del sprite por encima del origen logico (la calle mide 16)
DWAIT_FRAMES    EQU 111         ; DWAIT_TIME = 120 ticks del arcade x 0,914 (el bucle del arcade cuenta 121 pasadas: 110,6)

    ASSERT (HUD_H & 7) == 0
    ASSERT (GRID_TOP & 1) == 0
    ASSERT (EDGE_GAP & 1) == 0
    ASSERT EDGE_GAP >= 2
    ASSERT EDGE_GAP <= 8
    ASSERT WATER_FULL >= 1
    ASSERT FLOOR_TOP >= HUD_H+8
    ASSERT FLOOR_BOTTOM <= 183

; ---- Atributos provisionales (FLASH, BRIGHT, PAPER*8, INK) ------------------
ATTR_HUD        EQU %00000111   ; papel negro, tinta blanca
ATTR_FIELD      EQU %00100000   ; papel verde, tinta negra (todo el campo, monocromo)
ATTR_KEY_OFF    EQU %00000001   ; casilla apagada: tinta azul
ATTR_KEY_ON     EQU %01000110   ; casilla encendida: tinta amarilla brillante

; ---- Entrada: mascara de bits ----------------------------------------------
DIR_UP          EQU %00001
DIR_DOWN        EQU %00010
DIR_LEFT        EQU %00100
DIR_RIGHT       EQU %01000
KEY_ACTION      EQU %10000
KEY_CELLS       EQU 5           ; casillas de teclas en el HUD: 4 direcciones + accion
HUD_CELLS       EQU 6           ; simbolos del HUD (la sexta es la invencibilidad)
HUD_INV_CELL    EQU 6           ; columna de la casilla de invencibilidad
KEY_HOLD_FRAMES EQU 2           ; fotogramas que se recuerda una direccion tras soltar la tecla (arcade: 2 ticks)

; ---- Direcciones del dino (mismo orden que el arcade: 0 arriba, 1 abajo, 2 izquierda, 3 derecha) ----
; La contraria de una direccion es (direccion XOR 1).
D_UP            EQU 0
D_DOWN          EQU 1
D_LEFT          EQU 2
D_RIGHT         EQU 3

; ---- Intersecciones: la esquina superior izquierda de la calle (origen logico del dino) ----
NODE_STEP       EQU 24          ; separacion entre intersecciones (piedra 8 + calle 16), en X e Y
NODE_X0         EQU 24          ; X de la primera calle vertical (24 + 24*c, c 0..8)
NODE_Y0         EQU GRID_TOP+8  ; Y de la primera calle horizontal (+24*r, r 0..4)
DINO_START_X    EQU NODE_X0     ; salida: primera interseccion de arriba a la izquierda
DINO_START_Y    EQU NODE_Y0
NODE_COLS_M1    EQU 8           ; columnas de intersecciones - 1
NODE_ROWS_M1    EQU 4           ; filas de intersecciones - 1
NODE_COLS       EQU NODE_COLS_M1+1
NODE_ROWS       EQU NODE_ROWS_M1+1
NODE_COUNT      EQU NODE_COLS*NODE_ROWS         ; 45
NODE_X_LAST     EQU NODE_X0+NODE_COLS_M1*NODE_STEP      ; 216
NODE_Y_LAST     EQU NODE_Y0+NODE_ROWS_M1*NODE_STEP      ; 148
RESPAWN_X_MIN   EQU NODE_X0     ; intersecciones extremas: salida por la izquierda
RESPAWN_X_MAX   EQU NODE_X_LAST ; por la derecha
RESPAWN_Y_MIN   EQU NODE_Y0     ; por arriba
RESPAWN_Y_MAX   EQU NODE_Y_LAST ; por abajo

; ---- Huevos ----
EGG_TOTAL       EQU NODE_COUNT-2        ; 43: ALLOT_EGGS resta 2 (SUBK 2,A3: "leave a couple of spots for Trog")
EGG_H           EQU 16                  ; sello de 16x16
EGG_SEED        EQU $A5C3
HUD_EGG_LINE    EQU 8                   ; linea del contador (fila de celdas 1 del HUD)
HUD_EGG_COL     EQU 1                   ; columna del icono; despues: 2 digitos, barra y 2 digitos

; ---- Giro: tolerancia alrededor de la interseccion (arcade: PNT_IN_RANGE con [4,4]; PNTINBOX incluye el borde) ----
TURN_TOL        EQU 2
EGG_TOL         EQU 2           ; recogida de huevo: +-EGG_TOL px sobre la interseccion
    ASSERT EGG_TOL <= TURN_TOL
; ---- Velocidad del motor antiguo (main.asm): pasos de 2 px por cada 16 fotogramas (7 = 43,75 px/s; 8 = 50 px/s; 6 = 37,5 px/s) ----
DINO_SPEED      EQU 7           ; NO lo usa game_r7.asm: usa SPEED_BASE/SPEED_MOD de render.inc (26/64)

; ---- Tiempos. El arcade cuenta ticks de 54,71 Hz; aqui fotogramas de 50 Hz (x 0,914) ----
START_WAIT      EQU 55          ; quieto al aparecer: 60 ticks
INV_FRAMES      EQU 238         ; invencibilidad al aparecer: (60 + 200) ticks
INV_BLINK       EQU 73          ; el indicador parpadea en el ultimo tramo: 80 ticks
DEAD_WAIT_FRAMES EQU 55         ; apagado entre la muerte y la reaparicion: 60 ticks (DINO_ETERNAL); game_r7.asm usa FALL_DEAD de render.inc (mismo valor)

; ---- Caida: punto de pies (x + 8, y + 13) sobre el suelo X FLOOR_LEFT..FLOOR_RIGHT, Y FLOOR_TOP..FLOOR_BOTTOM ----
; Arriba e izquierda: pie en la primera linea o columna de suelo o antes. Abajo y derecha: pie en la primera linea o columna de agua o despues. Posiciones pares.
FALL_TOP_Y      EQU FLOOR_TOP-14        ; 26
FALL_BOTTOM_Y   EQU FLOOR_BOTTOM-11     ; 164
FALL_LEFT_X     EQU FLOOR_LEFT-8        ; 4
FALL_RIGHT_X    EQU FLOOR_RIGHT+1-8     ; 236
FALL_FRAMES     EQU 50          ; animacion de caida del motor antiguo (parpadeo). NO lo usa game_r7.asm: usa FALL_ANIM de render.inc (provisional); el arcade usa FRANIMATE_DIR con DEATH_UP

; ---- Parada ante el borde: base 4 px antes del punto auxiliar del borde (media calle, 12 px, de la ultima interseccion), desplazada EDGE_GAP hacia fuera; en Y, ademas el ajuste vertical de Carlos (17-oct) ----
STOP_TOP_SHIFT    EQU 4         ; limite superior 4 px mas abajo (hacia dentro del campo); par
STOP_BOTTOM_SHIFT EQU 4         ; limite inferior 4 px mas arriba (hacia dentro del campo); par
STOP_LEFT_X     EQU NODE_X0-8-EDGE_GAP          ; 12
STOP_RIGHT_X    EQU NODE_X_LAST+8+EDGE_GAP      ; 228
STOP_TOP_Y      EQU NODE_Y0-8-EDGE_GAP+STOP_TOP_SHIFT           ; 44 (antes 40)
STOP_BOTTOM_Y   EQU NODE_Y_LAST+8+EDGE_GAP-STOP_BOTTOM_SHIFT   ; 156 (antes 160)

    ASSERT (STOP_TOP_SHIFT & 1) == 0
    ASSERT (STOP_BOTTOM_SHIFT & 1) == 0
    ASSERT STOP_LEFT_X > FALL_LEFT_X
    ASSERT STOP_RIGHT_X < FALL_RIGHT_X
    ASSERT STOP_TOP_Y > FALL_TOP_Y
    ASSERT STOP_BOTTOM_Y < FALL_BOTTOM_Y
