; ---- Variables ---------------------------------------------------------------
frame_counter:  DB 0
key_mask:       DB 0
req_dir:        DB $FF          ; direccion pedida (con retencion) o $FF
req_timer:      DB 0
dino_x:         DB 0
dino_y:         DB 0
dino_dir:       DB 0
dino_state:     DB 0            ; 0 andando, 1 muriendo (parpadea), 2 apagado esperando la reaparicion
respawn_mode:   DB 0            ; 0 = muerte por caida (reaparece en la interseccion de salida), 1 = en el sitio (interseccion mas cercana)
fall_timer:     DB 0
speed_acc:      DB 0
wait_timer:     DB 0            ; fotogramas quieto antes de andar
dino_inv:       DB 0            ; fotogramas de invencibilidad que quedan
cheat_god:      DB 0
cheat_prev:     DB 0
kill_prev:      DB 0
reseed_prev:    DB 0
edge_stop:      DB 0            ; 1 = parado ante el borde por invencibilidad (arcade: DINO_SAVE_FROM_DEATH)
edge_reg:       DB 0            ; 1 = el ultimo punto registrado es el auxiliar del borde (arcade: TCURMPOS)
dwait:          DB 0            ; fotogramas parado ante el borde (arcade: DWAIT)
saved_valid:    DB 0            ; 1 si hay un sprite pintado cuyo fondo esta guardado
save_first:     DB 0            ; primera linea guardada
save_col:       DB 0            ; byte de pantalla de la primera columna guardada
save_count:     DB 0            ; lineas guardadas
save_width:     DB 0            ; bytes por linea guardados (2 o 3)
spr_phase:      DB 0
spr_width:      DB 0
spr_i0:         DB 0            ; lineas recortadas por arriba
spr_top:        DB 0            ; linea superior del sprite (dino_y - SPR_OFF_Y)
save_buf:       DS 72           ; 24 lineas x 3 bytes
; huevos y reorientacion
egg_map:        DS NODE_COUNT   ; 1 = hay huevo en la interseccion (indice fila*9 + columna)
eggs_left:      DB 0            ; huevos que quedan en el mapa
eggs_got:       DB 0            ; huevos recogidos
rng_state:      DW EGG_SEED     ; xorshift de 16 bits
dinitdir:       DB D_DOWN       ; arcade: DINITDIR (direccion de partida; la caida la deja hacia dentro)
cur_c:          DB 0            ; columna y fila del nodo de reaparicion
cur_r:          DB 0
best_c:         DB 0            ; huevo elegido por find_closest
best_r:         DB 0
best_d:         DB 0
br_rating:      DB 0            ; best_by_rating
br_steps:       DB 0
br_dir:         DB 0
br_i:           DB 0
br_n:           DB 0
resp_eat:       DB 0            ; 1 = al reaparecer se recoge el huevo del nodo (caida: siempre; en el sitio: solo si murio a TURN_TOL px o menos del nodo)
house_exit:     DB 0            ; salida elegida para la casa (0..27, ver house.inc)
house_prev:     DB 0            ; tecla B de depuracion (flanco)
house_state:    DB 0            ; 0 sin casa, 1 apareciendo (oculta), 2 visible y parpadeando
house_timer:    DB 0
house_frame:    DB 0            ; 0 = HOME, 1 = REX
house_ext_w:    DB 0
house_ext_h:    DB 0
house_ext_ptr:  DW 0
house_ext_addr: DW 0
house_plq_addr: DW 0
win_phase:      DB 0            ; 0 = primera pasada de win_update, 1 = esperando
win_timer:      DB 0            ; fotogramas que quedan antes de reiniciar el nivel
house_plq_y:    DB 0
house_plq_x:    DB 0
