#!/usr/bin/env python3
# tools/make_trog_placeholder.py - genera sprite_trog.inc: Trog PROVISIONAL de 16x24 con mascara, 4 direcciones x 4 fases de desplazamiento.
# Mismo formato que el dino provisional: indice = direccion * 4 + fase; cada fotograma son 24 lineas x 3 pares (mascara, grafico) = 144 bytes;
# direcciones en el orden del juego (0 arriba, 1 abajo, 2 izquierda, 3 derecha); fase p = desplazamiento de 2*p pixeles a la derecha.
# Mascara 1 = transparente (conserva el fondo); el grafico solo tiene bits donde la mascara es 0. Los graficos definitivos son de Carlos.
# Uso: python tools/make_trog_placeholder.py [sprite_trog.inc] [--preview]
import sys

W, H = 16, 24

def blank():
    return [['.'] * W for _ in range(H)]

def rect(c, x0, y0, x1, y1, ch):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if 0 <= x < W and 0 <= y < H:
                c[y][x] = ch

def ellipse(c, cx, cy, rx, ry, ch):
    for y in range(H):
        for x in range(W):
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0:
                c[y][x] = ch

def outline(c):
    out = [row[:] for row in c]
    for y in range(H):
        for x in range(W):
            if c[y][x] == '.':
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < W and 0 <= ny < H and c[ny][nx] != '.':
                        out[y][x] = 'I'
                        break
    return out

def checker(c, x0, y0, x1, y1):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if c[y][x] == 'P' and (x + y) % 2 == 0:
                c[y][x] = 'I'

def body_front(c):
    rect(c, 3, 10, 12, 17, 'P')        # torso
    rect(c, 1, 11, 2, 16, 'P')         # brazo izquierdo
    rect(c, 13, 11, 14, 16, 'P')       # brazo derecho
    rect(c, 4, 18, 6, 22, 'P')         # pierna izquierda
    rect(c, 9, 18, 11, 22, 'P')        # pierna derecha

def make(d):
    c = blank()
    if d in (0, 1):
        body_front(c)
        ellipse(c, 8, 5, 4.2, 4.2, 'P')
        c = outline(c)
        checker(c, 3, 10, 12, 17)      # tunica de piel con trama
        if d == 1:                      # frente: pelo arriba, ojos y boca; maza a la derecha
            rect(c, 5, 2, 11, 3, 'I')
            for x, y in ((6, 5), (10, 5), (7, 8), (8, 8), (9, 8)):
                c[y][x] = 'I'
            rect(c, 13, 3, 14, 15, 'I')
            rect(c, 12, 1, 15, 4, 'I')
        else:                           # espalda: pelo en toda la cabeza; maza a la izquierda
            rect(c, 4, 2, 12, 8, 'I')
            rect(c, 1, 3, 2, 15, 'I')
            rect(c, 0, 1, 3, 4, 'I')
    else:
        rect(c, 5, 10, 11, 17, 'P')    # torso de perfil
        rect(c, 5, 18, 7, 22, 'P')     # pierna trasera
        rect(c, 9, 18, 11, 22, 'P')    # pierna delantera
        ellipse(c, 8, 5, 3.8, 4.2, 'P')
        rect(c, 12, 11, 14, 14, 'P')   # brazo adelantado
        c = outline(c)
        checker(c, 5, 10, 11, 17)
        rect(c, 5, 2, 10, 3, 'I')      # pelo
        c[5][11] = 'I'                  # ojo
        rect(c, 12, 5, 13, 6, 'I')     # nariz
        rect(c, 13, 1, 14, 14, 'I')    # maza delante
        rect(c, 12, 0, 15, 3, 'I')
        if d == 2:                      # izquierda = espejo de la derecha
            c = [row[::-1] for row in c]
    return c

def frame_bytes(canvas, phase):
    out = []
    for y in range(H):
        mask = 0xFFFFFF
        gfx = 0
        for x in range(W):
            ch = canvas[y][x]
            if ch == '.':
                continue
            bit = 23 - (x + 2 * phase)
            mask &= ~(1 << bit)
            if ch == 'I':
                gfx |= (1 << bit)
        for k in (16, 8, 0):
            out.append((mask >> k) & 255)
            out.append((gfx >> k) & 255)
    return out

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dest = args[0] if args else 'sprite_trog.inc'
    canv = [make(d) for d in range(4)]
    if '--preview' in sys.argv:
        for d in range(4):
            print('direccion', d)
            for row in canv[d]:
                print(''.join(row))
    lines = [
        '; sprite_trog.inc - GENERADO por tools/make_trog_placeholder.py. No editar a mano. PLACEHOLDER (provisional).',
        '; Trog de 16x24 con mascara: indice = direccion * 4 + fase (0 arriba, 1 abajo, 2 izquierda, 3 derecha; fase = 2 * p pixeles a la derecha);',
        '; 24 lineas x 3 pares (mascara, grafico) = 144 bytes por fotograma. Mascara 1 = transparente.',
        'sprite_trog:',
    ]
    for d in range(4):
        for p in range(4):
            data = frame_bytes(canv[d], p)
            lines.append('    ; direccion %d fase %d' % (d, p))
            for i in range(0, len(data), 12):
                lines.append('    DB ' + ','.join('$%02X' % b for b in data[i:i + 12]))
    lines += ['sprite_trog_end:', '    ASSERT (sprite_trog_end - sprite_trog) == 16*144', '']
    with open(dest, 'w', newline='\r\n') as f:
        f.write('\n'.join(lines))
    print('escrito', dest)

main()
