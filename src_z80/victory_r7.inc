; victory_r7.inc revision0 (tanda 2b: llegada a casa y victoria provisional). PROPUESTA: el estado lo fija Carlos tras la prueba. Sin ejecucion del asistente.
; Origen logico: victory.inc del motor anterior (house_on_street, house_arrive_check, win_update, win_message, print_text). Cambia el dibujo: sin sprite_erase/sprite_draw;
;   el dino se vuelve a pintar por el renderer (E_IMG + E_DIRTY) y el texto se escribe directo en pantalla, en el centro del campo, lejos de las celdas de llegada.
; Arcade (observaciones del motor anterior): ISHOME se consulta en DINO_OUT_OF_BOUNDS; con casa propia y objeto casa existe -> CELEBRATE; si no, caida (DINODTH5).
;   DINO_SAFETY no frena al dino si DINO_LOOK_HOME ve la casa de frente. El objeto casa existe desde que HOUSEB lo guarda (antes de los 56 ticks ocultos): house_state >= 1.
;   CELEBRATE: DINOTURN a DOWN + STANDU, espera, mensaje (uno de cinco con un jugador), WAVE_END.
; Port: si house_state >= 1 y el dino va hacia fuera por la calle de la salida de la casa (misma direccion y misma columna o fila que el nodo de retorno),
;   check_fall no frena ni cae: avanza hasta la celda adicional (nodo del borde + NODE_STEP hacia fuera, decision de Carlos 13-oct) y ahi empieza la victoria
;   (E_STATE = 3). Victoria provisional: el dino mira abajo, uno de cinco textos y, tras WIN_WAIT_FRAMES, reinicio del nivel (jp start; con la misma semilla).
HOUSE_ARR_TOP_Y     EQU NODE_Y0-NODE_STEP       ; 28: celda adicional arriba (origen del dino)
HOUSE_ARR_BOTTOM_Y  EQU NODE_Y_LAST+NODE_STEP   ; 172
HOUSE_ARR_LEFT_X    EQU NODE_X0-NODE_STEP       ; 0
HOUSE_ARR_RIGHT_X   EQU NODE_X_LAST+NODE_STEP   ; 240
WIN_WAIT_FRAMES     EQU 100                     ; PROVISIONAL: 2 s con el texto antes de reiniciar
WIN_TEXT_LINE       EQU 96                      ; linea de pixeles del texto (multiplo de 8)
    ASSERT HOUSE_ARR_TOP_Y >= 0
    ASSERT HOUSE_ARR_LEFT_X >= 0
    ASSERT HOUSE_ARR_BOTTOM_Y <= CLIP_BOTTOM-16
    ASSERT HOUSE_ARR_RIGHT_X <= 240
    ASSERT (WIN_TEXT_LINE & 7) == 0
; house_on_street - NZ si el dino va hacia fuera por la calle de la salida de la casa (arcade: ISHOME + DINO_LOOK_HOME); Z si no.
;   Altera: A, BC, DE, HL
house_on_street:
    ld a,(house_state)
    or a
    ret z                       ; sin casa: Z
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
    ld a,(ENT0+E_DIR)
    cp (hl)
    jr nz,.no                   ; no va hacia fuera por esa salida
    push af                     ; A = direccion
    call node_line_col          ; B = linea (Y) del nodo, C = byte del nodo
    pop af
    cp D_LEFT
    jr nc,.horiz
    ld a,c                      ; vertical: la X del dino debe ser la del nodo (byte * 8)
    add a,a
    add a,a
    add a,a
    ld c,a
    ld a,(ENT0+E_X)
    cp c
    jr nz,.no
    jr .yes
.horiz:
    ld a,(ENT0+E_Y)             ; horizontal: la Y del dino debe ser la del nodo
    cp b
    jr nz,.no
.yes:
    or 1                        ; NZ
    ret
.no:
    xor a                       ; Z
    ret
; house_arrive_check - tras un paso por la calle de la casa: si el dino ha llegado a la celda adicional, empieza la victoria.
;   Se salta desde check_fall con JP, asi que sus RET vuelven al llamador de check_fall. Altera: A, BC, DE, HL
house_arrive_check:
    ld a,(ENT0+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    ld a,(ENT0+E_X)             ; derecha
    cp HOUSE_ARR_RIGHT_X
    ret c                       ; todavia no
    jr .arrive
.left:
    ld a,(ENT0+E_X)
    cp HOUSE_ARR_LEFT_X+1
    ret nc
    jr .arrive
.up:
    ld a,(ENT0+E_Y)
    cp HOUSE_ARR_TOP_Y+1
    ret nc
    jr .arrive
.down:
    ld a,(ENT0+E_Y)
    cp HOUSE_ARR_BOTTOM_Y
    ret c
.arrive:
    ld a,3
    ld (ENT0+E_STATE),a         ; 3 = victoria (dino_update -> win_update)
    xor a
    ld (win_phase),a            ; la primera pasada de win_update gira al dino y escribe el texto
    ret
; win_update - estado 3 (victoria provisional). Primera pasada: el dino mira abajo, se escribe el texto y arranca la espera. Despues espera y reinicia el nivel.
;   Devuelve A = 1 si hay que repintar al dino, 0 si no (como dino_update). El letrero de la casa sigue alternando (house_flash_update). Altera: todos
win_update:
    ld a,(win_phase)
    or a
    jr nz,.wait
    ld a,D_DOWN
    ld (ENT0+E_DIR),a           ; arcade: DINOTURN a DOWN y STANDU
    ld (ENT0+E_IMG),a
    call win_message
    ld a,WIN_WAIT_FRAMES
    ld (win_timer),a
    ld a,1
    ld (win_phase),a
    ret                         ; A = 1: repintar al dino mirando abajo
.wait:
    ld hl,win_timer
    dec (hl)
    jr nz,.idle
    jp start                    ; reinicio provisional del nivel (pantalla, huevos, salida y dino)
.idle:
    xor a
    ret
; win_message - elige uno de los cinco textos al azar y lo escribe centrado en WIN_TEXT_LINE. Altera: A, BC, DE, HL
win_message:
    call rand16
    ld a,h
.m:
    cp 5
    jr c,.ok
    sub 5
    jr .m
.ok:
    add a,a                     ; punteros de 16 bits
    ld e,a
    ld d,0
    ld hl,win_msgs
    add hl,de
    ld e,(hl)
    inc hl
    ld d,(hl)
    ex de,hl                    ; HL = cadena
    push hl
    ld b,0
.len:
    ld a,(hl)
    or a
    jr z,.centre
    inc hl
    inc b
    jr .len
.centre:
    ld a,32
    sub b
    srl a
    ld c,a                      ; byte inicial = (32 - longitud) / 2
    pop hl
    ld b,WIN_TEXT_LINE
; print_text - escribe una cadena ASCII (mayusculas, terminada en 0) con font_msg en la linea B (multiplo de 8), empezando en el byte C
;   Entrada: HL = cadena, B = linea, C = byte inicial. Altera: A, BC, DE, HL
print_text:
.l:
    ld a,(hl)
    or a
    ret z
    push hl
    push bc
    sub 32
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl                   ; * 8
    ld de,font_msg
    add hl,de
    ex de,hl                    ; DE = glifo
    call pixel_addr             ; HL = pantalla (B = linea, C = byte)
    call put_glyph              ; copia 8 lineas
    pop bc
    pop hl
    inc hl
    inc c
    jr .l
; ---- Textos provisionales (arcade: COMP_MESSAGES, un jugador) ----
win_msgs:
    DW msg_congrat, msg_goodjob, msg_excel, msg_waytogo, msg_dinomite
msg_congrat:    DB "CONGRATULATIONS!",0
msg_goodjob:    DB "GOOD JOB!",0
msg_excel:      DB "EXCELLENT!",0
msg_waytogo:    DB "WAY TO GO!",0
msg_dinomite:   DB "DINO-MITE!",0
win_phase:      DB 0            ; 0 = primera pasada de win_update, 1 = esperando
win_timer:      DB 0            ; fotogramas que quedan antes de reiniciar el nivel
