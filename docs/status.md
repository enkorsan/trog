# Estado del proyecto Trog ZX

Actualizado: 12-oct-2026

## Estado
- Ningun motor Z80 aceptado ni validado.
- Fuente original en la raiz del repositorio, sin modificar.
- Acceso al fuente: conector GitHub sobre este fork (enkorsan/trog).

## Hecho
- Inventario del repositorio: docs/analysis/00_inventario.md
- Analisis de documentos de diseno y construccion: docs/analysis/analisis_documentos_diseno.md
- Notas sobre el prototipo de la mano: docs/analysis/analisis_prototipo_mano.md
- Lectura de TROGDINO.ASM (7 partes), TROGMAPS.ASM, TROGEQU.ASM, TROGPROC.ASM, TROGCOL.ASM, TROGPOST.ASM (parcial).

## Pendiente de comprobar
- Que el fuente de la raiz genere las ROMs de LA5 (carpeta LA5/ solo contiene TROGU89.PP y TROGU105.PP).
- Fecha de TROG.MAP (31-ago-1990) frente a LA5 3/29/91 en MAME.

## Siguiente tarea
Analisis del Hito 1: construccion del mapa de movimiento (TROGPOST.ASM CONSMOVE, TROGEQU.ASM estructuras MOVEMAP/CMOVEMAP/AMOVEMAP/EMOVEMAP, TROGMAPS.ASM POSTMAP1) y bucle del dino (TROGDINO.ASM DINOWALK, DINO_STICK).
