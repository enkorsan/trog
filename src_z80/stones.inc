; stones.inc - piedras 8x8 con mascara y mapa de variantes (generado por tools/make_stones.py; se puede editar a mano).
; Formato por variante: 8 lineas x (mascara, grafico). Pantalla = (pantalla AND mascara) OR grafico.
STONE_VARIANTS  EQU 3
stone_gfx:
    DB $C3,$3C,$81,$52,$00,$83,$00,$A1,$00,$85,$00,$C1,$81,$4A,$C3,$3C    ; variante 0
    DB $E7,$18,$C3,$24,$81,$42,$81,$4A,$00,$81,$00,$91,$00,$83,$81,$7E    ; variante 1
    DB $FF,$00,$FF,$00,$C3,$3C,$81,$42,$00,$A1,$00,$85,$00,$C1,$81,$7E    ; variante 2
stone_gfx_end:
; variante de cada piedra: 6 filas x 10 columnas (fila r, columna c = indice r*10 + c)
stone_map:
    DB 2,1,2,0,2,0,2,0,2,1
    DB 0,2,0,2,0,1,0,2,1,2
    DB 2,1,2,1,2,0,1,0,2,0
    DB 1,2,1,0,1,2,0,1,0,2
    DB 2,0,2,1,2,0,2,0,2,0
    DB 1,2,0,2,1,2,1,2,1,2
stone_map_end:
