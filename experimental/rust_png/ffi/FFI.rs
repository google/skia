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
// spell out if it means `ffi::ColorType` vs `png::ColorType` (or `Reader`
// vs `png::Reader`).

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

    /// FFI-friendly simplification of `Option<png::DecodingError>`.
    enum DecodingResult {
        Success,
        FormatError,
        ParameterError,
        LimitsExceededError,
        // `ReadTrait` is infallible and therefore we expect no `png::DecodingError::IoError`
        // and provide no equivalent of this variant.

        // TODO(https://crbug.com/356923435): Add `IncompleteInput`.
    }

    unsafe extern "C++" {
        include!("experimental/rust_png/ffi/FFI.h");
        type ReadTrait;
        fn read(self: Pin<&mut ReadTrait>, buffer: &mut [u8]) -> usize;
    }

    extern "Rust" {
        fn new_reader(input: UniquePtr<ReadTrait>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn height(self: &Reader) -> u32;
        fn width(self: &Reader) -> u32;
        fn output_buffer_size(self: &Reader) -> usize;
        fn output_color_type(self: &Reader) -> ColorType;
        fn output_bits_per_component(self: &Reader) -> u8;
        fn next_frame(self: &mut Reader, output: &mut [u8]) -> DecodingResult;
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

impl From<Option<&png::DecodingError>> for ffi::DecodingResult {
    fn from(option: Option<&png::DecodingError>) -> Self {
        match option {
            None => Self::Success,
            Some(decoding_error) => match decoding_error {
                png::DecodingError::IoError(_) => {
                    // `ReadTrait` is infallible => we expect no `png::DecodingError::IoError`.
                    unreachable!()
                }
                png::DecodingError::Format(_) => Self::FormatError,
                png::DecodingError::Parameter(_) => Self::ParameterError,
                png::DecodingError::LimitsExceeded => Self::LimitsExceededError,
            },
        }
    }
}

impl<'a> Read for Pin<&'a mut ffi::ReadTrait> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        Ok(self.as_mut().read(buf))
    }
}

/// FFI-friendly wrapper around `Result<T, E>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct ResultOfReader(Result<Reader, png::DecodingError>);

impl ResultOfReader {
    fn err(&self) -> ffi::DecodingResult {
        self.0.as_ref().err().into()
    }

    fn unwrap(&mut self) -> Box<Reader> {
        // Leaving `self` in a C++-friendly "moved-away" state.
        let mut result = Err(png::DecodingError::LimitsExceeded);
        std::mem::swap(&mut self.0, &mut result);

        Box::new(result.unwrap())
    }
}

/// FFI-friendly wrapper around `png::Reader<R>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct Reader(png::Reader<cxx::UniquePtr<ffi::ReadTrait>>);

impl Reader {
    fn new(input: cxx::UniquePtr<ffi::ReadTrait>) -> Result<Self, png::DecodingError> {
        // By default, the decoder is limited to using 64 Mib. If we ever need to change
        // that, we can use `png::Decoder::new_with_limits`.
        let mut decoder = png::Decoder::new(input);

        // `EXPAND` will:
        // * Expand bit depth to at least 8 bits
        // * Translate palette indices into RGB or RGBA
        //
        // TODO(https://crbug.com/356882657): Consider handling palette expansion
        // via `SkSwizzler` instead of relying on `EXPAND` for this use case.
        decoder.set_transformations(png::Transformations::EXPAND);

        let reader = decoder.read_info()?;
        Ok(Self(reader))
    }

    fn height(&self) -> u32 {
        self.0.info().height
    }

    fn width(&self) -> u32 {
        self.0.info().width
    }

    fn output_buffer_size(&self) -> usize {
        self.0.output_buffer_size()
    }

    fn output_color_type(&self) -> ffi::ColorType {
        self.0.output_color_type().0.into()
    }

    fn output_bits_per_component(&self) -> u8 {
        self.0.output_color_type().1 as u8
    }

    /// Decodes the next frame - see
    /// https://docs.rs/png/latest/png/struct.Reader.html#method.next_frame
    ///
    /// C++/FFI safety: The caller has to guarantee that `output` doesn't
    /// contain uninitialized memory (this is a bit different from C++,
    /// where a write-only access may not need such guarantees).
    fn next_frame(&mut self, output: &mut [u8]) -> ffi::DecodingResult {
        self.0.next_frame(output).as_ref().err().into()
    }
}

/// This provides a public C++ API for decoding a PNG image.
fn new_reader(input: cxx::UniquePtr<ffi::ReadTrait>) -> Box<ResultOfReader> {
    Box::new(ResultOfReader(Reader::new(input)))
}
