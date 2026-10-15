; ============================================================================
; read_keys - lee Q A O P y ESPACIO (teclado)
;   Salida : A = mascara (DIR_UP=Q, DIR_DOWN=A, DIR_LEFT=O, DIR_RIGHT=P, KEY_ACTION=ESPACIO; 1 = pulsada)
;   Altera : A, BC, DE
; ============================================================================
read_keys:
    ld bc,$FBFE                 ; fila Q W E R T
    in a,(c)
    cpl
    and 1                       ; Q -> arriba (bit 0)
    ld d,a
    ld b,$FD                    ; fila A S D F G
    in a,(c)
    cpl
    and 1                       ; A -> abajo (bit 1)
    add a,a
    or d
    ld d,a
    ld b,$DF                    ; fila P O I U Y
    in a,(c)
    cpl
    ld e,a
    and 2                       ; O -> izquierda (bit 2)
    add a,a
    or d
    ld d,a
    ld a,e
    and 1                       ; P -> derecha (bit 3)
    add a,a
    add a,a
    add a,a
    or d
    ld d,a
    ld b,$7F                    ; fila ESPACIO SYMSHIFT M N B
    in a,(c)
    cpl
    and 1                       ; ESPACIO -> accion (bit 4)
    rlca
    rlca
    rlca
    rlca
    or d
    ret

; ============================================================================
; show_keys - ilumina las casillas de teclas del HUD segun la mascara
;   Entrada: A = mascara. Altera: A, BC, HL
; ============================================================================
show_keys:
    ld hl,ATTR_BASE+1
    ld c,a
    ld b,KEY_CELLS
.k:
    ld a,ATTR_KEY_OFF
    rr c
    jr nc,.off
    ld a,ATTR_KEY_ON
.off:
    ld (hl),a
    inc hl
    djnz .k
    ret

; ============================================================================
; draw_hud_glyphs - dibuja los simbolos de 8x8 en la fila 0 del HUD (columnas 1..6)
;   Altera : A, BC, DE, HL
; ============================================================================
draw_hud_glyphs:
    ld de,hud_glyphs
    ld hl,SCR_BASE+1
    ld c,HUD_CELLS
.g:
    push hl
    ld b,8
.l:
    ld a,(de)
    ld (hl),a
    inc de
    inc h
    djnz .l
    pop hl
    inc hl
    dec c
    jr nz,.g
    ret

hud_glyphs:
    DB $18,$3C,$7E,$18,$18,$18,$18,$00      ; arriba
    DB $18,$18,$18,$18,$7E,$3C,$18,$00      ; abajo
    DB $08,$18,$38,$7E,$7E,$38,$18,$08      ; izquierda
    DB $10,$18,$1C,$7E,$7E,$1C,$18,$10      ; derecha
    DB $00,$3C,$7E,$7E,$7E,$7E,$3C,$00      ; accion
    DB $3C,$7E,$7E,$7E,$7E,$3C,$18,$00      ; invencibilidad

glyph_egg:
    DB $18,$3C,$7E,$7E,$7E,$7E,$3C,$18      ; icono de huevo

font_digits:                                ; 0..9 y barra
    DB $3C,$66,$6E,$76,$66,$66,$3C,$00
    DB $18,$38,$18,$18,$18,$18,$7E,$00
    DB $3C,$66,$06,$0C,$18,$30,$7E,$00
    DB $3C,$66,$06,$1C,$06,$66,$3C,$00
    DB $0C,$1C,$3C,$6C,$7E,$0C,$0C,$00
    DB $7E,$60,$7C,$06,$06,$66,$3C,$00
    DB $3C,$60,$7C,$66,$66,$66,$3C,$00
    DB $7E,$06,$0C,$18,$30,$30,$30,$00
    DB $3C,$66,$66,$3C,$66,$66,$3C,$00
    DB $3C,$66,$66,$3E,$06,$06,$3C,$00
    DB $02,$06,$0C,$18,$30,$60,$40,$00

