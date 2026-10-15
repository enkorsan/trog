; ============================================================================
; bench_r2.asm - BANCO DE PRUEBAS R2 (render): variante B+ (preparar en memoria no contendida y volcar al principio del fotograma).
; Estado del codigo: PROPUESTO. La logica de ventanas se ha verificado con un modelo en Python (0 errores en 4.000 fotogramas con 1 y 2 entidades sin solapes).
; Colocar junto a main.asm (usa config.inc, video.inc, stones.inc, sprite_dino.inc y font_msg.inc con INCLUDE relativo). No toca el juego.
; Escenario identico a R1 (BENCH_N entidades con el sprite del dino que andan por las calles con el mismo acumulador de velocidad).
; Pipeline por fotograma (el dibujo de lo preparado en el fotograma n se vuelca al principio del n+1):
;   1. blit_all    (verde)  : vuelca a pantalla las ventanas preparadas el fotograma anterior (3 LDI por linea). Es lo unico que escribe en la pantalla.
;   2. update_all  (rojo)   : logica (igual que R1).
;   3. prepare_all (azul)   : para cada entidad que se ha movido compone en un buffer del banco 2 la ventana union (posicion vieja + nueva).
;   4. libre (negro)        : se cuenta mientras se espera la interrupcion (igual que R1).
; Ventana: siempre 3 bytes de ancho y 24 + |dy| lineas (24 o 26).
; LIMITACIONES de R2: no resuelve solapes entre sprites (SOLP cuenta las ventanas que se solapan con otra entidad) y no recorta por arriba ni por abajo.
; Medidas en el HUD: LIBRE, MIN, RETR, PERD como en R1, mas SOLP.
; Franjas del borde (BENCH_PROF = 1): verde = volcado, rojo = logica, azul = preparacion, negro = libre.
; ============================================================================
BENCH_N         EQU 8           ; entidades (1..8)
BENCH_STAGGER   EQU 1           ; 1 = acumuladores iniciales 3*i mod 16; 0 = todos en fase
BENCH_SPEED     EQU 7           ; pasos de 2 px cada 16 fotogramas
BENCH_PROF      EQU 1           ; 1 = franjas de color en el borde
BENCH_OVL       EQU 1           ; 1 = cuenta las ventanas que se solapan con otra entidad
    ASSERT BENCH_N >= 1 && BENCH_N <= 8

    DEVICE ZXSPECTRUM128

    INCLUDE "config.inc"

IM2_TABLE   EQU $B800
IM2_FILL    EQU $B9
IM2_VECTOR  EQU (IM2_FILL<<8)|IM2_FILL
BG_BASE     EQU $C000           ; fondo limpio: copia de los 6.144 bytes de pixeles (direccion = pantalla + $8000)

E_X         EQU 0
E_Y         EQU 1
E_DIR       EQU 2
E_ACC       EQU 3
E_SPD       EQU 4
E_COL       EQU 5
E_ROW       EQU 6
E_LEFT      EQU 7
E_MOVED     EQU 8
E_VALID     EQU 9
E_OX        EQU 10
E_OY        EQU 11
E_PEND      EQU 12
E_SC0       EQU 13
E_SL0       EQU 14
E_SH        EQU 15
E_RC        EQU 16
E_RL        EQU 17
E_PHASE     EQU 18
E_SPRW      EQU 19
E_SBUF      EQU 20
ENT_SIZE    EQU 24

    ORG $8000

start:
    di
    ld sp,$7FFF
    xor a
    out ($FE),a

    ld hl,SCR_BASE
    ld de,SCR_BASE+1
    ld bc,$17FF
    ld (hl),0
    ldir
    ld hl,ATTR_BASE
    ld de,ATTR_BASE+1
    ld bc,$02FF
    ld (hl),ATTR_HUD
    ldir
    ld hl,FIELD_ATTR_BASE
    ld de,FIELD_ATTR_BASE+1
    ld bc,FIELD_ATTR_LEN
    ld (hl),ATTR_FIELD
    ldir

    call draw_field
    call draw_stones

    ld bc,$7FFD
    ld a,$10
    out (c),a
    ld hl,SCR_BASE
    ld de,BG_BASE
    ld bc,$1800
    ldir

    call hud_labels
    call init_ents
    call prepare_all
    call blit_all

    xor a
    ld (frame_counter),a
    ld a,IM2_TABLE>>8
    ld i,a
    im 2
    ei
    ld a,(frame_counter)
    ld (frame_start),a

; main_loop - volcado, logica, preparacion, medida del tiempo libre y sincronizacion (sin HALT: se cuenta mientras se espera)
main_loop:
    IF BENCH_PROF
    ld a,4
    out ($FE),a
    ENDIF
    call blit_all
    IF BENCH_PROF
    ld a,2
    out ($FE),a
    ENDIF
    call update_all
    IF BENCH_PROF
    ld a,1
    out ($FE),a
    ENDIF
    call prepare_all
    IF BENCH_PROF
    xor a
    out ($FE),a
    ENDIF

    ld a,(frame_counter)
    ld hl,frame_start
    sub (hl)
    jr nz,.over
    ld c,(hl)
    ld de,0
.wait:
    inc de
    ld a,(frame_counter)
    cp c
    jr z,.wait
    ld (frame_start),a
    jr .account
.over:
    ld c,a
    ld hl,overruns
    inc (hl)
    ld a,(lostf)
    add a,c
    ld (lostf),a
    ld a,(frame_counter)
    ld (frame_start),a
    ld de,0
.account:
    ld hl,(idle_min)
    or a
    sbc hl,de
    jr c,.nomin
    ld (idle_min),de
.nomin:
    srl d
    rr e
    ld hl,(idle_sum)
    add hl,de
    ld (idle_sum),hl
    ld hl,win_count
    inc (hl)
    ld a,(hl)
    cp 64
    call nc,report
    jp main_loop

; report - cada 64 iteraciones escribe las medidas en el HUD y empieza otra ventana. Altera : todos
report:
    ld hl,(idle_sum)
    ld b,6
.s:
    srl h
    rr l
    djnz .s
    ld b,0
    ld c,6
    call b_print_dec4
    ld hl,(idle_min)
    ld b,0
    ld c,15
    call b_print_dec4
    ld a,(overruns)
    ld l,a
    ld h,0
    ld b,8
    ld c,5
    call b_print_dec4
    ld a,(lostf)
    ld l,a
    ld h,0
    ld b,8
    ld c,15
    call b_print_dec4
    ld hl,(ovl_cnt)
    ld b,16
    ld c,23
    call b_print_dec4
    xor a
    ld (win_count),a
    ld (overruns),a
    ld (lostf),a
    ld hl,0
    ld (idle_sum),hl
    ld (ovl_cnt),hl
    ld hl,$FFFF
    ld (idle_min),hl
    ret

frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti

; init_ents - entidad i en la interseccion (columna 2i mod 9, fila i mod 5) y buffer de ventana i. Altera : A, BC, DE, HL, IX
init_ents:
    ld ix,ents
    ld hl,scr_bufs
    ld b,BENCH_N
    ld c,0
.l:
    ld (ix+E_SBUF),l
    ld (ix+E_SBUF+1),h
    ld de,128
    add hl,de
    ld a,c
    add a,a
.mc:
    cp 9
    jr c,.cok
    sub 9
    jr .mc
.cok:
    ld (ix+E_COL),a
    ld e,a
    add a,a
    add a,e
    add a,a
    add a,a
    add a,a
    add a,NODE_X0
    ld (ix+E_X),a
    ld a,c
.mr:
    cp 5
    jr c,.rok
    sub 5
    jr .mr
.rok:
    ld (ix+E_ROW),a
    ld e,a
    add a,a
    add a,e
    add a,a
    add a,a
    add a,a
    add a,NODE_Y0
    ld (ix+E_Y),a
    IF BENCH_STAGGER
    ld a,c
    ld e,a
    add a,a
    add a,e
    and 15
    ELSE
    xor a
    ENDIF
    ld (ix+E_ACC),a
    ld (ix+E_SPD),BENCH_SPEED
    ld (ix+E_LEFT),0
    ld (ix+E_DIR),0
    ld (ix+E_MOVED),1
    ld (ix+E_VALID),0
    ld (ix+E_PEND),0
    ld de,ENT_SIZE
    add ix,de
    inc c
    djnz .l
    ret

; update_all / prepare_all / blit_all - recorren las BENCH_N entidades. Altera : todos
update_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    call ent_update
    ld (ix+E_MOVED),a
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

prepare_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    ld a,(ix+E_MOVED)
    or a
    call nz,ent_prepare
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

blit_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    call ent_blit
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

; ent_update / ent_arrive / ent_choose / get_bit / b_rand - identicos a R1 (movimiento por las calles). ent_update: A = 1 si se ha movido
ent_update:
    ld a,(ix+E_ACC)
    add a,(ix+E_SPD)
    cp 16
    jr c,.keep
    sub 16
    ld (ix+E_ACC),a
    jr .step
.keep:
    ld (ix+E_ACC),a
    xor a
    ret
.step:
    ld a,(ix+E_LEFT)
    or a
    call z,ent_choose
    ld a,(ix+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    inc (ix+E_X)
    inc (ix+E_X)
    jr .moved
.left:
    dec (ix+E_X)
    dec (ix+E_X)
    jr .moved
.up:
    dec (ix+E_Y)
    dec (ix+E_Y)
    jr .moved
.down:
    inc (ix+E_Y)
    inc (ix+E_Y)
.moved:
    dec (ix+E_LEFT)
    call z,ent_arrive
    ld a,1
    ret

ent_arrive:
    ld a,(ix+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    inc (ix+E_COL)
    ret
.left:
    dec (ix+E_COL)
    ret
.up:
    dec (ix+E_ROW)
    ret
.down:
    inc (ix+E_ROW)
    ret

ent_choose:
    ld b,0
    ld a,(ix+E_ROW)
    or a
    jr z,.no_up
    set 0,b
.no_up:
    ld a,(ix+E_ROW)
    cp NODE_ROWS_M1
    jr nc,.no_dn
    set 1,b
.no_dn:
    ld a,(ix+E_COL)
    or a
    jr z,.no_lf
    set 2,b
.no_lf:
    ld a,(ix+E_COL)
    cp NODE_COLS_M1
    jr nc,.no_rt
    set 3,b
.no_rt:
    ld a,(ix+E_DIR)
    xor 1
    ld e,a
    call get_bit
    cpl
    and b
    jr nz,.have
    ld a,b
.have:
    ld c,a
    call b_rand
    ld a,h
    and 3
    ld e,a
    ld d,4
.scan:
    call get_bit
    and c
    jr nz,.found
    ld a,e
    inc a
    and 3
    ld e,a
    dec d
    jr nz,.scan
.found:
    ld (ix+E_DIR),e
    ld (ix+E_LEFT),NODE_STEP/2
    ret

get_bit:
    ld hl,bit_tab
    ld a,l
    add a,e
    ld l,a
    ld a,(hl)
    ret

    ALIGN 4
bit_tab:
    DB 1,2,4,8

b_rand:
    ld hl,(b_rng)
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
    ld (b_rng),hl
    ret

; ent_prepare - compone en el buffer de la entidad la ventana union (posicion vieja + nueva): fondo limpio del banco 0 + sprite nuevo con mascara.
; Entrada: IX. Ventana de 3 bytes x (24 + |dy|) lineas. Altera : A, BC, DE, HL (y IY si BENCH_OVL)
ent_prepare:
    ld a,(ix+E_VALID)
    or a
    jr nz,.have_old
    ld a,(ix+E_X)
    ld (ix+E_OX),a
    ld a,(ix+E_Y)
    ld (ix+E_OY),a
.have_old:
    ld a,(ix+E_X)
    rrca
    rrca
    rrca
    and %00011111
    ld b,a
    ld a,(ix+E_OX)
    rrca
    rrca
    rrca
    and %00011111
    cp b
    jr c,.c_min
    ld a,b
.c_min:
    ld (ix+E_SC0),a
    ld c,a
    ld a,b
    sub c
    ld (ix+E_RC),a
    ld a,(ix+E_Y)
    sub SPR_OFF_Y
    ld b,a
    ld a,(ix+E_OY)
    sub SPR_OFF_Y
    cp b
    jr c,.l_min
    ld a,b
.l_min:
    ld (ix+E_SL0),a
    ld c,a
    ld a,b
    sub c
    ld (ix+E_RL),a
    ld a,(ix+E_OY)
    sub (ix+E_Y)
    jr nc,.dy
    neg
.dy:
    add a,SPR_H
    ld (ix+E_SH),a
    ld a,(ix+E_X)
    and 7
    rrca
    ld (ix+E_PHASE),a
    ld b,3
    or a
    jr nz,.w_ok
    ld b,2
.w_ok:
    ld (ix+E_SPRW),b
    ld a,(ix+E_SL0)
    ld b,a
    ld a,(ix+E_SC0)
    ld c,a
    call pixel_addr
    set 7,h
    ld e,(ix+E_SBUF)
    ld d,(ix+E_SBUF+1)
    ld a,(ix+E_SH)
    ld c,a
    add a,a
    add a,c
    ld c,a
    ld b,0
.cp:
    ldi
    ldi
    ldi
    dec l
    dec l
    dec l
    call down_hl
    ld a,b
    or c
    jr nz,.cp
    ld a,(ix+E_DIR)
    add a,a
    add a,a
    add a,(ix+E_PHASE)
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ld d,h
    ld e,l
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,de
    ld de,sprite_dino
    add hl,de
    ld a,(ix+E_RL)
    ld c,a
    add a,a
    add a,c
    add a,(ix+E_RC)
    add a,(ix+E_SBUF)
    ld e,a
    ld d,(ix+E_SBUF+1)
    ld b,SPR_H
    ld a,(ix+E_SPRW)
    cp 3
    jr z,.d3
.d2:
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    inc e
    inc hl
    inc hl
    djnz .d2
    jr .drawn
.d3:
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    djnz .d3
.drawn:
    ld (ix+E_PEND),1
    ld a,(ix+E_X)
    ld (ix+E_OX),a
    ld a,(ix+E_Y)
    ld (ix+E_OY),a
    ld (ix+E_VALID),1
    IF BENCH_OVL
    jp ent_overlap
    ELSE
    ret
    ENDIF

    IF BENCH_OVL
; ent_overlap - cuenta en ovl_cnt si otra entidad esta a menos de 18 px en X y 26 en Y (cota conservadora). Altera : A, B, DE, HL, IY
ent_overlap:
    ld iy,ents
    ld b,BENCH_N
.l:
    push ix
    pop de
    push iy
    pop hl
    or a
    sbc hl,de
    jr z,.next
    ld a,(ix+E_X)
    sub (iy+E_X)
    jr nc,.dx
    neg
.dx:
    cp 18
    jr nc,.next
    ld a,(ix+E_Y)
    sub (iy+E_Y)
    jr nc,.dy
    neg
.dy:
    cp 26
    jr nc,.next
    ld hl,(ovl_cnt)
    inc hl
    ld (ovl_cnt),hl
    ret
.next:
    ld de,ENT_SIZE
    add iy,de
    djnz .l
    ret
    ENDIF

; ent_blit - vuelca la ventana preparada a la pantalla (3 LDI por linea). Entrada: IX. Altera : A, BC, DE, HL
ent_blit:
    ld a,(ix+E_PEND)
    or a
    ret z
    ld a,(ix+E_SL0)
    ld b,a
    ld a,(ix+E_SC0)
    ld c,a
    call pixel_addr
    ex de,hl
    ld l,(ix+E_SBUF)
    ld h,(ix+E_SBUF+1)
    ld a,(ix+E_SH)
    ld c,a
    add a,a
    add a,c
    ld c,a
    ld b,0
.bl:
    ldi
    ldi
    ldi
    dec e
    dec e
    dec e
    ex de,hl
    call down_hl
    ex de,hl
    ld a,b
    or c
    jr nz,.bl
    ld (ix+E_PEND),0
    ret

; hud_labels - texto fijo y valores iniciales del HUD (font_msg: glifo = font_msg + (ASCII - 32) * 8)
hud_labels:
    ld hl,txt_libre
    ld b,0
    ld c,0
    call b_print_text
    ld hl,txt_min
    ld b,0
    ld c,11
    call b_print_text
    ld hl,txt_retr
    ld b,8
    ld c,0
    call b_print_text
    ld hl,txt_perd
    ld b,8
    ld c,10
    call b_print_text
    ld hl,txt_spr
    ld b,16
    ld c,0
    call b_print_text
    ld hl,txt_esc
    ld b,16
    ld c,9
    call b_print_text
    ld hl,txt_solp
    ld b,16
    ld c,18
    call b_print_text
    ld hl,txt_r2
    ld b,16
    ld c,29
    call b_print_text
    ld hl,0
    ld b,0
    ld c,6
    call b_print_dec4
    ld hl,0
    ld b,0
    ld c,15
    call b_print_dec4
    ld hl,0
    ld b,8
    ld c,5
    call b_print_dec4
    ld hl,0
    ld b,8
    ld c,15
    call b_print_dec4
    ld hl,BENCH_N
    ld b,16
    ld c,4
    call b_print_dec4
    ld hl,BENCH_STAGGER
    ld b,16
    ld c,13
    call b_print_dec4
    ld hl,0
    ld b,16
    ld c,23
    call b_print_dec4
    ret

txt_libre:  DB "LIBRE",0
txt_min:    DB "MIN",0
txt_retr:   DB "RETR",0
txt_perd:   DB "PERD",0
txt_spr:    DB "SPR",0
txt_esc:    DB "ESC",0
txt_solp:   DB "SOLP",0
txt_r2:     DB "R2",0

b_put_glyph:
    ld b,8
.l:
    ld a,(de)
    ld (hl),a
    inc de
    inc h
    djnz .l
    ret

; b_print_text - HL = cadena (0 al final), B = linea (multiplo de 8), C = byte inicial
b_print_text:
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
    add hl,hl
    ld de,font_msg
    add hl,de
    ex de,hl
    call pixel_addr
    call b_put_glyph
    pop bc
    pop hl
    inc hl
    inc c
    jr .l

; b_print_dec4 - HL = valor (< 10000), B = linea, C = byte inicial: escribe 4 cifras
b_print_dec4:
    ld a,b
    ld (pd_line),a
    ld a,c
    ld (pd_col),a
    ld de,pd_tab
    ld b,4
.digit:
    push bc
    push de
    ld a,(de)
    ld c,a
    inc de
    ld a,(de)
    ld b,a
    xor a
.sub:
    or a
    sbc hl,bc
    jr c,.done
    inc a
    jr .sub
.done:
    add hl,bc
    push hl
    add a,'0'-32
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,font_msg
    add hl,de
    ex de,hl
    ld a,(pd_line)
    ld b,a
    ld a,(pd_col)
    ld c,a
    call pixel_addr
    call b_put_glyph
    ld hl,pd_col
    inc (hl)
    pop hl
    pop de
    inc de
    inc de
    pop bc
    djnz .digit
    ret

pd_tab:     DW 1000,100,10,1

frame_counter:  DB 0
frame_start:    DB 0
overruns:       DB 0
lostf:          DB 0
win_count:      DB 0
idle_sum:       DW 0
idle_min:       DW $FFFF
ovl_cnt:        DW 0
pd_line:        DB 0
pd_col:         DB 0
b_rng:          DW $A5C3
ents:           DS ENT_SIZE*8

    ALIGN 128
scr_bufs:       DS 128*8

    INCLUDE "video.inc"
    INCLUDE "sprite_dino.inc"
    INCLUDE "stones.inc"
    INCLUDE "font_msg.inc"

    ASSERT $ < IM2_TABLE

    ORG IM2_TABLE
    DS 257,IM2_FILL
    ORG IM2_VECTOR
    jp frame_isr

    SAVESNA "build/bench_r2.sna", start
