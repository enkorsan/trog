; ============================================================================
; render.inc - CONTRATO DEL RENDERER (paso 0 de la integracion)
; Estado: PROPUESTO (sin ensamblar ni probar). Ver docs/decisions_render_update_3.md (rev. 1).
; Requiere config.inc antes (SPR_H, SPR_OFF_Y, CLIP_TOP, CLIP_BOTTOM, NODE_*, D_*).
; El fichero principal define ANTES de incluirlo: ENT_COUNT (1..MAX_ENTS).
; Opcional: -DTARGET_48K en la linea de comandos (sjasmplus 1.24.0: -D<NAME>[=<value>]) => sin paginacion ni AY.
; IFDEF/IFNDEF solo ven DEFINE (y -D), NO las EQU: por eso TARGET_48K es un define y lo demas son EQU.
; No contiene codigo ni datos: solo EQU, ASSERT y macros.
; ============================================================================
    IFNDEF RENDER_INC_LOADED
    DEFINE RENDER_INC_LOADED

; ---- Plataforma --------------------------------------------------------------
    IFDEF TARGET_48K
MACHINE_128     EQU 0
    ELSE
MACHINE_128     EQU 1
    ENDIF
HAS_AY          EQU MACHINE_128 ; el AY solo existe en 128K
SPR_BANK        EQU 2           ; banco de los sprites: 2 = residente (para otro valor, editar aqui)
    ASSERT SPR_BANK == 1 || SPR_BANK == 2 || SPR_BANK == 3 || SPR_BANK == 4 || SPR_BANK == 6 || SPR_BANK == 7

; ---- Mapa de memoria (decisions_render_update_3.md, seccion 9) -----------------
CODE_BASE       EQU $8000
STACK_TOP       EQU $7FFF
SCR_BUFS        EQU $B100       ; MAX_ENTS buffers de ventana de 128 bytes, alineados a 128
STAT_BUF        EQU $B500       ; buffer de la tesela de decorado (paso 2)
ENTS            EQU $B580       ; MAX_ENTS x ENT_SIZE
QPOS            EQU $B680       ; 2 bytes por entidad: columna y linea superior ($80 = no es vecino)
ZLIST           EQU $B690       ; 2 bytes por entidad: punteros ordenados por prioridad
WORK_END        EQU $B6A0       ; reservado hasta $B7FF
IM2_TABLE       EQU $B800
IM2_FILL        EQU $B9
IM2_VECTOR      EQU (IM2_FILL<<8)|IM2_FILL
BG_BASE         EQU $C000       ; fondo limpio lineal: 192 lineas x 32 bytes (banco 0 en 128K)
BG_SIZE         EQU 192*32

MAX_ENTS        EQU 8
WIN_BUF         EQU 128
ENT0            EQU ENTS        ; entidad 0 = dino
STAT_MAX        EQU 128         ; bytes maximos de una tesela de decorado
STATQ_MAX       EQU 8
STAT_PER_FRAME  EQU 1

    ASSERT ENT_COUNT >= 1 && ENT_COUNT <= MAX_ENTS
    ASSERT (SCR_BUFS & (WIN_BUF-1)) == 0
    ASSERT (STAT_BUF & (WIN_BUF-1)) == 0
    ASSERT SCR_BUFS + MAX_ENTS*WIN_BUF <= STAT_BUF
    ASSERT STAT_BUF + STAT_MAX <= ENTS
    ASSERT QPOS + 2*MAX_ENTS <= ZLIST
    ASSERT ZLIST + 2*MAX_ENTS <= WORK_END
    ASSERT WORK_END <= IM2_TABLE

; ---- Entidad (32 bytes). L = escribe la logica; R = solo el renderer ----------
E_X         EQU 0               ; L  posicion X en pixeles (origen logico)
E_Y         EQU 1               ; L  posicion Y
E_DIR       EQU 2               ; L  direccion de marcha (D_UP..D_RIGHT = 0..3)
E_ACC       EQU 3               ; L  acumulador de velocidad (modulo SPEED_MOD)
E_SPD       EQU 4               ; L  velocidad (< SPEED_MOD)
E_COL       EQU 5               ; L  columna de interseccion (IA)
E_ROW       EQU 6               ; L  fila de interseccion (IA)
E_LEFT      EQU 7               ; L  pasos restantes hasta la siguiente interseccion
E_DIRTY     EQU 8               ; L  1 = redibujar (posicion, imagen, aparece o desaparece)
E_VALID     EQU 9               ; R  1 = hay sprite pintado (E_OX, E_OY validos)
E_OX        EQU 10              ; R
E_OY        EQU 11              ; R
E_PEND      EQU 12              ; R  ventana pendiente de volcar
E_SC0       EQU 13              ; R  primera columna de la ventana
E_SL0       EQU 14              ; R  primera linea de la ventana
E_SH        EQU 15              ; R  alto de la ventana
E_RC        EQU 16              ; R  columna del sprite dentro de la ventana
E_RL        EQU 17              ; R  linea del sprite dentro de la ventana
E_IMG       EQU 18              ; L  imagen (IMG_*): 0..3 cardinales; 4..7 diagonales cuando existan
E_TMR       EQU 19              ; L  temporizador de accion (estado 0) o caida (estados 1 y 2)
E_SBUF      EQU 20              ; -  puntero al buffer de ventana (fijo al iniciar)
E_IDX       EQU 22              ; -  indice (0 = dino)
E_TIE       EQU 23              ; -  desempate de prioridad (dino 255)
E_DP        EQU 24              ; R  puntero a los datos del sprite
E_QW        EQU 26              ; R  ancho en bytes (2 o 3)
E_QC        EQU 27              ; R  columna del sprite ($80 = oculta)
E_QT        EQU 28              ; R  linea superior del sprite
E_PAUSE     EQU 29              ; L  fotogramas de parada
E_CLIP      EQU 30              ; R  1 = ventana recortada
E_STATE     EQU 31              ; L  0 andando, 1 animacion de caida, 2 oculta
ENT_SIZE    EQU 32
    ASSERT ENTS + MAX_ENTS*ENT_SIZE <= QPOS

; ---- Imagenes del sprite: 4 fases horizontales por imagen, 144 bytes por imagen y fase ---------
IMG_UP      EQU 0
IMG_DOWN    EQU 1
IMG_LEFT    EQU 2
IMG_RIGHT   EQU 3
IMG_UL      EQU 4               ; diagonales para el giro por pasos de 45 grados (placeholder de Carlos)
IMG_UR      EQU 5
IMG_DL      EQU 6
IMG_DR      EQU 7
IMG_COUNT   EQU 4               ; 4 con los sprites actuales; 8 cuando haya diagonales (bytes por juego de sprites: IMG_COUNT*4*144)

; ---- Animacion de caida (provisional, banco R6) -----------------------------
FALL_ANIM   EQU 24              ; fotogramas de animacion (E_IMG cambia cada 4)
FALL_DEAD   EQU 55              ; fotogramas desaparecida

; ---- Velocidad con modulo 64 ---------------------------------------------------
SPEED_MOD   EQU 64
SPEED_BASE  EQU 26              ; 13/32 de 2 px por fotograma
    ASSERT SPEED_BASE*2 < SPEED_MOD

; ---- Paginacion (no generan codigo en 48K). Alteran A y BC en 128K: tratarlos --
; ---- como alterados en TODAS las compilaciones -------------------------------
PORT_7FFD   EQU $7FFD
PORT_BG     EQU $10             ; ROM 1, banco 0 en $C000, pantalla normal
PORT_SPR    EQU $10+SPR_BANK

    MACRO PAGE_BG
        IF MACHINE_128
            ld bc,PORT_7FFD
            ld a,PORT_BG
            out (c),a
        ENDIF
    ENDM

    MACRO PAGE_SPR
        IF MACHINE_128 && SPR_BANK != 2
            ld bc,PORT_7FFD
            ld a,PORT_SPR
            out (c),a
        ENDIF
    ENDM

; ---- Interfaz que debe proporcionar render_core.inc (por extraer del R6.3) -----
;   ent_set_qpos   IX = entidad                  recalcula ancho, puntero, columna y linea
;   prepare_all                                    ventanas de las entidades sucias
;   blit_all                                       vuelca las ventanas pendientes
;   build_bg                                       copia la pantalla de pixeles a BG_BASE (con PAGE_BG activo)
;   b_print_dec4   HL = valor, B = linea, C = columna (visto en la copia local del R6.3)
; Variables que usa el renderer y que el juego no toca: z_sorted, ep_slow, ovl_cnt, dc_*, b_rng.
; Interfaz prevista para los pasos 2 y 3 (aun no implementada):
;   decor_rect     B = linea, C = columna, D = ancho (bytes), E = alto (lineas)

    ENDIF
