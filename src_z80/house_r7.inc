; house_r7.inc revision0 (tanda 2a: casa estatica con letrero). PROPUESTA: el estado lo fija Carlos tras la prueba. Sin ejecucion del asistente.
; Origen logico: house.inc del motor anterior (house_select, house_build, house_flash_update, exit_tab). Dibujo NUEVO: el motor anterior pintaba directo en pantalla;
;   aqui la extension y el letrero se funden en el fondo limpio BG_BASE y se piden al decorado con decor_push3 (teselas de 3 bytes), como egg_remove.
;   El decorado queda siempre debajo de los sprites (decisions_render_update_4.md, seccion 1): no hace falta proteger al dino al repintar.
; Arcade (observaciones del motor anterior): la salida se elige al empezar la fase (PLAYER_HOUSE -> FIND_OPEN_EXIT); la casa se construye al recoger el ultimo huevo
;   (EGGDINO_BUILD), aparece oculta 56 ticks y alterna HOME/REX. Port: 28 salidas (INTERPRETACION pendiente de confirmar con EXITPOINTS).
; Extension: arriba 32x20 en lineas 24..43; abajo 32x20 en 172..191; izquierda 16x32 en X 0..15; derecha 16x32 en X 240..255. Letrero 16x8 centrado.
; Datos graficos en house_gfx.inc (sin cambios). Llegada y victoria: tanda 2b.
EXIT_COUNT          EQU 28
HOUSE_APPEAR_FRAMES EQU 51      ; la casa aparece oculta 56 ticks del arcade (SLEEP 38H en HOUSEB) x 0,914
HOUSE_FLASH_FRAMES  EQU 21      ; fotogramas que dura cada letrero (HOME o REX); ciclo completo 0,83 s medido en MAME
HOUSE_FORCE_EXIT    EQU -1      ; >= 0 fuerza esa salida (0..27) en cada arranque, para pruebas; -1 = al azar
    ASSERT HOUSE_FORCE_EXIT < EXIT_COUNT
; house_select - elige al azar la salida de la casa (arcade: PLAYER_HOUSE -> FIND_OPEN_EXIT). Sin filtros: en la fase 1 todas valen.
;   Llamar tras eggs_setup. Salida: house_exit = 0..27. Altera: A, HL
house_select:
    call rand16
    ld a,h
.m:
    cp EXIT_COUNT
    jr c,.ok
    sub EXIT_COUNT
    jr .m
.ok:
    IF HOUSE_FORCE_EXIT >= 0
    ld a,HOUSE_FORCE_EXIT
    ENDIF
    ld (house_exit),a
    ret
; house_stamp - funde un sello (mascara, grafico por byte) en BG_BASE y pide el volcado por teselas de 3 bytes.
;   Entrada: HL = datos, B = linea superior, C = byte, D = ancho (bytes), E = alto (lineas, <= 42). Altera: todos
;   Solo en la fase de logica (antes de prepare_all). La tesela de la ultima columna se desplaza a la 29 si hace falta (decor_push3 exige columna <= 29).
house_stamp:
    ld (hs_data),hl
    ld (hs_byte),bc             ; C = byte, B = linea
    ld (hs_h),de                ; E = alto, D = ancho
    ld a,c
    add a,d
    ld (hs_end),a               ; byte final (exclusivo)
    IF SPR_BANK != 2
    PAGE_BG
    ENDIF
    ld a,(hs_line)
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl                   ; linea * 32
    ld de,BG_BASE
    add hl,de
    ld a,(hs_byte)
    ld e,a
    ld d,0
    add hl,de
    ex de,hl                    ; DE = direccion en el fondo limpio
    ld hl,(hs_data)
    ld a,(hs_h)
    ld b,a
.line:
    push bc
    push de
    ld a,(hs_w)
    ld b,a
.byte:
    ld a,(de)
    and (hl)                    ; fondo AND mascara
    inc hl
    or (hl)                     ; OR grafico
    inc hl
    ld (de),a
    inc de
    djnz .byte
    pop de
    ld a,e
    add a,32                    ; siguiente linea del fondo (32 bytes por linea)
    ld e,a
    jr nc,.nc
    inc d
.nc:
    pop bc
    djnz .line
    ld a,(hs_line)
    ld b,a
    ld a,(hs_h)
    ld d,a
    ld a,(hs_byte)
    ld c,a
.tile:
    push bc
    ld a,c
    cp 30
    jr c,.col
    ld c,29                     ; ultima columna posible de una tesela de 3 bytes
.col:
    call decor_push3            ; B = linea, C = columna, D = alto; altera AF, E, HL
    pop bc
    ld a,c
    add a,3
    ld c,a
    ld a,(hs_end)
    cp c
    jr z,.done
    jr nc,.tile                 ; mientras columna < byte final
.done:
    ret
; house_build - prepara la construccion de la casa en la salida elegida (arcade: EGGDINO_HOUSE -> FIRST_HOUSE -> HOUSEB).
;   Calcula la geometria y arranca la espera (la casa aparece oculta HOUSE_APPEAR_FRAMES); no dibuja: lo hace house_flash_update. Altera: todos
house_build:
    ld a,(house_state)
    or a
    ret nz                      ; ya construida o en construccion
    ld a,(house_exit)
    ld e,a
    add a,a
    add a,e
    ld e,a
    ld d,0
    ld hl,exit_tab
    add hl,de
    ld e,(hl)                   ; E = columna
    inc hl
    ld d,(hl)                   ; D = fila
    inc hl
    ld a,(hl)                   ; A = direccion de salida
    push af
    call node_line_col          ; B = linea superior de la calle, C = byte de la calle
    pop af
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld hl,ext_right
    ld de,$0220                 ; D = 2 bytes de ancho, E = 32 lineas
    ld c,30
    ld a,b
    sub 8
    ld b,a                      ; linea superior = calle - 8
    ld a,12                     ; letrero 12 lineas por debajo
    jr .store
.left:
    ld hl,ext_left
    ld de,$0220
    ld c,0
    ld a,b
    sub 8
    ld b,a
    ld a,12
    jr .store
.up:
    ld hl,ext_top
    ld de,$0414                 ; D = 4 bytes de ancho, E = 20 lineas
    dec c                       ; la extension empieza un byte antes que la calle
    ld b,24
    ld a,6
    jr .store
.down:
    ld hl,ext_bottom
    ld de,$0414
    dec c
    ld b,172
    ld a,6
.store:                         ; HL = datos, D = ancho, E = alto, B = linea superior, C = byte, A = desplazamiento del letrero
    ld (house_ext_ptr),hl
    ld (house_ext_byte),bc      ; C = byte, B = linea
    ld (house_ext_h),de         ; E = alto, D = ancho
    add a,b
    ld (house_plq_line),a
    ld a,d
    srl a
    dec a                       ; 1 si ancho = 4 bytes, 0 si ancho = 2
    add a,c
    ld (house_plq_byte),a
    xor a
    ld (house_frame),a
    ld a,HOUSE_APPEAR_FRAMES
    ld (house_timer),a
    ld a,1
    ld (house_state),a
    ret
; house_flash_update - una vez por fotograma: espera, aparicion de la casa y alternancia HOME/REX (arcade: HOUSE_FLASH_LP). Altera: todos
;   Estados: 0 sin casa, 1 apareciendo (oculta), 2 visible.
house_flash_update:
    ld a,(house_state)
    or a
    ret z
    ld hl,house_timer
    dec (hl)
    ret nz
    cp 1
    jr z,.appear
    ld a,(house_frame)          ; estado 2: alterna HOME/REX
    xor 1
    ld (house_frame),a
    ld a,HOUSE_FLASH_FRAMES
    ld (house_timer),a
    jr house_draw_plaque
.appear:
    ld a,2
    ld (house_state),a
    ld a,HOUSE_FLASH_FRAMES
    ld (house_timer),a
    ld hl,(house_ext_ptr)
    ld bc,(house_ext_byte)      ; C = byte, B = linea
    ld de,(house_ext_h)         ; E = alto, D = ancho
    call house_stamp            ; extension; sigue con el letrero
house_draw_plaque:
    ld a,(house_frame)
    rrca
    rrca
    rrca                        ; fotograma * 32
    ld e,a
    ld d,0
    ld hl,plaque_gfx
    add hl,de
    ld a,(house_plq_line)
    ld b,a
    ld a,(house_plq_byte)
    ld c,a
    ld de,$0208                 ; D = 2 bytes, E = 8 lineas
    jp house_stamp
; ---- Tabla de salidas: columna, fila del nodo de retorno, direccion de salida ----
exit_tab:
    DB 0,0,D_UP, 1,0,D_UP, 2,0,D_UP, 3,0,D_UP
    DB 4,0,D_UP, 5,0,D_UP, 6,0,D_UP, 7,0,D_UP
    DB 8,0,D_UP, 0,4,D_DOWN, 1,4,D_DOWN, 2,4,D_DOWN
    DB 3,4,D_DOWN, 4,4,D_DOWN, 5,4,D_DOWN, 6,4,D_DOWN
    DB 7,4,D_DOWN, 8,4,D_DOWN, 0,0,D_LEFT, 0,1,D_LEFT
    DB 0,2,D_LEFT, 0,3,D_LEFT, 0,4,D_LEFT, 8,0,D_RIGHT
    DB 8,1,D_RIGHT, 8,2,D_RIGHT, 8,3,D_RIGHT, 8,4,D_RIGHT
exit_tab_end:
    ASSERT (exit_tab_end - exit_tab) == EXIT_COUNT*3
; ---- Variables ----
house_exit:      DB 0           ; salida elegida 0..27
house_state:     DB 0           ; 0 sin casa, 1 apareciendo (oculta), 2 visible
house_timer:     DB 0
house_frame:     DB 0           ; 0 = HOME, 1 = REX
house_ext_ptr:   DW 0
house_ext_byte:  DB 0           ; (byte, linea) seguidos: ld (house_ext_byte),bc
house_ext_line:  DB 0
house_ext_h:     DB 0           ; (alto, ancho) seguidos: ld (house_ext_h),de
house_ext_w:     DB 0
house_plq_line:  DB 0
house_plq_byte:  DB 0
hs_data:         DW 0
hs_byte:         DB 0           ; (byte, linea) seguidos
hs_line:         DB 0
hs_h:            DB 0           ; (alto, ancho) seguidos
hs_w:            DB 0
hs_end:          DB 0
    INCLUDE "house_gfx.inc"
