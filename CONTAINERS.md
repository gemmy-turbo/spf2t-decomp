# Optional Docker/Podman environment

Build the tooling image from this repository:

```sh
docker build -t spf2t-decomp:local .
```

From a directory containing `roms/`, create the output directory and run:

```sh
mkdir -p output
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/roms:/roms:ro" -v "$PWD/output:/output" spf2t-decomp:local
```

The image supplies Python and the compiler, reads `/roms` and writes `/output`. Supply supported complete
ZIPs, split ZIPs or extracted members in `roms/`. The result is
`output/decomp/`, including everything needed by the native compilation process.
The complete handoff remains outside the image. ROMs/keys/output are excluded
from the image build context.

Build the native-only image from the native repository as described there.
Then, without changing directories or mounts, run the same command with the
native image name:

```sh
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/roms:/roms:ro" -v "$PWD/output:/output" spf2t-native:local
```

The game will be in `output/spf2t-native/`; run its executable on the host.
The native image consumes `output/decomp/` and does not access ROMs itself.
Alternatively use the native project's AIO image to run both processes at once.

The user flag keeps output owned by your host account. With rootless Docker,
omit `--user` when container root already maps to your host account. For Podman,
replace `docker` with `podman` and add `--userns=keep-id` to the run command.
On SELinux systems use appropriate bind-mount labels for your dedicated input/
output directories. Podman is documented but not executed in this environment.

Image construction downloads packages. The normal import needs no network.
Base identities are pinned; apt resolution is not snapshot-pinned, so independent
image rebuilds are not claimed byte-identical. An image digest identifies an exact image.
