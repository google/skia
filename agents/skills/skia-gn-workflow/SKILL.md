---
name: skia-gn-workflow
description: MANDATORY: Use this skill for ANY and ALL tasks involving building Skia with GN, Ninja, or compiling any Skia tool. It is the authoritative source for Skia's build workflows and MUST be used whenever compilation is required.
---

# skia-gn-workflow

This skill provides expert guidance for building and testing Skia using the GN build system and Ninja. It covers common workflows for correctness testing (`dm`), performance benchmarking (`nanobench`), and interactive visualization (`viewer`).

## Core Principles
- **Hermeticity**: Always use `./bin/gn` for configuration and `./bin/fetch-ninja` to ensure the correct version of Ninja is available.
- **Verification**: Always build and run tests/benchmarks before documenting or finalizing changes.
- **Filtering**: Use the `--match` flag (aliased as `-m`) to target specific tests and reduce execution time.
- **Build Directories**: **NEVER** override a user's existing build directory settings (e.g. `out/Debug`). If you need a different configuration, create a new directory (e.g. `out/Debug2`, `out/ASAN`). Only modify settings in an existing directory if explicitly asked by the user.
- **Toolchain Paths**: When a build requires specific toolchain paths (e.g. `cc`, `cxx`, or `clang_win` for Clang/LLVM), **ALWAYS ask the user** where these are extracted on their machine. Different developers have these in different locations.

## Setup & Troubleshooting

### 1. Syncing Dependencies
If you encounter compilation errors in `third_party` or missing headers, run:
```bash
python3 tools/git-sync-deps
```

### 2. Fetching Ninja
If the `ninja` command is not found or you want to ensure a hermetic version:
```bash
./bin/fetch-ninja
```

### 3. Formatting Code
Near the end of your workflow, before uploading a CL or finalizing changes, format your code using:
```bash
git clang-format origin/main
```

### 4. Cleaning Builds
If the build system gets into a broken state, clean the directory:
```bash
./bin/gn -q clean out/Dir
```

### 5. Inspecting the Build
To understand how a target is being built or to find available targets:
- **List all targets**: `./bin/gn ls out/Dir` or `ninja -C out/Dir -t targets`
- **Inspect a target**: `./bin/gn desc out/Dir //:dm` (shows flags, defines, sources, etc.)
- **Check why a file is included**: `./bin/gn path out/Dir //:dm //src/core/SkPath.cpp`

### 6. Rust Dependencies (Bazel)
If a build requires Rust dependencies (in `third_party`), use `bazelisk` to manage the toolchain:
- Download: https://github.com/bazelbuild/bazelisk/releases

## Build Workflows

### 1. Common Build Arguments
- `is_debug=false`: Create an optimized Release build.
- `is_official_build=true`: Optimized build, links against system libraries. Suitable for shipping.
- `is_component_build=true`: Build Skia as a shared library.
- `sanitize="ASAN"|"MSAN"|"TSAN"|"UBSAN"`: Enable memory, address, thread, or undefined behavior sanitizers.
- `extra_cflags=["..."]`, `extra_ldflags=["..."]`: Add custom compiler/linker flags.
- `cc="clang" cxx="clang++"`: Specify the compiler (highly recommended for performance).

### 2. Sanitizer Builds (`xsan`)
- **ASAN**: `./bin/gn gen -q out/ASAN --args='sanitize="ASAN" cc="<path_to_clang>" cxx="<path_to_clang++>"'`
- **MSAN**: `./bin/gn gen -q out/MSAN --args='sanitize="MSAN" cc="<path_to_clang>" cxx="<path_to_clang++>" skia_use_fontconfig=false'`
  - *Note*: Run `dm` with `--nogpu` under MSAN to avoid driver noise.
- **TSAN**: `./bin/gn gen -q out/TSAN --args='sanitize="TSAN" is_debug=false cc="<path_to_clang>" cxx="<path_to_clang++>"'`

### 3. Platform Specifics
- **Android**: `./bin/gn gen -q out/android --args='ndk="~/ndk" target_cpu="arm64"'`
- **iOS**: `./bin/gn gen -q out/ios --args='target_os="ios" target_cpu="arm64"'`
- **Windows**: Use `clang_win="<path_to_llvm>"` to build with clang-cl (highly recommended).

## Tool Reference

### `dm` (Correctness Testing)
`dm` runs GMs (graphics tests), unit tests, and compares images.
- **Common Flags**:
  - `--src <types>`: e.g., `--src tests gm skp image`.
  - `--config <configs>`: e.g., `--config 8888 gl`.
  - `--match` / `-m`: Case-sensitive substring filter. Supports `~` (exclude), `^` (start), `$` (end).
  - `--skip <config> <src> <srcOptions> <name>`: Skip specific test cases. Use `_` for wildcards.
  - `--writePath` / `-w`: Write result bitmaps as PNGs.
  - `--readPath` / `-r`: Read reference bitmaps for comparison.
  - `--threads` / `-j`: Number of threads (default: one per core).
  - `--verbose` / `-v`: Detailed output.
- **Extended Help**: For detailed documentation on specific flags, use `--help <flag_name>`.
  - Example: `out/Debug/dm --help config`.

### `nanobench` (Performance Benchmarking)
**Always use a Release build for benchmarking.**
- **Common Flags**:
  - `--samples <N>`: Number of samples per benchmark (default 10).
  - `--ms <time>`: Run each benchmark for at least `<time>` milliseconds.
  - `--cpuprofile <file>`: Write pprof CPU profile (requires `skia_use_pprof=true`).
  - `--match` / `-m`: Filter benchmarks.
- **Baseline Preservation**: It is recommended to copy a stable baseline binary to `nanobench_control` (outside the `out/` directory) to compare against a new build.

### `viewer` (Interactive Visualization)
- **Common Flags**:
  - `--backend` / `-b`: `sw`, `gl`, `vulkan`, `metal`, `d3d`.
  - `--slide <name>`: Jump to a specific GM or SKP.
  - `--match` / `-m`: Filter slides.
- **Hotkeys**: `i` (HUD), `z` (zoom), `[`/`]` (navigation).

## Advanced: "Via" Configs
`dm` supports "via" configs in the format `[via-]*backend`. These wrap a rendering sink to test additional logic.
- **Examples**:
  - `serialize-8888`: Serialize and then deserialize the canvas before drawing.
  - `srgb-8888`: Run the 8888 sink in the sRGB color space.
- **Common Vias**:
  - `serialize`: Serializes/Deserializes drawing.
  - `pic`: Records to `SkPicture` and plays back.
  - `rtblend`: Uses a runtime blend mode.
  - `matrix`: Applies a 2x2 matrix (requires `--matrix "s0 s1 s2 s3"`).
  - `upright`: Applies a matrix and then uprights it.
- **Color Space Vias**: `srgb`, `linear`, `p3`, `rec2020`, `narrow`.

## Writing Tests
- **Unit Tests**: Use `DEF_TEST(Name, reporter)` in `tests/`.
- **GMs**: Use `DEF_GM(return new MyGM;)` in `gm/`.

### 6. Debugging Fuzz Cases & Writing Reproduction Tests
To build and run a fuzzer (like Clusterfuzz or oss-fuzz) to reproduce a crash:
```bash
ninja --quiet -C out/Debug fuzz
out/Debug/fuzz --bytes <path_to_testcase>
```
For the full workflow (including writing reproduction unit tests), see [Fuzz Debugging](references/fuzz.md).

## Available Resources
- [Build Arguments](references/build_args.md)
- [dm Usage](references/dm_usage.md)
- [nanobench Usage](references/nanobench_usage.md)
- [viewer Usage](references/viewer_usage.md)
- [Testing](references/testing.md)
- [GMs](references/gms.md)
- [Fuzz Debugging](references/fuzz.md)
- [Other Tools](references/other_tools.md)
