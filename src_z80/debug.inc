    IF DEBUG_CHEAT
; ============================================================================
; update_cheat - la tecla H alterna la invencibilidad permanente (una vez por pulsacion)
;   Altera : A, BC
; ============================================================================
update_cheat:
    ld bc,$BFFE                 ; fila H J K L ENTER
    in a,(c)
    cpl
    and %00010000               ; H
    ld b,a
    ld a,(cheat_prev)
    ld c,a
    ld a,b
    ld (cheat_prev),a
    or a
    ret z                       ; no pulsada
    ld a,c
    or a
    ret nz                      ; ya estaba pulsada: sin flanco
    ld a,(cheat_god)
    xor 1
    ld (cheat_god),a
    ret

; ============================================================================
; update_kill - la tecla K mata al dino sin caida (una vez por pulsacion): parpadea, queda apagado y reaparece en la interseccion mas cercana
;   Sirve para probar la reorientacion de las muertes que no son caidas. Altera : A, BC
; ============================================================================
update_kill:
    ld bc,$BFFE                 ; fila H J K L ENTER
    in a,(c)
    cpl
    and %00000100               ; K
    ld b,a
    ld a,(kill_prev)
    ld c,a
    ld a,b
    ld (kill_prev),a
    or a
    ret z                       ; no pulsada
    ld a,c
    or a
    ret nz                      ; ya estaba pulsada: sin flanco
    ld a,(dino_state)
    or a
    ret nz                      ; solo si esta vivo
    ld a,1
    ld (respawn_mode),a
    ld (dino_state),a
    ld a,FALL_FRAMES
    ld (fall_timer),a
    ret

; ============================================================================
; update_reseed - la tecla R coloca otra distribucion de huevos (una vez por pulsacion): borra los huevos, mezcla la semilla con el contador de
;   fotogramas, coloca 43 y los pinta. Solo con el dino vivo. Altera : A, BC, DE, HL
; ============================================================================
update_reseed:
    ld bc,$FBFE                 ; fila Q W E R T
    in a,(c)
    cpl
    and %00001000               ; R
    ld b,a
    ld a,(reseed_prev)
    ld c,a
    ld a,b
    ld (reseed_prev),a
    or a
    ret z
    ld a,c
    or a
    ret nz
    ld a,(dino_state)
    or a
    ret nz
	ld a,(house_state)
    or a
    ret nz                      ; con la casa construida no se redistribuyen los huevos
    call sprite_erase           ; el sprite no puede estar encima al borrar los huevos
	call house_marker_toggle    ; borra el marcador anterior
    ld d,0
.r:
    ld e,0
.c:
    push de
    call erase_egg_area
    pop de
    inc e
    ld a,e
    cp NODE_COLS
    jr nz,.c
    inc d
    ld a,d
    cp NODE_ROWS
    jr nz,.r
    ld hl,(rng_state)
    ld a,(frame_counter)
    xor l
    ld l,a
    ld a,h
    or l
    jr nz,.seed_ok
    ld hl,EGG_SEED
.seed_ok:
    ld (rng_state),hl
    call place_eggs
    call house_select
    call draw_all_eggs
    call draw_egg_counter
    call house_marker_toggle    ; marca la nueva salida
    call sprite_draw
    ld a,1
    ld (saved_valid),a
    ret
    ENDIF

