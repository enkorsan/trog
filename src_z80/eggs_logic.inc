; ============================================================================
; rand16 - xorshift de 16 bits (periodo 65535). Salida: HL. Altera : A, HL
; ============================================================================
rand16:
    ld hl,(rng_state)
    ld a,h
    rra
    ld a,l
    rra
    xor h
    ld h,a
    ld a,l
    rra
    ld a,h
    rra
    xor l
    ld l,a
    xor h
    ld h,a
    ld (rng_state),hl
    ret

; ============================================================================
; place_eggs - 43 huevos: todas las intersecciones salvo la salida del dino (nodo 0) y otra al azar
;   (arcade: ALLOT_EGGS deja 2 libres para Trog; la salida queda libre porque EGGHATCH evita los dinos cercanos). Altera : A, BC, DE, HL
; ============================================================================
place_eggs:
    ld hl,egg_map
    ld b,NODE_COUNT
    ld a,1
.f:
    ld (hl),a
    inc hl
    djnz .f
    xor a
    ld (egg_map),a              ; la salida queda libre
    call rand16
    ld a,h
.m:
    cp NODE_COUNT-1
    jr c,.ok
    sub NODE_COUNT-1
    jr .m
.ok:
    inc a                       ; 1..44
    ld e,a
    ld d,0
    ld hl,egg_map
    add hl,de
    ld (hl),0
    ld a,EGG_TOTAL
    ld (eggs_left),a
    xor a
    ld (eggs_got),a
    ret

; ============================================================================
; node_line_col - posicion en pantalla de la esquina superior izquierda de la calle de una interseccion
;   Entrada: D = fila, E = columna. Salida: B = linea, C = byte. Altera : A
; ============================================================================
node_line_col:
    ld a,d
    add a,a
    add a,a
    add a,a                     ; 8 * fila
    ld b,a
    add a,a                     ; 16 * fila
    add a,b                     ; 24 * fila
    add a,NODE_Y0
    ld b,a
    ld a,e
    ld c,a
    add a,a
    add a,c                     ; 3 * columna
    add a,NODE_X0/8
    ld c,a
    ret

; ============================================================================
; draw_egg - pinta el huevo de la interseccion (D = fila, E = columna) con mascara. Altera : A, BC, DE, HL
; ============================================================================
draw_egg:
    call node_line_col
    call pixel_addr             ; HL = pantalla
    ld de,egg_gfx
    ld b,EGG_H
.l:
    ld a,(de)
    and (hl)
    ld c,a
    inc de
    ld a,(de)
    or c
    ld (hl),a
    inc de
    inc hl
    ld a,(de)
    and (hl)
    ld c,a
    inc de
    ld a,(de)
    or c
    ld (hl),a
    inc de
    dec hl
    call down_hl
    djnz .l
    ret

; ============================================================================
; erase_egg_area - pone a cero los 16x16 de la calle de una interseccion (el suelo es liso). Entrada: D = fila, E = columna. Altera : A, BC, HL
; ============================================================================
erase_egg_area:
    call node_line_col
    call pixel_addr
    ld b,EGG_H
.l:
    xor a
    ld (hl),a
    inc hl
    ld (hl),a
    dec hl
    call down_hl
    djnz .l
    ret

; ============================================================================
; draw_all_eggs - pinta todos los huevos de egg_map. Altera : A, BC, DE, HL
; ============================================================================
draw_all_eggs:
    ld hl,egg_map
    ld d,0                      ; fila
.r:
    ld e,0                      ; columna
.c:
    ld a,(hl)
    inc hl
    or a
    jr z,.skip
    push hl
    push de
    call draw_egg
    pop de
    pop hl
.skip:
    inc e
    ld a,e
    cp NODE_COLS
    jr nz,.c
    inc d
    ld a,d
    cp NODE_ROWS
    jr nz,.r
    ret

; ============================================================================
; egg_pickup - recoge el huevo de la interseccion si el dino esta a EGG_TOL px o menos de ella (arcade: DINO_NEXT_MOVE -> EGG_OBJ -> EGGDINO).
;   Se llama con el sprite del dino borrado de la pantalla. Altera : A, BC, DE, HL
; ============================================================================
egg_pickup:
    call node_offset            ; Z = cerca (TURN_TOL); D = coord. del nodo en el eje de marcha; E = eje
    ret nz
    ld a,e
    or a
    ld a,(dino_y)               ; ld no toca las banderas
    jr z,.pos_ok
    ld a,(dino_x)
.pos_ok:
    sub d
    jr nc,.abs
    neg
.abs:
    cp EGG_TOL+1
    ret nc                      ; a mas de EGG_TOL px: no se recoge
    ld a,e
    or a
    jr nz,.eje_x
    ld a,d                      ; marcha vertical: D = Y del nodo; la columna sale de dino_x
    sub NODE_Y0
    call div24
    ld d,a                      ; fila
    ld a,(dino_x)
    sub NODE_X0
    call div24
    ld e,a                      ; columna
    jr .have
.eje_x:
    ld a,d                      ; marcha horizontal: D = X del nodo; la fila sale de dino_y
    sub NODE_X0
    call div24
    ld e,a                      ; columna
    ld a,(dino_y)
    sub NODE_Y0
    call div24
    ld d,a                      ; fila
.have:
    ld a,d                      ; indice = fila*9 + columna
    add a,a
    add a,a
    add a,a
    add a,d
    add a,e
    ld c,a
    ld b,0
    ld hl,egg_map
    add hl,bc
    ld a,(hl)
    or a
    ret z                       ; no hay huevo
    ld (hl),0
    ld hl,eggs_left
    dec (hl)
    ld hl,eggs_got
    inc (hl)
    call erase_egg_area         ; D = fila, E = columna
    call draw_egg_counter
    ld a,(eggs_left)
    or a
    ret nz
    jp house_build              ; ultimo huevo: construye la casa (arcade: EGGDINO_BUILD)

; ============================================================================
; put_glyph - copia un glifo de 8 bytes a pantalla. Entrada: DE = glifo, HL = direccion de pantalla de la primera linea. Altera : A, B, DE, HL
; ============================================================================
put_glyph:
    ld b,8
.l:
    ld a,(de)
    ld (hl),a
    inc de
    inc h
    djnz .l
    ret

; ============================================================================
; digit_ptr - A = 0..10 (cifra o barra) -> DE = glifo. Altera : A, DE, HL
; ============================================================================
digit_ptr:                      ; A = 0..9 (cifra) -> DE = glifo de font_msg. Altera : A, DE, HL
    add a,'0'-32
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,font_msg
    add hl,de
    ex de,hl
    ret

; ============================================================================
; put_number2 - escribe un numero de dos cifras en la fila del contador. Entrada: A = valor 0..99, C = columna de celda. Altera : A, BC, DE, HL
; ============================================================================
put_number2:
    ld e,0                      ; decenas
.t:
    cp 10
    jr c,.u
    sub 10
    inc e
    jr .t
.u:
    ld d,a                      ; D = unidades
    push de
    push bc
    ld a,e
    call digit_ptr              ; DE = glifo de las decenas
    pop bc
    push bc
    ld b,HUD_EGG_LINE
    call pixel_addr
    call put_glyph
    pop bc
    pop de
    inc c
    ld a,d
    push bc
    call digit_ptr
    pop bc
    ld b,HUD_EGG_LINE
    call pixel_addr
    jp put_glyph

; ============================================================================
; draw_egg_counter - icono, huevos recogidos, barra y total en la fila 1 del HUD. Altera : A, BC, DE, HL
; ============================================================================
draw_egg_counter:
    ld b,HUD_EGG_LINE
    ld c,HUD_EGG_COL
    call pixel_addr
    ld de,glyph_egg
    call put_glyph
    ld a,(eggs_got)
    ld c,HUD_EGG_COL+1
    call put_number2
    ld b,HUD_EGG_LINE
    ld c,HUD_EGG_COL+3
    call pixel_addr
    ld de,font_msg+('/'-32)*8 ; barra
    call put_glyph
    ld a,EGG_TOTAL
    ld c,HUD_EGG_COL+4
    jp put_number2

