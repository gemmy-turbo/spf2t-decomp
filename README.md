# Gemmy Turbo — Puzzle Fighter recovered source

[gemmy-turbo/spf2t-decomp](https://github.com/gemmy-turbo/spf2t-decomp)

Readable, partially recovered implementation of Super Puzzle Fighter II Turbo
(USA 960620, `spf2tu`). This is an unofficial source reconstruction, not Capcom's
original source release or a complete decompilation. No CPS2 ROM build target
is provided. No ROM, board key, graphics, audio or generated game sequence is
included in this source repository. Supply your own supported ROM files.

## Quick start with Docker or Podman

Place your supported ROM ZIP(s) or extracted files in `roms/`, then run from
this repository:

```sh
docker build -t spf2t-decomp:local .
mkdir -p output
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/roms:/roms:ro" -v "$PWD/output:/output" spf2t-decomp:local
```

The complete decomp output appears in `output/decomp/`. For Podman, replace
`docker` with `podman` and add `--userns=keep-id` to the run command.
See [CONTAINERS.md](CONTAINERS.md) for using this output with the native image.

## Run with installed tools

Requirements: Python 3.9+ and a C++17 compiler (`c++`, or pass `--cxx`).
The process checks the compiler, output location and required ROM members
before exporting.
A complete ZIP, split ZIPs with shared members, or extracted members can be used.
The supported member identities are in `roms/spf2tu.json`.

```sh
python3 tools/decompile.py --rom-dir /path/to/roms --output /path/to/output
```

For a single complete archive, use `--rom /path/to/spf2tu.zip` instead of
`--rom-dir`. The complete native-build handoff is written to `output/decomp/`:
recovered source headers, imported game data, generated code, notices and a
versioned inventory. Identical intact output is reusable. Different tooling or
damaged output requires a fresh output directory; existing output is preserved.

## Repository defaults

Place ROM files in `roms/`, then run:

```sh
python3 decompile.py
```

This reads `roms/` and writes `output/decomp/`. To print the full command so you
can copy it and adjust the paths or options:

```sh
python3 decompile.py --dry-run
```

User ROMs, `output/`, and `build/` are ignored by Git regardless of filename.
`roms/spf2tu.json` is committed compatibility metadata, not a ROM.

## Use the output

The [native project](https://github.com/gemmy-turbo/spf2t-native) consumes the
complete `output/decomp/` directory. Its AIO image includes this tooling project
and runs both processes in sequence.
See [DATA_FORMAT.md](DATA_FORMAT.md) for the handoff contract.

The lower-level `tools/import_rom.py --rom ZIP --output DATA_DIRECTORY` and
`tools/verify_pack.py --pack DATA_DIRECTORY` remain available for data-only
integrations. They do not export the complete native-build handoff.

## License, ownership and AI assistance

Project source is GPL-3.0-only unless a file says otherwise. Required imported
GPL/BSD notices and origins are in LICENSE, THIRD_PARTY_NOTICES.md and UPSTREAM.md.
Names and original game content belong to their respective rights holders.
This project is not affiliated with or endorsed by Capcom; the source license
grants no rights over original game content. See [LICENSE_SCOPE.md](LICENSE_SCOPE.md).
Locally generated output contains game content and must not be committed or
included in public source packages. This is not a clean-room or whole-ROM parity claim.

AI tools were used extensively in reverse engineering, implementation, testing
and documentation. The reconstruction remains incomplete. The initial supported
profile is the factory paid USA cabinet; full service/operator/hardware coverage
and original-platform ROM rebuilding are outside this initial release.
