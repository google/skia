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

    // Rust functions, types, and methods that are exposed through FFI.
    //
    // To avoid duplication, there are no doc comments inside the `extern "Rust"`
    // section. The doc comments of these items can instead be found in the
    // actual Rust code, outside of the `#[cxx::bridge]` manifest.
    extern "Rust" {
        fn new_reader(input: UniquePtr<ReadTrait>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn height(self: &Reader) -> u32;
        fn width(self: &Reader) -> u32;
        fn is_srgb(self: &Reader) -> bool;
        fn try_get_chrm(
            self: &Reader,
            wx: &mut f32,
            wy: &mut f32,
            rx: &mut f32,
            ry: &mut f32,
            gx: &mut f32,
            gy: &mut f32,
            bx: &mut f32,
            by: &mut f32,
        ) -> bool;
        fn try_get_gama(self: &Reader, gamma: &mut f32) -> bool;
        unsafe fn try_get_iccp<'a>(self: &'a Reader, iccp: &mut &'a [u8]) -> bool;
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
        let mut transformations = png::Transformations::EXPAND;

        // TODO(https://crbug.com/359245096): Avoid stripping least signinficant 8 bits in G16 and
        // GA16 images.
        let info = decoder.read_header_info()?;
        if info.bit_depth == png::BitDepth::Sixteen {
            match info.color_type {
                png::ColorType::Grayscale | png::ColorType::GrayscaleAlpha => {
                    transformations = transformations | png::Transformations::STRIP_16;
                }
                png::ColorType::Rgb | png::ColorType::Rgba => (),
                // PNG says that the only allowed bit depths for color type 3 (indexed)
                // are 1,2,4,8.
                png::ColorType::Indexed => unreachable!(),
            }
        }

        decoder.set_transformations(transformations);
        Ok(Self(decoder.read_info()?))
    }

    fn height(&self) -> u32 {
        self.0.info().height
    }

    fn width(&self) -> u32 {
        self.0.info().width
    }

    /// Returns whether the decoded PNG image contained a `sRGB` chunk.
    fn is_srgb(&self) -> bool {
        self.0.info().srgb.is_some()
    }

    /// If the decoded PNG image contained a `cHRM` chunk then `try_get_chrm`
    /// returns `true` and populates the out parameters (`wx`, `wy`, `rx`,
    /// etc.).  Otherwise, returns `false`.
    ///
    /// C++/FFI safety: The caller has to guarantee that all the outputs /
    /// `&mut` values have been initialized (unlike in C++, where such
    /// guarantees are typically not needed for output parameters).
    fn try_get_chrm(
        &self,
        wx: &mut f32,
        wy: &mut f32,
        rx: &mut f32,
        ry: &mut f32,
        gx: &mut f32,
        gy: &mut f32,
        bx: &mut f32,
        by: &mut f32,
    ) -> bool {
        fn copy_channel(channel: &(png::ScaledFloat, png::ScaledFloat), x: &mut f32, y: &mut f32) {
            *x = channel.0.into_value();
            *y = channel.1.into_value();
        }

        match self.0.info().chrm_chunk.as_ref() {
            None => false,
            Some(chrm) => {
                copy_channel(&chrm.white, wx, wy);
                copy_channel(&chrm.red, rx, ry);
                copy_channel(&chrm.green, gx, gy);
                copy_channel(&chrm.blue, bx, by);
                true
            }
        }
    }

    /// If the decoded PNG image contained a `gAMA` chunk then `try_get_gama`
    /// returns `true` and populates the `gamma` out parameter.  Otherwise,
    /// returns `false`.
    ///
    /// C++/FFI safety: The caller has to guarantee that all the outputs /
    /// `&mut` values have been initialized (unlike in C++, where such
    /// guarantees are typically not needed for output parameters).
    fn try_get_gama(&self, gamma: &mut f32) -> bool {
        match self.0.info().gama_chunk.as_ref() {
            None => false,
            Some(scaled_float) => {
                *gamma = scaled_float.into_value();
                true
            }
        }
    }

    /// If the decoded PNG image contained an `iCCP` chunk then `try_get_iccp`
    /// returns `true` and sets `iccp` to the `rust::Slice`.  Otherwise,
    /// returns `false`.
    fn try_get_iccp<'a>(&'a self, iccp: &mut &'a [u8]) -> bool {
        match self.0.info().icc_profile.as_ref().map(|cow| cow.as_ref()) {
            None => false,
            Some(value) => {
                *iccp = value;
                true
            }
        }
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
