; ============================================================================
; sprite_draw - guarda el fondo y pinta el sprite del dino (16x24) con mascara
;   Entrada: dino_x, dino_y, dino_dir. La linea superior del sprite es dino_y - SPR_OFF_Y.
;   Recorta por arriba en CLIP_TOP y por abajo en CLIP_BOTTOM.
;   Altera : A, BC, DE, HL, IX
; ============================================================================
sprite_draw:
    ; --- fase (x & 7) / 2 y ancho en bytes (2 si esta alineado, 3 si no) ---
    ld a,(dino_x)
    and 7
    rrca                        ; x es par: queda 0..3
    ld (spr_phase),a
    ld b,3
    or a
    jr nz,.w_ok
    ld b,2
.w_ok:
    ld a,b
    ld (spr_width),a

    ; --- HL = sprite_dino + (direccion*4 + fase) * 144 ---
    ld a,(dino_dir)
    add a,a
    add a,a                     ; A = direccion * 4
    ld e,a
    ld a,(spr_phase)
    add a,e                     ; A = indice 0..15
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl                   ; HL = indice * 16
    ld d,h
    ld e,l                      ; DE = indice * 16
    add hl,hl
    add hl,hl
    add hl,hl                   ; HL = indice * 128
    add hl,de                   ; HL = indice * 144
    ld de,sprite_dino
    add hl,de

    ; --- linea superior = dino_y - SPR_OFF_Y; recorte superior: i0 = lineas por encima de CLIP_TOP ---
    ld a,(dino_y)
    sub SPR_OFF_Y
    ld (spr_top),a
    cp CLIP_TOP
    jr nc,.no_top_clip
    ld c,a
    ld a,CLIP_TOP
    sub c
    jr .have_i0
.no_top_clip:
    xor a
.have_i0:
    ld (spr_i0),a
    ld e,a
    add a,a
    add a,e
    add a,a                     ; A = i0 * 6 bytes
    ld e,a
    ld d,0
    add hl,de                   ; HL = datos de la primera linea visible

    ; --- primera linea visible, lineas visibles y byte de pantalla ---
    ld a,(spr_top)
    ld c,a
    ld a,(spr_i0)
    add a,c
    ld (save_first),a           ; primera linea visible
    ld a,c
    add a,SPR_H                 ; ultima linea + 1
    cp CLIP_BOTTOM
    jr c,.last_ok
    ld a,CLIP_BOTTOM
.last_ok:
    ld c,a
    ld a,(save_first)
    ld b,a
    ld a,c
    sub b
    ld (save_count),a           ; lineas visibles
    ld a,(spr_width)
    ld (save_width),a
    ld a,(dino_x)
    rrca
    rrca
    rrca
    and %00011111
    ld (save_col),a

    ; --- DE = direccion de pantalla de la primera linea ---
    ld c,a
    ld a,(save_first)
    ld b,a
    push hl
    call pixel_addr
    ex de,hl                    ; DE = pantalla
    pop hl                      ; HL = datos del sprite
    ld ix,save_buf
    ld a,(save_count)
    ld b,a
.line:
    push de
    ld a,(spr_width)
    ld c,a
.byte:
    ld a,(de)
    ld (ix+0),a                 ; guarda el fondo
    inc ix
    and (hl)                    ; mascara
    inc hl
    or (hl)                     ; grafico
    inc hl
    ld (de),a
    inc de
    dec c
    jr nz,.byte
    ld a,(spr_width)
    cp 3
    jr z,.no_skip
    inc hl                      ; ancho 2: salta el tercer par (mascara, grafico)
    inc hl
.no_skip:
    pop de
    ex de,hl
    call down_hl
    ex de,hl
    djnz .line
    ret

; ============================================================================
; sprite_erase - restaura el fondo guardado por sprite_draw
;   Altera : A, BC, DE, HL, IX
; ============================================================================
sprite_erase:
    ld a,(saved_valid)
    or a
    ret z
    ld a,(save_col)
    ld c,a
    ld a,(save_first)
    ld b,a
    call pixel_addr
    ex de,hl                    ; DE = pantalla
    ld ix,save_buf
    ld a,(save_count)
    ld b,a
.line:
    push de
    ld a,(save_width)
    ld c,a
.byte:
    ld a,(ix+0)
    ld (de),a
    inc ix
    inc de
    dec c
    jr nz,.byte
    pop de
    ex de,hl
    call down_hl
    ex de,hl
    djnz .line
    ret

