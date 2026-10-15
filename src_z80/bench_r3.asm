; ============================================================================
; bench_r3.asm - BANCO DE PRUEBAS R3 (render): B+ con solapes resueltos por prioridad, test de solape barato y medida sin picos.
; Estado del codigo: PROPUESTO (sin ensamblar ni probar). El algoritmo (ventanas, ruta lenta, recorte y prioridad) se verifico con un modelo en Python:
;   0 diferencias respecto a la imagen ideal en 2.500 fotogramas con 2, 3, 5 (dos semillas) y 8 entidades, con solapes.
; Colocar junto a main.asm (usa config.inc, video.inc, stones.inc, sprite_dino.inc y font_msg.inc). No toca el juego.
; Cambios respecto a R2:
;   1. Prioridad de pintado (observada por Carlos en MAME): mas abajo (mayor Y) = encima; con la misma Y gana el dino (entidad 0);
;      entre Trogs con la misma Y el desempate es fijo por indice (en el arcade no hay patron claro; revisar mas adelante, p. ej. acciones de comer).
;   2. Ruta lenta: si la ventana de una entidad toca a otra, se componen en la ventana todos los sprites que la cortan, recortados y en orden de
;      prioridad, de menor a mayor. La ruta rapida (sin solape) es la de R2. Con BENCH_SLOW = 0 se obtiene el comportamiento de R2.
;   3. Test de solape barato: tabla qpos (columna y linea superior de cada entidad) y bucle corto, sin PUSH/POP ni IY.
;   4. Medida sin picos: los numeros del HUD se escriben de uno en uno en los fotogramas siguientes al final de la ventana y esos fotogramas no
;      cuentan para LIBRE, MIN ni RETR.
; Pipeline por fotograma: blit_all (verde), update_all (rojo), prepare_all (azul), libre (negro). SOLP = ventanas compuestas por la ruta lenta.
; LIMITACIONES: no recorta por arriba ni por abajo; la prioridad es provisional.
; ============================================================================
BENCH_N         EQU 5           ; entidades (1..8)
BENCH_STAGGER   EQU 1           ; 1 = acumuladores iniciales 3*i mod 16; 0 = todos en fase
BENCH_SPEED     EQU 7           ; pasos de 2 px cada 16 fotogramas
BENCH_PROF      EQU 0           ; 1 = franjas de color en el borde
BENCH_SLOW      EQU 1           ; 1 = ruta lenta con prioridad (compone los sprites que se solapan); 0 = como R2
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
E_IDX       EQU 22
E_TIE       EQU 23
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
    ld a,(skip_acc)
    or a
    jr nz,.over_skip
    ld hl,overruns
    inc (hl)
    ld a,(lostf)
    add a,c
    ld (lostf),a
.over_skip:
    ld a,(frame_counter)
    ld (frame_start),a
    ld de,0
.account:
    ld a,(skip_acc)
    or a
    jr z,.measure
    xor a
    ld (skip_acc),a
    jr .staged
.measure:
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
    call nc,end_window
.staged:
    ld a,(rep_stage)
    or a
    call nz,report_step
    jp main_loop

; end_window - al completar 64 fotogramas medidos guarda los resultados y abre otra ventana. Altera : A, B, HL
end_window:
    ld hl,(idle_sum)
    ld b,6
.s:
    srl h
    rr l
    djnz .s
    ld (rep_avg),hl
    ld hl,(idle_min)
    ld (rep_min),hl
    ld a,(overruns)
    ld (rep_over),a
    ld a,(lostf)
    ld (rep_lost),a
    ld hl,(ovl_cnt)
    ld (rep_ovl),hl
    xor a
    ld (win_count),a
    ld (overruns),a
    ld (lostf),a
    ld hl,0
    ld (idle_sum),hl
    ld (ovl_cnt),hl
    ld hl,$FFFF
    ld (idle_min),hl
    ld a,1
    ld (rep_stage),a
    ret

; report_step - escribe un numero del HUD por fotograma (fases 1 a 5); el fotograma siguiente no se mide. Altera : todos
report_step:
    ld a,1
    ld (skip_acc),a
    ld a,(rep_stage)
    cp 1
    jr z,.s1
    cp 2
    jr z,.s2
    cp 3
    jr z,.s3
    cp 4
    jr z,.s4
.s5:
    ld hl,(rep_ovl)
    ld b,16
    ld c,23
    xor a
    ld (rep_stage),a
    jp b_print_dec4
.s1:
    ld hl,(rep_avg)
    ld b,0
    ld c,6
    ld a,2
    ld (rep_stage),a
    jp b_print_dec4
.s2:
    ld hl,(rep_min)
    ld b,0
    ld c,15
    ld a,3
    ld (rep_stage),a
    jp b_print_dec4
.s3:
    ld a,(rep_over)
    ld l,a
    ld h,0
    ld b,8
    ld c,5
    ld a,4
    ld (rep_stage),a
    jp b_print_dec4
.s4:
    ld a,(rep_lost)
    ld l,a
    ld h,0
    ld b,8
    ld c,15
    ld a,5
    ld (rep_stage),a
    jp b_print_dec4

frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti

; init_ents - entidad i en la interseccion (columna 2i mod 9, fila i mod 5), buffer de ventana i, qpos y zlist. Altera : A, BC, DE, HL, IX
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
    ld (ix+E_IDX),c
    ld a,c
    or a
    jr nz,.tie
    ld a,255
.tie:
    ld (ix+E_TIE),a
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
    push hl
    call ent_set_qpos
    pop hl
    ld de,ENT_SIZE
    add ix,de
    inc c
    djnz .l
    ld hl,ents
    ld de,zlist
    ld b,BENCH_N
.z:
    ld a,l
    ld (de),a
    inc de
    ld a,h
    ld (de),a
    inc de
    push de
    ld de,ENT_SIZE
    add hl,de
    pop de
    djnz .z
    ret

; update_all / prepare_all / blit_all - recorren las BENCH_N entidades. Altera : todos
update_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    call ent_update
    ld (ix+E_MOVED),a
    or a
    call nz,ent_set_qpos
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

prepare_all:
    xor a
    ld (z_sorted),a
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

; ent_update / ent_arrive / ent_choose / get_bit / b_rand - identicos a R1 y R2 (movimiento por las calles). ent_update: A = 1 si se ha movido
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

; ent_prepare - compone en el buffer de la entidad la ventana union (posicion vieja + nueva): fondo limpio del banco 0 + sprites.
; Entrada: IX. Ventana de 3 bytes x (24 + |dy|) lineas. Altera : todos salvo IX
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
    IF BENCH_SLOW
    call ent_hits_other
    jr z,.fast
    call ent_compose_slow
    jr .drawn
.fast:
    ENDIF
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
    ret

; ent_set_qpos - guarda en qpos[indice] la columna (byte) y la linea superior del sprite. Entrada: IX. Altera : A, DE, HL
ent_set_qpos:
    ld l,(ix+E_IDX)
    ld h,0
    add hl,hl
    ld de,qpos
    add hl,de
    ld a,(ix+E_X)
    rrca
    rrca
    rrca
    and %00011111
    ld (hl),a
    inc hl
    ld a,(ix+E_Y)
    sub SPR_OFF_Y
    ld (hl),a
    ret

    IF BENCH_SLOW
; ent_hits_other - NZ si algun otro sprite toca la ventana de IX (cota conservadora: ancho 3). Altera : A, BC, DE, HL
ent_hits_other:
    ld a,(ix+E_SH)
    add a,23
    ld e,a
    ld a,(ix+E_SC0)
    ld d,a
    ld hl,qpos
    ld b,BENCH_N
    ld c,0
.l:
    ld a,c
    cp (ix+E_IDX)
    jr z,.skip
    ld a,(hl)
    sub d
    add a,2
    cp 5
    jr nc,.skip
    inc hl
    ld a,(hl)
    dec hl
    sub (ix+E_SL0)
    add a,23
    cp e
    jr c,.hit
.skip:
    inc hl
    inc hl
    inc c
    djnz .l
    xor a
    ret
.hit:
    ld a,1
    or a
    ret

; ent_q_intersects - NZ si IY es IX o su sprite toca la ventana de IX. Altera : A, C, DE, HL
ent_q_intersects:
    push ix
    pop de
    push iy
    pop hl
    or a
    sbc hl,de
    jr z,.yes
    ld a,(iy+E_X)
    rrca
    rrca
    rrca
    and %00011111
    sub (ix+E_SC0)
    add a,2
    cp 5
    jr nc,.no
    ld a,(ix+E_SH)
    add a,23
    ld c,a
    ld a,(iy+E_Y)
    sub SPR_OFF_Y
    sub (ix+E_SL0)
    add a,23
    cp c
    jr nc,.no
.yes:
    ld a,1
    or a
    ret
.no:
    xor a
    ret

; ent_compose_slow - compone en la ventana de IX todos los sprites que la tocan, de menor a mayor prioridad. Altera : todos salvo IX
ent_compose_slow:
    ld a,(z_sorted)
    or a
    jr nz,.sorted
    push ix
    call sort_z
    pop ix
    ld a,1
    ld (z_sorted),a
.sorted:
    ld hl,(ovl_cnt)
    inc hl
    ld (ovl_cnt),hl
    ld hl,zlist
    ld b,BENCH_N
.z:
    push bc
    push hl
    ld e,(hl)
    inc hl
    ld d,(hl)
    push de
    pop iy
    call ent_q_intersects
    call nz,draw_clipped
    pop hl
    inc hl
    inc hl
    pop bc
    djnz .z
    ret

; draw_clipped - pinta con mascara el sprite de IY recortado a la ventana de IX. Altera : A, BC, DE, HL
draw_clipped:
    ld a,(iy+E_X)
    and 7
    rrca
    ld c,a
    or a
    ld a,3
    jr nz,.w_set
    ld a,2
.w_set:
    ld (dc_w),a
    ld a,(iy+E_DIR)
    add a,a
    add a,a
    add a,c
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
    ld (dc_base),hl
    ld a,(iy+E_X)
    rrca
    rrca
    rrca
    and %00011111
    sub (ix+E_SC0)
    ld (dc_off),a
    ld c,0
    or a
    jp p,.offpos
    neg
    ld c,a
.offpos:
    ld a,(dc_off)
    ld b,a
    ld a,3
    sub b
    ld b,a
    ld a,(dc_w)
    cp b
    jr c,.bend
    ld a,b
.bend:
    sub c
    ret z
    ret c
    ld (dc_cnt),a
    ld a,c
    ld (dc_bstart),a
    ld a,(dc_cnt)
    add a,a
    ld b,a
    ld a,6
    sub b
    ld (dc_hladv),a
    ld a,(dc_cnt)
    ld b,a
    ld a,3
    sub b
    ld (dc_deadv),a
    ld a,(iy+E_Y)
    sub SPR_OFF_Y
    sub (ix+E_SL0)
    ld e,a
    ld b,0
    ld d,0
    or a
    jp p,.rlpos
    neg
    ld b,a
    jr .rl_done
.rlpos:
    ld d,a
.rl_done:
    ld a,d
    ld (dc_i0),a
    ld a,e
    add a,24
    ld c,(ix+E_SH)
    cp c
    jr c,.i1
    ld a,c
.i1:
    sub d
    ret z
    ret c
    ld c,a
    ld a,b
    ld e,a
    add a,a
    add a,e
    add a,a
    ld e,a
    ld a,(dc_bstart)
    add a,a
    add a,e
    ld e,a
    ld d,0
    ld hl,(dc_base)
    add hl,de
    ld a,(dc_i0)
    ld e,a
    add a,a
    add a,e
    ld e,a
    ld a,(dc_off)
    add a,e
    ld e,a
    ld a,(dc_bstart)
    add a,e
    add a,(ix+E_SBUF)
    ld e,a
    ld d,(ix+E_SBUF+1)
.line:
    ld a,(dc_cnt)
    ld b,a
.byte:
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    djnz .byte
    ld a,(dc_hladv)
    add a,l
    ld l,a
    jr nc,.hl_ok
    inc h
.hl_ok:
    ld a,(dc_deadv)
    add a,e
    ld e,a
    dec c
    jr nz,.line
    ret

; sort_z - ordena zlist (punteros a entidades) de menor a mayor prioridad: (Y, E_TIE) ascendente. Altera : todos
sort_z:
    IF BENCH_N > 1
.again:
    xor a
    ld (z_swapped),a
    ld hl,zlist
    ld b,BENCH_N-1
.k:
    push bc
    ld e,(hl)
    inc hl
    ld d,(hl)
    inc hl
    ld c,(hl)
    inc hl
    ld b,(hl)
    push de
    pop ix
    push bc
    pop iy
    ld a,(ix+E_Y)
    cp (iy+E_Y)
    jr c,.ok
    jr nz,.swap
    ld a,(ix+E_TIE)
    cp (iy+E_TIE)
    jr c,.ok
.swap:
    ld (hl),d
    dec hl
    ld (hl),e
    dec hl
    ld (hl),b
    dec hl
    ld (hl),c
    inc hl
    inc hl
    ld a,1
    ld (z_swapped),a
    jr .next
.ok:
    dec hl
.next:
    pop bc
    djnz .k
    ld a,(z_swapped)
    or a
    jr nz,.again
    ENDIF
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
    ld hl,txt_r3
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
txt_r3:     DB "R3",0

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
skip_acc:       DB 0
rep_stage:      DB 0
rep_avg:        DW 0
rep_min:        DW 0
rep_over:       DB 0
rep_lost:       DB 0
rep_ovl:        DW 0
z_sorted:       DB 0
z_swapped:      DB 0
dc_w:           DB 0
dc_off:         DB 0
dc_cnt:         DB 0
dc_bstart:      DB 0
dc_hladv:       DB 0
dc_deadv:       DB 0
dc_i0:          DB 0
dc_base:        DW 0
qpos:           DS 16
zlist:          DS 16
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

    SAVESNA "build/bench_r3.sna", start
