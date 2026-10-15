; ============================================================================
; house.inc - PASO 2e-1/2e-2/2e-3: salidas de la isla, eleccion de la salida y construccion de la casa
; Estado del codigo: PROPUESTO (2e-1 y 2e-2 ensamblados y probados por Carlos; 2e-3 sin ensamblar ni probar).
; Arcade (observaciones): cada salida (EMOVE) cuelga de un punto auxiliar del borde (EMOVEMPTR), con direccion hacia fuera (EMOVEDIR) y un centro de
;   retorno (EMOVERET) (TROGPOST.ASM: CALCA1GT / CALCA1LT / LEGAL_EXIT; TROGEQU.ASM). PLAYER_HOUSE elige la salida al empezar la fase (FIND_OPEN_EXIT, PEXITPNT).
;   La casa se construye al recoger el ultimo huevo (EGGDINO -> FIRST_HOUSE -> HOUSEB): aparece oculta 56 ticks y parpadea HOME/REX cada vez mas rapido.
; Port (INTERPRETACION pendiente de confirmar con EXITPOINTS): una salida por cada centro del perimetro y direccion hacia fuera: 9 arriba, 9 abajo, 5 izquierda, 5 derecha = 28.
;   Cada entrada: columna y fila del nodo de retorno y direccion. Todas son validas en la fase 1 (BOUNDRY1 solo tiene el acantilado exterior).
; ============================================================================
EXIT_COUNT      EQU 28
HOUSE_APPEAR_FRAMES EQU 51      ; la casa aparece oculta 56 ticks del arcade (SLEEP 38H en HOUSEB) x 0,914
HOUSE_FLASH_FRAMES  EQU 21      ; fotogramas que dura cada letrero (HOME o REX); ciclo completo 0,83 s medido en MAME, velocidad constante

; ============================================================================
; house_select - elige al azar la salida de la casa (arcade: PLAYER_HOUSE -> FIND_OPEN_EXIT). Sin filtros: en la fase 1 todas las salidas valen.
;   Salida : house_exit = 0..27. Altera : A, HL
; ============================================================================
house_select:
    call rand16
    ld a,h
.m:
    cp EXIT_COUNT
    jr c,.ok
    sub EXIT_COUNT
    jr .m
.ok:
    ld (house_exit),a
    ret

; ============================================================================
; stamp_draw - pinta un sello con mascara: pantalla = (pantalla AND mascara) OR grafico
;   Entrada: DE = datos (mascara, grafico por byte), HL = pantalla (primera linea), B = lineas, C = bytes por linea (el sello no cruza el limite de fila)
;   Altera : A, BC, DE, HL
; ============================================================================
stamp_draw:
.line:
    push hl
    push bc
    ld b,c                      ; B = bytes de la linea
.byte:
    ld a,(de)                   ; mascara
    and (hl)
    ld c,a
    inc de
    ld a,(de)                   ; grafico
    or c
    ld (hl),a
    inc de
    inc l
    djnz .byte
    pop bc
    pop hl
    call down_hl
    djnz .line
    ret

; ============================================================================
; house_build - prepara la construccion de la casa en la salida elegida (arcade: EGGDINO_HOUSE -> FIRST_HOUSE -> HOUSEB).
;   Calcula la geometria y arranca la espera (la casa aparece oculta unos 51 fotogramas); no dibuja nada: lo hace house_flash_update.
;   Extension: arriba 32x20 en lineas 24..43; abajo 32x20 en 172..191; izquierda 16x32 en X 0..15; derecha 16x32 en X 240..255.
;   Letrero 16x8 centrado en la extension. Altera : A, BC, DE, HL
; ============================================================================
house_build:
    ld a,(house_state)
    or a
    ret nz                      ; ya construida o en construccion
    ld a,(house_exit)
    ld e,a
    add a,a
    add a,e
    ld e,a
    ld d,0
    ld hl,exit_tab
    add hl,de
    ld e,(hl)                   ; E = columna
    inc hl
    ld d,(hl)                   ; D = fila
    inc hl
    ld a,(hl)                   ; A = direccion
    push af
    call node_line_col          ; B = linea superior de la calle, C = byte de la calle
    pop af
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld hl,ext_right
    ld de,$0220                 ; D = 2 bytes de ancho, E = 32 lineas
    ld c,30
    ld a,b
    sub 8
    ld b,a                      ; linea superior = calle - 8
    ld a,12                     ; letrero 12 lineas por debajo
    jr .store
.left:
    ld hl,ext_left
    ld de,$0220
    ld c,0
    ld a,b
    sub 8
    ld b,a
    ld a,12
    jr .store
.up:
    ld hl,ext_top
    ld de,$0414                 ; D = 4 bytes de ancho, E = 20 lineas
    dec c                       ; la extension empieza un byte antes que la calle
    ld b,24
    ld a,6
    jr .store
.down:
    ld hl,ext_bottom
    ld de,$0414
    dec c
    ld b,172
    ld a,6
.store:                         ; HL = datos, D = ancho, E = alto, B = linea superior, C = byte, A = desplazamiento del letrero
    push af
    ld (house_ext_ptr),hl
    ld a,d
    ld (house_ext_w),a
    ld a,e
    ld (house_ext_h),a
    push bc
	ld a,b
    ld (house_plq_y),a
    ld a,c
    ld (house_plq_x),a
    call pixel_addr             ; HL = pantalla de la extension
    ld (house_ext_addr),hl
    pop bc
    pop af
    add a,b
    ld b,a                      ; B = linea del letrero
    ld a,(house_ext_w)
    srl a
    dec a                       ; 1 si ancho = 4 bytes, 0 si ancho = 2
    add a,c
    ld c,a                      ; C = byte del letrero
    call pixel_addr
    ld (house_plq_addr),hl
    xor a
    ld (house_frame),a
    ld a,HOUSE_APPEAR_FRAMES
    ld (house_timer),a
    ld a,1
    ld (house_state),a
    ret

; ============================================================================
; house_draw_ext / house_draw_plaque - pintan la extension y el letrero actual (HOME/REX segun house_frame). Altera : A, BC, DE, HL
; ============================================================================
house_draw_ext:
    ld de,(house_ext_ptr)
    ld hl,(house_ext_addr)
    ld a,(house_ext_h)
    ld b,a
    ld a,(house_ext_w)
    ld c,a
    jp stamp_draw

house_draw_plaque:
    ld a,(house_frame)
    rrca
    rrca
    rrca                        ; fotograma * 32 (0 o 32)
    ld e,a
    ld d,0
    ld hl,plaque_gfx
    add hl,de
    ex de,hl                    ; DE = datos del letrero
    ld hl,(house_plq_addr)
    ld bc,$0802                 ; B = 8 lineas, C = 2 bytes
    jp stamp_draw

; ============================================================================
; house_flash_update - una vez por fotograma: espera, aparicion de la casa y parpadeo HOME/REX cada vez mas rapido (arcade: HOUSE_FLASH_LP).
;   Al repintar protege el sprite del dino: lo borra, pinta la casa y lo vuelve a pintar (guarda el fondo nuevo). Altera : todos
; ============================================================================
house_flash_update:
    ld a,(house_state)
    or a
    ret z
    ld hl,house_timer
    dec (hl)
    ret nz
    cp 1
    jr z,.appear
    ld a,(house_frame)          ; estado 2: alterna HOME/REX a velocidad constante
    xor 1
    ld (house_frame),a
    ld a,HOUSE_FLASH_FRAMES
    ld (house_timer),a
    call house_plq_overlap
    jp nc,house_draw_plaque     ; sin solape: se pinta el rotulo y no se toca el sprite
    ld b,0                      ; solape: ruta protegida de siempre
    jr .paint
.appear:
    ld a,2
    ld (house_state),a
    ld a,HOUSE_FLASH_FRAMES
    ld (house_timer),a
    ld b,1                      ; extension y letrero
.paint:
    ld a,(saved_valid)
    ld c,a                      ; C = habia sprite pintado
    push bc
    call sprite_erase
    pop bc
    push bc
    ld a,b
    or a
    call nz,house_draw_ext
    call house_draw_plaque
    pop bc
    ld a,c
    or a
    ret z
    jp sprite_draw              ; vuelve a pintar el sprite (saved_valid sigue en 1)

    IF DEBUG_CHEAT
; ============================================================================
; house_marker_toggle - DEPURACION: dibuja (XOR) el glifo del huevo junto a la salida elegida, fuera de la zona de juego. Una segunda llamada lo borra.
;   Arriba: linea 28; abajo: linea 180; izquierda: byte 0; derecha: byte 31. Altera : A, BC, DE, HL
; ============================================================================
house_marker_toggle:
    ld a,(house_exit)
    ld e,a
    add a,a
    add a,e                     ; A = 3 * indice
    ld e,a
    ld d,0
    ld hl,exit_tab
    add hl,de                   ; HL = entrada (columna, fila, direccion)
    ld e,(hl)                   ; E = columna
    inc hl
    ld d,(hl)                   ; D = fila
    inc hl
    ld a,(hl)                   ; A = direccion
    push af
    call node_line_col          ; B = linea de la calle, C = byte de la columna
    pop af
    or a
    jr z,.up
    dec a
    jr z,.down
    dec a
    jr z,.left
.right:
    ld c,31
    jr .side
.left:
    ld c,0
.side:
    ld a,b
    add a,4                     ; centra el glifo de 8 lineas en la calle de 16
    ld b,a
    jr .draw
.up:
    ld b,28
    jr .draw
.down:
    ld b,180
.draw:
    call pixel_addr             ; HL = pantalla
    ld de,glyph_egg
    ld b,8
.l:
    ld a,(de)
    xor (hl)
    ld (hl),a
    inc de
    call down_hl
    djnz .l
    ret

; ============================================================================
; update_house_key - DEPURACION: la tecla B construye la casa en la salida elegida (una vez por pulsacion). Altera : A, BC
; ============================================================================
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
    ENDIF
	
; ============================================================================
; house_plq_overlap - ¿toca el sprite guardado el rotulo? Salida: C = si, NC = no (o sin sprite). Altera : A, BC
; ============================================================================
house_plq_overlap:
    ld a,(saved_valid)
    or a
    ret z                       ; NC
    ld a,(house_plq_x)
    inc a
    ld b,a                      ; B = columna derecha del rotulo + 1
    ld a,(save_col)
    cp b
    jr z,.h1
    jr nc,.no                   ; el sprite empieza a la derecha
.h1:
    ld a,(save_col)
    ld c,a
    ld a,(save_width)
    add a,c
    ld b,a                      ; B = primera columna tras el sprite
    ld a,(house_plq_x)
    cp b
    jr nc,.no                   ; el sprite acaba a la izquierda
    ld a,(house_plq_y)
    add a,7
    ld b,a                      ; B = ultima linea del rotulo
    ld a,(save_first)
    cp b
    jr z,.v1
    jr nc,.no                   ; el sprite esta debajo
.v1:
    ld a,(save_first)
    ld c,a
    ld a,(save_count)
    add a,c
    ld b,a                      ; B = primera linea tras el sprite
    ld a,(house_plq_y)
    cp b
    jr nc,.no                   ; el sprite esta encima
    scf
    ret
.no:
    or a                        ; NC
    ret

; ---- Tabla de salidas: columna, fila del nodo de retorno, direccion de salida ----
exit_tab:
    DB 0,0,D_UP, 1,0,D_UP, 2,0,D_UP, 3,0,D_UP
    DB 4,0,D_UP, 5,0,D_UP, 6,0,D_UP, 7,0,D_UP
    DB 8,0,D_UP, 0,4,D_DOWN, 1,4,D_DOWN, 2,4,D_DOWN
    DB 3,4,D_DOWN, 4,4,D_DOWN, 5,4,D_DOWN, 6,4,D_DOWN
    DB 7,4,D_DOWN, 8,4,D_DOWN, 0,0,D_LEFT, 0,1,D_LEFT
    DB 0,2,D_LEFT, 0,3,D_LEFT, 0,4,D_LEFT, 8,0,D_RIGHT
    DB 8,1,D_RIGHT, 8,2,D_RIGHT, 8,3,D_RIGHT, 8,4,D_RIGHT
exit_tab_end:
    ASSERT (exit_tab_end - exit_tab) == EXIT_COUNT*3
