; eggs_r7.inc revision7: paso 2a PROBADO por Carlos en 128K/48K, 17-oct00:45; rev6 (egg_pickup respeta resp_eat) PROBADA. Rev7 (tanda 2a): el ultimo huevo llama a house_build.
; eggs_setup antes de build_bg, PAGE_BG activo; egg_pickup despues de dino_update/qpos.
; FLOOR_SAVE=$D800-$DD9F (45x32); solo guardar nodos ocupados. Ver huevos_paso2a.md.
; Ultimo huevo all_eggs=1 y house_build (house_r7.inc). Llegada y victoria pendientes (tanda 2b). El estado de la rev7 lo fija Carlos tras la prueba.
; Guardado del suelo en banco 0 (128K) o RAM plana (48K), $D800-$DD9F.
; 45 huecos de 32 bytes; solo inicializados los nodos con huevo (43).
; all_eggs=1 al terminar. Reaparicion: respawn_r7.inc (paso 2b).
FLOOR_SAVE      EQU BG_BASE+BG_SIZE
FLOOR_SAVE_END  EQU FLOOR_SAVE+NODE_COUNT*32
    ASSERT FLOOR_SAVE_END <= $10000
    ASSERT EGG_H == 16
    ASSERT NODE_COUNT == 45
    ASSERT (egg_gfx_end-egg_gfx) == EGG_H*4

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

; A -> cociente A/24. Altera AF,C. Conserva B,DE,HL.
div24:
    ld c,0
.loop:
    cp NODE_STEP
    jr c,.done
    sub NODE_STEP
    inc c
    jr .loop
.done:
    ld a,c
    ret

place_eggs:
    ld hl,egg_map
    ld b,NODE_COUNT
    ld a,1
.fill:
    ld (hl),a
    inc hl
    djnz .fill
    xor a
    ld (egg_map),a
    call rand16
    ld a,h
.mod:
    cp NODE_COUNT-1
    jr c,.node
    sub NODE_COUNT-1
    jr .mod
.node:
    inc a
    ld e,a
    ld d,0
    ld hl,egg_map
    add hl,de
    ld (hl),0
    ld a,EGG_TOTAL
    ld (eggs_left),a
    xor a
    ld (eggs_got),a
    ld (all_eggs),a
    ret

; D=fila,E=columna -> B=linea,C=byte. Altera AF,BC; conserva DE,HL.
node_line_col:
    ld a,d
    add a,a
    add a,a
    add a,a
    ld b,a
    add a,a
    add a,b
    add a,NODE_Y0
    ld b,a
    ld a,e
    ld c,a
    add a,a
    add a,c
    add a,NODE_X0/8
    ld c,a
    ret

; D=fila,E=columna -> A=indice (0..44). Conserva BC,DE,HL.
egg_index:
    ld a,d
    add a,a
    add a,a
    add a,a
    add a,d
    add a,e
    ret

; D=fila,E=columna -> HL=FLOOR_SAVE+indice*32. Altera AF,BC,HL; conserva DE.
egg_floor_ptr:
    call egg_index
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ld bc,FLOOR_SAVE
    add hl,bc
    ret

; Preparar la pantalla inicial y guardar el suelo antes de cada huevo. Altera todos.
eggs_setup:
    ld hl,EGG_SEED
    ld (rng_state),hl
    call place_eggs
    ld d,0
.row:
    ld e,0
.col:
    push de
    IF EGG_TEST_FLOOR
    call egg_test_floor
    pop de
    push de
    ENDIF
    call egg_index
    ld c,a
    ld b,0
    ld hl,egg_map
    add hl,bc
    ld a,(hl)
    or a
    jr z,.next
    push de
    call save_egg_floor
    pop de
    call draw_egg
.next:
    pop de
    inc e
    ld a,e
    cp NODE_COLS
    jr c,.col
    inc d
    ld a,d
    cp NODE_ROWS
    jr c,.row
    jp draw_egg_counter

; D=fila,E=columna: guarda 2x16 bytes desde pantalla (SIN huevo). Altera AF,BC,DE,HL.
save_egg_floor:
    call egg_floor_ptr
    push hl
    call node_line_col
    call pixel_addr
    pop de
    ld b,EGG_H
.line:
    ld a,(hl)
    ld (de),a
    inc hl
    inc de
    ld a,(hl)
    ld (de),a
    inc de
    dec hl
    call down_hl
    djnz .line
    ret

; D=fila,E=columna: dibuja huevo con mascara, SOLO al montar el nivel.
draw_egg:
    call node_line_col
    call pixel_addr
    ld de,egg_gfx
    ld b,EGG_H
.line:
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
    djnz .line
    ret

    IF EGG_TEST_FLOOR
; Trama de diagnostico 16x16 bajo CADA nodo. No es arte definitivo.
egg_test_floor:
    call node_line_col
    call pixel_addr
    ld b,EGG_H
.line:
    ld (hl),$88
    inc hl
    ld (hl),$22
    dec hl
    call down_hl
    djnz .line
    ret
    ENDIF

; D=fila,E=columna: restaura suelo en fondo limpio y encola tesela 3x16. Altera todos.
egg_remove:
    IF SPR_BANK != 2
    PAGE_BG
    ENDIF
    call egg_floor_ptr
    push hl
    call node_line_col
    ld a,b
    ld (eg_line),a
    ld a,c
    ld (eg_byte),a
    ld l,b
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,BG_BASE
    add hl,de
    ld e,c
    ld d,0
    add hl,de
    ex de,hl
    pop hl
    ld b,EGG_H
.line:
    ld a,(hl)
    ld (de),a
    inc hl
    inc de
    ld a,(hl)
    ld (de),a
    inc hl
    ld a,e
    add a,31
    ld e,a
    jr nc,.down
    inc d
.down:
    djnz .line
    ld a,(eg_line)
    ld b,a
    ld a,(eg_byte)
    ld c,a
    ld d,EGG_H
    jp decor_push3

; Recogida cerca del nodo real. Protege indice y alineacion transversal. Altera todos.
egg_pickup:
    ld a,(ENT0+E_STATE)
    or a
    ret nz
    ld a,(resp_eat)
    or a
    ret z                       ; muerte lejos del nodo: el huevo del nodo espera al primer paso (rev6)
    call node_offset
    ret nz
    ld a,e
    or a
    ld a,(ENT0+E_Y)
    jr z,.position
    ld a,(ENT0+E_X)
.position:
    sub d
    jr nc,.abs
    neg
.abs:
    cp EGG_TOL+1
    ret nc
    ld a,e
    or a
    jr nz,.horizontal
    ld a,d
    sub NODE_Y0
    call div24
    ld d,a
    ld a,(ENT0+E_X)
    sub NODE_X0
    ret c
    cp NODE_X_LAST-NODE_X0+1
    ret nc
    ld b,a
    call mod_step
    ret nz
    ld a,b
    call div24
    ld e,a
    jr .node
.horizontal:
    ld a,d
    sub NODE_X0
    call div24
    ld e,a
    ld a,(ENT0+E_Y)
    sub NODE_Y0
    ret c
    cp NODE_Y_LAST-NODE_Y0+1
    ret nc
    ld b,a
    call mod_step
    ret nz
    ld a,b
    call div24
    ld d,a
.node:
    ld a,d
    cp NODE_ROWS
    ret nc
    ld a,e
    cp NODE_COLS
    ret nc
    call egg_index
    ld c,a
    ld b,0
    ld hl,egg_map
    add hl,bc
    ld a,(hl)
    or a
    ret z
    ld (hl),0
    ld hl,eggs_left
    dec (hl)
    ld hl,eggs_got
    inc (hl)
    call egg_remove
    call draw_egg_counter
    ld a,(eggs_left)
    or a
    ret nz
    ld a,1
    ld (all_eggs),a
    jp house_build              ; ultimo huevo: construye la casa (arcade: EGGDINO_BUILD)

put_glyph:
    ld b,8
.line:
    ld a,(de)
    ld (hl),a
    inc de
    inc h
    djnz .line
    ret

digit_ptr:
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

put_number2:
    ld e,0
.tens:
    cp 10
    jr c,.units
    sub 10
    inc e
    jr .tens
.units:
    ld d,a
    push de
    push bc
    ld a,e
    call digit_ptr
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
    ld de,font_msg+('/'-32)*8
    call put_glyph
    ld a,EGG_TOTAL
    ld c,HUD_EGG_COL+4
    jp put_number2

egg_map:    DS NODE_COUNT
eggs_left:  DB 0
eggs_got:   DB 0
all_eggs:   DB 0
rng_state:  DW EGG_SEED
eg_line:    DB 0
eg_byte:    DB 0
