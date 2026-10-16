@echo off
rem build_game_r7.bat - compila el esqueleto del juego (128K). sjasmplus 1.24.0 (confirmado por Carlos). Rutas con / para evitar el aviso backslash de sjasmplus.
if not exist build mkdir build
sjasmplus --nologo --msg=err --lst=build/game_r7.lst --sym=build/game_r7.sym -Isrc src/game_r7.asm
if errorlevel 1 (
  echo ERROR: no ensambla. Copia el mensaje completo.
  exit /b 1
)
echo OK: build/game_r7.sna
