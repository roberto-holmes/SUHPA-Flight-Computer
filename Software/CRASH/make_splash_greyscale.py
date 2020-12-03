#!/usr/bin/env python3
# pip install pillow to get the PIL module

import sys
from PIL import Image

def getGreyscale(c, numBits):
	value = (0.2126 * c[0]) + (0.7152 * c[1]) + (0.0722 * c[2])
	return int(value * float(1 << numBits) / 256.0)

def main(fn, id, numBits):
	image = Image.open(fn)
	print("\n"
			"#define {id}_width  {w}\n"
			"#define {id}_height {h}\n"
			"\n"
			"const uint8_t PROGMEM {id}_data[] = {{\n"
			.format(id=id, w=image.width, h=image.height), end='')
	for y in range(image.height):
		for x in range(image.width):
			print("0x{:02x}".format(getGreyscale(image.getpixel((x, y)), numBits)), end='')
									
			if x < image.width - 1:
				print(",", end='')

		if y < image.height - 1:
			print(",", end='')
		print()
	print("};")

if __name__ == '__main__':
	if len(sys.argv) < 4:
		print("Usage: {} <imagefile> <id> <numBits>\n".format(sys.argv[0]), file=sys.stderr);
		sys.exit(1)
	fn = sys.argv[1]
	id = sys.argv[2]
	numBits = int(sys.argv[3])
	main(fn, id, numBits)
