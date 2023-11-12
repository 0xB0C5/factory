from PIL import Image
from pathlib import Path
import os


class ImgData:
    def __init__(self):
        self.sprites = {}
        self.bgs = {}

    def add_sprites(self, name, img):
        # mask = green > red
        masks = self._get_patterns(img, 8, lambda color: color[1] > color[0])
        # pattern = red < 128
        patterns = self._get_patterns(img, 8, lambda color: color[0] < 128)

        sprites = [mask + pat for mask, pat in zip(masks, patterns)]

        self.sprites[name] = sprites

    def add_backgrounds(self, name, img):
        bgs = self._get_patterns(img, 7, lambda color: color[0] < 128)

        self.bgs[name] = bgs

    def get_header(self):
        lines = ['#include <stdint.h>']
        lines = ['#include <avr/pgmspace.h>']
        for bg_name in sorted(self.bgs):
            bgs = self.bgs[bg_name]
            lines.append(f'extern const uint8_t bg_tile_{bg_name}[{len(bgs)}][7];')

        for sprite_name in sorted(self.sprites):
            sprites = self.sprites[sprite_name]
            lines.append(f'extern const uint8_t sprite_{sprite_name}[{len(sprites)}][16];')

        return '\n'.join(lines) + '\n'

    def get_cpp(self):
        lines = ['#include "generated_graphics.h"']
        for bg_name in sorted(self.bgs):
            bgs = self.bgs[bg_name]
            lines.append(f'const uint8_t bg_tile_{bg_name}[{len(bgs)}][7] = ' + '{')
            for bg in bgs:
                lines.append('  {' + ', '.join(byte_to_hex(row) for row in bg) + '},')
            lines.append('};')

        for sprite_name in sorted(self.sprites):
            sprites = self.sprites[sprite_name]
            lines.append(f'const uint8_t sprite_{sprite_name}[{len(sprites)}][16] = ' + '{')
            for sprite in sprites:
                lines.append('  {' + ', '.join(byte_to_hex(row) for row in sprite) + '},')
            lines.append('};')

        return '\n'.join(lines) + '\n'

    def _get_patterns(self, img, stride_y, predicate):
        assert img.width % 8 == 0
        assert img.height % stride_y == 0

        patterns = []

        for y0 in range(0, img.height, stride_y):
            for x0 in range(0, img.width, 8):
                pattern = []

                for y in range(y0, y0+stride_y):
                    row = 0
                    for x in range(x0, x0+8):
                        row <<= 1
                        if predicate(img.getpixel((x, y))):
                            row |= 1
                    pattern.append(row)

                patterns.append(pattern)

        return patterns

def byte_to_hex(b):
    h = hex(b)[2:]
    h = '0'*(2-len(h)) + h
    return '0x' + h

def main():
    root_dir = Path(__file__).parent.parent

    bgs_dir = root_dir / 'img' / 'backgrounds'
    sprites_dir = root_dir / 'img' / 'sprites'
    
    img_data = ImgData()

    for filename in os.listdir(sprites_dir):
        sprite_path = sprites_dir / filename
        print(sprite_path)
        img = Image.open(sprite_path)
        img_data.add_sprites(filename.split('.')[0], img)

    for filename in os.listdir(bgs_dir):
        bg_path = bgs_dir / filename
        print(bg_path)
        img = Image.open(bg_path)
        img_data.add_backgrounds(filename.split('.')[0], img)

    print(img_data.get_header())
    print('// ---------- //')
    print(img_data.get_cpp())
    
    with open(root_dir / 'factory' / 'generated_graphics.h', 'w') as header_file:
        header_file.write(img_data.get_header())

    with open(root_dir / 'factory' / 'generated_graphics.cpp', 'w') as cpp_file:
        cpp_file.write(img_data.get_cpp())

if __name__ == '__main__':
    main()

'''

img = Image.open('factory.png')

data = []

for y0 in range(0, img.height, 8):
    for x in range(img.width):
        d = 0
        for i in range(8):
            r, g, b = img.getpixel((x, y0+i))
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
'''