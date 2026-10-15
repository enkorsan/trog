#!/usr/bin/env python3
"""Genera src/stones.inc: 3 variantes de piedra de 8x8 con mascara y el mapa de variantes (10 x 6).
Formato por variante: 8 lineas x (mascara, grafico) = 16 bytes. Mascara 1 = conserva el fondo; mascara 0 = escribe el grafico.
Carlos puede sustituir las piedras editando stones.inc a mano; este script solo da el punto de partida.
Uso: python tools/make_stones.py > src/stones.inc
"""
import random

V = [
    # A: redonda
    ["..####..", ".######.", "########", "########", "########", "########", ".######.", "..####.."],
    # B: de punta
    ["...##...", "..####..", ".######.", ".######.", "########", "########", "########", ".######."],
    # C: baja y ancha
    ["........", "........", "..####..", ".######.", "########", "########", "########", ".######."],
]


def build(rows, variant):
    shape = [[1 if ch == '#' else 0 for ch in r] for r in rows]
    out = []
    for y in range(8):
        m = 0
        g = 0
        for x in range(8):
            bit = 1 << (7 - x)
            if shape[y][x]:
                edge = False
                for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    yy, xx = y + dy, x + dx
                    if not (0 <= yy < 8 and 0 <= xx < 8) or not shape[yy][xx]:
                        edge = True
                ink = edge or ((x * 3 + y * 5 + variant) % 7 == 0)
                if ink:
                    g |= bit
            else:
                m |= bit
        out += [m, g]
    return out


def main():
    print("; stones.inc - piedras 8x8 con mascara y mapa de variantes (generado por tools/make_stones.py; se puede editar a mano).")
    print("; Formato por variante: 8 lineas x (mascara, grafico). Pantalla = (pantalla AND mascara) OR grafico.")
    print("STONE_VARIANTS  EQU %d" % len(V))
    print("stone_gfx:")
    for i, rows in enumerate(V):
        d = build(rows, i)
        print("    DB " + ",".join("$%02X" % b for b in d) + "    ; variante %d" % i)
    print("stone_gfx_end:")
    rnd = random.Random(24)
    grid = [[0] * 10 for _ in range(6)]
    for r in range(6):
        for c in range(10):
            while True:
                v = rnd.randrange(len(V))
                if c > 0 and grid[r][c - 1] == v:
                    continue
                if r > 0 and grid[r - 1][c] == v:
                    continue
                break
            grid[r][c] = v
    print("; variante de cada piedra: 6 filas x 10 columnas (fila r, columna c = indice r*10 + c)")
    print("stone_map:")
    for r in range(6):
        print("    DB " + ",".join(str(v) for v in grid[r]))
    print("stone_map_end:")


if __name__ == "__main__":
    main()
