# Documentos de diseno, construccion y revision

Estado: lectura de READ.ME, ROMS.DOC, MAKEFILE, TROG.CMD, TROG.MAP (cabecera), TROGIO.DOC, TROGPROJ.DOC, TROGIDEA.DOC y TROGWARE.DOC.
Version: 8-oct-2026, revision 3. Ubicacion prevista en el proyecto: docs/analysis/

## Revision

Observaciones directas:
1. READ.ME: "Final revion is LA5. To make use OPUSMAKE and GSP assmebler version 3.20. 4 bit Y-unit."
2. TROG.MAP (cabecera): enlazador GSP COFF 3.20, fecha Fri Aug 31 10:07:11 1990, punto de entrada SETUP (ffe09800).
3. ROMS.DOC habla de "PAC-TROG version PA3-PAC 8/15/90" con la lista de ROMs. Es otra version distinta de LA5.
4. La ficha de MAME del juego indica "Trog (rev LA5 3/29/91)", pantalla de 410x256 a 54,71 Hz. La fecha de LA5 (marzo de 1991) es posterior a la
   del mapa de enlace del archivo (agosto de 1990). Segun la misma ficha, MAME incluye tambien un prototipo ("Trog Proto 4") con el concepto original.

Interpretacion pendiente: el mapa de enlace puede ser anterior a LA5 aunque el fuente sea el de LA5. Confirmar que el fuente archivado corresponde
a la compilacion LA5 que Carlos ve en MAME.

## Sistema de construccion

5. MAKEFILE define dos grupos. OBJS (programa principal): TROGRAM, TROG, TROGUTIL, TROGPALL, TROGBGND, IMGPAL, IMGTBL, BGNDTBL, BGNDPAL,
   TROGPROC, TROGDISP, TROGSND, TROGTV, TROGPOST, TROGBAR, TROGDINO, TROGMISC, TROGTEXT, TROGCOL, TROGMAPS, TROGDDAT, TROGTROG, TROGTDAT,
   TROGENEM, TROGEGGS, TROGWHL, TROGDRON, TROGSPRG, TROGWAVE, TROGTREX, TROGDIN2.
   KOBJS (se enlazan como TROGKURT.RLO): TROGMENU, TROGHSTD, TROGJUNK, SYSFONT, TROGDUTL, TROGDTST, TROGCOIN, TROGSCOR, TROGATT, TROGADJ, TROGPRNT,
   TROGCAVE, TROGFADE, TROGWORD, TROGCAT, TROGHAUS.
6. TROG.CMD enlaza los mismos modulos de OBJS mas TROGKURT.RLO, deja TROGTIPS.OBJ comentado, y reparte la memoria en SCRATCH (RAM),
   ROM (programa) e IROM (imagenes).
7. Modulos que no aparecen ni en OBJS ni en KOBJS ni en las dependencias del MAKEFILE: TROGWAND, TROGTEE, TROGDIP y TROGDICT.
   Pendiente de comprobar si algun otro modulo los incluye con .INCLUDE.

## Diseno (TROGPROJ.DOC, TROGIO.DOC, TROGIDEA.DOC, TROGWARE.DOC)

8. TROGPROJ.DOC (concepto, 1989): "four player dedicated, two player kit game". El jugador tiene el poder de dibujar muros entre postes
   de una rejilla para evitar que su personaje salga de ella. Cuando el primer dinosaurio sale, la rejilla se completa.
   Tercera rejilla y una de cada cuatro despues son de bonus.
9. TROGIO.DOC: cada jugador tiene ARRIBA, ABAJO, IZQUIERDA, DERECHA, DRAW y START (hasta cuatro jugadores). El unico boton de accion se llama DRAW.
10. TROGIDEA.DOC (lista de ideas; las marcadas con X estan hechas): premios de planta "Brain Food, Speed, Bad Shroom (slow motion dude)".
11. TROGWARE.DOC: TMS34010 a 40 MHz, 256 x 395 x 8, 16 colores por objeto.

## Interpretaciones

- El nombre DRAW del boton y el concepto de dibujar muros son herencia del prototipo de la mano (Carlos, 8-oct-2026). En LA5 el boton es el
  punetazo. Ver analisis_prototipo_mano.md.
- La version de dos jugadores fue un formato "kit" del arcade.

## Desconocido

- Funcion exacta de TROGWAND, TROGTEE, TROGDIP y TROGDICT.
- Relacion entre PAC-TROG PA3 y LA5, y fecha exacta del fuente archivado.
