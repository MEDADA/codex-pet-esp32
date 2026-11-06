#!/usr/bin/env python3
"""Run all firmware host suites without any device IO."""
from pathlib import Path
import subprocess,tempfile,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
    with tempfile.TemporaryDirectory(prefix='pet-p0-') as temp:
        for name,sources in [
            ('scene',['main/buddy_scene.c']),('requests',['main/buddy_requests.c']),
            ('calendar',[]),('io_contracts',[]),('display_guard',[]),('imu6_decode',[]),('content_binding',['main/buddy_requests.c']),
            ('sensors',['main/buddy_motion.c','main/buddy_power.c']),
            ('pet_presentation',['main/pet_presentation.c','main/buddy_state_evidence.c']),
            ('state_evidence',['main/buddy_state_evidence.c']),
            ('pet_activity',['main/pet_animation.c']),
            ('pet_frame_schedule',[]),
            ('pet_animation',['main/pet_animation.c','main/pet_presentation.c','main/buddy_state_evidence.c'])]:
            binary=str(Path(temp)/name)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-Itests/display_stub','-Imain',f'tests/test_{name}.c',*sources,'-lm','-o',binary],cwd=ROOT,check=True)
            subprocess.run([binary],check=True)
        binary=str(Path(temp)/'pet_motion')
        subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror','-Itests/display_stub','-Imain','-Ivendor/codex_pet_device_mvp_v11_1/include','tests/test_pet_motion.cpp','-o',binary],cwd=ROOT,check=True)
        subprocess.run([binary],check=True)
        subprocess.run([sys.executable,'tests/test_pet_assets.py'],cwd=ROOT,check=True)
if __name__=='__main__':main()
