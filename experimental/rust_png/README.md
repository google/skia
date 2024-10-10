# Rust PNG decoder

This directory contains experimental code for providing `SkCodec` API
for decoding PNG images using Rust `png` crate.  See the following
document for more details:
https://docs.google.com/document/d/1glx5ue5JDlCld5WzWgTOGK3wsMErQFnkY5N5Dsbi91Y/edit?usp=sharing

## Chromium build instructions

To use and test the code from this directory from Chromium:

1. Ensure that https://crrev.com/c/4723145 and https://crrev.com/c/5747366 have
   landed or have been cherry-picked to the local repo
1. `gn args out/...` and set `enable_rust_png = true`
1. `autoninja -C out/... gfx_unittests`
1. `out/.../gfx_unittests --gtest_filter=RustEnabled*`

## Skia build instructions

TODO(https://crbug.com/356875275): Code in this directory is not yet covered by
Skia standalone builds.

## Differences between `SkPngCodec` and `SkRustPngCodec`

* `SkPngCodec`:
    - No APNG support.
* `SkPngRustCodec`:
    - No support to decode a `fSubset` of pixels (https://crbug.com/362830091).
    - `sBIT` chunk is ignored (https://crbug.com/359279096)
