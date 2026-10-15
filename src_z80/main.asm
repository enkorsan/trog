; ============================================================================
; Trog ZX - PASO 2d: huevos en las intersecciones, recogida, contador en el HUD y reorientacion al reaparecer (DINO_FIND_CLOSEST + GET_BEST_DIRECTION)
; Base: paso 3g (layout D, mascara, flecha 16x24, giro con tolerancia, parada ante el borde, hueco de 4 px, tecla K).
; Cambios respecto al 3g (fuente: TROGEGGS.ASM: ALLOT_EGGS, EGGHATCH, EGG_OBJ, EGGDINO, DINO_FIND_CLOSEST; TROGDINO.ASM: DINO_SKIPANIM, DINO_START_SAFE; rutinas pegadas por Carlos):
;   1. 43 huevos (45 intersecciones - 2: la salida y otra al azar; ALLOT_EGGS resta 2 para Trog) en intersecciones al azar. Sello de 16x16 con mascara (eggs.inc).
;   2. Recogida: al llegar a TURN_TOL px de una interseccion con huevo (arcade: DINO_NEXT_MOVE registra el centro y llama a EGG_OBJ -> EGGDINO). Se borra el huevo.
;   3. Contador en el HUD (fila 1): icono, recogidos y total.
;   4. Al reaparecer: DINO_FIND_CLOSEST (huevo con menor |dy|+|dx| desde el punto auxiliar de la caida; ties: el primero en orden de filas) y GET_BEST_DIRECTION
;      (primera y segunda opcion de GET_DIRECTIONS, descartando la inversa de DINITDIR, validadas con CHECK_DIRECTION). Si falla: DINO_FIND_BEST (valoracion de
;      DINO_SEARCH_SAFE: 2 por huevo en la linea, y mas pasos; con empate gana la ultima direccion). Sin huevos coincide con el 3g.
;   5. DEBUG_CHEAT: tecla R coloca otra distribucion de huevos (con otra semilla).
; Estado del codigo: PROPUESTO. Sin ensamblar ni probar en emulador. Faltan: casa, victoria, vidas y puntos (siguientes pasos).
; Geometria con HUD_H = 24, GRID_TOP = 44 y EDGE_GAP = 4:
;   Y   0.. 23  HUD (tres filas de atributos; fila 0: teclas e invencibilidad; fila 1: contador de huevos)
;   Y  24.. 39  agua superior
;   Y  40..175  suelo X 12..243 (piedras en X 16..239, Y 44..171)
;   Y 176..191  agua inferior; agua lateral X 0..11 y X 244..255
; ============================================================================
    DEVICE ZXSPECTRUM128

    INCLUDE "config.inc"            ; EQUs, opciones de compilacion y ASSERTs
; ---- Interrupciones IM2 (tabla de 257 bytes en la RAM fija del banco 2) ------
IM2_TABLE       EQU $B800
IM2_FILL        EQU $B9
IM2_VECTOR      EQU (IM2_FILL<<8)|IM2_FILL

    ORG $8000

start:
    di
    ld sp,$7FFF

    xor a
    out ($FE),a                 ; borde negro

    ; --- limpiar pixeles ---
    ld hl,SCR_BASE
    ld de,SCR_BASE+1
    ld bc,$17FF
    ld (hl),0
    ldir

    ; --- atributos: HUD en toda la pantalla y despues el campo ---
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

    ; --- escenario: agua, piedras, HUD y huevos ---
    call draw_field
    call draw_stones
    call draw_hud_glyphs
    call place_eggs
    call house_select
    call draw_all_eggs
    call draw_egg_counter
    IF DEBUG_CHEAT
    call house_marker_toggle
    ENDIF

    ; --- estado inicial del dino ---
    xor a
    ld (cheat_god),a
    ld (cheat_prev),a
    ld (kill_prev),a
    ld (reseed_prev),a
    ld (house_state),a
    ld (house_prev),a
    ld (win_phase),a
    ld a,D_DOWN
    ld (dinitdir),a
    call dino_reset

    ; --- activar IM2: una interrupcion por fotograma (50 Hz) ---
    ld a,IM2_TABLE>>8
    ld i,a
    im 2
    ei

main_loop:
    halt                        ; espera al siguiente fotograma
    call read_keys              ; A = mascara de teclas
    ld (key_mask),a
    call show_keys
    IF DEBUG_CHEAT
    call update_cheat
    call update_kill
    call update_reseed
	call update_house_key
    ENDIF
    call tick_inv
    call show_inv
    call update_dino
	ld a,2
	out ($FE),a
	call house_flash_update
	xor a
	out ($FE),a
    jr main_loop

; ============================================================================
; frame_isr - rutina de interrupcion: cuenta fotogramas
; ============================================================================
frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti

    INCLUDE "vars.inc"              ; variables
    INCLUDE "ai.inc"                ; reaparicion y reorientacion: dino_reset..best_by_rating
    INCLUDE "eggs_logic.inc"        ; huevos: rand16, colocacion, dibujo, recogida y contador
	INCLUDE "house.inc"             ; salidas y casa
	INCLUDE "victory.inc"           ; llegada a casa y victoria
    INCLUDE "inv.inc"               ; invencibilidad: tick_inv, is_inv, show_inv
    INCLUDE "debug.inc"             ; teclas de depuracion H, K, R (IF DEBUG_CHEAT)
    INCLUDE "dino.inc"              ; movimiento, giro, caida y muerte del dino
    INCLUDE "sprite.inc"            ; sprite_draw, sprite_erase
    INCLUDE "input_hud.inc"         ; teclado y HUD (glifos y fuente de digitos)
    INCLUDE "video.inc"             ; pixel_addr, down_hl, campo y piedras

; ---- Datos ------------------------------------------------------------------
    INCLUDE "sprite_dino.inc"
    INCLUDE "stones.inc"
    INCLUDE "eggs.inc"
	INCLUDE "house_gfx.inc"
	INCLUDE "font_msg.inc"          ; fuente de mensajes

    ASSERT (sprite_dino_end - sprite_dino) == 2304
    ASSERT (stone_gfx_end - stone_gfx) == STONE_VARIANTS*16
    ASSERT (stone_map_end - stone_map) == STONE_COLS*STONE_ROWS
    ASSERT (egg_gfx_end - egg_gfx) == EGG_H*4
    ASSERT $ < IM2_TABLE

; ---- Tabla IM2 y salto a la rutina (el bus flotante suele dar $FF: por eso 257 bytes iguales) ----
    ORG IM2_TABLE
    DS 257,IM2_FILL
    ORG IM2_VECTOR
    jp frame_isr

; ---- Salida ----------------------------------------------------------------
    SAVESNA "build/trogzx_paso1.sna", start
