# Skia `viewer` Tool Usage

`viewer` is an interactive tool for visualizing GMs, SKPs, and SVGs. It provides a GUI for inspecting drawing results and shader code.

## Basic Usage
```bash
out/Debug/viewer --match <pattern>
```

## Common Flags

### Matching/Filtering
- `--match <pattern>`: Case-insensitive glob on test names.
- `--config <name>`: Start with a specific config (e.g., `8888`, `gpu`, `vk`).
- `--skps <dir>`: Directory to read SKP files.
- `--svgs <dir>`: Directory to read SVG files.

### Window/UI
- `--width <n>`, `--height <n>`: Initial window dimensions.
- `--notitle`: Disable the window title bar.

## Interactive Controls (Keyboard)
- `[` / `]`: Previous/Next slide.
- `b`: Cycle backends (8888, GPU, etc.).
- `m`: Toggle MSAA.
- `i`: Show/Hide ImGui overlay (HUD).
- `z`: Toggle zoom window.
- `h`: Toggle color histogram.
- `s`: Save current slide to PNG.

## Shader Inspection (ImGui HUD)
1. Press `i` to show the HUD.
2. Go to the **Shaders** window.
3. You can view the SkSL, GLSL/SPIR-V, and even **apply changes** to SkSL in real-time to debug shader issues.
4. Use **Dump SkSL** to save generated shaders to `resources/sksl/`.

## Example Workflows

### View a specific GM
```bash
out/Debug/viewer --match alphagradients
```

### Inspect an SKP file
```bash
out/Debug/viewer --skps /path/to/skp --match <file_name_without_extension>
```
