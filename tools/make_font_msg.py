#!/usr/bin/env python3
# tools/make_font_msg.py - genera font_msg.inc a partir de la fuente de Carlos (font.asm).
# Fuente: etiqueta new_font, 96 glifos de 8 bytes (ASCII 32..127), "defb 0x..,..." . Un glifo de ocho 0xFF significa "sin dibujar".
# Salida: font_msg.inc con ASCII 32..LAST (por defecto 90 = 'Z'): glifo = font_msg + (ASCII - 32) * 8.
# Los glifos sin dibujar se sustituyen por un PLACEHOLDER de este script (si existe) o por un glifo vacio, y se listan en la cabecera y por pantalla.
# Uso: python tools/make_font_msg.py [font.asm] [font_msg.inc] [ultimo_ascii]
import re, sys

PLACEHOLDER = {   # 5x7 en los bits 6..2; solo para los glifos que faltan en la fuente de Carlos
'"': [" # # "," # # ","     ","     ","     ","     ","     "],
'6': ["  ## "," #   ","#    ","#### ","#   #","#   #"," ### "],
'7': ["#####","    #","   # ","  #  "," #   "," #   "," #   "],
'8': [" ### ","#   #","#   #"," ### ","#   #","#   #"," ### "],
'<': ["   # ","  #  "," #   ","#    "," #   ","  #  ","   # "],
'=': ["     ","     ","#####","     ","#####","     ","     "],
'>': [" #   ","  #  ","   # ","    #","   # ","  #  "," #   "],
'@': [" ### ","#   #","# ###","# # #","# ###","#    "," ### "],
}

def art_to_bytes(rows):
    out = [sum((1 << (4 - i)) for i, c in enumerate(r) if c == '#') << 2 for r in rows]
    return out + [0]

def main():
    src = sys.argv[1] if len(sys.argv) > 1 else "font.asm"
    dst = sys.argv[2] if len(sys.argv) > 2 else "font_msg.inc"
    last = int(sys.argv[3]) if len(sys.argv) > 3 else 90
    glyphs = []
    for line in open(src, encoding="latin-1"):
        m = re.match(r"\s*(?:defb|db)\s+(.*)", line, re.I)
        if not m:
            continue
        vals = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", m.group(1))]
        if len(vals) != 8:
            sys.exit("linea con %d bytes (se esperaban 8): %s" % (len(vals), line.strip()))
        glyphs.append(vals)
    if len(glyphs) < last - 31:
        sys.exit("la fuente tiene %d glifos y se piden hasta ASCII %d" % (len(glyphs), last))
    subst, blank = [], []
    body = []
    for code in range(32, last + 1):
        g = glyphs[code - 32]
        ch = chr(code)
        if all(b == 0xFF for b in g):
            if ch in PLACEHOLDER:
                g = art_to_bytes(PLACEHOLDER[ch]); subst.append(ch)
            else:
                g = [0] * 8; blank.append(ch)
        body.append("    DB " + ",".join("$%02X" % b for b in g) + "    ; '" + ch + "'")
    n = last - 31
    head = [
        "; font_msg.inc - GENERADO por tools/make_font_msg.py a partir de %s. No editar a mano." % src,
        "; %d glifos de 8 bytes (ASCII 32..%d): glifo = font_msg + (ASCII - 32) * 8. Fuente unica del juego (mensajes, HUD, puntuacion)." % (n, last),
        "; Glifos sin dibujar en la fuente, sustituidos por placeholder: " + (" ".join(subst) if subst else "ninguno"),
        "; Glifos sin dibujar y vacios: " + (" ".join(blank) if blank else "ninguno"),
        "font_msg:",
    ]
    tail = ["font_msg_end:", "    ASSERT (font_msg_end - font_msg) == %d*8" % n, ""]
    with open(dst, "w", newline="\r\n") as f:
        f.write("\n".join(head + body + tail))
    print("escrito %s (%d glifos)" % (dst, n))
    print("placeholder:", " ".join(subst) or "ninguno")
    print("vacios:", " ".join(blank) or "ninguno")

main()
