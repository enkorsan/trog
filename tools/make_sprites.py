#!/usr/bin/env python3
"""Genera src/sprite_dino.inc: flecha provisional del dino, 16 x 24 px, con mascara.
4 direcciones (arriba, abajo, izquierda, derecha) x 4 fases (0, 2, 4, 6 px) x 24 lineas x 6 bytes = 2304 bytes.
Por linea: mascara0, grafico0, mascara1, grafico1, mascara2, grafico2. Pantalla = (pantalla AND mascara) OR grafico.
Mascara 1 = conserva el fondo; mascara 0 = escribe el grafico (grafico 0 con mascara 0 = opaco, color de papel).
Uso: python tools/make_sprites.py > src/sprite_dino.inc
"""
import sys

W, H = 16, 24


def arrow_up():
    s = [[0] * W for _ in range(H)]
    for r in range(12):
        width = min(16, 2 + (14 * r) // 11)
        width -= width % 2
        x0 = 8 - width // 2
        for x in range(x0, x0 + width):
            s[r][x] = 1
    for r in range(12, H):
        for x in range(5, 11):
            s[r][x] = 1
    return s


def flip_v(s):
    return [row[:] for row in reversed(s)]


def arrow_right():
    s = [[0] * W for _ in range(H)]
    for c in range(6, 16):
        h = 2 + round(22 * (15 - c) / 9)
        h -= h % 2
        y0 = 12 - h // 2
        for y in range(y0, y0 + h):
            s[y][c] = 1
    for c in range(0, 6):
        for y in range(8, 16):
            s[y][c] = 1
    return s


def flip_h(s):
    return [list(reversed(row)) for row in s]


def to_gfx(shape):
    """Contorno de tinta + puntos interiores (para ver la mascara)."""
    g = [[0] * W for _ in range(H)]
    for y in range(H):
        for x in range(W):
            if not shape[y][x]:
                continue
            edge = False
            for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                yy, xx = y + dy, x + dx
                if not (0 <= yy < H and 0 <= xx < W) or not shape[yy][xx]:
                    edge = True
            g[y][x] = 1 if (edge or (x + y) % 4 == 0) else 0
    return g


def pack(shape, gfx, shift):
    out = []
    for y in range(H):
        m = 0xFFFFFF
        g = 0
        for x in range(W):
            if shape[y][x]:
                bit = 1 << (23 - (x + shift))
                m &= ~bit
                if gfx[y][x]:
                    g |= bit
        for k in range(3):
            out.append((m >> (16 - 8 * k)) & 0xFF)
            out.append((g >> (16 - 8 * k)) & 0xFF)
    return out


def main():
    up = arrow_up()
    right = arrow_right()
    shapes = [up, flip_v(up), flip_h(right), right]   # arriba, abajo, izquierda, derecha
    data = []
    for sh in shapes:
        gf = to_gfx(sh)
        for phase in range(4):
            data += pack(sh, gf, 2 * phase)
    assert len(data) == 4 * 4 * H * 6 == 2304
    print("; sprite_dino.inc - GENERADO por tools/make_sprites.py. No editar a mano.")
    print("; 4 direcciones (arriba, abajo, izquierda, derecha) x 4 fases (0, 2, 4, 6 px) x 144 bytes (24 lineas x 6).")
    print("; Por linea: mascara0, grafico0, mascara1, grafico1, mascara2, grafico2. Pantalla = (pantalla AND mascara) OR grafico.")
    print("sprite_dino:")
    for i in range(0, len(data), 16):
        print("    DB " + ",".join("$%02X" % b for b in data[i:i + 16]))
    print("sprite_dino_end:")


if __name__ == "__main__":
    main()
