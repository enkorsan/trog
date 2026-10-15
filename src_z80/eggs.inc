; eggs.inc - sello del huevo 16x16 con mascara (provisional; Carlos lo sustituira). Por linea: mascara0, grafico0, mascara1, grafico1.
; Pantalla = (pantalla AND mascara) OR grafico. Se dibuja en la esquina superior izquierda de la calle de cada interseccion (alineado a byte).
egg_gfx:
    DB $FF,$00,$FF,$00,$FF,$00,$FF,$00,$FF,$00,$FF,$00,$FC,$03,$3F,$C0
    DB $F8,$04,$1F,$20,$F0,$08,$0F,$10,$F0,$08,$0F,$10,$F0,$09,$0F,$10
    DB $F0,$08,$0F,$50,$F0,$08,$0F,$90,$F0,$08,$0F,$10,$F8,$04,$1F,$20
    DB $FC,$03,$3F,$C0,$FF,$00,$FF,$00,$FF,$00,$FF,$00,$FF,$00,$FF,$00
egg_gfx_end:
