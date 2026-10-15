; ============================================================================
; bench_r1.asm - BANCO DE PRUEBAS R1 (render): linea base con el motor actual (borrar y pintar) para N entidades.
; Estado del codigo: PROPUESTO (sin ensamblar ni probar). No toca el juego: colocar junto a main.asm (usa sus .inc por INCLUDE relativo).
; Usa del juego: config.inc, video.inc (pixel_addr, down_hl, draw_field, draw_stones), stones.inc, sprite_dino.inc, font_msg.inc.
; Escenario: BENCH_N entidades (todas con el sprite del dino de 16x24) que andan por las calles a BENCH_SPEED pasos de 2 px cada 16 fotogramas
;   (acumulador como el dino), giran al azar en cada interseccion y no se evitan entre si. Cada fotograma: logica de todas, borrado de las que se
;   han movido y pintado de las que se han movido (rutinas clonadas de sprite.inc con la entidad en IX; mismo algoritmo que el motor actual).
; Medidas en el HUD (ventana de 64 fotogramas):
;   LIBRE = tiempo libre medio por fotograma en unidades de 70 T-states (un fotograma de 128K = 70.908 T = ~1013 unidades).
;   MIN   = menor tiempo libre de un fotograma de la ventana, en unidades de 35 T-states (~2026 = fotograma entero libre). 0 = hubo retraso.
;   RETR  = iteraciones del bucle que acabaron despues de la siguiente interrupcion (el juego va mas lento).
;   PERD  = fotogramas perdidos en la ventana (suma de los retrasos).
; Franjas del borde (BENCH_PROF = 1): rojo = logica, azul = borrado, verde = pintado, negro = libre. 1 linea de borde = 228 T-states.
; ============================================================================
BENCH_N         EQU 5           ; entidades (1..8)
BENCH_STAGGER   EQU 1           ; 1 = acumuladores iniciales escalonados (3*i mod 16); 0 = todos en fase
BENCH_SPEED     EQU 7           ; pasos de 2 px cada 16 fotogramas (7 = DINO_SPEED actual)
BENCH_PROF      EQU 1           ; 1 = franjas de color en el borde
    ASSERT BENCH_N >= 1 && BENCH_N <= 8

    DEVICE ZXSPECTRUM128

    INCLUDE "config.inc"

IM2_TABLE   EQU $B800
IM2_FILL    EQU $B9
IM2_VECTOR  EQU (IM2_FILL<<8)|IM2_FILL

; ---- Estructura de entidad (IX apunta al principio) ----
E_X         EQU 0
E_Y         EQU 1
E_DIR       EQU 2
E_ACC       EQU 3
E_SPD       EQU 4
E_COL       EQU 5               ; columna de la interseccion (0..8)
E_ROW       EQU 6               ; fila de la interseccion (0..4)
E_LEFT      EQU 7               ; pasos que quedan hasta la siguiente interseccion (0 = en una interseccion)
E_MOVED     EQU 8               ; 1 = se ha movido este fotograma
E_VALID     EQU 9               ; 1 = hay sprite pintado con el fondo guardado
E_FIRST     EQU 10              ; primera linea guardada
E_SCOL      EQU 11              ; byte de la primera columna guardada
E_COUNT     EQU 12              ; lineas guardadas
E_WIDTH     EQU 13              ; bytes por linea guardados
E_PHASE     EQU 14
E_SPRW      EQU 15
E_I0        EQU 16              ; lineas recortadas por arriba
E_TOP       EQU 17              ; linea superior del sprite
E_SAVE      EQU 20              ; fondo guardado: 24 lineas x 3 bytes
ENT_SIZE    EQU E_SAVE+72

    ORG $8000

start:
    di
    ld sp,$7FFF
    xor a
    out ($FE),a                 ; borde negro

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
    call hud_labels
    call init_ents
    call erase_all              ; E_VALID = 0: no hace nada
    call draw_all               ; E_MOVED = 1 en todas: primer pintado

    xor a
    ld (frame_counter),a
    ld a,IM2_TABLE>>8
    ld i,a
    im 2
    ei
    ld a,(frame_counter)
    ld (frame_start),a

; ============================================================================
; main_loop - logica, borrado, pintado, medida del tiempo libre y sincronizacion con la interrupcion (sin HALT: se cuenta mientras se espera)
; ============================================================================
main_loop:
    IF BENCH_PROF
    ld a,2
    out ($FE),a                 ; rojo: logica
    ENDIF
    call update_all
    IF BENCH_PROF
    ld a,1
    out ($FE),a                 ; azul: borrado
    ENDIF
    call erase_all
    IF BENCH_PROF
    ld a,4
    out ($FE),a                 ; verde: pintado
    ENDIF
    call draw_all
    IF BENCH_PROF
    xor a
    out ($FE),a                 ; negro: libre
    ENDIF

    ld a,(frame_counter)
    ld hl,frame_start
    sub (hl)                    ; A = fotogramas transcurridos desde el principio de la iteracion
    jr nz,.over
    ld c,(hl)                   ; C = valor de frame_counter al empezar
    ld de,0
.wait:
    inc de                      ; 35 T-states por vuelta: inc de 6 + ld a,(nn) 13 + cp c 4 + jr z 12
    ld a,(frame_counter)
    cp c
    jr z,.wait
    ld (frame_start),a          ; A = nuevo valor de frame_counter
    jr .account
.over:
    ld c,a                      ; C = fotogramas de retraso (>= 1)
    ld hl,overruns
    inc (hl)
    ld a,(lostf)
    add a,c
    ld (lostf),a
    ld a,(frame_counter)
    ld (frame_start),a
    ld de,0
.account:                       ; DE = tiempo libre en unidades de 35 T-states
    ld hl,(idle_min)
    or a
    sbc hl,de
    jr c,.nomin                 ; el minimo guardado ya es menor
    ld (idle_min),de
.nomin:
    srl d
    rr e                        ; DE / 2: unidades de 70 T-states
    ld hl,(idle_sum)
    add hl,de
    ld (idle_sum),hl
    ld hl,win_count
    inc (hl)
    ld a,(hl)
    cp 64
    call nc,report
    jp main_loop

; ============================================================================
; report - cada 64 iteraciones escribe las medidas en el HUD y empieza otra ventana. Altera : todos
; ============================================================================
report:
    ld hl,(idle_sum)
    ld b,6
.s:
    srl h
    rr l
    djnz .s                     ; HL = media / 64
    ld b,0
    ld c,6
    call b_print_dec4
    ld hl,(idle_min)
    ld b,0
    ld c,15
    call b_print_dec4
    ld a,(overruns)
    ld l,a
    ld h,0
    ld b,8
    ld c,5
    call b_print_dec4
    ld a,(lostf)
    ld l,a
    ld h,0
    ld b,8
    ld c,15
    call b_print_dec4
    xor a
    ld (win_count),a
    ld (overruns),a
    ld (lostf),a
    ld hl,0
    ld (idle_sum),hl
    ld hl,$FFFF
    ld (idle_min),hl
    ret

frame_isr:
    push af
    ld a,(frame_counter)
    inc a
    ld (frame_counter),a
    pop af
    ei
    reti

; ============================================================================
; init_ents - entidad i en la interseccion (columna 2i mod 9, fila i mod 5). Altera : A, BC, DE, IX
; ============================================================================
init_ents:
    ld ix,ents
    ld b,BENCH_N
    ld c,0                      ; C = i
.l:
    ld a,c
    add a,a                     ; 2i
.mc:
    cp 9
    jr c,.cok
    sub 9
    jr .mc
.cok:
    ld (ix+E_COL),a
    ld e,a
    add a,a
    add a,e
    add a,a
    add a,a
    add a,a                     ; 24 * columna
    add a,NODE_X0
    ld (ix+E_X),a
    ld a,c
.mr:
    cp 5
    jr c,.rok
    sub 5
    jr .mr
.rok:
    ld (ix+E_ROW),a
    ld e,a
    add a,a
    add a,e
    add a,a
    add a,a
    add a,a                     ; 24 * fila
    add a,NODE_Y0
    ld (ix+E_Y),a
    IF BENCH_STAGGER
    ld a,c
    ld e,a
    add a,a
    add a,e
    and 15                      ; 3i mod 16
    ELSE
    xor a
    ENDIF
    ld (ix+E_ACC),a
    ld (ix+E_SPD),BENCH_SPEED
    ld (ix+E_LEFT),0
    ld (ix+E_DIR),0
    ld (ix+E_MOVED),1
    ld (ix+E_VALID),0
    ld de,ENT_SIZE
    add ix,de
    inc c
    djnz .l
    ret

; ============================================================================
; update_all / erase_all / draw_all - recorren las BENCH_N entidades. Altera : todos
; ============================================================================
update_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    call ent_update             ; A = 1 si se ha movido
    ld (ix+E_MOVED),a
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

erase_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    ld a,(ix+E_MOVED)
    or a
    call nz,ent_erase
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

draw_all:
    ld ix,ents
    ld b,BENCH_N
.l:
    push bc
    ld a,(ix+E_MOVED)
    or a
    call nz,ent_draw
    ld bc,ENT_SIZE
    add ix,bc
    pop bc
    djnz .l
    ret

; ============================================================================
; ent_update - un fotograma de una entidad (acumulador de velocidad, eleccion de direccion en intersecciones, paso de 2 px)
;   Entrada: IX. Salida: A = 1 si se ha movido, 0 si no. Altera : A, BC, DE, HL
; ============================================================================
ent_update:
    ld a,(ix+E_ACC)
    add a,(ix+E_SPD)
    cp 16
    jr c,.keep
    sub 16
    ld (ix+E_ACC),a
    jr .step
.keep:
    ld (ix+E_ACC),a
    xor a
    ret
.step:
    ld a,(ix+E_LEFT)
    or a
    call z,ent_choose           ; en una interseccion: elige direccion y pone E_LEFT = 12
    ld a,(ix+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    inc (ix+E_X)
    inc (ix+E_X)
    jr .moved
.left:
    dec (ix+E_X)
    dec (ix+E_X)
    jr .moved
.up:
    dec (ix+E_Y)
    dec (ix+E_Y)
    jr .moved
.down:
    inc (ix+E_Y)
    inc (ix+E_Y)
.moved:
    dec (ix+E_LEFT)
    call z,ent_arrive           ; llegada a la interseccion: actualiza columna o fila
    ld a,1
    ret

ent_arrive:
    ld a,(ix+E_DIR)
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
    inc (ix+E_COL)
    ret
.left:
    dec (ix+E_COL)
    ret
.up:
    dec (ix+E_ROW)
    ret
.down:
    inc (ix+E_ROW)
    ret

; ent_choose - elige direccion al azar entre las validas (la inversa solo si no hay otra). Altera : A, BC, DE, HL
ent_choose:
    ld b,0                      ; B = mascara de direcciones validas (bit = direccion: 0 arriba, 1 abajo, 2 izquierda, 3 derecha)
    ld a,(ix+E_ROW)
    or a
    jr z,.no_up
    set 0,b
.no_up:
    ld a,(ix+E_ROW)
    cp NODE_ROWS_M1
    jr nc,.no_dn
    set 1,b
.no_dn:
    ld a,(ix+E_COL)
    or a
    jr z,.no_lf
    set 2,b
.no_lf:
    ld a,(ix+E_COL)
    cp NODE_COLS_M1
    jr nc,.no_rt
    set 3,b
.no_rt:
    ld a,(ix+E_DIR)
    xor 1
    ld e,a
    call get_bit                ; A = bit de la direccion inversa
    cpl
    and b                       ; validas sin la inversa
    jr nz,.have
    ld a,b                      ; solo queda la inversa
.have:
    ld c,a                      ; C = mascara de eleccion
    call b_rand
    ld a,h
    and 3
    ld e,a                      ; E = direccion inicial del barrido
    ld d,4
.scan:
    call get_bit
    and c
    jr nz,.found
    ld a,e
    inc a
    and 3
    ld e,a
    dec d
    jr nz,.scan
.found:
    ld (ix+E_DIR),e
    ld (ix+E_LEFT),NODE_STEP/2
    ret

; get_bit - Entrada: E = 0..3. Salida: A = 1 << E. Altera : A, HL
get_bit:
    ld hl,bit_tab
    ld a,l
    add a,e
    ld l,a
    ld a,(hl)
    ret

    ALIGN 4
bit_tab:
    DB 1,2,4,8

; b_rand - xorshift de 16 bits (copia de rand16 de eggs_logic.inc). Salida: HL. Altera : A, HL
b_rand:
    ld hl,(b_rng)
    ld a,h
    rra
    ld a,l
    rra
    xor h
    ld h,a
    ld a,l
    rra
    ld a,h
    rra
    xor l
    ld l,a
    xor h
    ld h,a
    ld (b_rng),hl
    ret

; ============================================================================
; ent_draw - guarda el fondo y pinta el sprite (clon de sprite_draw con la entidad en IX). Altera : A, BC, DE, HL, IY
; ============================================================================
ent_draw:
    ld a,(ix+E_X)
    and 7
    rrca
    ld (ix+E_PHASE),a
    ld b,3
    or a
    jr nz,.w_ok
    ld b,2
.w_ok:
    ld (ix+E_SPRW),b
    ld a,(ix+E_DIR)
    add a,a
    add a,a
    add a,(ix+E_PHASE)          ; indice = direccion * 4 + fase
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl                   ; indice * 16
    ld d,h
    ld e,l
    add hl,hl
    add hl,hl
    add hl,hl                   ; indice * 128
    add hl,de                   ; indice * 144
    ld de,sprite_dino
    add hl,de
    ld a,(ix+E_Y)
    sub SPR_OFF_Y
    ld (ix+E_TOP),a
    cp CLIP_TOP
    jr nc,.no_top_clip
    ld c,a
    ld a,CLIP_TOP
    sub c
    jr .have_i0
.no_top_clip:
    xor a
.have_i0:
    ld (ix+E_I0),a
    ld e,a
    add a,a
    add a,e
    add a,a                     ; i0 * 6 bytes
    ld e,a
    ld d,0
    add hl,de                   ; HL = datos de la primera linea visible
    ld a,(ix+E_TOP)
    ld c,a
    ld a,(ix+E_I0)
    add a,c
    ld (ix+E_FIRST),a
    ld a,c
    add a,SPR_H
    cp CLIP_BOTTOM
    jr c,.last_ok
    ld a,CLIP_BOTTOM
.last_ok:
    ld c,a
    ld a,(ix+E_FIRST)
    ld b,a
    ld a,c
    sub b
    ld (ix+E_COUNT),a
    ld a,(ix+E_SPRW)
    ld (ix+E_WIDTH),a
    ld a,(ix+E_X)
    rrca
    rrca
    rrca
    and %00011111
    ld (ix+E_SCOL),a
    ld c,a
    ld a,(ix+E_FIRST)
    ld b,a
    push hl
    call pixel_addr
    ex de,hl                    ; DE = pantalla
    pop hl                      ; HL = datos del sprite
    push ix
    pop iy
    ld bc,E_SAVE
    add iy,bc                   ; IY = fondo guardado de esta entidad
    ld a,(ix+E_COUNT)
    ld b,a
.line:
    push de
    ld a,(ix+E_SPRW)
    ld c,a
.byte:
    ld a,(de)
    ld (iy+0),a                 ; guarda el fondo
    inc iy
    and (hl)                    ; mascara
    inc hl
    or (hl)                     ; grafico
    inc hl
    ld (de),a
    inc de
    dec c
    jr nz,.byte
    ld a,(ix+E_SPRW)
    cp 3
    jr z,.no_skip
    inc hl                      ; ancho 2: salta el tercer par (mascara, grafico)
    inc hl
.no_skip:
    pop de
    ex de,hl
    call down_hl
    ex de,hl
    djnz .line
    ld (ix+E_VALID),1
    ret

; ============================================================================
; ent_erase - restaura el fondo guardado (clon de sprite_erase con la entidad en IX). Altera : A, BC, DE, HL, IY
; ============================================================================
ent_erase:
    ld a,(ix+E_VALID)
    or a
    ret z
    ld a,(ix+E_SCOL)
    ld c,a
    ld a,(ix+E_FIRST)
    ld b,a
    call pixel_addr
    ex de,hl                    ; DE = pantalla
    push ix
    pop iy
    ld bc,E_SAVE
    add iy,bc
    ld a,(ix+E_COUNT)
    ld b,a
.line:
    push de
    ld a,(ix+E_WIDTH)
    ld c,a
.byte:
    ld a,(iy+0)
    ld (de),a
    inc iy
    inc de
    dec c
    jr nz,.byte
    pop de
    ex de,hl
    call down_hl
    ex de,hl
    djnz .line
    ret

; ============================================================================
; Texto del HUD (font_msg: glifo = font_msg + (ASCII - 32) * 8)
; ============================================================================
hud_labels:
    ld hl,txt_libre
    ld b,0
    ld c,0
    call b_print_text
    ld hl,txt_min
    ld b,0
    ld c,11
    call b_print_text
    ld hl,txt_retr
    ld b,8
    ld c,0
    call b_print_text
    ld hl,txt_perd
    ld b,8
    ld c,10
    call b_print_text
    ld hl,txt_spr
    ld b,16
    ld c,0
    call b_print_text
    ld hl,txt_esc
    ld b,16
    ld c,9
    call b_print_text
    ld hl,txt_r1
    ld b,16
    ld c,19
    call b_print_text
    ld hl,0
    ld b,0
    ld c,6
    call b_print_dec4
    ld hl,0
    ld b,0
    ld c,15
    call b_print_dec4
    ld hl,0
    ld b,8
    ld c,5
    call b_print_dec4
    ld hl,0
    ld b,8
    ld c,15
    call b_print_dec4
    ld hl,BENCH_N
    ld b,16
    ld c,4
    call b_print_dec4
    ld hl,BENCH_STAGGER
    ld b,16
    ld c,13
    call b_print_dec4
    ret

txt_libre:  DB "LIBRE",0
txt_min:    DB "MIN",0
txt_retr:   DB "RETR",0
txt_perd:   DB "PERD",0
txt_spr:    DB "SPR",0
txt_esc:    DB "ESC",0
txt_r1:     DB "R1 BORRAR/PINTAR",0

; b_put_glyph - DE = glifo, HL = pantalla (primera linea). Altera : A, B, DE, HL
b_put_glyph:
    ld b,8
.l:
    ld a,(de)
    ld (hl),a
    inc de
    inc h
    djnz .l
    ret

; b_print_text - HL = cadena (0 al final), B = linea (multiplo de 8), C = byte inicial. Altera : todos salvo IX
b_print_text:
.l:
    ld a,(hl)
    or a
    ret z
    push hl
    push bc
    sub 32
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,font_msg
    add hl,de
    ex de,hl
    call pixel_addr
    call b_put_glyph
    pop bc
    pop hl
    inc hl
    inc c
    jr .l

; b_print_dec4 - HL = valor (< 10000), B = linea, C = byte inicial: escribe 4 cifras. Altera : todos salvo IX
b_print_dec4:
    ld a,b
    ld (pd_line),a
    ld a,c
    ld (pd_col),a
    ld de,pd_tab
    ld b,4
.digit:
    push bc
    push de
    ld a,(de)
    ld c,a
    inc de
    ld a,(de)
    ld b,a                      ; BC = divisor
    xor a
.sub:
    or a
    sbc hl,bc
    jr c,.done
    inc a
    jr .sub
.done:
    add hl,bc                   ; restaura el resto
    push hl
    add a,'0'-32
    ld l,a
    ld h,0
    add hl,hl
    add hl,hl
    add hl,hl
    ld de,font_msg
    add hl,de
    ex de,hl                    ; DE = glifo de la cifra
    ld a,(pd_line)
    ld b,a
    ld a,(pd_col)
    ld c,a
    call pixel_addr
    call b_put_glyph
    ld hl,pd_col
    inc (hl)
    pop hl
    pop de
    inc de
    inc de
    pop bc
    djnz .digit
    ret

pd_tab:     DW 1000,100,10,1

; ---- Variables ----
frame_counter:  DB 0
frame_start:    DB 0
overruns:       DB 0
lostf:          DB 0
win_count:      DB 0
idle_sum:       DW 0
idle_min:       DW $FFFF
pd_line:        DB 0
pd_col:         DB 0
b_rng:          DW $A5C3
ents:           DS ENT_SIZE*8

; ---- Codigo y datos del juego ----
    INCLUDE "video.inc"         ; pixel_addr, down_hl, draw_field, draw_stones
    INCLUDE "sprite_dino.inc"
    INCLUDE "stones.inc"
    INCLUDE "font_msg.inc"

    ASSERT $ < IM2_TABLE

; ---- Tabla IM2 y salto a la rutina ----
    ORG IM2_TABLE
    DS 257,IM2_FILL
    ORG IM2_VECTOR
    jp frame_isr

    SAVESNA "build/bench_r1.sna", start
