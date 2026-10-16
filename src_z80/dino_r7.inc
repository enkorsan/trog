; ============================================================================
; dino_r7.inc - logica del dino como ENTIDAD 0 del renderer (esqueleto, paso 1)
; Estado: PROBADO POR CARLOS (informado, 16-oct) dentro de game_r7.asm, en RVM 128K y 48K: control OPQA, giros, caidas con la animacion
;   provisional, desaparicion y reaparicion, sin restos en pantalla ni perdida de fotogramas. La reaparicion es SIEMPRE en el nodo de salida
;   (provisional). Velocidad cronometrada 4,75 a 4,80 s para 8 intervalos (MAME 4,78 s). Probado solo con ENT_COUNT = 1.
; Origen: src_z80/dino.inc (version previa): mod_step, node_offset, update_request, dir_from_mask, turn_node, move_dino, check_fall
;   se copian con estos cambios: dino_x/dino_y/dino_dir -> (ENT0+E_X/E_Y/E_DIR); sin sprite_erase/sprite_draw/saved_valid
;   (se marca dino_dirty y game_update llama a ent_set_qpos); acumulador modulo SPEED_MOD; caida = animacion del banco R6.
; NO incluidos aun (siguen en la version previa, pendientes de adaptar): invencibilidad y parada ante el borde (is_inv, edge_stop, dwait),
;   casa (house_on_street, house_arrive_check), huevos (egg_pickup), dino_respawn de ai.inc (aqui: reaparicion provisional en el nodo de salida).
; PENDIENTE (decisions_render_update_3.md, seccion 8): el giro pasa por imagenes intermedias en diagonal (pasos de 45 grados, sentido
;   al azar en la media vuelta, segun Carlos); aqui el giro es todavia directo. Falta leer en el fuente arcade su duracion y si hay avance durante el giro.
; Requiere: config.inc, render.inc, vars del esqueleto (req_dir, req_timer, wait_timer, key_mask, dino_dirty).
; ============================================================================

; mod_step - A = A mod NODE_STEP. Salida: Z = multiplo. Altera: A
mod_step:
    cp NODE_STEP
    jr c,.done
    sub NODE_STEP
    jr mod_step
.done:
    or a
    ret

; node_offset - Z si el dino esta a TURN_TOL px o menos de una interseccion real en su eje de marcha.
;   Salida: D = coordenada de esa interseccion, E = eje (0 = Y, 1 = X). Altera: A, B, C, D, E
node_offset:
    ld a,(ENT0+E_DIR)
    cp D_LEFT
    jr nc,.horiz
    ld a,(ENT0+E_Y)
    ld c,NODE_Y0
    ld b,NODE_Y_LAST
    ld e,0
    jr .calc
.horiz:
    ld a,(ENT0+E_X)
    ld c,NODE_X0
    ld b,NODE_X_LAST
    ld e,1
.calc:
    ld d,a
    sub c
    jr nc,.past
    neg
    cp TURN_TOL+1
    jr nc,.no
    ld d,c
    jr .yes
.past:
    call mod_step
    cp TURN_TOL+1
    jr nc,.far
    ld c,a
    ld a,d
    sub c
    ld d,a
    jr .yes
.far:
    cp NODE_STEP-TURN_TOL
    jr c,.no
    ld c,a
    ld a,NODE_STEP
    sub c
    add a,d
    ld d,a
.yes:
    ld a,b
    cp d
    jr c,.no
    xor a
    ret
.no:
    or 1
    ret

; turn_node - giro perpendicular: acepta si esta a TURN_TOL px o menos de una interseccion y lo alinea.
;   Salida: Z = giro aceptado. Si cambia de posicion marca dino_dirty. Conserva BC y DE. Altera: A, HL
turn_node:
    push bc
    push de
    call node_offset
    jr nz,.out
    ld a,e
    or a
    jr nz,.sx
    ld a,(ENT0+E_Y)
    cp d
    jr z,.out
    ld a,d
    ld (ENT0+E_Y),a
    jr .moved
.sx:
    ld a,(ENT0+E_X)
    cp d
    jr z,.out
    ld a,d
    ld (ENT0+E_X),a
.moved:
    ld a,1
    ld (dino_dirty),a
    xor a                       ; Z
.out:
    pop de
    pop bc
    ret

; update_request - direccion pedida con retencion KEY_HOLD_FRAMES tras soltar la tecla. Altera: A, DE, HL
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

dir_from_mask:
    DB $FF,D_UP,D_DOWN,$FF,D_LEFT,$FF,$FF,$FF
    DB D_RIGHT,$FF,$FF,$FF,$FF,$FF,$FF,$FF

; move_dino - avanza 2 px en la direccion actual. Altera: A
move_dino:
    ld a,(ENT0+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld a,(ENT0+E_X)
    add a,2
    ld (ENT0+E_X),a
    ret
.left:
    ld a,(ENT0+E_X)
    sub 2
    ld (ENT0+E_X),a
    ret
.up:
    ld a,(ENT0+E_Y)
    sub 2
    ld (ENT0+E_Y),a
    ret
.down:
    ld a,(ENT0+E_Y)
    add a,2
    ld (ENT0+E_Y),a
    ret

; check_fall - umbral de caida por direccion (FALL_*). Sin parada ante el borde (pendiente). Altera: A
check_fall:
    ld a,(ENT0+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld a,(ENT0+E_X)
    cp FALL_RIGHT_X
    ret c
    jr .fall
.left:
    ld a,(ENT0+E_X)
    cp FALL_LEFT_X+1
    ret nc
    jr .fall
.up:
    ld a,(ENT0+E_Y)
    cp FALL_TOP_Y+1
    ret nc
    jr .fall
.down:
    ld a,(ENT0+E_Y)
    cp FALL_BOTTOM_Y
    ret c
.fall:
    ld a,(ENT0+E_DIR)
    xor 1
    ld (dinitdir),a             ; DINITDIR = NOTDIR(EMOVEDIR): para la reaparicion real (ai.inc)
    ld a,1
    ld (ENT0+E_STATE),a
    ld a,FALL_ANIM
    ld (ENT0+E_TMR),a
    ret

; dino_update - un paso de logica. Salida: A = 1 si hay que redibujar. Altera: todos salvo IX
dino_update:
    xor a
    ld (dino_dirty),a
    ld a,(ENT0+E_STATE)
    or a
    jp nz,dino_fall_update
    call update_request
    ld a,(wait_timer)
    or a
    jr z,.active
    dec a
    ld (wait_timer),a
    jr .out
.active:
    ld a,(req_dir)
    ld b,a
    cp $FF
    jr z,.walk
    ld a,(ENT0+E_DIR)
    cp b
    jr z,.walk
    xor 1
    cp b
    jr z,.do_turn
    call turn_node
    jr nz,.walk
.do_turn:
    ld a,b
    ld (ENT0+E_DIR),a
    ld (ENT0+E_IMG),a
    ld a,1
    ld (dino_dirty),a
.walk:
    ld a,(ENT0+E_ACC)
    ld hl,ENT0+E_SPD
    add a,(hl)
    cp SPEED_MOD
    jr c,.keep
    sub SPEED_MOD
    ld (ENT0+E_ACC),a
    call move_dino
    ld a,1
    ld (dino_dirty),a
    call check_fall
    jr .out
.keep:
    ld (ENT0+E_ACC),a
.out:
    ld a,(dino_dirty)
    ret

; dino_fall_update - estados 1 (animacion) y 2 (desaparecido), como ent_update del banco R6. Salida: A = dirty
dino_fall_update:
    ld a,(ENT0+E_STATE)
    cp 1
    jr z,.falling
    ld hl,ENT0+E_TMR
    dec (hl)
    jr nz,.idle
    call dino_respawn
    ld a,1
    ret
.idle:
    xor a
    ret
.falling:
    ld hl,ENT0+E_TMR
    dec (hl)
    jr z,.fall_end
    ld a,(hl)
    and 3
    jr nz,.idle
    ld a,(ENT0+E_IMG)
    xor 2
    ld (ENT0+E_IMG),a
    ld a,1
    ret
.fall_end:
    ld a,2
    ld (ENT0+E_STATE),a
    ld a,FALL_DEAD
    ld (ENT0+E_TMR),a
    ld a,1
    ret

; dino_respawn - reaparicion PROVISIONAL en el nodo de salida, mirando abajo, quieto START_WAIT pasos.
;   Pendiente: usar dino_respawn de ai.inc (DINO_FIND_CLOSEST + GET_BEST_DIRECTION). Altera: A
dino_respawn:
    ld a,DINO_START_X
    ld (ENT0+E_X),a
    ld a,DINO_START_Y
    ld (ENT0+E_Y),a
    ld a,D_DOWN
    ld (ENT0+E_DIR),a
    ld (ENT0+E_IMG),a
    xor a
    ld (ENT0+E_STATE),a
    ld (ENT0+E_VALID),a         ; sin imagen previa: solo se compone la ventana nueva
    ld (ENT0+E_TMR),a
    ld (ENT0+E_LEFT),a
    ld (ENT0+E_PAUSE),a
    ld a,START_WAIT
    ld (wait_timer),a
    ld a,$FF
    ld (req_dir),a
    ret

; dino_init - entidad 0 al arrancar (render_init ya fijo E_SBUF, E_IDX y E_TIE). Altera: todos
dino_init:
    ld a,SPEED_BASE
    ld (ENT0+E_SPD),a
    xor a
    ld b,ENT_COUNT
    call stagger_phase
    ld (ENT0+E_ACC),a
    call dino_respawn
    ld a,1
    ld (ENT0+E_DIRTY),a
    ld ix,ENTS
    jp ent_set_qpos

    ASSERT SPEED_MOD == 64      ; stagger_phase multiplica por 64 con 6 desplazamientos
; stagger_phase - fase inicial de la entidad A (0..N-1) de B entidades: floor(SPEED_MOD * A / B). Altera: A, BC, D, HL
stagger_phase:
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl                   ; HL = A * 64
    ld c,b
    ld b,0
    ld d,0
.l:
    or a
    sbc hl,bc
    jr c,.done
    inc d
    jr .l
.done:
    ld a,d
    ret
