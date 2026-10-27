# Skia `dm` Tool Usage

`dm` is Skia's primary correctness testing tool. It runs GMs (graphics tests), unit tests, and compares images/SKP/SVG sources against expected outputs.

## Basic Usage
```bash
out/Debug/dm --match <pattern>
```

## Common Flags

### Matching/Filtering
- `--match` (or `-m`): Filter tests by name. Supports:
  - `substring`: Matches any part of the name.
  - `~exclude`: Skips matching names.
  - `^start`: Matches start of the name.
  - `$end`: Matches end of the name.
  - `^exact$`: Exact match.
- `--config <names>`: Configs to run (e.g., `8888`, `gl`, `vk`).
- `--src <names>`: Source types to run. Common sources:
    - `tests`: C++ unit tests.
    - `gm`: Graphics tests.
    - `skp`: Test SKP files. Requires `--skps <dir>`.
    - `image`: Test image files. Requires `--images <dir>`.
- `--skip <config> <src> <srcOptions> <name>`: Space-separated quadruples to skip. Use `_` as a wildcard.

### Verification
- `--resourcePath` (or `-i`): Directory with test resources (images, fonts, etc.). **ALWAYS verify this path (default: `resources`) if tests fail to load data.**
- `-w <dir>`: Write output images to `<dir>`.
- `-r <dir>`: Read reference images from `<dir>` for comparison. Fail if they don't match.
- `-v`: Verbose output.
- `--verbose`: Extra verbose logging.

### Execution Control
- `--threads` (or `-j`): Number of worker threads (default is CPU count).
- `--quiet` (or `-q`): Minimal output.
- `--nogpu`: Skip all GPU-bound work. Useful for MSAN.

## Extended Help
For detailed documentation on complex flags, pass the flag name to `--help`:
```bash
out/Debug/dm --help config
```
This is the best way to see all supported predefined configs and "via" wrappers.

## Advanced: "Via" Configs
"Via" configs wrap a sink in another sink to test specific features. The format is `[via-]*backend`.
Common Vias:
- `serialize`: Serialize and then deserialize the canvas before drawing. (e.g., `serialize-8888`)
- `pic`: Record to an `SkPicture` and then play back.
- `rtblend`: Use a runtime blend mode.
- `matrix`: Apply a 2x2 matrix (requires `--matrix "s0 s1 s2 s3"`).
- `upright`: Apply a matrix and then upright it.

Color Space Vias:
- `srgb`, `linear`, `p3`, `rec2020`, `narrow`: Run the sink in a specific color space. (e.g., `srgb-8888`)

## Example Workflows

### Run all unit tests matching "String"
```bash
out/Debug/dm --src tests --match String
```

### Run a specific GM in 8888 and GPU
```bash
out/Debug/dm --src gm --config 8888 gl --match alphagradients
```

### Test serialization of a GM
```bash
out/Debug/dm --src gm --config serialize-8888 --match Gradient
```
