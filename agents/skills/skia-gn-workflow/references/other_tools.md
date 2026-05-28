# Other Skia Developer Tools

Beyond `dm`, `nanobench`, and `viewer`, Skia provides several other utilities for working with SKPs, images, and modules like Skottie.

## SKP Utilities

### `skpinfo`
Prints information about an SKP file (version, bounds, etc.).
```bash
out/Debug/skpinfo -i /path/to/file.skp
```

### `dump_record`
Dumps the drawing commands recorded in an SKP or other serialized format.
```bash
out/Debug/dump_record /path/to/file.skp
```

## Image Utilities

### `skdiff`
Compares two images or directories of images and produces a diff.
```bash
out/Debug/skdiff <baseDir> <comparisonDir> [outputDir]
```

## Module-Specific Tools

### `skottie_tool`
A CLI tool for rendering and inspecting Skottie (Lottie) animations.
```bash
out/Debug/skottie_tool --input <animation.json> --writePath <outputDir>
```

### `svg_tool`
A CLI tool for rendering SVG files using Skia's SVG module.
```bash
out/Debug/svg_tool --input <file.svg> --output <file.png>
```

## Fuzzing

### `fuzz`
A tool for running various Skia fuzzers on local inputs.
```bash
# Run the path_deserialize fuzzer on a file
out/Debug/fuzz -t path_deserialize -b /path/to/fuzz/bytes
```
Use `out/Debug/fuzz --help` to see all valid types.
