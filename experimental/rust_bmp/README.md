# Rust BMP decoder

This directory contains helpers, simplifications, and wrappers for exposing a
custom Rust BMP decoder to other Skia code:

* to `SkBmpRustCodec`

## Chromium build instructions

To build and test the code from this directory from Chromium:

1. `autoninja -C out/... gfx_unittests blink_platform_unittests chrome`
1. `out/.../gfx_unittests --gtest_filter=*BMP*`
1. `out/.../blink_platform_unittests --gtest_filter=*BMP*`

## Skia build instructions

### Bazel

To build the code from this directory from Skia:

```
$ cd skia-repo-root
$ bazelisk build //src/codec:rust_bmp_decoder //experimental/rust_bmp/...
```

To run unit tests:

```
$ bazelisk test //experimental/rust_bmp/ffi:test_bmp_ffi
```

To build the fuzzer:

```
$ bazelisk build //experimental/rust_bmp/ffi:fuzz_rust_bmp
```

To run the fuzzer with the corpus:

```
$ for bmp in experimental/rust_bmp/fuzz/corpus/*.bmp; do
    cat "$bmp" | bazel-bin/experimental/rust_bmp/ffi/fuzz_rust_bmp
  done
```

See `validation_artifacts/TESTING.md` for detailed test documentation and `validation_artifacts/FUZZING.md` for fuzzing documentation.

### gn / ninja

To build the code from this directory from Skia:

1. `gn args out/RustBmp` and set `skia_use_rust_bmp_decode = true`
1. `gn gen out/RustBmp`
1. `autoninja -C out/RustBmp dm`
```

To test the code:

```
$ out/RustBmp/dm --src tests --nogpu \
    --match Codec_bmp
```

## Features

### **Complete Format Support**

- **Bit Depths**: 1-bit, 4-bit, 8-bit, 16-bit, 24-bit, 32-bit
- **Compression**: Uncompressed (BI_RGB), RLE4, RLE8, Bitfields (BI_BITFIELDS)
- **Advanced**: OS/2 bitmap variants
- **Color Spaces**: sRGB, embedded ICC profiles (when available)

### **Security & Robustness**

- **Memory Safety**: Complete overflow protection with u64 arithmetic
- **Input Validation**: Comprehensive header and stream validation
- **Corruption Detection**: Advanced detection of malformed files
- **Standards Compliance**: 100% conformance with bmptestsuite-0.9

### **Performance & Integration**

- **Zero-Copy**: Efficient memory handling with minimal allocations
- **FFI Optimized**: Clean Rust-to-C++ interface via CXX bridge
- **Conditional Build**: Integrated with Skia's build system

## Architecture

```
experimental/rust_bmp/
├── ffi/                              # Rust implementation core
│   ├── lib.rs                        # Crate root
│   ├── FFI.rs                        # C++ interface bridge
│   ├── bmp_decoder.rs                # Main BMP decoding logic
│   ├── bmp_header.rs                 # BMP header parsing and validation
│   ├── bmp_constants.rs              # Format constants and definitions
│   ├── bmp_types.rs                  # Type definitions
│   ├── bmp_icc.rs                    # ICC profile support through moxcms crate
│   ├── bmp_jpeg_decoder.rs           # Embedded JPEG handling through zune-jpeg crate
│   ├── bmp_png_decoder.rs            # Embedded PNG handling through png crate
│   └── BUILD.bazel                   # Bazel build configuration
├── decoder/                          # C++ integration layer
│   ├── SkBmpRustDecoder.h/.cpp       # Skia SkCodec factory
│   ├── impl/
│   │   └── SkBmpRustCodec.h/.cpp     # Core codec implementation
│   └── BUILD.bazel                   # Bazel build configuration
└── README.md                         # This file
```

## Differences between `SkBmpCodec` and `SkBmpRustCodec`

* `SkBmpCodec`:
    - C++ implementation with manual memory management
    - Legacy codebase with accumulated technical debt
* `SkBmpRustCodec` differences:
    - Memory-safe Rust implementation
    - Comprehensive overflow protection with u64 arithmetic
    - Enhanced corruption detection for malformed files
    - Improved BMP standards compliance