; ============================================================================
; pixel_addr - direccion de pantalla de (byte, linea)
;   Entrada: B = linea (0..191), C = columna en bytes (0..31)
;   Salida : HL = direccion en la pantalla de pixeles
;   Altera : A, HL
; ============================================================================
pixel_addr:
    ld a,b
    and %00000111               ; y2 y1 y0
    or  %01000000               ; base $40xx
    ld h,a
    ld a,b
    rra
    rra
    rra
    and %00011000               ; y7 y6 -> bits 4-3 de H
    or h
    ld h,a
    ld a,b
    rla
    rla
    and %11100000               ; y5 y4 y3 -> bits 7-5 de L
    or c
    ld l,a
    ret

; ============================================================================
; down_hl - baja HL una linea de pixeles dentro de la pantalla
;   Entrada: HL = direccion de pantalla. Salida: HL = linea siguiente
;   Altera : A, HL
; ============================================================================
down_hl:
    inc h
    ld a,h
    and 7
    ret nz
    ld a,l
    add a,32
    ld l,a
    ret c
    ld a,h
    sub 8
    ld h,a
    ret

; ============================================================================
; draw_field - pinta el agua del campo (Y FIELD_TOP..191) con una trama de damero.
;   El suelo (X FLOOR_LEFT..FLOOR_RIGHT, Y FLOOR_TOP..FLOOR_BOTTOM) queda vacio: en esas lineas solo se escribe el agua lateral de cada lado
;   (WATER_FULL bytes completos y, si WATER_PART = 1, un byte parcial con mascara; con EDGE_GAP = 4: bytes 0 y 31 completos, byte 1 con $F0 y byte 30 con $0F).
;   Requiere la pantalla de pixeles a cero. Altera : A, BC, DE, HL
; ============================================================================
draw_field:
    ld b,FIELD_TOP
.line:
    push bc
    ld c,0
    call pixel_addr             ; HL = byte 0 de la linea B
    ld a,b
    and 1
    ld a,WATER_EVEN             ; ld no modifica las banderas
    jr z,.have
    ld a,WATER_ODD
.have:
    ld d,a                      ; D = trama de esta linea
    ld a,b
    cp FLOOR_TOP
    jr c,.full                  ; agua superior
    cp FLOOR_BOTTOM+1
    jr nc,.full                 ; agua inferior
    ld e,WATER_FULL             ; agua lateral izquierda: bytes completos
.wl:
    ld (hl),d
    inc l
    dec e
    jr nz,.wl
    IF WATER_PART
    ld a,d
    and WATER_MASK_L
    ld (hl),a                   ; byte parcial (sin avanzar)
    ENDIF
    ld a,l
    add a,32-2*WATER_FULL-WATER_PART
    ld l,a                      ; primera columna de agua de la derecha
    IF WATER_PART
    ld a,d
    and WATER_MASK_R
    ld (hl),a                   ; byte parcial derecho
    inc l
    ENDIF
    ld e,WATER_FULL
.wr:
    ld (hl),d
    inc l
    dec e
    jr nz,.wr
    jr .next
.full:
    ld e,32
.f:
    ld (hl),d
    inc l
    dec e
    jr nz,.f
.next:
    pop bc
    inc b
    ld a,b
    cp CLIP_BOTTOM
    jr nz,.line
    ret

; ============================================================================
; draw_stones - pinta la rejilla de 10 x 6 piedras de 8x8 con mascara, alineadas a byte.
;   X = 16 + 24*c (byte 2 + 3*c), Y = GRID_TOP + 24*r. La variante de cada piedra es stone_map[10*r + c];
;   cada variante son 16 bytes en stone_gfx (8 lineas x mascara, grafico): pantalla = (pantalla AND mascara) OR grafico.
;   Altera : A, BC, DE, HL
; ============================================================================
draw_stones:
    ld d,0                      ; D = fila r
.r:
    ld e,0                      ; E = columna c
.c:
    push de
    ld a,d
    add a,a
    ld b,a
    add a,a
    add a,a
    add a,b                     ; A = 10 * r
    add a,e                     ; A = 10 * r + c
    ld c,a
    ld b,0
    ld hl,stone_map
    add hl,bc
    ld a,(hl)                   ; A = variante
    add a,a
    add a,a
    add a,a
    add a,a                     ; A = variante * 16
    ld c,a
    ld hl,stone_gfx
    add hl,bc                   ; HL = datos de la variante (B sigue en 0)
    push hl
    ld c,d
    ld hl,stone_y
    add hl,bc
    ld a,(hl)
    push af                     ; linea inicial
    ld c,e
    ld hl,stone_x
    add hl,bc
    ld c,(hl)                   ; C = byte de pantalla
    pop af
    ld b,a                      ; B = linea
    call pixel_addr             ; HL = pantalla
    pop de                      ; DE = datos de la variante
    ld b,STONE_H
.line:
    ld a,(de)                   ; mascara
    and (hl)
    ld c,a
    inc de
    ld a,(de)                   ; grafico
    or c
    ld (hl),a
    inc de
    call down_hl
    djnz .line
    pop de
    inc e
    ld a,e
    cp STONE_COLS
    jr nz,.c
    inc d
    ld a,d
    cp STONE_ROWS
    jr nz,.r
    ret

stone_x:    DB 2,5,8,11,14,17,20,23,26,29                       ; byte de pantalla de X = 16 + 24*c
stone_y:    DB GRID_TOP,GRID_TOP+24,GRID_TOP+48,GRID_TOP+72,GRID_TOP+96,GRID_TOP+120   ; linea de Y = GRID_TOP + 24*r
