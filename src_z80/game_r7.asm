; game_r7.asm revision8 (tanda 2b: llegada a casa y victoria provisional). Revisiones hasta la 7 (casa estatica, teclas B y J) PROBADAS y VALIDADAS POR CARLOS, 17-oct.
; Rev8: victory_r7.inc (house_on_street, house_arrive_check, win_update), estado E_STATE=3 = victoria. El estado de la rev8 lo fija Carlos tras la prueba. Sin ejecucion del asistente.
; Base: rev7. Ver docs/continuation.md y docs/analysis/reaparicion_paso2b.md.
; ENT_COUNT=1; rumbo hacia la casa, tecla R, Trogs pendientes. Local src/, GitHub src_z80/.
ENT_COUNT       EQU 1
EGG_TEST_FLOOR  EQU 1
DBG_HUD         EQU 1
    IFDEF TARGET_48K
    DEVICE ZXSPECTRUM48
    ELSE
    DEVICE ZXSPECTRUM128
    ENDIF
    INCLUDE "config.inc"
    INCLUDE "render.inc"
    ORG CODE_BASE
start:
    di
    ld sp,STACK_TOP
    xor a
    out ($FE),a
    ld hl,SCR_BASE
    ld de,SCR_BASE+1
    ld bc,$17FF
    ld (hl),0
    ldir
    ld hl,ATTR_BASE
    ld de,ATTR_BASE+1
    ld bc,$02FF
    ld (hl),ATTR_HUD
    ldir
    ld hl,FIELD_ATTR_BASE
    ld de,FIELD_ATTR_BASE+1
    ld bc,FIELD_ATTR_LEN
    ld (hl),ATTR_FIELD
    ldir
    call draw_field
    call draw_stones
    call draw_hud_glyphs
    IF DBG_HUD
    call dbg_labels
    ENDIF
    PAGE_BG
    call decor_init
    call eggs_setup
    xor a
    ld (house_state),a          ; sin casa (tambien en un reinicio)
    call house_select
    IF DEBUG_CHEAT
    call dbg_show_exit
    ENDIF
    call build_bg
    call render_init
    call dino_init
    call prepare_all
    xor a
    ld (frame_counter),a
    ld (hud_iter),a
    ld (dbg_tick),a
    ld hl,0
    ld (lt_count),hl
    ld (df_count),hl
    ld a,IM2_TABLE>>8
    ld i,a
    im 2
    ei
    ld a,(frame_counter)
    ld (frame_start),a
main_loop:
    call blit_all
    call blit_decor
    call game_update
    call prepare_all
    call prepare_decor
    IF DBG_HUD
    call dbg_hud
    ENDIF
    call frame_wait
    jp main_loop
game_update:
    call read_keys
    ld (key_mask),a
    call show_keys
    IF DEBUG_CHEAT
    call update_cheat
    call update_kill
    call update_house_key
    call update_exit_key
    ENDIF
    call tick_inv
    call show_inv
    call house_flash_update
    call dino_update
    ld ix,ENTS
    ld (ix+E_DIRTY),a
    or a
    call nz,ent_set_qpos
    jp egg_pickup
frame_wait:
    ld a,(frame_counter)
    ld c,a
    ld hl,frame_start
    sub (hl)
    jr nz,.late
.w:
    ld a,(frame_counter)
    cp c
    jr z,.w
    ld (frame_start),a
    xor a
    ld (hud_iter),a
    ret
.late:
    ld b,a
    ld a,c
    ld (frame_start),a
    ld a,(hud_iter)
    or a
    jr nz,.nocount
    ld hl,(lt_count)
    inc hl
    ld (lt_count),hl
    ld a,b
    dec a
    jr z,.nocount
    ld e,a
    ld d,0
    ld hl,(df_count)
    add hl,de
    ld (df_count),hl
.nocount:
    xor a
    ld (hud_iter),a
    ret
    IF DBG_HUD
dbg_labels:
    ld hl,txt_pc
    ld b,16
    ld c,16
    call b_print_text
    ld hl,txt_lt
    ld b,16
    ld c,24
    call b_print_text
    ld hl,0
    ld b,16
    ld c,19
    call b_print_dec4
    ld hl,0
    ld b,16
    ld c,27
    call b_print_dec4
    ret
txt_pc:     DB "PC",0
txt_lt:     DB "LT",0
dbg_hud:
    ld a,(dbg_tick)
    inc a
    ld (dbg_tick),a
    and 15
    jr z,.pc
    cp 8
    ret nz
    ld hl,(lt_count)
    ld c,27
    jr .print
.pc:
    ld hl,(df_count)
    ld c,19
.print:
    ld a,1
    ld (hud_iter),a
    ld de,10000
    or a
    sbc hl,de
    jr c,.ok
    ld hl,9999
    jr .go
.ok:
    add hl,de
.go:
    ld b,16
    jp b_print_dec4
    ENDIF
frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti
frame_counter:  DB 0
frame_start:    DB 0
hud_iter:       DB 0
dbg_tick:       DB 0
lt_count:       DW 0
df_count:       DW 0
key_mask:       DB 0
req_dir:        DB $FF
req_timer:      DB 0
wait_timer:     DB 0
dinitdir:       DB 0
dino_dirty:     DB 0
dino_inv:       DB 0
edge_stop:      DB 0
edge_reg:       DB 0
dwait:          DB 0
respawn_mode:   DB 0            ; 0 = caida (reaparece en la interseccion de salida), 1 = muerte en sitio (interseccion mas cercana)
resp_eat:       DB 1            ; 1 = egg_pickup activo; 0 = no recoger el huevo del nodo hasta el primer paso (muerte lejos del nodo)
    IF DEBUG_CHEAT
cheat_god:      DB 0            ; tecla H alterna 1/0; la lee is_inv de inv.inc
cheat_prev:      DB 0            ; flanco de la tecla H
kill_prev:       DB 0            ; flanco de la tecla K
house_prev:      DB 0            ; flanco de la tecla B
exit_prev:       DB 0            ; flanco de la tecla J
    ENDIF
    INCLUDE "dino_r7.inc"
    INCLUDE "respawn_r7.inc"
    INCLUDE "inv.inc"
    INCLUDE "debug_r7.inc"
    INCLUDE "render_core.inc"
    INCLUDE "decor_r7.inc"
    INCLUDE "eggs_r7.inc"
    INCLUDE "house_r7.inc"
    INCLUDE "victory_r7.inc"
    INCLUDE "input_hud.inc"
    INCLUDE "video.inc"
    IF SPR_BANK == 2
    INCLUDE "sprite_dino.inc"
    INCLUDE "sprite_trog.inc"
    ENDIF
    INCLUDE "stones.inc"
    INCLUDE "eggs.inc"
    INCLUDE "font_msg.inc"
    ASSERT $ <= SCR_BUFS
    ORG IM2_TABLE
    DS 257,IM2_FILL
    ORG IM2_VECTOR
    jp frame_isr
    IF SPR_BANK != 2
    SLOT 3
    PAGE SPR_BANK
    ORG $C000
    INCLUDE "sprite_dino.inc"
    INCLUDE "sprite_trog.inc"
    ENDIF
    ASSERT (sprite_dino_end - sprite_dino) == IMG_COUNT*4*144
    IFDEF TARGET_48K
    SAVESNA "build/game_r7_48.sna", start
    ELSE
    SAVESNA "build/game_r7.sna", start
    ENDIF
