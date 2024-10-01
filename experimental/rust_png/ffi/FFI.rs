// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `png` Rust crate.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

use std::io::{ErrorKind, Read};
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
        /// `IncompleteInput` is equivalent to `png::DecodingError::IoError(
        /// std::io::ErrorKind::UnexpectedEof.into())`.  It is named after
        /// `SkCodec::Result::kIncompleteInput`.
        ///
        /// `ReadTrait` is infallible and therefore we provide no generic
        /// equivalent of the `png::DecodingError::IoError` variant
        /// (other than the special case of `IncompleteInput`).
        IncompleteInput,
    }

    /// FFI-friendly equivalent of `png::DisposeOp`.
    enum DisposeOp {
        None,
        Background,
        Previous,
    }

    /// FFI-friendly equivalent of `png::BlendOp`.
    enum BlendOp {
        Source,
        Over,
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
        fn interlaced(self: &Reader) -> bool;
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
        fn has_actl_chunk(self: &Reader) -> bool;
        fn get_actl_num_frames(self: &Reader) -> u32;
        fn get_actl_num_plays(self: &Reader) -> u32;
        fn has_fctl_chunk(self: &Reader) -> bool;
        fn get_fctl_info(
            self: &Reader,
            width: &mut u32,
            height: &mut u32,
            x_offset: &mut u32,
            y_offset: &mut u32,
            dispose_op: &mut DisposeOp,
            blend_op: &mut BlendOp,
            duration_ms: &mut u32,
        );
        fn output_buffer_size(self: &Reader) -> usize;
        fn output_color_type(self: &Reader) -> ColorType;
        fn output_bits_per_component(self: &Reader) -> u8;
        unsafe fn next_interlaced_row<'a>(
            self: &'a mut Reader,
            row: &mut &'a [u8],
        ) -> DecodingResult;
        fn expand_last_interlaced_row(
            self: &Reader,
            img: &mut [u8],
            img_row_stride: usize,
            row: &[u8],
            bits_per_pixel: u8,
        );
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

impl From<png::DisposeOp> for ffi::DisposeOp {
    fn from(value: png::DisposeOp) -> Self {
        match value {
            png::DisposeOp::None => Self::None,
            png::DisposeOp::Background => Self::Background,
            png::DisposeOp::Previous => Self::Previous,
        }
    }
}

impl From<png::BlendOp> for ffi::BlendOp {
    fn from(value: png::BlendOp) -> Self {
        match value {
            png::BlendOp::Source => Self::Source,
            png::BlendOp::Over => Self::Over,
        }
    }
}

impl From<Option<&png::DecodingError>> for ffi::DecodingResult {
    fn from(option: Option<&png::DecodingError>) -> Self {
        match option {
            None => Self::Success,
            Some(decoding_error) => match decoding_error {
                png::DecodingError::IoError(e) => {
                    if e.kind() == ErrorKind::UnexpectedEof {
                        Self::IncompleteInput
                    } else {
                        // `ReadTrait` is infallible => we expect no other kind of
                        // `png::DecodingError::IoError`.
                        unreachable!()
                    }
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
struct Reader {
    reader: png::Reader<cxx::UniquePtr<ffi::ReadTrait>>,
    last_interlace_info: Option<png::InterlaceInfo>,
}

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
        Ok(Self { reader: decoder.read_info()?, last_interlace_info: None })
    }

    fn height(&self) -> u32 {
        self.reader.info().height
    }

    fn width(&self) -> u32 {
        self.reader.info().width
    }

    /// Returns whether the PNG image is interlaced.
    fn interlaced(&self) -> bool {
        self.reader.info().interlaced
    }

    /// Returns whether the decoded PNG image contained a `sRGB` chunk.
    fn is_srgb(&self) -> bool {
        self.reader.info().srgb.is_some()
    }

    /// If the decoded PNG image contained a `cHRM` chunk then `try_get_chrm`
    /// returns `true` and populates the out parameters (`wx`, `wy`, `rx`,
    /// etc.).  Otherwise, returns `false`.
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

        match self.reader.info().chrm_chunk.as_ref() {
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
    fn try_get_gama(&self, gamma: &mut f32) -> bool {
        match self.reader.info().gama_chunk.as_ref() {
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
        match self.reader.info().icc_profile.as_ref().map(|cow| cow.as_ref()) {
            None => false,
            Some(value) => {
                *iccp = value;
                true
            }
        }
    }

    /// Returns whether the `acTL` chunk exists.
    fn has_actl_chunk(&self) -> bool {
        self.reader.info().animation_control.is_some()
    }

    /// Returns `num_frames` from the `acTL` chunk.  Panics if there is no
    /// `acTL` chunk.
    ///
    /// The returned value is equal the number of `fcTL` chunks.  (Note that it
    /// doesn't count `IDAT` nor `fdAT` chunks.  In particular, if an `fcTL`
    /// chunk doesn't appear before an `IDAT` chunk then `IDAT` is not part
    /// of the animation.)
    ///
    /// See also
    /// <https://wiki.mozilla.org/APNG_Specification#.60acTL.60:_The_Animation_Control_Chunk>.
    fn get_actl_num_frames(&self) -> u32 {
        self.reader.info().animation_control.as_ref().unwrap().num_frames
    }

    /// Returns `num_plays` from the `acTL` chunk.  Panics if there is no `acTL`
    /// chunk.
    ///
    /// `0` indicates that the animation should play indefinitely. See
    /// <https://wiki.mozilla.org/APNG_Specification#.60acTL.60:_The_Animation_Control_Chunk>.
    fn get_actl_num_plays(&self) -> u32 {
        self.reader.info().animation_control.as_ref().unwrap().num_plays
    }

    /// Returns whether a `fcTL` chunk has been parsed (and can be read using
    /// `get_fctl_info`).
    fn has_fctl_chunk(&self) -> bool {
        self.reader.info().frame_control.is_some()
    }

    /// Returns `png::FrameControl` information.
    ///
    /// Panics if no `fcTL` chunk hasn't been parsed yet.
    fn get_fctl_info(
        self: &Reader,
        width: &mut u32,
        height: &mut u32,
        x_offset: &mut u32,
        y_offset: &mut u32,
        dispose_op: &mut ffi::DisposeOp,
        blend_op: &mut ffi::BlendOp,
        duration_ms: &mut u32,
    ) {
        let frame_control = self.reader.info().frame_control.as_ref().unwrap();
        *width = frame_control.width;
        *height = frame_control.height;
        *x_offset = frame_control.x_offset;
        *y_offset = frame_control.y_offset;
        *dispose_op = frame_control.dispose_op.into();
        *blend_op = frame_control.blend_op.into();

        // https://wiki.mozilla.org/APNG_Specification#.60fcTL.60:_The_Frame_Control_Chunk
        // says:
        //
        // > "The `delay_num` and `delay_den` parameters together specify a fraction
        // > indicating the time to display the current frame, in seconds. If the
        // > denominator is 0, it is to be treated as if it were 100 (that is,
        // > `delay_num` then specifies 1/100ths of a second).
        *duration_ms = if frame_control.delay_den == 0 {
            10 * frame_control.delay_num as u32
        } else {
            1000 * frame_control.delay_num as u32 / frame_control.delay_den as u32
        };
    }

    fn output_buffer_size(&self) -> usize {
        self.reader.output_buffer_size()
    }

    fn output_color_type(&self) -> ffi::ColorType {
        self.reader.output_color_type().0.into()
    }

    fn output_bits_per_component(&self) -> u8 {
        self.reader.output_color_type().1 as u8
    }

    /// Decodes the next row - see
    /// https://docs.rs/png/latest/png/struct.Reader.html#method.next_interlaced_row
    ///
    /// TODO(https://crbug.com/357876243): Consider using `read_row` to avoid an extra copy.
    /// See also https://github.com/image-rs/image-png/pull/493
    fn next_interlaced_row<'a>(&'a mut self, row: &mut &'a [u8]) -> ffi::DecodingResult {
        let result = self.reader.next_interlaced_row();
        if let Ok(maybe_row) = result.as_ref() {
            self.last_interlace_info = maybe_row.as_ref().map(|r| r.interlace()).copied();
            *row = maybe_row.map(|r| r.data()).unwrap_or(&[]);
        }
        result.as_ref().err().into()
    }

    /// Expands the last decoded interlaced row - see
    /// https://docs.rs/png/latest/png/fn.expand_interlaced_row
    fn expand_last_interlaced_row(
        &self,
        img: &mut [u8],
        img_row_stride: usize,
        row: &[u8],
        bits_per_pixel: u8,
    ) {
        let Some(png::InterlaceInfo::Adam7(ref adam7info)) = self.last_interlace_info.as_ref()
        else {
            panic!("This function should only be called after decoding an interlaced row");
        };
        png::expand_interlaced_row(img, img_row_stride, row, adam7info, bits_per_pixel);
    }
}

/// This provides a public C++ API for decoding a PNG image.
fn new_reader(input: cxx::UniquePtr<ffi::ReadTrait>) -> Box<ResultOfReader> {
    Box::new(ResultOfReader(Reader::new(input)))
}
