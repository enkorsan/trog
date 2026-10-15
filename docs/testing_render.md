# Pruebas del renderer R1-R6

Version: 15-oct-2026, suplemento 1 a testing.md revision 12.
No sustituye los procedimientos ni regresiones del juego del documento anterior.

## Entorno
- sjasmplus 1.24.0.
- RVM, ZX Spectrum 128K original toastrack. Registrar version del emulador si se quiere reproducibilidad completa; aun no confirmada.
- Carlos compila mediante su BAT; ejemplo: build_bench_r2.bat src/bench_r6.
- El asistente no ha ensamblado ni ejecutado emulacion; los resultados reales son los informados por Carlos.

## Estado de validacion
| Banco | Ensambla / ejecuta | Observaciones |
|---|---|---|
| R1 | confirmado por Carlos | suciedad al solapar; referencia |
| R2 | confirmado tras corregir directiva sin indentar | ventanas aisladas; solapes incorrectos |
| R3 | confirmado por Carlos | prioridades correctas; picos de ruta lenta |
| R4 | confirmado tras preservar BC en init_ents | mejoras de coste, acciones futuras aun ausentes |
| R5 | confirmado tras corregir DJNZ largo | recorte/bancos/HUD; borde derecho corregido posteriormente |
| R6 | confirmado tras corregir DJNZ largo | Trogs distintos, acciones/caidas; borde derecho corregido posteriormente |

## Correcciones que debe contener la copia vigente
1. init_ents: PUSH BC / PUSH HL / CALL ent_set_qpos / POP HL / POP BC.
2. Bucle largo de init_ents en R5/R6: DEC B / JP NZ,.l en lugar de DJNZ .l.
3. ent_blit: tres DEC DE en lugar de tres DEC E tras los tres LDI y JP PO,.done.

## Instrumentacion actual: limites
- Ventanas de 64 iteraciones medidas; HUD escalonado y exclusion mediante skip_acc.
- MEDIA = promedio de LIBRE de ultimas 16 ventanas; MINA = minimo desde el arranque; SOLX = maximo SOLP por ventana.
- LIBRE/MEDIA en unidades aproximadas de 70 T-states, MIN/MINA de 35.
- PERD excluye actualmente perdidas en iteraciones con skip_acc: NO es un total absoluto.
- Espera muy corta puede producir MIN=0 por cuantizacion sin detectarse interrupcion adicional.
- No usar PERD=0 como prueba definitiva de ausencia de cualquier perdida.
- VENT refleja iteraciones medidas, no una duracion exacta en segundos; con HUD/pérdidas cambia el tiempo de pared.

## Siguiente prueba recomendada
Primero corregir instrumentacion:
- Separar MEDIA/MINA sin HUD de contador absoluto de interrupciones adicionales.
- Registrar muestras excluidas y configuracion completa (N,E,S,P,G,A,F,B, velocidad y semilla).
- Medir misma duracion o mismo numero de muestras y anotar cual se usa.
- Conservar extremos para picos y promedio para carga tipica; no sustituir el peor caso por una media.
Despues repetir como minimo:
| Banco | N | E | S | P | G | A | F | B | Objetivo |
|---|---|---|---|---|---|---|---|---|---|
| R5 | 5 | 1 | 1 | 0 | 1 | - | - | 2 | borde derecho y recorte |
| R6 | 5 | 1 | 1 | 0 | 0 | 1 | 1 | 2 | acciones, caida y reaparicion con solapes |
| R6 | 8 | 2 | 1 | 1 | 0 | 1 | 1 | 2 | estres, no requisito de rendimiento cerrado |

## Regresiones visuales del renderer
- Mascaras correctas sobre piedras, agua y vecinos.
- Prioridad estable: mayor Y y dino con igual Y.
- No rectangulos ni restos al cruzarse.
- Derecha: x cercano a 236/240, ventanas en columna 29; comprobar varias filas de pantalla.
- Arriba: y=28 y umbral de caida y=26, sin tocar HUD.
- Acciones en el sitio redibujan aunque X/Y no cambien.
- Al desaparecer, restaurar fondo y vecinos; no dejar imagen antigua.
- Al reaparecer lejos, solo ventana nueva; no limpiar un rectangulo enorme entre ambas posiciones.
- Verificar solapes con entidades quietas.

## Al integrar en el juego
Ademas de lo anterior: teclado, giros, caidas/paradas/invencibilidad, recogida de huevos y contador, casa/letrero, llegada/victoria/reinicio. Las animaciones del bench no validan las reglas arcade ni sustituyen pruebas de juego.

Tablas completas disponibles en analysis/render_progress.md.
