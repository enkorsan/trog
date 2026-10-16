; ============================================================================
; game_r7.asm - ESQUELETO del juego sobre el renderer (paso 1 del plan de integracion)
; Estado: PROPUESTO (sin ensamblar ni probar). Contrato: docs/decisions_render_update_3.md (rev. 1) y render.inc.
; Contenido: bucle blit_all -> logica -> prepare_all -> frame_wait (un paso por iteracion, espera por contador, sin HALT),
;   dino como entidad 0 (dino_r7.inc), velocidad SPEED_BASE/SPEED_MOD = 26/64, HUD de teclas a la izquierda y contadores LT/PC.
; DEPENDE DE render_core.inc, que AUN NO EXISTE: debe extraerse del R6.3 (bench_r6.asm / src/bench_r6_integracion.asm):
;   ent_set_qpos, ent_prep1, ent_prep2, ent_hits_other, ent_q_intersects, ent_pre, ent_post, nb_range, draw_clipped, sort_z,
;   ent_blit, prepare_all, blit_all, build_bg, b_print_dec4 (y sus datos: pdtab, pdline, pdcol), con los labels ents/scr_bufs/qpos/zlist
;   sustituidos por ENTS/SCR_BUFS/QPOS/ZLIST y sin BENCH_*. Hasta entonces este fichero no ensambla.
; Para 48K: sjasmplus -DTARGET_48K (no probado).
; Compilacion prevista (Windows, desde la raiz): sjasmplus --nologo --msg=err --lst=build/game_r7.lst --sym=build/game_r7.sym -Isrc src/game_r7.asm
; ============================================================================
ENT_COUNT       EQU 1           ; solo el dino
DBG_HUD         EQU 1           ; 1 = LT y PC en la mitad derecha del HUD (fila 2, columnas 19 y 27)

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

    PAGE_BG
    call build_bg               ; fondo limpio (sin sprites) en BG_BASE
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
    call blit_all               ; vuelca lo preparado en la iteracion anterior
    call game_update            ; UN paso de logica
    call prepare_all            ; compone las ventanas de las entidades sucias
    IF DBG_HUD
    call dbg_hud
    ENDIF
    call frame_wait
    jp main_loop

; game_update - lee el teclado, mueve el dino (entidad 0) y recalcula su posicion de dibujo. Altera: todos
game_update:
    call read_keys
    ld (key_mask),a
    call show_keys
    call dino_update            ; A = 1 si hay que redibujar
    ld ix,ENTS
    ld (ix+E_DIRTY),a
    or a
    call nz,ent_set_qpos
    ret

; frame_wait - espera por contador. Una sola lectura de frame_counter; una iteracion tardia no espera y no se recupera.
;   LT += 1 y PC += n-1 (salvo en iteraciones con HUD, hud_iter = 1). Altera: A, BC, DE, HL
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
    ld b,a                      ; B = n interrupciones cruzadas
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
; dbg_hud - cada 16 iteraciones escribe PC (columna 19) y 8 despues LT (columna 27), linea 16 (fila de caracteres 2). Altera: todos
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

; render_init - fija E_SBUF, E_IDX y E_TIE de las ENT_COUNT entidades, y rellena ZLIST. (Pertenece al renderer: mover a render_core.inc.)
render_init:
    ld hl,ENTS
    ld de,ENTS+1
    ld bc,MAX_ENTS*ENT_SIZE-1
    ld (hl),0
    ldir
    ld ix,ENTS
    ld hl,SCR_BUFS
    ld b,ENT_COUNT
    ld c,0
.l:
    ld (ix+E_SBUF),l
    ld (ix+E_SBUF+1),h
    ld (ix+E_IDX),c
    ld a,c
    or a
    jr nz,.tie
    ld a,255
.tie:
    ld (ix+E_TIE),a
    ld de,WIN_BUF
    add hl,de
    ld de,ENT_SIZE
    add ix,de
    inc c
    djnz .l
    ld hl,ENTS
    ld de,ZLIST
    ld b,ENT_COUNT
.z:
    ld a,l
    ld (de),a
    inc de
    ld a,h
    ld (de),a
    inc de
    push de
    ld de,ENT_SIZE
    add hl,de
    pop de
    djnz .z
    ret

frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti

; ---- Variables del esqueleto (en el area de codigo) ---------------------------
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

    INCLUDE "dino_r7.inc"
    INCLUDE "render_core.inc"       ; POR EXTRAER del R6.3
    INCLUDE "input_hud.inc"         ; read_keys, show_keys, draw_hud_glyphs, font_digits
    INCLUDE "video.inc"             ; pixel_addr, down_hl, draw_field, draw_stones

; ---- Datos ------------------------------------------------------------------
    INCLUDE "sprite_dino.inc"
    INCLUDE "sprite_trog.inc"       ; ent_set_qpos referencia sprite_trog aunque ENT_COUNT = 1
    INCLUDE "stones.inc"
    INCLUDE "font_msg.inc"

    ASSERT (sprite_dino_end - sprite_dino) == IMG_COUNT*4*144
    ASSERT $ <= SCR_BUFS

; ---- Tabla IM2 y salto a la rutina ---------------------------------------------
    ORG IM2_TABLE
    DS 257,IM2_FILL
    ORG IM2_VECTOR
    jp frame_isr

    SAVESNA "build/game_r7.sna", start
