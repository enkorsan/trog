; decor_r7.inc, paso 2a PROBADO por Carlos en 128K/48K, 17-oct00:45.
; decor_push3: B=linea,C=byte,D=alto. Teselas3x1..42, columna<=29, dentro del campo.
; FIFO8; prepare_decor despues de prepare_all; blit_decor junto a blit_all.
; No es decor_rect general. Reutiliza nb_range/sort_z/ent_blit sin cambios.
; decor_push3 altera AF,E,HL; prepare_decor todos; blit_decor AF,BC,DE,HL,IX.
    ASSERT STAT_MAX >= 126
    ASSERT STATQ_MAX == 8
    ASSERT STAT_PER_FRAME == 1
decor_init:
    xor a
    ld (decor_n),a
    ld (decor_rd),a
    ld (decor_wr),a
    ld (decor_ent+E_PEND),a
    ret
decor_push3:
    ld a,c
    cp 30
    jp nc,decor_error
    ld a,d
    or a
    jp z,decor_error
    cp 43
    jp nc,decor_error
    ld a,b
    cp CLIP_TOP
    jp c,decor_error
    add a,d
    jp c,decor_error
    cp CLIP_BOTTOM+1
    jp nc,decor_error
    ld a,(decor_n)
    cp STATQ_MAX
    jp nc,decor_error
    ld a,(decor_wr)
    call decor_slot
    ld (hl),b
    inc hl
    ld (hl),c
    inc hl
    ld (hl),d
    ld a,(decor_wr)
    inc a
    and STATQ_MAX-1
    ld (decor_wr),a
    ld hl,decor_n
    inc (hl)
    ret
decor_slot:
    ld e,a
    add a,a
    add a,e
    ld e,a
    ld hl,decor_q
    ld a,l
    add a,e
    ld l,a
    ret nc
    inc h
    ret
prepare_decor:
    ld a,(decor_n)
    or a
    ret z
    ld a,(decor_ent+E_PEND)
    or a
    jp nz,decor_error
    IF SPR_BANK != 2
    PAGE_BG
    ENDIF
    ld a,(decor_rd)
    call decor_slot
    ld ix,decor_ent
    ld a,(hl)
    ld (ix+E_SL0),a
    inc hl
    ld a,(hl)
    ld (ix+E_SC0),a
    inc hl
    ld a,(hl)
    ld (ix+E_SH),a
    ld a,(decor_rd)
    inc a
    and STATQ_MAX-1
    ld (decor_rd),a
    ld hl,decor_n
    dec (hl)
    ld hl,STAT_BUF
    ld (ix+E_SBUF),l
    ld (ix+E_SBUF+1),h
    ld l,(ix+E_SL0)
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,BG_BASE
    add hl,de
    ld e,(ix+E_SC0)
    ld d,0
    add hl,de
    ld a,(ix+E_SH)
    ld c,a
    add a,a
    add a,c
    ld c,a
    ld b,0
    ld de,STAT_BUF
.copy:
    ldi
    ldi
    ldi
    jp po,.copied
    ld a,l
    add a,29
    ld l,a
    jr nc,.copy
    inc h
    jr .copy
.copied:
    PAGE_SPR
    ld a,(z_sorted)
    or a
    jr nz,.sorted
    push ix
    call sort_z
    pop ix
    ld a,1
    ld (z_sorted),a
.sorted:
    ld hl,ZLIST
    ld b,ENT_COUNT
    call nb_range
    ld ix,decor_ent
    ld (ix+E_PEND),1
    ret
blit_decor:
    ld ix,decor_ent
    jp ent_blit
decor_error:
    ld a,2
    out ($FE),a
.stop:
    jr .stop
decor_n: DB 0
decor_rd: DB 0
decor_wr: DB 0
decor_q: DS STATQ_MAX*3
decor_ent: DS ENT_SIZE
