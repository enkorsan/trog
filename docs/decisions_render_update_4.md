# Suplemento 4 de decisiones: decorado, giro y tamanos de sprite

Version: 16-oct-2026, 20:1x, revision 0. Suplemento 4 a decisions.md revision 15; se suma a los suplementos 1, 2 y 3. No sustituye ninguno. Las decisiones marcadas "Decidido por Carlos" son suyas; lo demas son interpretaciones del asistente. Nada de lo de aqui esta ensamblado ni probado.

## 1. Decorado (decidido por Carlos, 16-oct, 19:57)
- Casa, letrero de la casa y las animaciones de aparicion de Trogs, pozas y fosos se tratan como DECORADO, siempre que no tengan colision durante la animacion de aparicion.
- Se pintan siempre POR DEBAJO de las entidades: la prioridad la tiene el sprite.
- Consecuencia: ninguna capa de decorado se compone por encima de una entidad; decor_rect (decisions_render_update_3.md, seccion 4) basta.
- Pendiente de verificar en el fuente arcade: que el Trog y el pozo no tengan colision mientras emerge (TROG_PIT, TINIT_TAB; no leidos). Si alguno colisiona, esa animacion no es decorado.

## 2. Giro del dino (decidido por Carlos, 16-oct, 19:57)
- Giro de 90 grados: 1 imagen diagonal intermedia.
- Media vuelta (180 grados): pasa por la diagonal y la perpendicular, por ejemplo derecha, derecha-abajo, abajo, abajo-izquierda, izquierda; al azar tambien gira por arriba. Se aplica a las cuatro direcciones.
- Puñetazo: 1 cuadro por direccion. Giro y puñetazo del Trog: pendiente de estudio.
- Las imagenes intermedias (IMG_UL, IMG_UR, IMG_DL, IMG_DR) ya estan reservadas en render.inc (E_IMG 4..7).

## 3. Lo que se sabe del giro en el arcade (observado)
Fuente: docs/analysis del proyecto (analisis_dino_stick_y_giro.md, de una conversacion anterior de Carlos; no se ha vuelto a comprobar contra el fuente) y TROGDDAT.ASM (SPIKEUL..SPIKELD, SPIKEABOUT_*).
- La direccion contraria a la actual se acepta siempre, en cualquier punto del pasillo (DS_CK_NEW en TROGDINO.ASM).
- Un giro perpendicular solo se acepta si el dino esta a menos de [4,4] de una interseccion registrada; entonces DINO_CENTER lo alinea (DS_CENT_CK, DS_ND_CENTER). El port ya lo hace con TURN_TOL = 2.
- PJOY mantiene un unico bit de direccion y no hay cola de giros.
- Los guiones de giro de 90 grados duran unos 45 ticks (30 de "mirar" con SPLK* y 15 de rotacion del cuerpo con SPSTP*), con 3 imagenes intermedias; los de media vuelta SPIKEABOUT_*, entre unos 130 ticks (UL, con 110 de mirar) y menos en las demas.

## 4. Desconocido
- Cual de esos guiones usa realmente el arcade al invertir la marcha del jugador (DINOTURN, DINOLOOK_TURN, ANIMATE_TURN; no leidos) y si el dino avanza mientras gira. Los 110 ticks de SPIKEABOUT_UL parecen un "mirar a los lados" y no un giro de marcha: no se debe copiar esa duracion sin leer DINOTURN.
- Duracion que debe tener en el Spectrum la media vuelta con una diagonal para que el juego se sienta igual.

## 5. Abiertas
1. Leer DINOTURN, DINOLOOK_TURN y ANIMATE_TURN antes de fijar la duracion del giro.
2. Confirmar la ausencia de colision en TROG_PIT y en las pozas durante la aparicion.
3. Si la diagonal intermedia necesita 4 fases (media vuelta en cualquier punto de la calle) o 1 (giro de 90 grados en un nodo): ver tamanos_sprites.md, seccion 8.
