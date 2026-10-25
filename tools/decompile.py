#!/usr/bin/env python3
"""Validate local ROMs and export the complete native-build handoff."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
from zipfile import BadZipFile, ZipFile, ZIP_STORED

from import_rom import ROOT, prepare

FORMAT = "spf2t-decomp-v1"
API_VERSION = 1


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rom_members(rom, rom_dir):
    expected = {row["name"]: row for row in json.loads((ROOT / "roms/spf2tu.json").read_text())["members"]}
    found = {}
    def consider(name, data):
        name = Path(name).name.lower()
        record = expected.get(name)
        if record and len(data) == record["size"] and hashlib.sha1(data).hexdigest() == record["sha1"]:
            found[name] = data
    if rom is not None:
        if not rom.is_file():
            raise ValueError("ROM archive does not exist: " + str(rom))
        paths = [rom]
    else:
        if not rom_dir.is_dir():
            raise ValueError("ROM directory does not exist: " + str(rom_dir))
        paths = sorted(rom_dir.rglob("*"))
    for path in paths:
        if not path.is_file():
            continue
        if path.suffix.lower() == ".zip":
            with ZipFile(path) as archive:
                for member in archive.infolist():
                    record = expected.get(Path(member.filename).name.lower())
                    if record and member.file_size == record["size"]:
                        consider(member.filename, archive.read(member))
        elif path.name.lower() in expected:
            consider(path.name, path.read_bytes())
    missing = sorted(set(expected) - set(found))
    if missing:
        raise ValueError("Missing or wrong USA960620 ROM members: " + ", ".join(missing))
    return found


def preflight(args):
    errors = []
    if sys.version_info < (3, 9):
        errors.append("Python 3.9 or newer is required")
    compiler = shlex.split(args.cxx)
    if not compiler or shutil.which(compiler[0]) is None:
        errors.append("C++17 compiler not found: " + args.cxx)
    else:
        check = subprocess.run(compiler + ["-std=c++17", "-x", "c++", "-fsyntax-only", "-"], input="#include <filesystem>\nint main() { std::filesystem::path p; }\n", text=True, capture_output=True)
        if check.returncode:
            errors.append("C++17 compiler check failed: " + check.stderr.strip())
    required = ["include/puzzle_fighter", "source-manifest.json", "roms/spf2tu.json", "tools/rom_views.cpp", "tools/generate_tutorial.py", "third_party/cps2crypt/cps2crypt.cpp", "third_party/cps2crypt/cps2crypt.h", "third_party/BSD-3-Clause.txt", "LICENSE", "LICENSE_SCOPE.md", "THIRD_PARTY_NOTICES.md", "UPSTREAM.md"]
    for relative in required:
        path = ROOT / relative
        if not path.exists():
            errors.append("Required project file/directory is missing: " + str(path))
    parent = args.output.resolve()
    while not parent.exists():
        parent = parent.parent
    if not parent.is_dir() or not os.access(parent, os.W_OK | os.X_OK):
        errors.append("Output location is not writable: " + str(args.output))
    members = None
    try:
        members = rom_members(args.rom, args.rom_dir)
    except (ValueError, OSError, BadZipFile) as error:
        errors.append(str(error))
    if errors:
        raise ValueError("Preflight failed:\n- " + "\n- ".join(errors))
    return members


def verify_handoff(path):
    manifest = json.loads((path / "manifest.json").read_text())
    if manifest.get("format") != FORMAT or manifest.get("api_version") != API_VERSION:
        raise ValueError("Existing decomp output has an unsupported format")
    for relative, record in manifest["files"].items():
        item = (path / relative).resolve()
        item.relative_to(path.resolve())
        if item.stat().st_size != record["size"] or sha256(item) != record["sha256"]:
            raise ValueError("Existing decomp output is damaged: " + relative)
    return manifest


def run(args):
    members = preflight(args)
    destination = args.output.resolve() / "decomp"
    inputs = {name: hashlib.sha256(data).hexdigest() for name, data in sorted(members.items())}
    # Actual inputs identify local source changes too; no Git checkout is required.
    sources = [path for directory in ("include", "tools", "third_party") for path in (ROOT / directory).rglob("*") if path.is_file() and path.suffix in (".hpp", ".h", ".cpp", ".py")]
    sources.extend(ROOT / name for name in ("roms/spf2tu.json", "LICENSE", "LICENSE_SCOPE.md", "THIRD_PARTY_NOTICES.md", "UPSTREAM.md", "third_party/BSD-3-Clause.txt"))
    source_hashes = {str(path.relative_to(ROOT)): sha256(path) for path in sorted(sources)}
    identity = hashlib.sha256(json.dumps(source_hashes, sort_keys=True).encode()).hexdigest()
    if destination.exists():
        manifest = verify_handoff(destination)
        if manifest["members_sha256"] != inputs or manifest["source_id"] != identity:
            raise ValueError("Existing decomp output comes from different inputs/tooling; choose a fresh --output directory")
        print("Verified existing handoff: " + str(destination))
        return
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".decomp-", dir=destination.parent) as temporary:
        stage = Path(temporary)
        archive = stage / "inputs.zip"
        with ZipFile(archive, "w", compression=ZIP_STORED) as target:
            for name, data in sorted(members.items()):
                target.writestr(name, data)
        handoff = stage / "decomp"
        handoff.mkdir()
        os.environ["CXX"] = args.cxx
        prepare(archive, handoff / "data")
        shutil.copytree(ROOT / "include", handoff / "source/include")
        notices = handoff / "licenses"
        notices.mkdir()
        for name in ("LICENSE", "LICENSE_SCOPE.md", "THIRD_PARTY_NOTICES.md", "UPSTREAM.md"):
            shutil.copy2(ROOT / name, notices / name)
        shutil.copy2(ROOT / "third_party/BSD-3-Clause.txt", notices / "BSD-3-Clause.txt")
        files = {str(path.relative_to(handoff)): {"size": path.stat().st_size, "sha256": sha256(path)} for path in sorted(handoff.rglob("*")) if path.is_file()}
        manifest = {"format": FORMAT, "api_version": API_VERSION, "game_revision": "USA960620", "source_id": identity, "members_sha256": inputs, "files": files}
        (handoff / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        handoff.rename(destination)
    print("Decomp handoff ready: " + str(destination))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    inputs = parser.add_mutually_exclusive_group(required=True)
    inputs.add_argument("--rom", type=Path)
    inputs.add_argument("--rom-dir", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="Shared output directory; writes its decomp/ child")
    parser.add_argument("--cxx", default=os.environ.get("CXX", "c++"))
    args = parser.parse_args()
    try:
        run(args)
    except (ValueError, KeyError, OSError, BadZipFile, subprocess.CalledProcessError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
