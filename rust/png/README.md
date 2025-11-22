# Rust PNG decoder

This directory contains helpers, simplifications, and wrappers for exposing the
`png` Rust crate to other Skia code:

* to `SkPngRustCodec`
* to `SkPngRustEncoderImpl`

See the following document for more details:
https://docs.google.com/document/d/1glx5ue5JDlCld5WzWgTOGK3wsMErQFnkY5N5Dsbi91Y/edit?usp=sharing

## Chromium build instructions

To build and test the code from this directory from Chromium:

1. `autoninja -C out/... gfx_unittests blink_platform_unittests chrome`
1. `out/.../gfx_unittests --gtest_filter=*PNG*`
1. `out/.../blink_platform_unittests --gtest_filter=*PNG*`

## Skia build instructions

### Bazel

To build the code from this directory from Skia (testing via Bazel is not
supported yet):

```
$ cd skia-repo-root
$ bazelisk build //src/codec:rust_png_decoder //src/encode:rust_png_encoder rust/png/...
```

### gn / ninja

To build the code from this directory from Skia:

1. `gn args out/RustPng` and set `skia_use_rust_png_decode = true`
   as well as `skia_use_rust_png_encode = true`
1. `gn gen out/RustPng`
1. `autoninja -C out/RustPng dm`
```

To test the code (via `tests/SkPngRustDecoderTest.cpp` and
`tests/SkPngRustEncoderTest.cpp`):

```
$ out/RustPng/dm --src tests --nogpu \
    --match RustPngCodec \
            RustEncodePng
```

TODO(https://crbug.com/356875275): Add support for running older tests
(e.g. ones from `tests/CodecTest.cpp`) against `SkPngRustCodec`.

## Differences between `SkPngCodec` and `SkRustPngCodec`

* `SkPngCodec`:
    - No APNG support.
    - No CICP support.
* `SkPngRustCodec` differences - see
  https://issues.chromium.org/issues?q=parentid:362829876%2B

## Differences between `SkPngEncoder` and `SkPngRustEncoder`

* `SkPngRustEncoder` differences - see
  https://issues.chromium.org/issues?q=parentid:381140294%2B
