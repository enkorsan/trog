@echo off
rem build_game_r7_48.bat - igual que build_game_r7.bat con -DTARGET_48K (sin paginacion). Rutas con / para evitar el aviso backslash de sjasmplus.
if not exist build mkdir build
sjasmplus --nologo --msg=err -DTARGET_48K --lst=build/game_r7_48.lst --sym=build/game_r7_48.sym -Isrc src/game_r7.asm
if errorlevel 1 (
  echo ERROR: no ensambla. Copia el mensaje completo.
  exit /b 1
)
echo OK: build/game_r7_48.sna
