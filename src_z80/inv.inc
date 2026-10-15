; ============================================================================
; tick_inv - descuenta un fotograma de invencibilidad
;   Altera : A
; ============================================================================
tick_inv:
    ld a,(dino_inv)
    or a
    ret z
    dec a
    ld (dino_inv),a
    ret

; ============================================================================
; is_inv - ¿es invencible el dino? (anillo de invencibilidad o tecla H; la herradura usara dino_inv)
;   Salida : NZ = invencible. Altera : A
; ============================================================================
is_inv:
    ld a,(dino_inv)
    or a
    ret nz
    IF DEBUG_CHEAT
    ld a,(cheat_god)
    or a
    ENDIF
    ret

; ============================================================================
; show_inv - casilla del HUD: encendida con invencibilidad (fija; parpadea al final) o con el truco activo
;   Altera : A, HL
; ============================================================================
show_inv:
    ld hl,ATTR_BASE+HUD_INV_CELL
    ld a,(dino_inv)
    or a
    jr z,.cheat
    cp INV_BLINK+1
    jr nc,.on                   ; mas de INV_BLINK fotogramas: fija
    ld a,(frame_counter)
    and 4
    jr nz,.on                   ; ultimo tramo: parpadea
.cheat:
    IF DEBUG_CHEAT
    ld a,(cheat_god)
    or a
    jr nz,.on
    ENDIF
    ld (hl),ATTR_KEY_OFF
    ret
.on:
    ld (hl),ATTR_KEY_ON
    ret

