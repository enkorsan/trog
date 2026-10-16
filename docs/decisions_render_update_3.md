# Suplemento 3 de decisiones: contrato del renderer (render.inc)

Version: 16-oct-2026, revision 1 (13:5x). PROPUESTA: las decisiones de Carlos del 16-oct (tarde) estan marcadas "Decidido por Carlos"; el resto sigue pendiente de su aprobacion. Suplemento 3 a decisions.md revision 15; se suma a decisions_render_update.md y decisions_render_update_2.md (rev. 2). No sustituye ninguno.
Estado del codigo asociado: PROPUESTO. El asistente no ha ejecutado sjasmplus ni emulador. Nada de lo de aqui esta ensamblado ni probado.
Fuentes leidas: docs/status.md (rev. 20), decisions_render_update_2.md (rev. 2), analysis/integracion_inventario.md, analysis/render_mediciones_r6_2.md, testing_render.md (rev. 3), src_z80/bench_r6.asm (R6.3, SHA ab2d42e...; la lectura por el conector se corta dentro de draw_clipped), la copia local adjunta bench_r6_integracion.asm (36.123 bytes frente a los 34.233 del repositorio; no se ha comparado linea a linea), config.inc, video.inc, main.asm, dino.inc, input_hud.inc y la documentacion de sjasmplus 1.24.0.

## 1. Entidades
Observacion directa (bench_r6.asm): 32 bytes por entidad, offsets E_X=0 ... E_STATE=31, ENT_SIZE=32. Se mantienen: el renderer los usa con (IX+d).
Propiedad de los campos (propuesta):
- Escribe la logica: E_X, E_Y, E_DIR, E_IMG, E_STATE, E_TMR, E_ACC, E_SPD, E_PAUSE, E_COL, E_ROW, E_LEFT y E_DIRTY.
- Escribe solo el renderer: E_VALID, E_OX, E_OY, E_PEND, E_SC0, E_SL0, E_SH, E_RC, E_RL, E_QW, E_QC, E_QT, E_DP, E_CLIP.
- Fijados al iniciar: E_SBUF (buffer de ventana propio), E_IDX, E_TIE (desempate; dino = 255).
Entidad 0 = dino (E_IDX = 0, sprite_dino, gana a igual Y). Las demas = Trogs (sprite_trog).
Protocolo de cambio: tras cambiar E_X, E_Y, E_IMG o E_STATE, la logica pone E_DIRTY = 1 y llama a ent_set_qpos con IX = entidad (como update_all del banco). Sin cambios, E_DIRTY = 0.
E_STATE: 0 andando, 1 animacion de caida, 2 oculta. El renderer solo distingue "2" (sin sprite, E_QC = $80, no cuenta como vecino). Otros estados de juego (victoria, etc.) van en variables del juego.
Numero de entidades: constante de ensamblado ENT_COUNT (1..MAX_ENTS = 8), como BENCH_N. Una entidad ausente queda en E_STATE = 2. Motivo: los bucles de update_all, prepare_all, blit_all y ent_hits_other usan la constante; no se introduce una cuenta dinamica sin medirla.

## 2. Invariante de imagen ideal
Toda ventana que se vuelca contiene la imagen ideal de su zona para el estado de entidades y de fondo en el momento de prepare_all. Consecuencias:
- Orden por fotograma: blit_all (de lo preparado en la iteracion anterior), logica (cambia entidades y fondo limpio), prepare_all. Las ventanas de un mismo fotograma pueden volcarse en cualquier orden; las de fotogramas distintos, en orden cronologico.
- Todo cambio de fondo ocurre en la fase de logica, antes de prepare_all.
- El HUD (lineas 0-23) no pasa por el renderer: CLIP_TOP = 24 (config.inc).

## 3. Decorado: suelo con dibujos y huevos
Decidido por Carlos (16-oct, tarde): el suelo del campo debe tener dibujos, no ser plano; los huevos se pintan siempre POR DEBAJO del dino, sin respetar Y (el arcade parece hacerlo asi). Cierra la pregunta abierta de si un huevo puede tapar al dino por Y: no puede. El decorado queda siempre debajo de todas las entidades; no hay capa "encima" para huevos. El letrero de la casa sigue sin decidir.
Consecuencias para el diseno:
- El fondo limpio (BG_BASE) es un mapa de bits arbitrario de 192 x 32 bytes: el renderer solo lo copia, asi que un suelo con dibujos no cuesta nada en tiempo de ejecucion. Hoy draw_field deja el suelo vacio (placeholder plano); el suelo real lo pintara Carlos y debe estar en la pantalla de pixeles antes de build_bg.
- Retirar un huevo restaura el patron del suelo guardado al empezar el nivel (2 bytes x 16 lineas = 32 bytes por nodo; 43 nodos = 1.376 bytes). Esto es lo que ya decidia decisions_render_update_2.md (punto 2), y ahora queda explicito. Alternativa a evaluar cuando exista el arte: si el suelo es periodico y alineado a la rejilla de 24 px, se regeneraria desde la baldosa y no haria falta guardar nada.
- Atributos: el campo es hoy monocromo (ATTR_FIELD). Si el suelo lleva color por celda de 8x8 habra choque de atributos con los sprites; el renderer no mueve atributos y no se ha analizado. A tratar cuando Carlos defina el estilo grafico.
- El rectangulo de decorado (seccion 4) sigue haciendo falta: la tesela se compone con el fondo y el dino (u otras entidades) pintados encima.

## 4. Rectangulo de decorado compuesto por prioridad
Para huevos retirados, casa/letrero y mensajes (pasos 2 y 3 del plan; NO se implementa en el esqueleto del paso 1).
- Escribir en el fondo limpio (BG_BASE, banco 0) y pedir el volcado con decor_rect: B = linea superior, C = columna (byte), D = ancho en bytes, E = alto en lineas. Se recorta a CLIP_TOP..CLIP_BOTTOM y se parte en teselas de como maximo STAT_MAX = 128 bytes.
- Cola de STATQ_MAX = 8 teselas; se procesa 1 por fotograma (STAT_PER_FRAME = 1, ajustable). Desbordar la cola es un error de diseno: en depuracion, borde rojo y parada.
- Composicion de una tesela: copiar el fondo limpio a STAT_BUF y pintar encima las entidades cuyo sprite la toca, en orden de prioridad (misma ruta que ent_pre/nb_range). Despues se vuelca como una ventana mas.
- Confirmado con la copia local: ent_prep1 copia 3 bytes por linea (tres LDI) y ent_blit vuelca 3 bytes por linea (tres LDI y tres DEC DE), asi que la tesela de ancho variable necesita copia y volcado propios.

## 5. Sincronizacion por contador
Confirmado en decisions_render_update_2.md (punto 1). Contrato de frame_wait:
- Sin HALT. frame_counter lo incrementa la ISR IM2 (tabla en IM2_TABLE = $B800, vector $B9B9).
- Se lee frame_counter una sola vez por iteracion (C). Resta frame_start: 0 -> esperar a que cambie y fijar frame_start; n >= 1 -> iteracion tardia: frame_start = C, sin esperar, LT += 1 y deficit (PC) += n-1.
- Sin recuperar pasos perdidos: un paso de logica por iteracion; los temporizadores cuentan pasos.
- El contador es de 8 bits: valido mientras una iteracion dure menos de 256 fotogramas.

## 6. Paginacion, 48K y sjasmplus
### 6.1 Compatibilidad con -DTARGET_48K (comprobado en la documentacion de sjasmplus 1.24.0)
- La linea de comandos admite -D<NAME>[=<value>] (y --define desde 1.20.3). DEFINE sin reemplazo vale para IFDEF/IFNDEF.
- IFDEF/IFNDEF solo ven identificadores DEFINE (y los de -D), NO las etiquetas ni EQU. Por eso TARGET_48K es un define y el resto (MACHINE_128, SPR_BANK, ENT_COUNT) son EQU usadas en expresiones de IF. Correccion hecha en render.inc: la guarda del fichero usa DEFINE, y SPR_BANK ya no se condiciona con IFNDEF.
- IF dentro de MACRO esta corregido en el changelog de sjasmplus; el esqueleto lo usa en PAGE_BG y PAGE_SPR. Sigue sin ensamblarse: Carlos debe confirmarlo con su sjasmplus 1.24.0.
- Un -D en build.bat es una opcion mas; DEVICE ZXSPECTRUM48 / ZXSPECTRUM128 se elige con IFDEF en el fichero principal.
### 6.2 Macros
- PAGE_BG y PAGE_SPR escriben en $7FFD (valor $10 + banco) solo si MACHINE_128; PAGE_SPR ademas solo si SPR_BANK != 2. En 48K el fondo limpio es RAM plana en $C000.
- Regla: el llamador trata A y BC como alterados en todas las compilaciones.
- Ninguna escritura directa a $7FFD fuera de las macros. Nada residente en $C000-$FFFF en 128K (pila en $7FFF, ISR y variables por debajo de $C000).
- Si la ISR llega a paginar (AY, hito 3), habra que guardar el valor de $7FFD en una variable. No decidido.
### 6.3 Que cuesta mantener el 48K
Respuesta a Carlos: ahora mismo casi nada. Lo que cuesta es disciplina, no codigo:
- Las macros no generan bytes en 48K y las direcciones son EQU: coste cero en tamano y en tiempo.
- El fondo limpio de $C000 es la misma direccion en ambas maquinas, y en las dos esta fuera de la memoria contendida (en 128K, banco 0; en 48K, RAM alta).
- Restricciones que hay que mantener: (a) la base jugable no puede depender de los bancos 1, 3, 4, 6 y 7; (b) todo el renderer, el fondo y los sprites activos caben en $5B00-$FFFF (unos 42,5 KB; el conjunto residente del banco ocupa unos 9,5 KB y el fondo 6 KB); (c) el AY va detras de HAS_AY (= MACHINE_128).
- Lo que SI lo encarece, y vendra mas adelante: las diagonales del giro suman 2.304 bytes por juego de sprites (seccion 8), y los datos de nivel deben poder cargarse en un area fija que se sobrescribe en 48K (seccion 7).
### 6.4 Mejora posible del mapa de memoria (propuesta M1, no adoptada)
$5B00-$7EFF (9.216 bytes) esta libre en ambas maquinas: hoy solo se usa $7FFF para la pila. Es memoria contendida, asi que solo serviria para datos "frios" que se leen poco: tablas de nivel, texto, piedras y los 1.376 bytes del suelo guardado bajo los huevos. Los sprites y el fondo limpio no deben ir alli. Reserva a respetar: si en 48K se cargan niveles con la rutina de la ROM, hay que dejar intactas las variables del sistema ($5C00 en adelante) o decidir otra via de carga; sin verificar. No se cambia render.inc hasta que Carlos lo apruebe.

## 7. Estrategia de carga (idea de Carlos, 16-oct)
- 128K: cargar todo si cabe (niveles en bancos, sprites y fondo; musica AY solo aqui).
- 48K: base jugable residente y niveles en cargas posteriores; sin AY.
- Esto no cambia la hoja de ruta (el hito 5 sigue siendo la version 48K reducida). Para no cerrar la puerta: los datos de nivel se tratan como un bloque con direccion fija (LEVEL_AREA, por definir) que en 128K se paginaria y en 48K se sobrescribe con cada carga. Formato de mapas y tamano: sin decidir.

## 8. Giro con imagenes intermedias (decidido por Carlos, 16-oct)
- Al girar hay imagenes intermedias en diagonal (placeholder de momento). Media vuelta de derecha a izquierda: derecha, derecha-abajo, abajo, izquierda-abajo, izquierda. En el arcade el sentido es al azar (por abajo o por arriba); lo mismo en todas las direcciones.
- Lectura del asistente (por confirmar): giro de 90 grados = 1 diagonal; media vuelta = 3 intermedias (diagonal, perpendicular, diagonal), con sentido al azar; la media vuelta de arriba a abajo pasa por la izquierda o la derecha.
- Contrato: E_IMG admite 0..7 (IMG_UP, IMG_DOWN, IMG_LEFT, IMG_RIGHT, IMG_UL, IMG_UR, IMG_DL, IMG_DR). El calculo de ent_set_qpos (imagen x 4 + fase, por 144 bytes) no cambia. IMG_COUNT pasa de 4 a 8 cuando existan los graficos; cada juego de sprites crece de 2.304 a 4.608 bytes (dino y Trog: +4.608 bytes en total).
- El giro es una secuencia de la logica del dino (variables propias: paso, destino, contador); el renderer solo ve cambios de E_IMG con E_DIRTY, como las acciones en el sitio del R6.
- NO SE SABE todavia: cuantos fotogramas dura cada imagen intermedia, si el dino avanza durante el giro y como se aplica a los Trogs. Hay que leer el fuente arcade (rutinas de giro de TROGDINO.ASM) antes de implementarlo. Mientras tanto el esqueleto gira directamente.

## 9. Direcciones por EQU (aprobado por Carlos el 16-oct: "el mapa de memoria lo veo genial")
| Simbolo | Direccion | Bytes | Nota |
|---|---|---:|---|
| CODE_BASE | $8000 | - | ORG |
| STACK_TOP | $7FFF | - | como R6.3 |
| SCR_BUFS | $B100 | 8 x 128 | alineado a 128: ent_prep2 y ent_blit incrementan solo E |
| STAT_BUF | $B500 | 128 | alineado a 128, para el paso 2 |
| ENTS | $B580 | 8 x 32 | |
| QPOS | $B680 | 16 | |
| ZLIST | $B690 | 16 | |
| reservado | $B6A0-$B7FF | 352 | |
| IM2_TABLE | $B800 | 257 | |
| BG_BASE | $C000 | 6144 | banco 0 en 128K |
El ensamblado comprueba ASSERT (fin de codigo y datos <= SCR_BUFS). No se declara margen de memoria cerrado: se mide con el listado de la primera compilacion del esqueleto. Las variables del juego van junto al codigo.

## 10. Velocidad con modulo 64
Decision ya tomada (decisions_render_update_2.md, punto 4); forma en el contrato:
- SPEED_MOD = 64, SPEED_BASE = 26. El acumulador suma E_SPD y, al llegar a SPEED_MOD, resta SPEED_MOD y da un paso de 2 px (en el banco: CP 16 / SUB 16).
- Condicion: E_SPD < SPEED_MOD (maximo un paso por fotograma). Con la base, 2 x 26 = 52 < 64 (ASSERT en render.inc). El acumulador nunca pasa de 126.
- Fase inicial de la entidad i de N: floor(64 * i / N) (stagger_phase). N=5 -> 0, 12, 25, 38, 51; N=8 -> 0, 8, ... 56 (calculado con Python).
- 26/64 x 2 px x 50 fps = 40,6 px/s (teoria del arcade escalada 0,75: 41,25 px/s). Falta la lectura real en el Spectrum, que debe hacer Carlos (protocolo en testing_render.md).

## 11. Diferencias deliberadas frente al juego previo
- El giro cambia la imagen del dino de inmediato (E_DIRTY); antes se repintaba en el siguiente paso. Se sustituira por la secuencia de la seccion 8.
- La caida es la animacion provisional del banco (FALL_ANIM = 24, E_IMG alternando cada 4 fotogramas; FALL_DEAD = 55), no el parpadeo de FALL_FRAMES (50) de config.inc, que queda sin uso.
- En el esqueleto del paso 1 no hay invencibilidad, parada ante el borde, casa ni huevos: el dino cae siempre al llegar al borde. Es una simplificacion de prueba, no una regla.

## 12. Abiertas
1. Extraer render_core.inc de la copia local completa del R6.3 (siguiente tarea).
2. Fuente arcade del giro: duracion, avance durante el giro, Trogs (seccion 8).
3. Letrero de la casa: ¿puede tapar al dino?
4. Mejora M1 (datos frios en $5B00-$7EFF) y carga de niveles en 48K.
5. Cuenta de entidades dinamica frente a ENT_COUNT constante, a medir cuando haya Trogs.
6. Pila fuera de la zona contendida; pantalla sombra; ordenacion con claves de un byte (sin cambios).
7. Arte del suelo (dibujos y, en su caso, color) y si es periodico (seccion 3).
