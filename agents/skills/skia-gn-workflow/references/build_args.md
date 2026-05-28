# Skia GN Build Configurations

This reference documents common GN argument sets used for building Skia. Use these with `./bin/gn gen out/<dir> --args='<args>'`.

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

## Sanitizers
Requires a recent Clang. See `site/docs/dev/testing/xsan.md`.

### ASAN (Address Sanitizer)
```gn
sanitize = "ASAN"
```

### MSAN (Memory Sanitizer)
Requires an MSAN-instrumented libc++.
```gn
sanitize = "MSAN"
skia_use_fontconfig = false
```

### TSAN (Thread Sanitizer)
```gn
sanitize = "TSAN"
is_debug = false
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
```

## Feature Toggles
Commonly used flags from `gn/skia.gni`:
- `skia_enable_ganesh = true/false` (Default: true)
- `skia_enable_graphite = true/false` (Default: false)
- `skia_use_vulkan = true/false`
- `skia_use_metal = true/false`
- `skia_use_dawn = true/false`
- `skia_enable_skottie = true/false`
