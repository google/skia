# Skia GN Build Configurations

This reference documents common GN argument sets used for building Skia. Use these with `./bin/gn gen out/<dir> --args='<args>'`.

## Core Principle: Build Directories
**NEVER** override a user's existing build directory settings (e.g., `out/Debug`). If you need a different configuration, create a new directory (e.g., `out/Debug2`, `out/ASAN`). Only modify settings in an existing directory if explicitly asked by the user.

## Core Principle: Toolchain Paths
When using arguments like `cc`, `cxx`, or `clang_win`, **ALWAYS ask the user** where the clang/LLVM toolchain is extracted on their system. Locations vary significantly between developers.

## Base Configurations

### Debug (Default)
Optimizations off, symbols on, assertions enabled.
```gn
is_debug = true
```

### Release
Optimizations on, symbols off (usually), assertions disabled.
```gn
is_debug = false
```

### Official Build
Optimized build, links against system libraries. Suitable for shipping.
```gn
is_official_build = true
```

## Sanitizers
Requires a recent Clang. See `site/docs/dev/testing/xsan.md`.

### ASAN (Address Sanitizer)
```bash
# Replace <path_to_clang> with the path provided by the user
./bin/gn gen out/ASAN --args='sanitize="ASAN" cc="<path_to_clang>" cxx="<path_to_clang++>"'
```

### MSAN (Memory Sanitizer)
Requires an MSAN-instrumented libc++. Run `dm` with `--nogpu`.
```bash
# Replace <path_to_clang> with the path provided by the user
./bin/gn gen out/MSAN --args='sanitize="MSAN" cc="<path_to_clang>" cxx="<path_to_clang++>" skia_use_fontconfig=false'
```

### TSAN (Thread Sanitizer)
```bash
# Replace <path_to_clang> with the path provided by the user
./bin/gn gen out/TSAN --args='sanitize="TSAN" is_debug=false cc="<path_to_clang>" cxx="<path_to_clang++>"'
```

## Performance & Profiling

### Profiling with pprof
Requires `libgoogle-perftools-dev`.
```gn
skia_use_pprof = true
extra_cflags = ["-Og"] # Recommended for accurate line attribution
```

## Platform Specifics

### Android
Requires `ndk` path to be set.
```gn
target_os = "android"
ndk = "/path/to/ndk"
target_cpu = "arm64" # or "arm", "x64", "x86"
```

### iOS
```gn
target_os = "ios"
target_cpu = "arm64"
```

### Windows (Clang-cl)
Highly recommended for performance.
```gn
# Replace with the LLVM path provided by the user
clang_win = "<path_to_llvm>"
```

## Feature Toggles
Commonly used flags from `gn/skia.gni`:
- `skia_enable_ganesh = true/false` (Default: true)
- `skia_enable_graphite = true/false` (Default: false)
- `skia_use_vulkan = true/false`
- `skia_use_metal = true/false`
- `skia_use_dawn = true/false`
- `skia_enable_skottie = true/false`
