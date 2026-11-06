#!/usr/bin/env python3
"""Deterministic extraction of approved imagegen backgrounds and installed Rem v2.
No character regeneration: sprites are cropped from the user-selected original atlas.
"""
from pathlib import Path
from io import BytesIO
import json
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
W,H=152,192
COUNTS=[6,8,8,4,5,8,6,6,6,8,8]
MAPPING={'sleep':(0,6),'work':(7,6),'alert':(3,4),'question':(8,6),
         'success':(4,5),'error':(5,8),'offline':(6,6)}

def rgb565a8(image):
    rgb=bytearray();alpha=bytearray()
    for r,g,b,a in image.convert('RGBA').getdata():
        value=((r&248)<<8)|((g&252)<<3)|(b>>3)
        rgb.extend((value&255,value>>8));alpha.append(a)
    return bytes(rgb+alpha)

def array(name,data):
    rows=['  '+','.join(f'0x{x:02x}' for x in data[i:i+32])+',' for i in range(0,len(data),32)]
    return f'static const LV_ATTRIBUTE_MEM_ALIGN uint8_t {name}[]={{\n'+ '\n'.join(rows)+'\n};\n'

def descriptor(name,cf,w,h,size,stride):
    return f'const lv_image_dsc_t {name}={{.header={{.magic=LV_IMAGE_HEADER_MAGIC,.cf={cf},.w={w},.h={h},.stride={stride}}},.data_size={size},.data={name}_data}};\n'

def main():
    out=ROOT/'main/generated';out.mkdir(exist_ok=True)
    preview=ROOT/'assets/rem-v2';preview.mkdir(exist_ok=True)
    sheet=Image.open(Path.home()/'.codex/pets/cute-rem/spritesheet.webp').convert('RGBA')
    assert sheet.size==(1536,2288), 'Rem v2 required'
    code=['#include "rem_assets.h"\n'];header=['#pragma once\n#include "lvgl.h"\n']
    for name,(row,count) in MAPPING.items():
        for col in range(count):
            # Keep baseline and all animation displacement; no per-frame trimming.
            im=sheet.crop((col*192+14,row*208,(col+1)*192-14,(row+1)*208)).resize((W,H),Image.Resampling.LANCZOS)
            im.save(preview/f'{name}-{col}.png')
            data=rgb565a8(im);stem=f'rem_{name}_{col}'
            code += [array(stem+'_data',data),descriptor(stem,'LV_COLOR_FORMAT_RGB565A8',W,H,len(data),W*2)]
        code += [f'const lv_image_dsc_t *const rem_{name}[{count}]={{'+','.join(f'&rem_{name}_{n}' for n in range(count))+'};\n']
        header += [f'extern const lv_image_dsc_t *const rem_{name}[{count}];\n']
    # Direction poses share the same v2 crop/baseline as animation rows.
    for n in range(16):
        row=9+n//8;col=n%8
        im=sheet.crop((col*192+14,row*208,(col+1)*192-14,(row+1)*208)).resize((W,H),Image.Resampling.LANCZOS)
        data=rgb565a8(im);stem=f'rem_look_{n}'
        code += [array(stem+'_data',data),descriptor(stem,'LV_COLOR_FORMAT_RGB565A8',W,H,len(data),W*2)]
    code += ['const lv_image_dsc_t *const rem_look[16]={'+','.join(f'&rem_look_{n}' for n in range(16))+'};\n']
    header += ['extern const lv_image_dsc_t *const rem_look[16];\n']
    atlas=Image.open(ROOT/'assets/rem-background-atlas.png').convert('RGB')
    assert atlas.size==(1536,1024), atlas.size
    backgrounds=[]
    for n in range(6):
        x=n%3*512;y=n//3*512
        im=atlas.crop((x,y,x+512,y+512)).resize((466,466),Image.Resampling.LANCZOS)
        im.save(preview/f'background-{n}.png')
        buffer=BytesIO();im.save(buffer,format='JPEG',quality=87,progressive=False,subsampling=0)
        data=buffer.getvalue();stem=f'rem_bg_{n}'
        code += [array(stem+'_data',data),descriptor(stem,'LV_COLOR_FORMAT_UNKNOWN',466,466,len(data),0)]
        backgrounds.append(len(data))
    code+=['const lv_image_dsc_t *const rem_backgrounds[6]={'+','.join(f'&rem_bg_{n}' for n in range(6))+'};\n']
    header+=['extern const lv_image_dsc_t *const rem_backgrounds[6];\n#define REM_WIDTH 152\n#define REM_HEIGHT 192\n']
    (out/'rem_assets.c').write_text(''.join(code));(out/'rem_assets.h').write_text(''.join(header))
    report={'sheet':list(sheet.size),'spriteSize':[W,H],'animations':MAPPING,'directionFrames':16,'backgroundJpegBytes':backgrounds}
    (preview/'manifest.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report))

if __name__=='__main__':main()
