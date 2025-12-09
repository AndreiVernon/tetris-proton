import argparse
from PIL import Image

# Mapping: pixel_value → text_token
# Pixel values must be 3-tuples of (R, G, B)
COLOR_MAP = {
    (0,   0,   0):   "K",           # BLACK
    (255, 255, 255): "W",           # WHITE
    (63,  63,  63):  "DGY",   # DARK GREY
    # (96, 80, 80):  "GY",    # GREY
    # (249, 255, 178): "WW",   # WARM WHITE
    # (0,   0, 255):   "DB",   # DARK_BLUE
    # (0, 255, 255):   "LB",  # LIGHT_BLUE (cyan)
    # (255, 0,   0):   "R",         # RED
    # (255, 170, 0):   "O",      # ORANGE
    # (255, 255, 0):   "Y",      # YELLOW
    # (153, 0, 255):   "P",      # PURPLE
    # (0, 255,   0):   "GR",       # GREEN
    (73, 22, 22):   "DR",        # DARK_RED
}

# Default token for unmatched pixels
DEFAULT_TOKEN = "K"
# ------------------------------

parser = argparse.ArgumentParser()
parser.add_argument("input", help="Input 64x64 image file")
parser.add_argument("output", help="Output text file")
parser.add_argument("--raw", action="store_true",
                    help="Output raw pixel values instead of mapped tokens")

args = parser.parse_args()

img = Image.open(args.input).convert("RGB")
w, h = img.size

if (w, h) != (64, 64):
    raise ValueError("Image must be 64x64.")

pixels = img.load()

with open(args.output, "w") as f:
    f.write("uint8_t arr[64][64][3] = {\n")
    for y in range(63, -1, -1):
        row_entries = []
        for x in range(64):
            r, g, b = pixels[x, y]

            if args.raw:
                row_entries.append(f"{{{r}, {g}, {b}}}")
            else:
                token = COLOR_MAP.get((r, g, b), DEFAULT_TOKEN)
                row_entries.append(token)

        f.write(f"    {{{', '.join(row_entries)}}},\n")
    f.write("};\n")