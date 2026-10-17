; debug_r7.inc revision7. Teclas H y K (rev6) PROBADAS por Carlos, 17-oct. Rev7 (tanda 2a): B construye la casa, J cambia la salida elegida. El estado de la rev7 lo fija Carlos tras la prueba.
; H: alterna la invencibilidad permanente (cheat_god, que lee is_inv). K: mata al dino sin caida (respawn_mode=1). B: house_build en la salida elegida. J: siguiente salida (0..27, solo sin casa); se muestra en el HUD (EX).
; R (nueva distribucion de huevos) NO esta: depende de eggs_r7.inc.
    IF DEBUG_CHEAT
; update_cheat - la tecla H alterna cheat_god (una vez por pulsacion). Altera: A, BC
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
; update_kill - la tecla K mata al dino sin caida (una vez por pulsacion), solo si esta vivo (E_STATE=0). Altera: A, BC
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
    ret z
    ld a,c
    or a
    ret nz
    ld a,(ENT0+E_STATE)
    or a
    ret nz                      ; solo si esta vivo
    ld a,1
    ld (respawn_mode),a
    ld (ENT0+E_STATE),a
    ld a,FALL_ANIM
    ld (ENT0+E_TMR),a
    ret
; update_house_key - la tecla B construye la casa en la salida elegida (una vez por pulsacion). Altera: A, BC
update_house_key:
    ld bc,$7FFE                 ; fila ESPACIO SYM M N B
    in a,(c)
    cpl
    and %00010000               ; B
    ld b,a
    ld a,(house_prev)
    ld c,a
    ld a,b
    ld (house_prev),a
    or a
    ret z
    ld a,c
    or a
    ret nz
    jp house_build
; update_exit_key - la tecla J pasa a la siguiente salida de la casa (una vez por pulsacion; solo si aun no hay casa). Altera: todos
update_exit_key:
    ld bc,$BFFE                 ; fila H J K L ENTER
    in a,(c)
    cpl
    and %00001000               ; J
    ld b,a
    ld a,(exit_prev)
    ld c,a
    ld a,b
    ld (exit_prev),a
    or a
    ret z
    ld a,c
    or a
    ret nz
    ld a,(house_state)
    or a
    ret nz
    ld a,(house_exit)
    inc a
    cp EXIT_COUNT
    jr c,.ok
    xor a
.ok:
    ld (house_exit),a
; dbg_show_exit - escribe EX y el numero de la salida elegida en el HUD (linea 8, columnas 16..21). Altera: todos
dbg_show_exit:
    ld hl,txt_ex
    ld b,8
    ld c,16
    call b_print_text
    ld a,(house_exit)
    ld l,a
    ld h,0
    ld b,8
    ld c,18
    jp b_print_dec4
txt_ex:  DB "EX",0
    ENDIF
