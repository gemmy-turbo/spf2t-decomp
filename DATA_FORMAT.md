# Complete decomp handoff v1

The primary process writes `<output>/decomp/`:

- `source/include/puzzle_fighter/`: the recovered headers needed to compile.
- `data/`: the verified ROM-derived pack described below, including generated code.
- `licenses/`: required source origins, license texts and notices.
- `manifest.json`: format `spf2t-decomp-v1`, API version1, game revision
  USA960620, actual tooling/source identity, input-member hashes and a size/SHA256
  inventory of every exported file. No machine-specific paths or board key.

Native compilation consumes this directory alone. It requires no separate
checkout or import tool. Native preflight checks the format/API/revision,
required inputs and file identities. The source identity records the actual
local tools/headers, so a locally modified decomp project is distinguishable
without relying on Git metadata. Incompatible layout/API changes require a new
contract version and a compatible consumer.

# Local data format v1

`manifest.json` identifies `puzzle-fighter-data-v1`, set `spf2tu`, revision
`USA960620`, input-member SHA256 identities and every immutable output's size
and SHA256. It contains no keys or machine-specific absolute paths. Repacking
identical ZIP members produces identical immutable pack contents and manifest.

| Path | Representation |
| --- | --- |
| tables/main-program.bin | 1 MiB, big-endian original DATA-read view |
| tables/main-opcodes.bin | 1 MiB, big-endian original opcode-fetch view, used as table/address data by native source |
| assets/graphics.bin | 4 MiB, original graphics lanes combined and address permutation reversed |
| assets/samples.bin | 4 MiB, original sample ROMs combined and byte ordering normalized |
| assets/sound-program.bin | 256 KiB, original sound program/data concatenated; native sequencer reads its tables/sequences |
| generated/puzzle_fighter/generated/source_howto_sequence.inc | Locally generated fixed C++ tutorial continuations |

The decoder receives the supplied board key through a temporary input file;
temporary inputs are removed when the importer exits. Only compatibility
identifiers and member hashes appear in the manifest. Artwork/audio is not
redrawn, recorded or transcoded to PNG/WAV. The pack is a directory, with no
custom archive codec or database requirement.

The native frontend uses `--data-dir` to read the pack. Its user state is separate
and writable under `--user-dir` (default `./user`), and is outside pack checksums. The packaged executable defaults to adjacent data/user directories.
Changing this contract requires a new format version and a compatible consumer.
