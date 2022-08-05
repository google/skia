piet-gpu Utilities
==================

This directory provides utilities to ease integrating Skia C++ code with the piet-gpu/pgpu-render
Rust crate's C FFI bindings.

### Building the piet-gpu library

The code depends on a third-party Rust library which must be compiled manually:

1. First make sure that a recent version of `cargo` is installed on your system. Simply follow the
   instructions on https://doc.rust-lang.org/cargo/getting-started/installation.html to get started.

2. Use the Makefile under `//third_party/piet-gpu` to compile the library:
```
$ cd $SKIA_ROOT/third_party/piet-gpu
$ make debug
```
This should create `$SKIA_ROOT/third_party/piet-gpu/out/debug/libpgpu_render.dylib` if the build is
successful. For a release build, run `make release` instead.

### Building Skia with piet support

Build rules are currently only provided for the GN build. To enable piet support, add
`skia_use_piet=true` to your GN args. This will enable both the Skia GN targets and define the
`SK_ENABLE_PIET_GPU` macro which can be used in C++ code to query for support.
