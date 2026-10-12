# Documentos de diseno, construccion y revision

Estado: lectura de READ.ME, ROMS.DOC, MAKEFILE, TROG.CMD, TROG.MAP (cabecera) y TROGIO.DOC. TROGPROJ.DOC, TROGIDEA.DOC y TROGWARE.DOC existen en el repositorio; su contenido citado aqui procede de una lectura anterior y esta pendiente de volver a comprobar.
Version: 12-oct-2026, revision 4.

## Observaciones directas
1. READ.ME: Final revision is LA5. Se usa OPUSMAKE y el ensamblador GSP 3.20. 4 bit Y-unit.
2. TROG.MAP (cabecera): enlazador GSP COFF 3.20, fecha 31-ago-1990, punto de entrada SETUP (ffe09800).
3. ROMS.DOC: PAC-TROG version PA3-PAC 8/15/90 con la lista de ROMs. Version distinta de LA5.
4. MAKEFILE: dos grupos. OBJS (programa principal) y KOBJS (se enlazan como TROGKURT.RLO). La lista completa esta en 00_inventario.md.
5. TROG.CMD enlaza los modulos de OBJS mas TROGKURT.RLO y deja TROGTIPS.OBJ comentado.
6. TROGIO.DOC: cada jugador tiene ARRIBA, ABAJO, IZQUIERDA, DERECHA, DRAW y START (hasta cuatro jugadores). El unico boton de accion se llama DRAW.
7. TROGDIP.ASM: solo ecuaciones de interruptores DIP.
8. TROGEQU.ASM: define las estructuras MOVEMAP, CMOVEMAP, AMOVEMAP y EMOVEMAP y el bloque de datos de jugador.

## Datos de una lectura anterior (por recomprobar)
- TROGPROJ.DOC (concepto, 1989): juego de cuatro jugadores en maquina dedicada, dos en kit. El jugador dibuja muros entre postes de una rejilla.
- TROGIDEA.DOC: lista de ideas, entre ellas premios de planta (Brain Food, Speed, Bad Shroom).
- TROGWARE.DOC: TMS34010 a 40 MHz, 256 x 395 x 8, 16 colores por objeto.
- Ficha de MAME: Trog (rev LA5 3/29/91), 410x256 a 54,71 Hz. MAME incluye tambien un prototipo (Trog Proto 4).

## Interpretaciones pendientes de comprobar
- El mapa de enlace (agosto de 1990) puede ser anterior a LA5 aunque el fuente sea el de LA5. La carpeta LA5/ solo contiene las ROMs TROGU89.PP y TROGU105.PP. Hay que confirmar que el fuente de la raiz produce esa compilacion.
- El nombre DRAW del boton y el concepto de dibujar muros podrian ser herencia del prototipo de la mano. En LA5, segun Carlos, el boton es el punetazo.

## Desconocido
- Funcion exacta de TROGWAND, TROGTEE y TROGDICT.
- Relacion entre PAC-TROG PA3 y LA5, y fecha exacta del fuente archivado.
