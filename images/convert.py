#!/usr/bin/python3

from PIL import Image

def rp6502_rgb_sprite_bpp16(r,g,b):
    if r==0 and g==0 and b==0:
        return 0
    else:
        return ((((b>>3)<<11)|((g>>3)<<6)|(r>>3))|1<<5)

def conv_spr(name_in, size, name_out):
    with Image.open(name_in) as im:
        with open("./" + name_out, "wb") as o:
            rgb_im = im.convert("RGB")
            im2 = rgb_im.resize(size=[size,size])
            for y in range(0, im2.height):
                for x in range(0, im2.width):
                    r, g, b = im2.getpixel((x, y))
                    o.write(
                        rp6502_rgb_sprite_bpp16(r,g,b).to_bytes(
                            2, byteorder="little", signed=False
                        )
                    )


conv_spr("altair.png", 128, "altair.bin")
conv_spr("radio.png", 64, "radio.bin")
