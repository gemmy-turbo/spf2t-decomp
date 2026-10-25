#!/usr/bin/env python3
"""Check all immutable files named by a local data-pack manifest."""
from pathlib import Path
import argparse, hashlib, json

def verify(root):
    root = root.resolve()
    manifest = json.loads((root / 'manifest.json').read_text())
    if manifest.get('format') != 'puzzle-fighter-data-v1' or manifest.get('revision') != 'USA960620':
        raise ValueError('Unsupported data pack; import the supported USA960620 ROM')
    expected = {'assets/graphics.bin', 'assets/samples.bin', 'assets/sound-program.bin',
                'tables/main-program.bin', 'tables/main-opcodes.bin',
                'generated/puzzle_fighter/generated/source_howto_sequence.inc'}
    if set(manifest['outputs']) != expected: raise ValueError('Unexpected data-pack inventory')
    for name, record in manifest['outputs'].items():
        file = (root / name).resolve()
        file.relative_to(root)
        data = file.read_bytes()
        if len(data) != record['size'] or hashlib.sha256(data).hexdigest() != record['sha256']:
            raise ValueError('Data-pack checksum mismatch: ' + name)
    return manifest

def main():
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--pack', type=Path, required=True)
    args = parser.parse_args()
    try: print('Verified ' + verify(args.pack)['revision'] + ' data pack')
    except (ValueError, OSError, KeyError) as error: parser.exit(1, str(error) + '\n')

if __name__ == '__main__': main()
