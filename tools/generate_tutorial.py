#!/usr/bin/env python3
"""Development-only compiler for the fixed original USA how-to continuation.

The product contains concrete C++ states, not this source command decoder.
Pass explicit --data and --output paths. Generated game content stays local.
"""
from pathlib import Path
import argparse,hashlib,json
parser=argparse.ArgumentParser(description='Generate local USA tutorial continuations')
parser.add_argument('--data',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
cli_args=parser.parse_args();rom=cli_args.data.read_bytes()
assert hashlib.sha256(rom).hexdigest()=='8067a254cd52951ed05acb94dd80b0e4e14af9623b0f2bb1dd37a3dec9da5724'
widths={0:0,2:0,4:4,6:4,8:4,10:2,12:2,14:2,16:2,18:2,20:2,22:2,24:0,26:2,28:0,30:0,32:0,34:0,36:0,38:2}
rows=[];pc=0x4cd4
while True:
    start=pc;command=int.from_bytes(rom[pc:pc+2],'big');pc+=2
    assert command>>8 in (0,2,4)
    width=widths[command&255] if command>>8==0 else 0
    argument=rom[pc:pc+width].hex();pc+=width;rows.append([start,command,argument])
    if command==34:break
assert pc==0x5454 and len(rows)==592
phases=[];now=[]
for row in rows:
    now.append(row)
    if row[1]>>8==4 or row[1] in (0x22,0x24):phases.append(now);now=[]
assert not now and len(phases)==154
motions=['enter','face_left','face_right','gesture_2','gesture_3','gesture_4','gesture_5','jump_6','jump_7','exit']
def operation(cmd,args,after):
    v=int(args or '0',16)
    if cmd>>8==2:return 'motion(SourceTutorialMotion::'+motions[(cmd&255)//2]+');'
    return {
        0:'m.write_byte(g-0x738b,255);',2:'m.write_byte(g-0x738b,0);',
        4:f'place_pair(0x{v>>16:x},0x{v&65535:x});',6:f'm.write_long(g+0x452,0x{v:x});',8:f'm.write_long(g+0x456,0x{v:x});',
        10:f'm.write_word(g-0x7388,0x{v:x});',12:f'board_pattern({v});',14:f'input(2,{v});',16:f'input(1,{v});',18:f'input(4,{v});',
        20:f'input(0x100,{v});',22:f'input(0x200,{v});',24:'input(0,0);',26:f'caption({v});',28:'caption(0);',30:'show_controls();',
        32:'m.write_byte(g-0x738a,0);',34:'finish();',36:f'reset_players(0x{after:x});',38:f'byte(0x782,{v>>8});byte(0x89e,{v&255});'
    }[cmd]
lines=['// Generated development-time translation of verified USA DATA4CD4..5453.', '// Fixed source-specific continuations: no product command fetch/decode.']
for index,group in enumerate(phases):
    lines.append(f'case {index}:{{ // original USA sequence segment {group[0][0]:04X}..{group[-1][0]:04X}')
    prefix=group[:-1];at,cmd,args=group[-1];after=at+2+len(args)//2
    if prefix:
        lines.append(' if(!entered){')
        for start,command,argument in prefix:
            end=start+2+len(argument)//2
            lines.append(f'  word(0xc,0x{command:x});m.write_byte(g-0x738c,255);{operation(command,argument,end)}m.write_long(g-0x7394,0x{end:x});')
        lines.extend(['  entered=true;',' }'])
    lines.append(f' word(0xc,0x{cmd:x});m.write_byte(g-0x738c,0);')
    if cmd==0x400:
        lines.append(' m.write_word(g-0x7388,m.read_word(g-0x7388)-1);')
        lines.append(f' if(m.read_word(g-0x7388)&0x8000){{m.write_word(g-0x7388,0);m.write_long(g-0x7394,0x{after:x});phase={index+1};entered=false;}}else m.write_long(g-0x7394,0x{at:x});')
    elif cmd==0x402:
        lines.append(f' if(m.read_byte(g+0x48e)){{m.write_word(g-0x7388,0);m.write_long(g-0x7394,0x{after:x});phase={index+1};entered=false;}}else m.write_long(g-0x7394,0x{at:x});')
    else:lines.append(f' {operation(cmd,args,after)}m.write_long(g-0x7394,0x{after:x});phase={index+1};entered=false;')
    lines.append(' break;}')
text='\n'.join(lines)+'\n';target=cli_args.output
target.parent.mkdir(parents=True,exist_ok=True);target.write_text(text)
print(json.dumps({'source_records':len(rows),'native_stages':len(phases),'source_end':hex(pc),'sha256':hashlib.sha256(text.encode()).hexdigest()}))
