from PIL import Image

# ------------------------------
# User-configurable section
# ------------------------------
IMAGE_PATH = "mockup_asset_gen.png"

# Mapping: pixel_value → text_token
# Pixel values must be 3-tuples of (R, G, B)
COLOR_MAP = {
    (0, 0, 0): "K",              # black
    (255, 255, 255): "Y",    # white
    (63, 63, 63): "DARK_GREY"
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