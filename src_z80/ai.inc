; ============================================================================
; dino_reset - dino en la salida del nivel, andando hacia abajo (inicio de la fase)
;   Altera : A
; ============================================================================
dino_reset:
    ld a,DINO_START_X
    ld (dino_x),a
    ld a,DINO_START_Y
    ld (dino_y),a
    ld a,D_DOWN
    ld (dino_dir),a             ; como en el arcade, empieza andando hacia abajo
    jr dino_arm

; ============================================================================
; dino_respawn - tras morir: reaparece en la interseccion de salida (caida) o en la mas cercana (muerte en el sitio), mirando segun choose_dir
;   Entrada: respawn_mode; si es 0, dino_dir = direccion de la caida y dino_x/dino_y = posicion de la caida
;   Salida : resp_eat (1 = recoger el huevo del nodo de reaparicion; ver fall_update). Altera : A, BC, DE, HL
; ============================================================================
dino_respawn:
    ld a,1
    ld (resp_eat),a             ; caida: siempre se recoge el huevo del nodo de salida
    ld a,(respawn_mode)
    or a
    jr nz,.nearest
    ld a,(dino_dir)
    or a
    jr z,.from_up
    dec a
    jr z,.from_down
    dec a
    jr z,.from_left
.from_right:
    ld a,RESPAWN_X_MAX
    ld (dino_x),a
    jr .pos_done
.from_left:
    ld a,RESPAWN_X_MIN
    ld (dino_x),a
    jr .pos_done
.from_up:
    ld a,RESPAWN_Y_MIN
    ld (dino_y),a
    jr .pos_done
.from_down:
    ld a,RESPAWN_Y_MAX
    ld (dino_y),a
    jr .pos_done
.nearest:
    call node_offset            ; Z = murio a TURN_TOL px o menos de un nodo real (usa dino_dir de marcha, antes de choose_dir)
    jr z,.eat_ok
    xor a
    ld (resp_eat),a             ; murio mas lejos: el huevo del nodo no se recoge
.eat_ok:
    ld a,(dino_x)               ; muerte en el sitio: interseccion mas cercana en cada eje
    ld c,NODE_X0
    ld b,NODE_COLS_M1
    call snap_axis
    ld (dino_x),a
    ld a,(dino_y)
    ld c,NODE_Y0
    ld b,NODE_ROWS_M1
    call snap_axis
    ld (dino_y),a
.pos_done:
    call choose_dir
    ; sigue en dino_arm

; ============================================================================
; dino_arm - estado de "recien aparecido": quieto START_WAIT fotogramas e invencible INV_FRAMES
;   Altera : A
; ============================================================================
dino_arm:
    xor a
    ld (dino_state),a
    ld (respawn_mode),a
    ld (saved_valid),a
    ld (speed_acc),a
    ld (req_timer),a
    ld (edge_stop),a
    ld (edge_reg),a
    ld (dwait),a
    ld a,$FF
    ld (req_dir),a
    ld a,START_WAIT
    ld (wait_timer),a
    ld a,INV_FRAMES
    ld (dino_inv),a
    ret

; ============================================================================
; snap_axis - interseccion mas cercana en un eje
;   Entrada: A = coordenada, C = primera interseccion, B = indice maximo. Salida: A = coordenada de la interseccion. Altera : A, D
; ============================================================================
snap_axis:
    sub c
    jr nc,.past
    ld a,c                      ; antes de la primera: la primera
    ret
.past:
    ld d,0                      ; D = indice
.l:
    cp NODE_STEP
    jr c,.rem
    sub NODE_STEP
    inc d
    jr .l
.rem:
    cp NODE_STEP/2
    jr c,.keep
    inc d                       ; mas cerca de la siguiente
.keep:
    ld a,d
    cp b
    jr c,.ok
    ld d,b                      ; limita al ultimo
.ok:
    ld a,c
    inc d
.mul:
    dec d
    ret z
    add a,NODE_STEP
    jr .mul

; ============================================================================
; div24 - A = A / 24 (division entera). Altera : A, B
; ============================================================================
div24:
    ld b,0
.l:
    cp NODE_STEP
    jr c,.d
    sub NODE_STEP
    inc b
    jr .l
.d:
    ld a,b
    ret

; ============================================================================
; choose_dir - direccion al reaparecer (arcade: DINO_SKIPANIM -> DINO_FIND_CLOSEST + GET_BEST_DIRECTION; si falla, DINO_START_SAFE -> DINO_FIND_BEST)
;   Entrada: dino_x, dino_y en una interseccion; respawn_mode; dino_dir = direccion de la caida (si respawn_mode = 0).
;   Salida : dino_dir. DINITDIR solo cambia cuando se usa DINO_FIND_BEST. Altera : A, BC, DE, HL
; ============================================================================
choose_dir:
    ld a,(dino_x)
    sub NODE_X0
    call div24
    ld (cur_c),a
    ld a,(dino_y)
    sub NODE_Y0
    call div24
    ld (cur_r),a
    IF EGG_AI
    ld a,(eggs_left)
    or a
    jr z,.ratings               ; sin huevos: DINO_FIND_CLOSEST no encuentra nada (no hay casa todavia)
    call find_closest
    ld a,(best_d)
    inc a
    jr z,.ratings               ; best_d = 255: no hay ningun huevo distinto del nodo actual
    call best_direction         ; NC = ninguna direccion valida
    ret c                       ; direccion elegida ya en dino_dir
    ENDIF
.ratings:
    call best_by_rating         ; A = direccion
    ld (dinitdir),a             ; DINO_START_SAFE: MOVE A2,*A13(DINITDIR)
    ld (dino_dir),a
    ret

; ============================================================================
; find_closest - DINO_FIND_CLOSEST: huevo con menor |dy| + |dx| desde TCURMPOS.
;   En el arcade TCURMPOS es el ultimo punto registrado antes de morir: tras una caida, el punto auxiliar del borde (media calle mas alla de la
;   interseccion de salida). Unidades de 12 px con sesgo +1. Con empate gana el primero (orden por filas; en el arcade, el de la lista @EGGS).
;   Se ignora el huevo a distancia 0. Salida : best_c, best_r; best_d = 255 si no hay ninguno. Altera : A, BC, DE, HL
; ============================================================================
find_closest:
    ld a,(cur_c)
    add a,a
    inc a
    ld e,a                      ; E = ox
    ld a,(cur_r)
    add a,a
    inc a
    ld d,a                      ; D = oy
    ld a,(respawn_mode)
    or a
    jr nz,.org_ok               ; muerte en el sitio: origen = la interseccion
    ld a,(dino_dir)             ; caida: el auxiliar esta una unidad mas alla
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    inc e                       ; derecha
    jr .org_ok
.left:
    dec e
    jr .org_ok
.up:
    dec d
    jr .org_ok
.down:
    inc d
.org_ok:
    ld a,255
    ld (best_d),a
    ld hl,egg_map
    ld b,0                      ; B = fila
.rr:
    ld c,0                      ; C = columna
.cc:
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
    jr nc,.px
    neg
.px:
    ld l,a                      ; |ex - ox|
    ld a,b
    add a,a
    inc a
    sub d
    jr nc,.py
    neg
.py:
    add a,l                     ; distancia
    jr z,.worse                 ; distancia 0: huevo bajo el propio dino, se ignora (en el arcade TCURMPOS ya tiene el huevo comido)
    ld l,a
    ld a,(best_d)
    cp l
    jr c,.worse                 ; el mejor ya es menor
    jr z,.worse                 ; igual: gana el primero
    ld a,l
    ld (best_d),a
    pop bc
    ld a,c
    ld (best_c),a
    ld a,b
    ld (best_r),a
    push bc
.worse:
    pop bc
    pop hl
.next:
    inc c
    ld a,c
    cp NODE_COLS
    jr nz,.cc
    inc b
    ld a,b
    cp NODE_ROWS
    jr nz,.rr
    ret

; ============================================================================
; dir_valid - CHECK_DIRECTION desde un centro: valida si hay una interseccion vecina en esa direccion (aun no hay huesos, alquitran, fuego ni fosos)
;   Entrada: A = direccion; cur_c, cur_r. Salida: NZ = valida, Z = no. Altera : A
; ============================================================================
dir_valid:
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    ld a,(cur_c)                ; derecha: columna < 8
    cp NODE_COLS_M1
    jr c,.yes
    jr .no
.left:
    ld a,(cur_c)
    or a
    jr nz,.yes
    jr .no
.up:
    ld a,(cur_r)
    or a
    jr nz,.yes
    jr .no
.down:
    ld a,(cur_r)
    cp NODE_ROWS_M1
    jr c,.yes
.no:
    xor a
    ret
.yes:
    or 1
    ret

; ============================================================================
; best_direction - GET_BEST_DIRECTION con GET_DIRECTIONS: primera opcion = eje con mayor distancia (con empate, el vertical); segunda = el otro eje.
;   La direccion de cada eje sale de la diferencia buscador - objetivo (positiva: arriba / izquierda; si no, abajo / derecha). Se descarta la inversa de DINITDIR.
;   Entrada: cur_c, cur_r, best_c, best_r, dinitdir. Salida: C = direccion en dino_dir; NC = ninguna. Altera : A, BC, DE, HL
; ============================================================================
best_direction:
    ld a,(cur_c)
    ld b,a
    ld a,(best_c)
    ld c,a
    ld a,b
    sub c                       ; cur_c - best_c
    jr z,.x_eq
    jr c,.x_neg
    ld e,D_LEFT                 ; buscador a la derecha del objetivo: izquierda
    ld d,a                      ; D = |dx|
    jr .ydir
.x_neg:
    neg
    ld d,a
    ld e,D_RIGHT
    jr .ydir
.x_eq:
    ld d,0
    ld e,D_RIGHT                ; con dx = 0 el arcade deja DERECHA como direccion del eje X
.ydir:
    ld a,(cur_r)
    ld b,a
    ld a,(best_r)
    ld c,a
    ld a,b
    sub c                       ; cur_r - best_r
    jr z,.y_eq
    jr c,.y_neg
    ld h,D_UP                   ; buscador debajo del objetivo: arriba
    ld l,a                      ; L = |dy|
    jr .cmp
.y_neg:
    neg
    ld l,a
    ld h,D_DOWN
    jr .cmp
.y_eq:
    ld l,0
    ld h,D_DOWN                 ; con dy = 0 el arcade deja ABAJO
.cmp:
    ld a,l
    cp d                        ; |dy| - |dx|
    jr c,.xfirst                ; |dy| < |dx|
    jr z,.yfirst                ; empate: primero el vertical
.yfirst:
    ld b,h                      ; B = primera, C = segunda
    ld c,e
    jr .check
.xfirst:
    ld b,e
    ld c,h
.check:
    ld a,(dinitdir)
    xor 1
    ld d,a                      ; D = inversa de la direccion de partida
    ld a,b
    cp d
    jr z,.second                ; la primera es la inversa: se prueba la segunda
    call dir_valid
    jr nz,.first_ok
.second:
    ld a,c
    cp d
    jr z,.fail                  ; la segunda es la inversa: sin direccion
    call dir_valid
    jr z,.fail
    ld a,c
    jr .ok
.first_ok:
    ld a,b
.ok:
    ld (dino_dir),a
    scf
    ret
.fail:
    or a                        ; NC
    ret

; ============================================================================
; best_by_rating - DINO_FIND_BEST con DINO_SEARCH_SAFE: para cada direccion recorre la calle hasta el final; valoracion 2 por huevo, 2*n+1 pasos
;   (n intersecciones por delante). Gana la mayor valoracion y, a igualdad, mas pasos; con empate total, la ultima de arriba, abajo, izquierda, derecha.
;   Entrada: cur_c, cur_r. Salida: A = direccion. Altera : A, BC, DE, HL
; ============================================================================
best_by_rating:
    xor a
    ld (br_rating),a
    ld (br_steps),a
    ld (br_dir),a
    ld (br_i),a
.dir:
    ld a,(cur_r)                ; HL = &egg_map[fila*9 + columna]
    ld b,a
    add a,a
    add a,a
    add a,a
    add a,b
    ld b,a
    ld a,(cur_c)
    add a,b
    ld e,a
    ld d,0
    ld hl,egg_map
    add hl,de
    ld a,(br_i)                 ; B = n (intersecciones por delante), DE = incremento
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld a,(cur_c)
    ld b,a
    ld a,NODE_COLS_M1
    sub b
    ld b,a
    ld de,1
    jr .count
.left:
    ld a,(cur_c)
    ld b,a
    ld de,-1
    jr .count
.up:
    ld a,(cur_r)
    ld b,a
    ld de,-NODE_COLS
    jr .count
.down:
    ld a,(cur_r)
    ld b,a
    ld a,NODE_ROWS_M1
    sub b
    ld b,a
    ld de,NODE_COLS
.count:
    ld a,b
    ld (br_n),a
    ld c,0                      ; C = huevos en la linea
    or a
    jr z,.scored
.step:
    add hl,de
    ld a,(hl)
    or a
    jr z,.nox
    inc c
.nox:
    djnz .step
.scored:
    ld a,c
    add a,a
    ld c,a                      ; C = valoracion
    ld a,(br_n)
    add a,a
    inc a
    ld b,a                      ; B = pasos
    ld a,(br_rating)
    cp c
    jr c,.better                ; el mejor tiene menos valoracion
    jr nz,.next                 ; el mejor tiene mas
    ld a,(br_steps)
    cp b
    jr c,.better                ; mismos puntos, el nuevo tiene mas pasos
    jr z,.better                ; empate total: gana el posterior
    jr .next
.better:
    ld a,c
    ld (br_rating),a
    ld a,b
    ld (br_steps),a
    ld a,(br_i)
    ld (br_dir),a
.next:
    ld a,(br_i)
    inc a
    ld (br_i),a
    cp 4
    jp nz,.dir
    ld a,(br_dir)
    ret

