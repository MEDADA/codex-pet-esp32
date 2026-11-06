#!/usr/bin/env python3
"""Generate LVGL 9 native descriptors: RGB565 color plane followed by A8 plane."""
from pathlib import Path
from PIL import Image
import struct
ROOT=Path(__file__).resolve().parents[1]
def pack_image(image, alpha):
    colors=bytearray(); mask=bytearray()
    raw=image.convert('RGBA').tobytes()
    for off in range(0,len(raw),4):
        r,g,b,a=raw[off:off+4]
        if alpha and a==0:r=g=b=0
        colors.extend(struct.pack('<H',((r>>3)<<11)|((g>>2)<<5)|(b>>3)))
        if alpha:mask.append(a)
    return bytes(colors+mask)
def main():
    generated=ROOT/'main/generated';generated.mkdir(exist_ok=True)
    lines=['/* Generated from V11.1 approved PNGs. Run tools/build_pet_v11_assets.py. */','#include "pet_v11_assets.h"','']
    decl=['#pragma once','#include "lvgl.h"']
    for name,alpha,size in [('background',False,(466,466)),('open',True,(264,264)),('closed',True,(264,264))]:
        with Image.open(ROOT/'vendor/codex_pet_device_mvp_v11_1/generated'/f'{name}.png') as im:
            if im.size!=size:raise ValueError(f'{name}: wrong size')
            data=pack_image(im,alpha)
        lines.append(f'static const uint8_t {name}_data[] = {{')
        for pos in range(0,len(data),24):lines.append(','.join(f'0x{v:02x}' for v in data[pos:pos+24])+',')
        lines+=['};',f'const lv_image_dsc_t pet_v11_{name} = {{',
          f'.header = {{.magic=LV_IMAGE_HEADER_MAGIC,.cf={"LV_COLOR_FORMAT_RGB565A8" if alpha else "LV_COLOR_FORMAT_RGB565"},.w={size[0]},.h={size[1]},.stride={size[0]*2}}},',
          f'.data_size=sizeof({name}_data),.data={name}_data','};']
        decl.append(f'extern const lv_image_dsc_t pet_v11_{name};')
        print(name,len(data))
    (generated/'pet_v11_assets.c').write_text('\n'.join(lines)+'\n')
    (generated/'pet_v11_assets.h').write_text('\n'.join(decl)+'\n')
if __name__=='__main__':main()
