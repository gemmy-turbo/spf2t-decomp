#!/usr/bin/env python3
"""Run the primary process with repository-local defaults."""
import argparse
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--dry-run", action="store_true", help="Print the explicit primary command without running it")
args = parser.parse_args()
command = [sys.executable, str(root / "tools/decompile.py"), "--rom-dir", str(root / "roms"), "--output", str(root / "output")]
if args.dry_run:
    print(shlex.join(command))
else:
    sys.exit(subprocess.call(command))
