// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `png` Rust crate.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

use std::io::Read;
use std::pin::Pin;

// No `use png::...` nor `use ffi::...` because we want the code to explicitly
// spell out if it means `ffi::ColorType` vs `png::ColorType`.

#[cxx::bridge(namespace = "rust_png")]
mod ffi {
    /// FFI-friendly equivalent of `png::ColorType`.
    enum ColorType {
        Grayscale = 0,
        Rgb = 2,
        Indexed = 3,
        GrayscaleAlpha = 4,
        Rgba = 6,
    }

    /// Result of decoding a PNG image.
    ///
    /// TODO(https://crbug.com/356878144): Expose `png::Reader` instead of
    /// returning a `DecodedImage`.
    struct DecodedImage {
        width: u32,
        height: u32,
        color: ColorType,
        bits_per_component: u8,

        // TODO(https://crbug.com/357876243): Avoid an extra buffer if possible.
        data: Vec<u8>,
    }

    unsafe extern "C++" {
        include!("experimental/rust_png/ffi/FFI.h");
        type ReadTrait;
        fn read(self: Pin<&mut ReadTrait>, buffer: &mut [u8]) -> usize;
    }

    extern "Rust" {
        fn read_png(input: UniquePtr<ReadTrait>) -> UniquePtr<DecodedImage>;
    }
}

impl From<png::ColorType> for ffi::ColorType {
    fn from(value: png::ColorType) -> Self {
        match value {
            png::ColorType::Grayscale => Self::Grayscale,
            png::ColorType::Rgb => Self::Rgb,
            png::ColorType::Indexed => Self::Indexed,
            png::ColorType::GrayscaleAlpha => Self::GrayscaleAlpha,
            png::ColorType::Rgba => Self::Rgba,
        }
    }
}

impl Read for Pin<&mut ffi::ReadTrait> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        Ok(self.as_mut().read(buf))
    }
}

/// Internal helper that `read_png` is build on top of.
fn read_png_and_return_result(
    input: Pin<&mut ffi::ReadTrait>,
) -> Result<ffi::DecodedImage, png::DecodingError> {
    // Note that the default `png::Limits::bytes` is 64MiB (based on
    // https://docs.rs/png/latest/png/struct.Limits.html#structfield.bytes).
    let mut decoder = png::Decoder::new(input);

    // `EXPAND` will:
    // * Expand bit depth to at least 8 bits
    // * Translate palette indices into RGB or RGBA
    //
    // TODO(https://crbug.com/356882657): Consider handling palette expansion
    // via `SkSwizzler` instead of relying on `EXPAND` for this use case.
    decoder.set_transformations(png::Transformations::EXPAND);

    let mut reader = decoder.read_info()?;

    // TODO(https://crbug.com/356923435): Refactor to use `next_row` instead of
    // `next_frame` to support incremental, row-by-row decoding.
    let mut output = vec![0; reader.output_buffer_size()];
    let info = reader.next_frame(&mut output)?;

    let color = info.color_type.into();
    let width = info.width;
    let height = info.height;
    let bits_per_component = info.bit_depth as u8;

    Ok(ffi::DecodedImage { width, height, color, bits_per_component, data: output })
}

/// Public API that C++ can use to decode a PNG image.
fn read_png(mut input: cxx::UniquePtr<ffi::ReadTrait>) -> cxx::UniquePtr<ffi::DecodedImage> {
    match read_png_and_return_result(input.pin_mut()) {
        Ok(image) => cxx::UniquePtr::new(image),

        // TODO(https://crbug.com/356878144): Translate `png::DecodingError`
        // into roughly equivalent `SkCodec::Result`.
        Err(_) => cxx::UniquePtr::null(),
    }
}
