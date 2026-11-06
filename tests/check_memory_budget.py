from pathlib import Path
import re,sys
path=Path(sys.argv[1] if len(sys.argv)>1 else 'build/codeisland_buddy_1_75c.map')
text=path.read_text()
config=Path(sys.argv[2] if len(sys.argv)>2 else 'sdkconfig').read_text()
# .ext_ram.dummy skips FLASH virtual addresses even when XIP is off.
# Only enabled copy-to-PSRAM segments consume physical PSRAM.
reserved=0
for key,section in [('CONFIG_SPIRAM_FETCH_INSTRUCTIONS','.flash.text'),('CONFIG_SPIRAM_RODATA','.flash.rodata')]:
    if re.search(r'^'+key+r'=y$',config,re.M):
        match=re.search(r'^'+re.escape(section)+r'\s+0x[0-9a-f]+\s+(0x[0-9a-f]+)',text,re.M)
        assert match,'Missing linked section '+section
        reserved+=(int(match.group(1),16)+65535)//65536*65536
remaining=8*1024*1024-reserved
print(f'PSRAM static reservation={reserved}, remaining before runtime={remaining}')
assert remaining>=4*1024*1024,'Asset XIP leaves insufficient PSRAM for display/fonts/960000-byte capture'
