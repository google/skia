# Skia `dm` Tool Usage

`dm` is Skia's primary correctness testing tool. It runs GMs (graphics tests), unit tests, and compares image/SKP/SVG sources against expected outputs.

## Basic Usage
```bash
out/Debug/dm --match <pattern>
```

## Common Flags

### Matching/Filtering
- `--match <pattern>`: Case-insensitive glob on test/bench names. Multiple patterns can be separated by spaces. Use `~` for negative matches (e.g., `~skp`).
- `--config <names>`: Configs to run (e.g., `8888`, `gpu`, `vk`). Default is `8888`.
- `--src <names>`: Source types to run. Common sources:
    - `tests`: C++ unit tests (from `tests/`).
    - `gm`: Graphics tests (from `gm/`).
    - `skp`: Test SKP files. Requires `--skps <dir>`.
    - `image`: Test image files. Requires `--images <dir>`.

### Verification
- `-w <dir>`: Write output images to `<dir>`. Useful for inspecting failures.
- `-r <dir>`: Read reference images from `<dir>` for comparison.
- `-v`: Verbose output.
- `--verbose`: Extra verbose logging.

### Execution Control
- `--threads <n>`: Number of worker threads (default is CPU count).
- `--quiet` (or `-q`): Minimal output.

## Example Workflows

### Run all unit tests matching "String"
```bash
out/Debug/dm --src tests --match String
```

### Run a specific GM in 8888 and GPU
```bash
out/Debug/dm --src gm --config 8888 gpu --match alphagradients
```

### Debug failing GMs matching a pattern
```bash
out/Debug/dm --src gm --match gradient -w /tmp/output -r /path/to/expected
```
