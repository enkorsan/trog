; ============================================================================
; bench_r4.asm - BANCO DE PRUEBAS R4 (render): R3 con menor coste (ruta lenta, fondo lineal, volcado en linea) y escalonado aleatorio.
; Estado del codigo: PROPUESTO (sin ensamblar ni probar). El algoritmo (ventanas, prioridad con la entidad propia por la ruta rapida y los vecinos
;   recortados antes y despues, pausas y acumuladores aleatorios) se verifico con un modelo en Python: 0 diferencias respecto a la imagen ideal en
;   2.500 fotogramas con 2, 3, 5 (dos semillas) y 8 entidades, con solapes. Las rutinas en Z80 no se han ejecutado.
; Colocar junto a main.asm (usa config.inc, video.inc, stones.inc, sprite_dino.inc y font_msg.inc). No toca el juego.
; Cambios respecto a R3:
;   1. Ruta lenta: la entidad propia se pinta por la ruta rapida; antes se pintan (recortados) los vecinos de menor prioridad y despues los de mayor.
;   2. draw_clipped con tres bucles (1, 2 y 3 bytes por linea) sin variables en el bucle; puntero de datos, ancho, columna y linea superior de cada
;      entidad precalculados al moverse (E_DP, E_QW, E_QC, E_QT).
;   3. Fondo limpio LINEAL en el banco 0 (32 bytes por linea, direccion = $C000 + linea * 32 + byte): la copia a la ventana avanza 29 bytes por linea
;      y el volcado a pantalla baja de linea en linea, sin llamar a down_hl.
;   4. BENCH_STAGGER = 2: acumuladores iniciales aleatorios. BENCH_STOP = 1: las entidades se paran al llegar a una interseccion (1 de cada 8, 16 a
;      47 fotogramas).
; Pipeline por fotograma: blit_all (verde), update_all (rojo), prepare_all (azul), libre (negro). SOLP = ventanas con ruta lenta.
; Prioridad (Carlos, MAME): mayor Y encima; con la misma Y el dino (entidad 0); entre Trogs desempate fijo por indice (provisional).
; LIMITACIONES: no recorta por arriba ni por abajo.
; ============================================================================
BENCH_N         EQU 8           ; entidades (1..8)
BENCH_STAGGER   EQU 1           ; 0 = todos en fase; 1 = acumuladores iniciales 3*i mod 16; 2 = aleatorios
BENCH_SPEED     EQU 7           ; pasos de 2 px cada 16 fotogramas
BENCH_PROF      EQU 0           ; 1 = franjas de color en el borde
BENCH_SLOW      EQU 1           ; 1 = ruta lenta con prioridad (compone los sprites que se solapan); 0 = sin ruta lenta
BENCH_STOP      EQU 1           ; 1 = las entidades se paran a veces en las intersecciones
    ASSERT BENCH_N >= 1 && BENCH_N <= 8

    DEVICE ZXSPECTRUM128

    INCLUDE "config.inc"

IM2_TABLE   EQU $B800
IM2_FILL    EQU $B9
IM2_VECTOR  EQU (IM2_FILL<<8)|IM2_FILL
BG_BASE     EQU $C000           ; fondo limpio lineal: 192 lineas x 32 bytes

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
E_SBUF      EQU 20
E_IDX       EQU 22
E_TIE       EQU 23
E_DP        EQU 24              ; puntero (2 bytes) a los datos del sprite actual (direccion y fase)
E_QW        EQU 26              ; ancho actual en bytes (2 si la fase es 0, si no 3)
E_QC        EQU 27              ; byte de pantalla de la primera columna actual
E_QT        EQU 28              ; linea superior actual
E_PAUSE     EQU 29              ; fotogramas de parada que quedan
ENT_SIZE    EQU 32

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
    call build_bg

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

; build_bg - copia la pantalla (pixeles) al fondo lineal del banco 0: linea y en BG_BASE + y * 32. Altera : A, BC, DE, HL
build_bg:
    ld de,BG_BASE
    xor a
.y:
    push af
    ld b,a
    ld c,0
    call pixel_addr
    ld bc,32
    ldir
    pop af
    inc a
    cp 192
    jr nz,.y
    ret

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
    IF BENCH_STAGGER == 1
    ld a,c
    ld e,a
    add a,a
    add a,e
    and 15
    ELSE
    IF BENCH_STAGGER == 2
    push hl
    call b_rand
    ld a,l
    and 15
    pop hl
    ELSE
    xor a
    ENDIF
    ENDIF
    ld (ix+E_ACC),a
    ld (ix+E_SPD),BENCH_SPEED
    ld (ix+E_LEFT),0
    ld (ix+E_DIR),0
    ld (ix+E_MOVED),1
    ld (ix+E_VALID),0
    ld (ix+E_PEND),0
    ld (ix+E_PAUSE),0
    push bc
    push hl
    call ent_set_qpos
    pop hl
    pop bc
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

; ent_update - un fotograma de una entidad. Entrada: IX. Salida: A = 1 si se ha movido. Altera : A, BC, DE, HL
ent_update:
    IF BENCH_STOP
    ld a,(ix+E_PAUSE)
    or a
    jr z,.go
    dec (ix+E_PAUSE)
    xor a
    ret
.go:
    ENDIF
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

; ent_arrive - llegada a una interseccion: actualiza columna o fila y, con BENCH_STOP, a veces para la entidad. Altera : A, HL
ent_arrive:
    ld a,(ix+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    inc (ix+E_COL)
    jr .done
.left:
    dec (ix+E_COL)
    jr .done
.up:
    dec (ix+E_ROW)
    jr .done
.down:
    inc (ix+E_ROW)
.done:
    IF BENCH_STOP
    call b_rand
    ld a,h
    and 7
    ret nz
    ld a,l
    and 31
    add a,16
    ld (ix+E_PAUSE),a
    ENDIF
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

; ent_set_qpos - recalcula ancho, puntero de datos, columna y linea superior del sprite actual y los guarda en la entidad y en qpos.
; Entrada: IX. Altera : A, BC, DE, HL
ent_set_qpos:
    ld a,(ix+E_X)
    ld b,a
    and 7
    rrca
    ld c,a
    or a
    ld a,3
    jr nz,.w
    ld a,2
.w:
    ld (ix+E_QW),a
    ld a,(ix+E_DIR)
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
    ld (ix+E_DP),l
    ld (ix+E_DP+1),h
    ld a,b
    rrca
    rrca
    rrca
    and %00011111
    ld (ix+E_QC),a
    ld c,a
    ld a,(ix+E_Y)
    sub SPR_OFF_Y
    ld (ix+E_QT),a
    ld b,a
    ld l,(ix+E_IDX)
    ld h,0
    add hl,hl
    ld de,qpos
    add hl,de
    ld (hl),c
    inc hl
    ld (hl),b
    ret

; ent_prepare - compone en el buffer de la entidad la ventana union (posicion vieja + nueva): fondo limpio + sprites.
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
    ld a,(ix+E_SL0)
    ld b,a
    rrca
    rrca
    rrca
    and %00011111
    add a,BG_BASE>>8
    ld h,a
    ld a,b
    and 7
    rrca
    rrca
    rrca
    add a,(ix+E_SC0)
    ld l,a
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
    jp po,.cp_done
    ld a,l
    add a,29
    ld l,a
    jr nc,.cp
    inc h
    jr .cp
.cp_done:
    IF BENCH_SLOW
    xor a
    ld (ep_slow),a
    call ent_hits_other
    jr z,.nohit
    call ent_pre
.nohit:
    ENDIF
    ld l,(ix+E_DP)
    ld h,(ix+E_DP+1)
    ld a,(ix+E_RL)
    ld c,a
    add a,a
    add a,c
    add a,(ix+E_RC)
    add a,(ix+E_SBUF)
    ld e,a
    ld d,(ix+E_SBUF+1)
    ld b,SPR_H
    ld a,(ix+E_QW)
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
    jr .fdone
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
.fdone:
    IF BENCH_SLOW
    ld a,(ep_slow)
    or a
    call nz,ent_post
    ENDIF
    ld (ix+E_PEND),1
    ld a,(ix+E_X)
    ld (ix+E_OX),a
    ld a,(ix+E_Y)
    ld (ix+E_OY),a
    ld (ix+E_VALID),1
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
    ld a,(iy+E_QC)
    sub (ix+E_SC0)
    add a,2
    cp 5
    jr nc,.no
    ld a,(ix+E_SH)
    add a,23
    ld c,a
    ld a,(iy+E_QT)
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

; ent_pre - ventana con solape: ordena zlist (una vez por fotograma), busca la posicion de IX y pinta los vecinos de menor prioridad. Altera : todos salvo IX
ent_pre:
    ld a,1
    ld (ep_slow),a
    ld hl,(ovl_cnt)
    inc hl
    ld (ovl_cnt),hl
    ld a,(z_sorted)
    or a
    jr nz,.sorted
    push ix
    call sort_z
    pop ix
    ld a,1
    ld (z_sorted),a
.sorted:
    push ix
    pop de
    ld hl,zlist
    ld b,0
.f:
    ld a,(hl)
    cp e
    jr z,.found
    inc hl
    inc hl
    inc b
    jr .f
.found:
    ld a,b
    ld (z_epos),a
    ld hl,zlist
    jp nb_range

; ent_post - pinta los vecinos de mayor prioridad que la entidad IX. Altera : todos salvo IX
ent_post:
    ld a,(z_epos)
    inc a
    ld c,a
    add a,a
    ld e,a
    ld d,0
    ld hl,zlist
    add hl,de
    ld a,BENCH_N
    sub c
    ld b,a
    jp nb_range

; nb_range - pinta (recortados a la ventana de IX) los sprites de B entradas de zlist desde HL que la tocan. Altera : todos salvo IX
nb_range:
    ld a,b
    or a
    ret z
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
    ld a,(iy+E_QT)
    sub (ix+E_SL0)
    ld c,a
    ld b,0
    ld d,0
    jp p,.rlpos
    neg
    ld b,a
    jr .rl_ok
.rlpos:
    ld d,a
.rl_ok:
    ld a,c
    add a,24
    ld e,(ix+E_SH)
    cp e
    jr c,.i1
    ld a,e
.i1:
    sub d
    ret z
    ret c
    ld (dc_n),a
    ld a,d
    add a,a
    add a,d
    ld (dc_wr),a
    ld a,b
    add a,a
    add a,b
    add a,a
    ld (dc_dr),a
    ld a,(iy+E_QC)
    sub (ix+E_SC0)
    ld c,a
    ld d,0
    jp p,.offp
    neg
    ld d,a
.offp:
    ld a,3
    sub c
    ld b,(iy+E_QW)
    cp b
    jr c,.be
    ld a,b
.be:
    sub d
    ret z
    ret c
    ld b,a
    ld a,c
    or a
    jp p,.cw
    xor a
.cw:
    ld c,a
    ld a,d
    add a,a
    ld d,a
    ld l,(iy+E_DP)
    ld h,(iy+E_DP+1)
    ld a,(dc_dr)
    add a,d
    ld e,a
    ld d,0
    add hl,de
    ld a,(dc_wr)
    add a,c
    add a,(ix+E_SBUF)
    ld e,a
    ld d,(ix+E_SBUF+1)
    ld a,(dc_n)
    ld c,a
    ld a,b
    cp 3
    jr z,.l3
    cp 2
    jr z,.l2
.l1:
    ld a,(de)
    and (hl)
    inc hl
    or (hl)
    inc hl
    ld (de),a
    inc e
    inc e
    inc e
    inc hl
    inc hl
    inc hl
    inc hl
    dec c
    jr nz,.l1
    ret
.l2:
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
    dec c
    jr nz,.l2
    ret
.l3:
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
    dec c
    jr nz,.l3
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

; ent_blit - vuelca la ventana preparada a la pantalla (3 LDI por linea, bajando de linea en linea). Entrada: IX. Altera : A, BC, DE, HL
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
    jp po,.done
    dec e
    dec e
    dec e
    inc d
    ld a,d
    and 7
    jr nz,.bl
    ld a,e
    add a,32
    ld e,a
    jr c,.bl
    ld a,d
    sub 8
    ld d,a
    jr .bl
.done:
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
    ld hl,txt_r4
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
txt_r4:     DB "R4",0

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
z_epos:         DB 0
ep_slow:        DB 0
dc_n:           DB 0
dc_wr:          DB 0
dc_dr:          DB 0
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

    SAVESNA "build/bench_r4.sna", start
