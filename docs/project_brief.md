# Trog ZX — Encargo del proyecto

## Objetivo

Crear un port lógico de Trog para ZX Spectrum 128K usando
ensamblador Z80 y sjasmplus.

El objetivo principal es hacer una conversión jugable a Trog LA5 en MAME.
Analizar el material original fuente incluido y traducir los comportamientos de forma fiel
teniendo en cuenta la sensación jugable y las limitaciones del Spectrum.

Si una rutina es compleja se permite tomar atajos, pero la sensación jugable ha de prevalecer.

## Material de partida

Repositorio original:
https://github.com/historicalsource/trog

El material se conservará en `original/` sin modificar.
No importar como hechos conclusiones de conversaciones anteriores.

## Entorno y colaboración

El desarrollo se realizará desde un proyecto de Perplexity.

Carlos:
- Crea los gráficos y recursos visuales.
- Mantiene los archivos locales.
- Ejecuta compilaciones y herramientas.
- Prueba el port y lo contrasta con MAME.
- Aprueba alcance y adaptaciones.

Asistente:
- Analiza el fuente y sus dependencias.
- Diseña e implementa el código Z80.
- Prepara herramientas y scripts.
- Documenta decisiones y resultados.
- Revisa errores y pruebas.

No depender de Claude Code, agentes externos ni servicios
adicionales de pago. GitHub es opcional.

## Decisiones

- Ensamblador: sjasmplus.
- Plataforma inicial: Spectrum 128K convencional.
- No requerir características de Spectrum Next.
- Scripts .bat para el flujo local en Windows.
- Confirmar herramientas y versiones antes de fijar comandos.
- ASM del port en `src/`; fuente original en `original/`.

## Hoja de ruta

1. Un nivel de LA5 jugable con gráficos provisionales.
2. Varias fases, IA completa de Trog y power-ups básicos.
3. Sonido AY, menús y puntuaciones.
4. Modo prototipo con la mano, condicionado a recursos e información.
5. Versión 48K reducida, si cabe y los recortes son aceptables.

Definir criterios de aceptación antes de implementar cada hito.
No modificar estas decisiones sin aprobación de Carlos.

## Análisis

- Inventariar los archivos disponibles.
- Leer documentación y sistema de construcción original.
- Identificar dependencias y puntos de entrada.
- Proponer un plan orientado al hito actual.
- Indicar archivos o recursos que falten.

Distinguir observaciones directas, interpretaciones y cuestiones
desconocidas. Referenciar archivos y etiquetas; añadir líneas
cuando su numeración sea fiable.

No asumir funciones por nombres de archivo ni considerar activa
una rutina únicamente porque exista.

## Implementación y pruebas

Trabajar con cambios pequeños y pruebas concretas.
Entregar archivos completos o parches con base claramente identificada.

No fijar arquitectura interna antes de justificarla.
No exigir un modelo Python completo del juego.

La validación inicial será visual y jugable. Añadir mediciones
o instrumentación cuando resulte útil.

No exigir igualdad frame a frame entre arcade y Spectrum.
Documentar las adaptaciones y diferencias aceptadas.

No afirmar que un programa ensambla, arranca o está validado
sin haber ejecutado la prueba correspondiente.

## Continuidad

- `docs/status.md`: estado, pruebas y siguiente tarea.
- `docs/decisions.md`: decisiones técnicas y motivos.
- `docs/analysis/`: análisis del fuente.
- `docs/testing.md`: procedimientos reproducibles.

Actualizar este encargo cuando cambien decisiones generales
o se cierre un hito.
