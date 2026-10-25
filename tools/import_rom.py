#!/usr/bin/env python3
"""Prepare local USA960620 game data from a user-supplied ROM ZIP."""
from array import array
from pathlib import Path
from zipfile import ZipFile, BadZipFile
import argparse, hashlib, json, os, shlex, subprocess, tempfile

ROOT = Path(__file__).resolve().parents[1]
FORMAT = 'puzzle-fighter-data-v1'
EXPECTED_VIEWS = {
    'tables/main-program.bin': '8067a254cd52951ed05acb94dd80b0e4e14af9623b0f2bb1dd37a3dec9da5724',
    'tables/main-opcodes.bin': '810bf4b57680a1c9ec86b37ac8583dd79ba2e14c1c83991cd077226a37a26131',
}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def load_members(archive):
    identity = json.loads((ROOT / 'roms/spf2tu.json').read_text())
    members = {}
    with ZipFile(archive) as source:
        names = {}
        for item in source.infolist():
            if item.is_dir(): continue
            name = Path(item.filename).name.lower()
            if name in names: raise ValueError('Duplicate ROM member: ' + name)
            names[name] = item
        for expected in identity['members']:
            name = expected['name']
            if name not in names: raise ValueError('Missing ROM member: ' + name)
            item = names[name]
            if item.file_size != expected['size']: raise ValueError('Wrong ROM size: ' + name)
            data = source.read(item)
            if hashlib.sha1(data).hexdigest() != expected['sha1']: raise ValueError('Wrong USA960620 ROM identity: ' + name)
            members[name] = data
    return members

def graphics(members):
    gfx = bytearray(0x400000)
    for lane, name in enumerate(('pzf.14m', 'pzf.16m', 'pzf.18m', 'pzf.20m')):
        gfx[lane * 2::8] = members[name][0::2]
        gfx[lane * 2 + 1::8] = members[name][1::2]
    def unshuffle(words, at, size):
        if size == 2: return
        half = size // 2
        unshuffle(words, at, half); unshuffle(words, at + half, half)
        for n in range(half // 2):
            words[at + half // 2 + n], words[at + half + n] = words[at + half + n], words[at + half // 2 + n]
    for at in range(0, len(gfx), 0x200000):
        words = array('Q'); words.frombytes(gfx[at:at + 0x200000])
        unshuffle(words, 0, len(words)); gfx[at:at + 0x200000] = words.tobytes()
    return gfx

def prepare(archive, output, views_tool=None):
    # Validate every input before creating output. Existing packs are never
    # overwritten; replacement is an explicit user filesystem operation.
    members = load_members(archive)
    if output.exists(): raise ValueError('Output already exists; choose a fresh data directory')
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.pf-import-', dir=output.parent) as temporary:
        stage = Path(temporary); pack = stage / 'pack'
        for directory in ('assets', 'tables', 'generated/puzzle_fighter/generated'):
            (pack / directory).mkdir(parents=True)
        (stage / 'program.input').write_bytes(members['pzfu.03a'] + members['pzf.04'])
        (stage / 'key.input').write_bytes(members['spf2tu.key'])
        if views_tool is None:
            command = shlex.split(os.environ.get('CXX', 'c++'))
            views_tool = stage / 'pf-rom-views'
            subprocess.run([*command, '-std=c++17', '-O2', str(ROOT / 'tools/rom_views.cpp'),
                            str(ROOT / 'third_party/cps2crypt/cps2crypt.cpp'),
                            '-I' + str(ROOT / 'third_party/cps2crypt'), '-o', str(views_tool)], check=True)
        subprocess.run([str(views_tool), str(stage / 'program.input'), str(stage / 'key.input'),
                        str(pack / 'tables/main-program.bin'), str(pack / 'tables/main-opcodes.bin')], check=True)
        for name, expected in EXPECTED_VIEWS.items():
            if digest((pack / name).read_bytes()) != expected: raise ValueError('Program view does not match verified USA baseline: ' + name)
        (pack / 'assets/graphics.bin').write_bytes(graphics(members))
        (pack / 'assets/sound-program.bin').write_bytes(members['pzf.01'] + members['pzf.02'])
        raw = members['pzf.11m'] + members['pzf.12m']; samples = bytearray(len(raw))
        samples[0::2] = raw[1::2]; samples[1::2] = raw[0::2]
        (pack / 'assets/samples.bin').write_bytes(samples)
        subprocess.run([os.sys.executable, str(ROOT / 'tools/generate_tutorial.py'),
                        '--data', str(pack / 'tables/main-program.bin'), '--output',
                        str(pack / 'generated/puzzle_fighter/generated/source_howto_sequence.inc')], check=True)
        outputs = {str(p.relative_to(pack)): {'sha256': digest(p.read_bytes()), 'size': p.stat().st_size}
                   for p in sorted(pack.rglob('*')) if p.is_file()}
        manifest = {'format': FORMAT, 'game': 'spf2tu', 'revision': 'USA960620',
                    'members_sha256': {n: digest(b) for n, b in sorted(members.items())},
                    'outputs': outputs, 'content': 'User-derived game data; do not redistribute with project source.'}
        (pack / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
        pack.rename(output)
    return output

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--views-tool', type=Path, help='Optional prebuilt pf-rom-views executable')
    args = parser.parse_args()
    try: print(prepare(args.rom.resolve(), args.output.resolve(), args.views_tool.resolve() if args.views_tool else None))
    except (ValueError, OSError, BadZipFile, subprocess.CalledProcessError) as error: parser.exit(1, str(error) + '\n')

if __name__ == '__main__': main()
