; level1_map.inc - Mapa provisional del nivel 1 (15 columnas x 11 filas de tiles 16x16)
; 0 = pasillo, 1 = roca. Fuente: layout aportado por Carlos.
; En las filas 0 y 10, los 8 px exteriores pertenecen al contorno de caida.
level1_map:
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 0
    DB 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; fila 1
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 2
    DB 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; fila 3
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 4
    DB 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; fila 5
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 6
    DB 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; fila 7
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 8
    DB 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0    ; fila 9
    DB 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1    ; fila 10
level1_map_end:
