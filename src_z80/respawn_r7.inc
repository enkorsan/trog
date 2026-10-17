; respawn_r7.inc, paso2b PROBADO Y VALIDADO POR CARLOS, 17-oct02:14.
; Adaptacion ai.inc blob f1df8a3: solo caidas, no muerte en sitio/casa/proteccion.
; Vuelve al extremo de la MISMA calle, elige rumbo ANTES de recoger huevo.
; div24 altera AF,C. Rutinas alteran AF,BC,DE,HL; conservan IX/IY.
; game_update recalcula qpos y recoge despues. Ver docs/analysis/reaparicion_paso2b.md.
dino_reset:
    ld a,DINO_START_X
    ld (ENT0+E_X),a
    ld a,DINO_START_Y
    ld (ENT0+E_Y),a
    ld a,D_DOWN
    ld (ENT0+E_DIR),a
    ld (dinitdir),a
    jp rsp_arm
dino_respawn:
    ld a,(ENT0+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    ld a,RESPAWN_X_MAX
    ld (ENT0+E_X),a
    jr .position
.left:
    ld a,RESPAWN_X_MIN
    ld (ENT0+E_X),a
    jr .position
.up:
    ld a,RESPAWN_Y_MIN
    ld (ENT0+E_Y),a
    jr .position
.down:
    ld a,RESPAWN_Y_MAX
    ld (ENT0+E_Y),a
.position:
    call rsp_choose
rsp_arm:
    ld a,(ENT0+E_DIR)
    ld (ENT0+E_IMG),a
    xor a
    ld (ENT0+E_STATE),a
    ld (ENT0+E_VALID),a
    ld (ENT0+E_TMR),a
    ld (ENT0+E_LEFT),a
    ld (ENT0+E_PAUSE),a
    ld (ENT0+E_ACC),a
    ld (req_timer),a
    ld a,$FF
    ld (req_dir),a
    ld a,START_WAIT
    ld (wait_timer),a
    ret
rsp_choose:
    ld a,(ENT0+E_X)
    sub NODE_X0
    call div24
    ld (rsp_col),a
    ld a,(ENT0+E_Y)
    sub NODE_Y0
    call div24
    ld (rsp_row),a
    IF EGG_AI
    ld a,(eggs_left)
    or a
    jr z,.ratings
    call rsp_closest
    ld a,(rsp_dist)
    inc a
    jr z,.ratings
    call rsp_direction
    ret c
    ENDIF
.ratings:
    call rsp_rating
    ld (dinitdir),a
    ld (ENT0+E_DIR),a
    ret
rsp_closest:
    ld a,(rsp_col)
    add a,a
    inc a
    ld e,a
    ld a,(rsp_row)
    add a,a
    inc a
    ld d,a
    ld a,(ENT0+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    inc e
    jr .origin
.left:
    dec e
    jr .origin
.up:
    dec d
    jr .origin
.down:
    inc d
.origin:
    ld a,255
    ld (rsp_dist),a
    ld hl,egg_map
    ld b,0
.row:
    ld c,0
.col:
    ld a,(hl)
    inc hl
    or a
    jr z,.next
    push hl
    push bc
    ld a,c
    add a,a
    inc a
    sub e
    jr nc,.xpos
    neg
.xpos:
    ld l,a
    ld a,b
    add a,a
    inc a
    sub d
    jr nc,.ypos
    neg
.ypos:
    add a,l
    jr z,.worse
    ld l,a
    ld a,(rsp_dist)
    cp l
    jr c,.worse
    jr z,.worse
    ld a,l
    ld (rsp_dist),a
    pop bc
    ld a,c
    ld (rsp_best_col),a
    ld a,b
    ld (rsp_best_row),a
    push bc
.worse:
    pop bc
    pop hl
.next:
    inc c
    ld a,c
    cp NODE_COLS
    jr nz,.col
    inc b
    ld a,b
    cp NODE_ROWS
    jr nz,.row
    ret
rsp_valid:
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    ld a,(rsp_col)
    cp NODE_COLS_M1
    jr c,.yes
    jr .no
.left:
    ld a,(rsp_col)
    or a
    jr nz,.yes
    jr .no
.up:
    ld a,(rsp_row)
    or a
    jr nz,.yes
    jr .no
.down:
    ld a,(rsp_row)
    cp NODE_ROWS_M1
    jr c,.yes
.no:
    xor a
    ret
.yes:
    or 1
    ret
rsp_direction:
    ld a,(rsp_col)
    ld b,a
    ld a,(rsp_best_col)
    ld c,a
    ld a,b
    sub c
    jr z,.xequal
    jr c,.xnegative
    ld e,D_LEFT
    ld d,a
    jr .ydir
.xnegative:
    neg
    ld d,a
    ld e,D_RIGHT
    jr .ydir
.xequal:
    ld d,0
    ld e,D_RIGHT
.ydir:
    ld a,(rsp_row)
    ld b,a
    ld a,(rsp_best_row)
    ld c,a
    ld a,b
    sub c
    jr z,.yequal
    jr c,.ynegative
    ld h,D_UP
    ld l,a
    jr .compare
.ynegative:
    neg
    ld l,a
    ld h,D_DOWN
    jr .compare
.yequal:
    ld l,0
    ld h,D_DOWN
.compare:
    ld a,l
    cp d
    jr c,.xfirst
    ld b,h
    ld c,e
    jr .check
.xfirst:
    ld b,e
    ld c,h
.check:
    ld a,(dinitdir)
    xor 1
    ld d,a
    ld a,b
    cp d
    jr z,.second
    call rsp_valid
    jr nz,.first
.second:
    ld a,c
    cp d
    jr z,.fail
    call rsp_valid
    jr z,.fail
    ld a,c
    jr .ok
.first:
    ld a,b
.ok:
    ld (ENT0+E_DIR),a
    scf
    ret
.fail:
    or a
    ret
rsp_rating:
    xor a
    ld (rsp_score),a
    ld (rsp_steps),a
    ld (rsp_dir),a
    ld (rsp_i),a
.dir:
    ld a,(rsp_row)
    ld b,a
    add a,a
    add a,a
    add a,a
    add a,b
    ld b,a
    ld a,(rsp_col)
    add a,b
    ld e,a
    ld d,0
    ld hl,egg_map
    add hl,de
    ld a,(rsp_i)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    ld a,(rsp_col)
    ld b,a
    ld a,NODE_COLS_M1
    sub b
    ld b,a
    ld de,1
    jr .count
.left:
    ld a,(rsp_col)
    ld b,a
    ld de,-1
    jr .count
.up:
    ld a,(rsp_row)
    ld b,a
    ld de,-NODE_COLS
    jr .count
.down:
    ld a,(rsp_row)
    ld b,a
    ld a,NODE_ROWS_M1
    sub b
    ld b,a
    ld de,NODE_COLS
.count:
    ld a,b
    ld (rsp_n),a
    ld c,0
    or a
    jr z,.scored
.step:
    add hl,de
    ld a,(hl)
    or a
    jr z,.empty
    inc c
.empty:
    djnz .step
.scored:
    ld a,c
    add a,a
    ld c,a
    ld a,(rsp_n)
    add a,a
    inc a
    ld b,a
    ld a,(rsp_score)
    cp c
    jr c,.better
    jr nz,.next
    ld a,(rsp_steps)
    cp b
    jr c,.better
    jr z,.better
    jr .next
.better:
    ld a,c
    ld (rsp_score),a
    ld a,b
    ld (rsp_steps),a
    ld a,(rsp_i)
    ld (rsp_dir),a
.next:
    ld a,(rsp_i)
    inc a
    ld (rsp_i),a
    cp 4
    jp nz,.dir
    ld a,(rsp_dir)
    ret
rsp_col:       DB 0
rsp_row:       DB 0
rsp_best_col:  DB 0
rsp_best_row:  DB 0
rsp_dist:      DB 255
rsp_score:     DB 0
rsp_steps:     DB 0
rsp_dir:       DB 0
rsp_i:         DB 0
rsp_n:         DB 0
