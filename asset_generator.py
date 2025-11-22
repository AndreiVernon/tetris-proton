from PIL import Image

# ------------------------------
# User-configurable section
# ------------------------------
IMAGE_PATH = "mockup_asset_gen.png"

# Mapping: pixel_value → text_token
# Pixel values must be 3-tuples of (R, G, B)
COLOR_MAP = {
    (0,   0,   0):   "K",           # BLACK
    (255, 255, 255): "W",           # WHITE
    (63,  63,  63):  "DGY",   # DARK GREY
    # (249, 255, 178): "WW",   # WARM WHITE
    # (0,   0, 255):   "DB",   # DARK_BLUE
    # (0, 255, 255):   "LB",  # LIGHT_BLUE (cyan)
    # (255, 0,   0):   "R",         # RED
    # (255, 170, 0):   "O",      # ORANGE
    # (255, 255, 0):   "Y",      # YELLOW
    # (153, 0, 255):   "P",      # PURPLE
    # (0, 255,   0):   "GR",       # GREEN
    # (119, 50, 50):   "RU",        # RUST
}

# Default token for unmatched pixels
DEFAULT_TOKEN = "K"
# ------------------------------

img = Image.open(IMAGE_PATH).convert("RGB")
w, h = img.size

if (w, h) != (64, 64):
    raise ValueError("Image must be 64x64.")

pixels = img.load()

print("uint8_t background[64][64][3] = {")
for y in range(63, -1, -1):
    row_tokens = []
    for x in range(64):
        rgb = pixels[x, y]
        token = COLOR_MAP.get(rgb, DEFAULT_TOKEN)
        row_tokens.append(token)

    row_str = ", ".join(row_tokens)
    print(f"    {{{row_str}}},")
print("};")