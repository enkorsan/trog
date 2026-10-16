; ============================================================================
; render_core.inc - NUCLEO DEL RENDERER extraido del R6.3 (paso 1 de la integracion)
; Estado: ENSAMBLADO Y PROBADO POR CARLOS (informado, 16-oct) con ENT_COUNT = 1, en RVM 128K y 48K. El asistente no ha ejecutado nada.
;   Con ENT_COUNT = 1 sort_z se reduce a un RET: la ruta de prioridad con varias entidades sigue SIN probar en esta integracion.
; Contrato: docs/decisions_render_update_3.md y render.inc.
; Origen: bench_r6_integracion.asm (R6.3, copia local de Carlos, 36.123 bytes con CRLF). Extraccion automatica de las rutinas build_bg, prepare_all,
;   blit_all, ent_set_qpos, ent_prep1, ent_prep2, ent_hits_other, ent_q_intersects, ent_pre, ent_post, nb_range, draw_clipped, sort_z, ent_blit,
;   b_put_glyph, b_print_text y b_print_dec4, con SOLO estos cambios:
;   1. BENCH_N -> ENT_COUNT; ents, scr_bufs, qpos y zlist -> ENTS, SCR_BUFS, QPOS y ZLIST (EQU de render.inc; ya no son etiquetas).
;   2. Se eliminan los interruptores BENCH_SLOW y BENCH_FALL (ambas rutas quedan siempre activas) y las franjas de color BENCH_PROF.
;   3. sort_z SE EJECUTA (BENCH_SKIP_SORT = 1 en el R6.3 la desactivaba): la prioridad por Y ya es real. Las mediciones del R6.3 con SKIP_SORT = 1
;      no incluyen su coste (unos 1.000 T por fotograma de media en N8 segun render_mediciones_r6_2.md).
;   4. La paginacion de $7FFD pasa por las macros PAGE_BG y PAGE_SPR.
;   5. El contador ovl_cnt (SX) queda detras de RENDER_STATS.
;   Rutinas del banco que NO se extraen (logica de prueba y medida): ent_update, ent_arrive, ent_choose, ent_respawn, init_ents, update_all, b_rand,
;   main_loop, end_window, report_step, hud_labels y los acumuladores del HUD del banco.
; Entrada habitual: IX = entidad. Salvo que se indique otra cosa, las rutinas alteran A, BC, DE, HL (y IY en nb_range y draw_clipped).
; Requiere antes: config.inc, ENT_COUNT, render.inc, video.inc (pixel_addr), font_msg.inc (font_msg), sprite_dino.inc y sprite_trog.inc.
; ============================================================================
RENDER_STATS    EQU 1           ; 1 = cuenta ventanas con solape (ovl_cnt, "SX" del banco)
    ASSERT MACHINE_128 || SPR_BANK == 2     ; en 48K no hay bancos: los sprites son residentes

; render_init - deja a cero las MAX_ENTS entidades, fija E_SBUF, E_IDX y E_TIE de las ENT_COUNT primeras y rellena ZLIST. Altera: A, BC, DE, HL, IX
render_init:
    ld hl,ENTS
    ld de,ENTS+1
    ld bc,MAX_ENTS*ENT_SIZE-1
    ld (hl),0
    ldir
    ld ix,ENTS
    ld hl,SCR_BUFS
    ld b,ENT_COUNT
    ld c,0
.l:
    ld (ix+E_SBUF),l
    ld (ix+E_SBUF+1),h
    ld (ix+E_IDX),c
    ld a,c
    or a
    jr nz,.tie
    ld a,255
.tie:
    ld (ix+E_TIE),a
    ld de,WIN_BUF
    add hl,de
    ld de,ENT_SIZE
    add ix,de
    inc c
    djnz .l
    ld hl,ENTS
    ld de,ZLIST
    ld b,ENT_COUNT
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

; prepare_all - dos pasadas: 1) ventana y copia del fondo (banco 0); 2) sprites (banco de sprites). Altera : todos
prepare_all:
    xor a
    ld (z_sorted),a
    IF SPR_BANK != 2
    PAGE_BG                     ; los sprites estan en otro banco: volver a paginar el fondo limpio
    ENDIF
    ld ix,ENTS
    ld b,ENT_COUNT
.p1:
    push bc
    ld a,(ix+E_DIRTY)
    or a
    call nz,ent_prep1
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .p1
    PAGE_SPR                    ; paginar el banco de sprites (sin codigo si SPR_BANK = 2 o en 48K)
    ld ix,ENTS
    ld b,ENT_COUNT
.p2:
    push bc
    ld a,(ix+E_DIRTY)
    or a
    call nz,ent_prep2
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .p2
    ret

blit_all:
    ld ix,ENTS
    ld b,ENT_COUNT
.l:
    push bc
    call ent_blit
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

; ent_set_qpos - recalcula ancho, puntero de datos (segun imagen y fase; entidad 0 = dino, las demas = Trog), columna y linea superior del sprite actual y los
; guarda en la entidad y en qpos. Con la entidad desaparecida marca su columna con $80 (no cuenta como vecino). Entrada: IX. Altera : A, BC, DE, HL
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
    ld a,(ix+E_IMG)
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
    ld a,(ix+E_IDX)
    or a
    ld de,sprite_dino
    jr z,.s
    ld de,sprite_trog
.s:
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
    ld de,QPOS
    add hl,de
    ld (hl),c
    inc hl
    ld (hl),b
    ld a,(ix+E_STATE)
    cp 2
    ret nz
    ld (ix+E_QC),$80
    dec hl
    ld (hl),$80
    ret

; ent_prep1 - pasada 1: ventana union (posicion vieja + nueva) recortada a CLIP_TOP..CLIP_BOTTOM y con la primera columna <= 29, y copia del fondo limpio.
; Entrada: IX. Altera : A, BC, DE, HL
ent_prep1:
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
    cp 30
    jr c,.c_ok
    ld a,29
.c_ok:
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
    ld c,a
    ld a,(ix+E_OY)
    sub (ix+E_Y)
    jr nc,.dy
    neg
.dy:
    add a,SPR_H
    add a,c
    ld d,a
    xor a
    ld (ix+E_CLIP),a
    ld a,c
    cp CLIP_TOP
    jr nc,.t_ok
    ld (ix+E_CLIP),1
    ld c,CLIP_TOP
.t_ok:
    ld a,d
    cp CLIP_BOTTOM+1
    jr c,.b_ok
    ld (ix+E_CLIP),1
    ld d,CLIP_BOTTOM
.b_ok:
    ld (ix+E_SL0),c
    ld a,d
    sub c
    ld (ix+E_SH),a
    ld a,b
    sub c
    ld (ix+E_RL),a
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
    ret po
    ld a,l
    add a,29
    ld l,a
    jr nc,.cp
    inc h
    jr .cp

; ent_prep2 - pasada 2: vecinos de menor prioridad (si hay solape), sprite propio (ruta rapida, o recortado si la ventana esta recortada; no se pinta si la
; entidad ha desaparecido) y vecinos de mayor prioridad; deja la ventana pendiente de volcar. Entrada: IX. Altera : todos salvo IX
ent_prep2:
    xor a
    ld (ep_slow),a
    call ent_hits_other
    jr z,.nohit
    call ent_pre
.nohit:
    ld a,(ix+E_STATE)
    cp 2
    jr z,.fdone
    ld a,(ix+E_CLIP)
    or a
    jr nz,.clipped
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
    jr .fdone
.clipped:
    push ix
    pop iy
    call draw_clipped
.fdone:
    ld a,(ep_slow)
    or a
    call nz,ent_post
    ld (ix+E_PEND),1
    ld a,(ix+E_STATE)
    cp 2
    jr nz,.alive
    ld (ix+E_VALID),0
    ret
.alive:
    ld a,(ix+E_X)
    ld (ix+E_OX),a
    ld a,(ix+E_Y)
    ld (ix+E_OY),a
    ld (ix+E_VALID),1
    ret

; ent_hits_other - NZ si algun otro sprite toca la ventana de IX (cota conservadora: ancho 3). Altera : A, BC, DE, HL
ent_hits_other:
    ld a,(ix+E_SH)
    add a,23
    ld e,a
    ld a,(ix+E_SC0)
    ld d,a
    ld hl,QPOS
    ld b,ENT_COUNT
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
    IF RENDER_STATS
    ld hl,(ovl_cnt)
    inc hl
    ld (ovl_cnt),hl
    ENDIF
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
    ld hl,ZLIST
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
    ld hl,ZLIST
    jp nb_range

; ent_post - pinta los vecinos de mayor prioridad que la entidad IX. Altera : todos salvo IX
ent_post:
    ld a,(z_epos)
    inc a
    ld c,a
    add a,a
    ld e,a
    ld d,0
    ld hl,ZLIST
    add hl,de
    ld a,ENT_COUNT
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


; draw_clipped - pinta con mascara el sprite de IY recortado a la ventana de IX (IY puede ser IX). Altera : A, BC, DE, HL
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
    IF ENT_COUNT > 1
.again:
    xor a
    ld (z_swapped),a
    ld hl,ZLIST
    ld b,ENT_COUNT-1
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
    dec de
    dec de
    dec de
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


; ---- Variables del renderer (el juego no las toca) ------------------------------
z_sorted:   DB 0
z_swapped:  DB 0
z_epos:     DB 0
ep_slow:    DB 0
dc_n:       DB 0
dc_wr:      DB 0
dc_dr:      DB 0
ovl_cnt:    DW 0
pd_line:    DB 0
pd_col:     DB 0
