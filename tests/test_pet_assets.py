import importlib.util
from pathlib import Path
import unittest
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
class Conversion(unittest.TestCase):
    def test_lvgl9_pixel_planes(self):
        spec=importlib.util.spec_from_file_location('converter',ROOT/'tools/build_pet_v11_assets.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        image=Image.new('RGBA',(2,1));image.putdata([(255,0,0,255),(0,0,255,0)])
        self.assertEqual(module.pack_image(image,True),bytes.fromhex('00f8 0000 ff00'))
        self.assertEqual(module.pack_image(image,False),bytes.fromhex('00f8 1f00'))
    def test_approved_resource_dimensions(self):
        for name,sz in [('background',(466,466)),('open',(264,264)),('closed',(264,264))]:
            with Image.open(ROOT/'vendor/codex_pet_device_mvp_v11_1/generated'/f'{name}.png') as im:
                self.assertEqual(im.size,sz)
if __name__=='__main__':unittest.main()
