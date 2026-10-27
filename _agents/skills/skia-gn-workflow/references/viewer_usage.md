# Skia `viewer` Tool Usage

`viewer` is an interactive tool for visualizing GMs, SKPs, and SVGs. It provides a GUI for inspecting drawing results and shader code.

## Basic Usage
```bash
out/Debug/viewer --match <pattern>
```

## Common Flags

### Matching/Filtering
- `--match` (or `-m`): Filter slides by name (case-sensitive substring).
- `--slide <name>`: Start on a specific slide.
- `--backend` (or `-b`): Backend to use: `sw` (default), `gl`, `vulkan`, `metal`, `d3d`.
- `--skps <dir>`: Directory to read SKP files.
- `--svgs <dir>`: Directory to read SVG files.

### Window/UI
- `--resourcePath` (or `-i`): Path to resources. **Ensure this is correct if assets fail to load.**
- `--width <n>`, `--height <n>`: Initial window dimensions.
- `--stats`: Show stats overlay on startup.
- `--redraw`: Toggle continuous redraw.

## Interactive Controls (Keyboard)
- `[` / `]`: Previous/Next slide.
- `b`: Cycle backends (if multiple are available).
- `m`: Toggle MSAA.
- `i`: Show/Hide ImGui overlay (HUD).
- `z`: Toggle zoom window.
- `h`: Toggle color histogram.
- `s`: Save current slide to PNG.

## Shader Inspection (ImGui HUD)
1. Press `i` to show the HUD.
2. Go to the **Shaders** window.
3. You can view the SkSL, GLSL/SPIR-V, and even **apply changes** to SkSL in real-time to debug shader issues.

## Example Workflows

### View a specific GM
```bash
out/Debug/viewer --match alphagradients
```

### Inspect an SKP file
```bash
out/Debug/viewer --skps /path/to/skp --match <file_name_without_extension>
```
