from PIL import Image
from pathlib import Path

root_dir = Path(__file__).parent.parent

img = Image.open(root_dir / 'img' / 'factory.png')

data = []

for y0 in range(0, img.height, 8):
    for x in range(img.width):
        d = 0
        for i in range(8):
            r = img.getpixel((x, y0+i))[0]
            if r < 128:
                d |= 1 << i
        data.append(d)

def byte_to_hex(b):
    h = hex(b)[2:]
    h = '0'*(2-len(h)) + h
    return '0x' + h


chunk_size = 21
for i in range(0, len(data), chunk_size):
    print(','.join(map(byte_to_hex, data[i:i+chunk_size])) + ',')