; ============================================================================
; mod_step - resto de A entre NODE_STEP (24)
;   Entrada: A. Salida: A = A mod 24, Z = multiplo de 24. Altera : A
; ============================================================================
mod_step:
    cp NODE_STEP
    jr c,.done
    sub NODE_STEP
    jr mod_step
.done:
    or a
    ret

; ============================================================================
; node_offset - ¿esta el dino a TURN_TOL px o menos de una interseccion REAL en su eje de marcha?
;   Salida : Z = si; D = coordenada de esa interseccion en el eje de marcha; E = eje (0 = Y, 1 = X). NZ = lejos.
;   Antes de la primera interseccion solo cuenta la primera; pasada la ultima, ninguna. Altera : A, B, C, D, E
; ============================================================================
node_offset:
    ld a,(dino_dir)
    cp D_LEFT
    jr nc,.horiz
    ld a,(dino_y)               ; marcha vertical: eje Y
    ld c,NODE_Y0
    ld b,NODE_Y_LAST
    ld e,0
    jr .calc
.horiz:
    ld a,(dino_x)               ; marcha horizontal: eje X
    ld c,NODE_X0
    ld b,NODE_X_LAST
    ld e,1
.calc:
    ld d,a                      ; D = posicion
    sub c                       ; A = posicion - primera interseccion
    jr nc,.past
    neg                         ; antes de la primera: A = distancia
    cp TURN_TOL+1
    jr nc,.no
    ld d,c                      ; destino = primera interseccion
    jr .yes
.past:
    call mod_step               ; A = resto (0..23)
    cp TURN_TOL+1
    jr nc,.far
    ld c,a                      ; pasada la interseccion: destino = posicion - resto
    ld a,d
    sub c
    ld d,a
    jr .yes
.far:
    cp NODE_STEP-TURN_TOL
    jr c,.no                    ; entre 5 y 19: lejos
    ld c,a                      ; antes de la siguiente: destino = posicion + (24 - resto)
    ld a,NODE_STEP
    sub c
    add a,d
    ld d,a
.yes:
    ld a,b                      ; ultima interseccion real
    cp d
    jr c,.no                    ; el destino esta pasada la ultima: no existe
    xor a                       ; Z
    ret
.no:
    or 1                        ; NZ
    ret

; ============================================================================
; turn_node - giro perpendicular: acepta si el dino esta a TURN_TOL px o menos de una interseccion y lo alinea con ella
;   (arcade: DS_CENT_CK, DS_ND_CENTER -> DINO_CENTER). Si cambia de posicion borra el sprite y fuerza repintarlo.
;   Salida : Z = giro aceptado. Conserva BC y DE. Altera : A, HL, IX
; ============================================================================
turn_node:
    push bc
    push de
    call node_offset
    jr nz,.out
    ld a,e
    or a
    jr nz,.sx
    ld a,(dino_y)
    cp d
    jr z,.out                   ; ya alineado (Z)
    ld a,d
    ld (dino_y),a
    jr .moved
.sx:
    ld a,(dino_x)
    cp d
    jr z,.out
    ld a,d
    ld (dino_x),a
.moved:
    call sprite_erase           ; el sprite se repinta en esta misma pasada
    xor a
    ld (saved_valid),a          ; deja Z
.out:
    pop de
    pop bc
    ret

; ============================================================================
; update_request - direccion pedida con retencion tras soltar la tecla
;   Una unica direccion pulsada la fija y rearma el temporizador; si no hay tecla (o hay varias)
;   se conserva KEY_HOLD_FRAMES fotogramas y luego se olvida.
;   Altera : A, DE, HL
; ============================================================================
update_request:
    ld a,(key_mask)
    and %1111
    ld e,a
    ld d,0
    ld hl,dir_from_mask
    add hl,de
    ld a,(hl)
    cp $FF
    jr z,.none
    ld (req_dir),a
    ld a,KEY_HOLD_FRAMES
    ld (req_timer),a
    ret
.none:
    ld a,(req_timer)
    or a
    jr z,.clear
    dec a
    ld (req_timer),a
    ret
.clear:
    ld a,$FF
    ld (req_dir),a
    ret

; mascara de direcciones (bits 0..3: arriba, abajo, izquierda, derecha) -> direccion del dino
dir_from_mask:
    DB $FF,D_UP,D_DOWN,$FF,D_LEFT,$FF,$FF,$FF
    DB D_RIGHT,$FF,$FF,$FF,$FF,$FF,$FF,$FF

; ============================================================================
; update_dino - espera, giro, parada ante el borde, paso (acumulador de velocidad), caida, recogida de huevos y repintado
;   Altera : todos
; ============================================================================
update_dino:
    ld a,(dino_state)
    or a
    jr z,.alive
    cp 3
    jp nz,fall_update
    jp win_update
.alive:
    call update_request
    ld a,(wait_timer)
    or a
    jr z,.active
    dec a
    ld (wait_timer),a
    jr .no_step                 ; quieto mientras espera
.active:
    ld a,(req_dir)
    ld b,a                      ; B = direccion pedida, o $FF
    cp $FF
    jr z,.no_turn
    ld a,(dino_dir)
    cp b
    jr z,.no_turn               ; misma direccion: parado ante el borde no hace nada
    xor 1
    cp b
    jr z,.do_turn               ; contraria: media vuelta en cualquier punto
    call turn_node              ; perpendicular: a TURN_TOL px o menos de una interseccion (la alinea)
    jr nz,.no_turn
.do_turn:
    ld a,b
    ld (dino_dir),a
    xor a                       ; al girar con el control deja de estar parado ante el borde
    ld (edge_stop),a
    ld (dwait),a
.no_turn:
    ld a,(edge_stop)
    or a
    jr z,.walk
    IF EDGE_TURN
    ld a,(dwait)                ; parado ante el borde: cuenta fotogramas (arcade: DWAIT en DINOWLP_MOVE)
    inc a
    ld (dwait),a
    cp DWAIT_FRAMES+1
    jr c,.no_step               ; todavia espera
    xor a                       ; vencido: media vuelta (arcade: NOTDIR + DINOTURN)
    ld (edge_stop),a
    ld (dwait),a
    ld a,(dino_dir)
    xor 1
    ld (dino_dir),a
    ELSE
    jr .no_step                 ; se queda parado
    ENDIF
.walk:
    ld a,(speed_acc)
    add a,DINO_SPEED
    cp 16
    jr c,.acc_keep
    sub 16
    ld (speed_acc),a
    call move_dino
    call check_fall
    jr .redraw
.acc_keep:
    ld (speed_acc),a
.no_step:
    ld a,(saved_valid)
    or a
    ret nz                      ; nada que repintar
.redraw:
    call sprite_erase
    call egg_pickup             ; con el sprite borrado de la pantalla
    call sprite_draw
    ld a,1
    ld (saved_valid),a
    ret

; ============================================================================
; move_dino - avanza 2 px en la direccion actual
;   Altera : A
; ============================================================================
move_dino:
    ld a,(dino_dir)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld a,(dino_x)
    add a,2
    ld (dino_x),a
    ret
.left:
    ld a,(dino_x)
    sub 2
    ld (dino_x),a
    ret
.up:
    ld a,(dino_y)
    sub 2
    ld (dino_y),a
    ret
.down:
    ld a,(dino_y)
    add a,2
    ld (dino_y),a
    ret

; ============================================================================
; check_fall - tras un paso: cae al llegar al umbral de caida; el invencible se detiene antes, en el punto de parada (STOP_*)
;   Arcade: DINO_NEAR_EXIT solo se evalua al registrar el punto auxiliar del borde (DINO_NEXT_MOVE); BOUNDRY_CHECK (caida) no mira DINVIN.
;   Invencible = anillo, tecla H o (futuro) herradura: mismo trato. Altera : A, B, C, D, E
; ============================================================================
check_fall:
    IF STOP_QUIRK
    call node_offset            ; a TURN_TOL px o menos de una interseccion: el auxiliar del borde deja de ser el ultimo punto registrado
    jr nz,.nofree
    xor a
    ld (edge_reg),a
.nofree:
    ENDIF
    call house_on_street
    jp nz,house_arrive_check
    ld a,(dino_dir)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld a,(dino_x)
    cp FALL_RIGHT_X
    jr nc,.fall
    cp STOP_RIGHT_X
    ret c                       ; x < 228: sigue andando
    jr .stop_chk
.left:
    ld a,(dino_x)
    cp FALL_LEFT_X+1
    jr c,.fall
    cp STOP_LEFT_X+1
    ret nc                      ; x > 12: sigue andando
    jr .stop_chk
.up:
    ld a,(dino_y)
    cp FALL_TOP_Y+1
    jr c,.fall
    cp STOP_TOP_Y+1
    ret nc                      ; y > 40: sigue andando
    jr .stop_chk
.down:
    ld a,(dino_y)
    cp FALL_BOTTOM_Y
    jr nc,.fall
    cp STOP_BOTTOM_Y
    ret c                       ; y < 160: sigue andando
.stop_chk:
    IF STOP_QUIRK
    ld a,(edge_reg)
    or a
    ret nz                      ; el auxiliar del borde ya estaba registrado: no se vuelve a evaluar
    ld a,1
    ld (edge_reg),a
    ENDIF
    call is_inv
    ret z                       ; no invencible: sigue andando hasta caer
    ld a,1                      ; parado ante el borde (arcade: DINO_SAVE_FROM_DEATH + DWAIT)
    ld (edge_stop),a
    xor a
    ld (dwait),a
    ret
.fall:
    ld a,(dino_dir)             ; DINODTH5: DINITDIR = NOTDIR(EMOVEDIR), es decir, hacia dentro
    xor 1
    ld (dinitdir),a
    xor a
    ld (respawn_mode),a         ; muerte por caida
    inc a
    ld (dino_state),a
    ld a,FALL_FRAMES
    ld (fall_timer),a
    ret

; ============================================================================
; fall_update - muerte: parpadea FALL_FRAMES (estado 1), queda apagado DEAD_WAIT_FRAMES (estado 2) y reaparece
;   Arcade: DINO_DEATH_DONE apaga el dino (OBJ_OFF) y DINO_ETERNAL duerme 60 ticks (SLEEP_START) antes de DINODEATH_XFER.
;   Altera : todos
; ============================================================================
fall_update:
    ld a,(dino_state)
    cp 2
    jr z,.dead
    ld a,(fall_timer)
    dec a
    ld (fall_timer),a
    jr z,.vanish
    and 4                       ; 4 fotogramas visible, 4 oculto
    jr z,.show
    ld a,(saved_valid)          ; fase oculta
    or a
    ret z
    call sprite_erase
    xor a
    ld (saved_valid),a
    ret
.show:
    ld a,(saved_valid)          ; fase visible
    or a
    ret nz
    call sprite_draw
    ld a,1
    ld (saved_valid),a
    ret
.vanish:
    call sprite_erase           ; fin de la animacion: dino apagado
    xor a
    ld (saved_valid),a
    ld a,2
    ld (dino_state),a
    ld a,DEAD_WAIT_FRAMES
    ld (fall_timer),a
    ret
.dead:
    ld a,(fall_timer)
    dec a
    ld (fall_timer),a
    ret nz                      ; sigue apagado
    call dino_respawn           ; vuelve a estado 0, quieto START_WAIT e invencible
    ld a,(resp_eat)             ; el huevo del nodo de reaparicion se recoge ahora (despues de elegir la direccion, como el arcade)
    or a                        ; solo si la muerte fue a TURN_TOL px o menos del nodo (o caida)
    call nz,egg_pickup
    call sprite_draw
    ld a,1
    ld (saved_valid),a
    ret

