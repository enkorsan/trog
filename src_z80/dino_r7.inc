; dino_r7.inc revision7. Pasos 1/2a/2b, rev5 (parada ante el borde) y rev6 (resp_eat) PROBADOS Y VALIDADOS POR CARLOS, 17-oct. Rev7 (tanda 2b): check_fall respeta la calle de la casa; dino_update atiende E_STATE=3 (victoria).
; Base: rev6. check_fall con parada (STOP_*, STOP_QUIRK); dino_update con edge_stop/dwait/EDGE_TURN. El estado de la rev7 lo fija Carlos tras la prueba.
; Origen logico: dino.inc del motor anterior (check_fall, update_dino). Ver docs/analysis/reaparicion_paso2b.md.
; dino_reset/dino_respawn/rsp_arm en respawn_r7.inc. Umbrales STOP_*/FALL_* en config.inc. house_on_street, house_arrive_check y win_update en victory_r7.inc.
mod_step:
    cp NODE_STEP
    jr c,.done
    sub NODE_STEP
    jr mod_step
.done:
    or a
    ret
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
    xor a
.out:
    pop de
    pop bc
    ret
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
; check_fall: tras un paso, cae al llegar a FALL_*; el invencible se detiene antes, en STOP_* (edge_stop).
; Con STOP_QUIRK el punto de parada se evalua una vez (edge_reg) hasta pasar a <=TURN_TOL px de una interseccion.
; Altera A,B,C,D,E.
check_fall:
    IF STOP_QUIRK
    call node_offset
    jr nz,.nofree
    xor a
    ld (edge_reg),a
.nofree:
    ENDIF
    call house_on_street        ; calle de la salida de la casa: ni parada ni caida (arcade: ISHOME + DINO_LOOK_HOME)
    jp nz,house_arrive_check
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
    jr nc,.fall
    cp STOP_RIGHT_X
    ret c
    jr .stop_chk
.left:
    ld a,(ENT0+E_X)
    cp FALL_LEFT_X+1
    jr c,.fall
    cp STOP_LEFT_X+1
    ret nc
    jr .stop_chk
.up:
    ld a,(ENT0+E_Y)
    cp FALL_TOP_Y+1
    jr c,.fall
    cp STOP_TOP_Y+1
    ret nc
    jr .stop_chk
.down:
    ld a,(ENT0+E_Y)
    cp FALL_BOTTOM_Y
    jr nc,.fall
    cp STOP_BOTTOM_Y
    ret c
.stop_chk:
    IF STOP_QUIRK
    ld a,(edge_reg)
    or a
    ret nz
    ld a,1
    ld (edge_reg),a
    ENDIF
    call is_inv
    ret z
    ld a,1
    ld (edge_stop),a
    xor a
    ld (dwait),a
    ret
.fall:
    ld a,(ENT0+E_DIR)
    xor 1
    ld (dinitdir),a
    ld a,1
    ld (ENT0+E_STATE),a
    ld a,FALL_ANIM
    ld (ENT0+E_TMR),a
    ret
dino_update:
    xor a
    ld (dino_dirty),a
    ld a,(ENT0+E_STATE)
    or a
    jr z,.alive
    cp 3
    jp nz,dino_fall_update
    jp win_update               ; estado 3: victoria (victory_r7.inc)
.alive:
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
    jr z,.no_turn
    ld a,(ENT0+E_DIR)
    cp b
    jr z,.no_turn
    xor 1
    cp b
    jr z,.do_turn
    call turn_node
    jr nz,.no_turn
.do_turn:
    ld a,b
    ld (ENT0+E_DIR),a
    ld (ENT0+E_IMG),a
    ld a,1
    ld (dino_dirty),a
    xor a
    ld (edge_stop),a
    ld (dwait),a
.no_turn:
    ld a,(edge_stop)
    or a
    jr z,.walk
    IF EDGE_TURN
    ld a,(dwait)
    inc a
    ld (dwait),a
    cp DWAIT_FRAMES+1
    jr c,.out
    xor a
    ld (edge_stop),a
    ld (dwait),a
    ld a,(ENT0+E_DIR)
    xor 1
    ld (ENT0+E_DIR),a
    ld (ENT0+E_IMG),a
    ld a,1
    ld (dino_dirty),a
    ELSE
    jr .out
    ENDIF
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
    ld (resp_eat),a             ; tras un paso vuelve a valer la recogida de huevo (rev6)
    call check_fall
    jr .out
.keep:
    ld (ENT0+E_ACC),a
.out:
    ld a,(dino_dirty)
    ret
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
dino_init:
    ld a,SPEED_BASE
    ld (ENT0+E_SPD),a
    xor a
    ld b,ENT_COUNT
    call stagger_phase
    ld (ENT0+E_ACC),a
    call dino_reset
    ld a,1
    ld (ENT0+E_DIRTY),a
    ld ix,ENTS
    jp ent_set_qpos
    ASSERT SPEED_MOD == 64
stagger_phase:
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
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
