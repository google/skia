# Rust ICC profile parser

This directory contains helpers and wrappers for exposing ICC (International
Color Consortium) profile parsing from the `moxcms` Rust crate to Skia codecs:

* Used by `SkPngRustCodec` and `SkBmpRustCodec`

## Architecture

```
Image Codec → ICC bytes → rust_icc::parse_icc_profile()
                               ↓ (moxcms parsing)
                          IccProfile struct
                               ↓
                     rust_icc::ToSkcmsIccProfile()
                               ↓
                          skcms_ICCProfile
                               ↓
                     Skia color transformations
```

### Memory Safety

All ICC profile parsing happens in Rust using the `moxcms` crate. This prevents
memory safety vulnerabilities from malformed ICC data. The validated, parsed
data is then converted to `skcms_ICCProfile` structures for use by Skia's
color management system (skcms).

### Profile Support

The parser supports:
- Matrix-based profiles (RGB/Gray with toXYZD50 matrix + transfer curves)
- CICP metadata (for HDR/wide-gamut color spaces)
- Complex LUT-based profiles (A2B/B2A transforms with multi-dimensional lookup tables)

## Building

### Bazel

```
$ cd skia-repo-root
$ bazelisk build //rust/icc:ffi_rs
$ bazelisk test //rust/icc:ffi_rs_test
```

### gn / ninja

Enabled automatically when Rust codecs are enabled:

```
$ gn args out/RustEnabled
# Set: skia_use_rust_png_decode = true
$ gn gen out/RustEnabled
$ autoninja -C out/RustEnabled
```

## Testing

### Unit Tests

**Rust unit tests** (test Rust-side parsing and FFI data structures):

```bash
$ bazelisk test //rust/icc:ffi_rs_test --test_output=all
```

**C++ unit tests** (test the complete FFI bridge to skcms):

Build and run via Skia's `dm` test runner:

```bash
$ gn gen out/Debug --args='skia_use_rust_png_decode=true'
$ ninja -C out/Debug dm
$ out/Debug/dm --src tests --match RustIcc
```

The C++ tests are located in `tests/RustIccTest.cpp`.

## Dependencies

- **cxx**: FFI bridge between Rust and C++
- **moxcms** 0.7.9: ICC profile parser
- **rust/common**: Shared FFI utilities

## References

- ICC Specification: https://www.color.org/specification/ICC.1-2022-05.pdf
- moxcms: https://crates.io/crates/moxcms
- skcms: https://skia.googlesource.com/skcms/
