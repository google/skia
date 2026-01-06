// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `png` Rust crate.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

use std::borrow::Cow;
use std::io::{BufReader, ErrorKind, Write};
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
        IncompleteInput,
        OtherIoError,
        EndOfFrame,
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

    /// FFI-friendly simplification of `png::Compression`.
    enum Compression {
        /// In png-0.18.0-rc `Fastest` level would fall back to `Level1WithUpFilter` when using
        /// `StreamWriter`.  See also code links below:
        /// * In 0.18-rc2 `Fastest` initially maps to `FdeflateUltraFast`, but in the end falls back to `flate2`
        ///   when using `StreamWriter`:
        ///     - https://github.com/image-rs/image-png/blob/9294c26dc3ca7622f791e880810b575193fb6c29/src/common.rs#L414
        ///     - https://github.com/image-rs/image-png/blob/33afddab77449bcd93b1783d2d0ca8ba744cc3c3/src/encoder.rs#L1402
        ///     - https://github.com/image-rs/image-png/blob/33afddab77449bcd93b1783d2d0ca8ba744cc3c3/src/common.rs#L413
        /// * In 0.18-rc2 `Fastest` maps to `Up`:
        ///   https://github.com/image-rs/image-png/blob/9294c26dc3ca7622f791e880810b575193fb6c29/src/filter.rs#L47
        ///
        /// In newer versions, `Fastest` may map to `fdeflate` backend.
        /// We export `Level1WithUpFilter` as an explicit, separate level to preserve the M136
        /// behavior that was tested in a field trial and approved for shipping.
        ///
        /// TODO(https://crbug.com/406072770): Revisit this in the future and only use the built-in
        /// levels in the long term.
        Level1WithUpFilter,
        /// Maps to `png::Compression::Fastest`.
        Fastest,
        /// Maps to `png::Compression::Fast`.
        Fast,
        /// Maps to `png::Compression::Balanced`.
        Balanced,
        /// Maps to `png::Compression::High`.
        High,
    }

    /// FFI-friendly simplification of `Option<png::EncodingError>`.
    enum EncodingResult {
        Success,
        IoError,
        FormatError,
        ParameterError,
        LimitsExceededError,
    }

    /// FFI/layering-friendly equivalent of `SkColorSpacePrimaries from C/C++.
    struct ColorSpacePrimaries {
        fRX: f32,
        fRY: f32,
        fGX: f32,
        fGY: f32,
        fBX: f32,
        fBY: f32,
        fWX: f32,
        fWY: f32,
    }

    /// FFI/layering-friendly equivalent of `skhdr::MasteringDisplayColorVolume` from C/C++.
    struct MasteringDisplayColorVolume {
        fDisplayPrimaries: ColorSpacePrimaries,
        fMaximumDisplayMasteringLuminance: f32,
        fMinimumDisplayMasteringLuminance: f32,
    }

    /// FFI/layering-friendly equivalent of `skhdr::ContentLightLevelInformation` from C/C++.
    struct ContentLightLevelInfo {
        fMaxCLL: f32,
        fMaxFALL: f32,
    }

    unsafe extern "C++" {
        include!("rust/png/FFI.h");
        include!("rust/common/SkStreamAdapter.h");

        // Reference the SkStreamAdapter type from skia_rust_common.
        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;

        type WriteTrait;
        fn write(self: Pin<&mut WriteTrait>, buffer: &[u8]) -> bool;
        fn flush(self: Pin<&mut WriteTrait>);
    }

    // Rust functions, types, and methods that are exposed through FFI.
    //
    // To avoid duplication, there are no doc comments inside the `extern "Rust"`
    // section. The doc comments of these items can instead be found in the
    // actual Rust code, outside of the `#[cxx::bridge]` manifest.
    extern "Rust" {
        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn height(self: &Reader) -> u32;
        fn width(self: &Reader) -> u32;
        fn interlaced(self: &Reader) -> bool;
        fn is_srgb(self: &Reader) -> bool;
        fn try_get_chrm(self: &Reader, chrm: &mut ColorSpacePrimaries) -> bool;
        fn try_get_cicp_chunk(
            self: &Reader,
            primaries_id: &mut u8,
            transfer_id: &mut u8,
            matrix_id: &mut u8,
            is_full_range: &mut bool,
        ) -> bool;
        fn try_get_mdcv_chunk(self: &Reader, mdcv: &mut MasteringDisplayColorVolume) -> bool;
        fn try_get_clli_chunk(self: &Reader, clli: &mut ContentLightLevelInfo) -> bool;
        fn try_get_gama(self: &Reader, gamma: &mut f32) -> bool;
        fn has_exif_chunk(self: &Reader) -> bool;
        fn get_exif_chunk(self: &Reader) -> &[u8];
        fn has_iccp_chunk(self: &Reader) -> bool;
        fn get_iccp_chunk(self: &Reader) -> &[u8];
        fn has_trns_chunk(self: &Reader) -> bool;
        fn get_trns_chunk(self: &Reader) -> &[u8];
        fn has_plte_chunk(self: &Reader) -> bool;
        fn get_plte_chunk(self: &Reader) -> &[u8];
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
        fn has_sbit_chunk(self: &Reader) -> bool;
        fn get_sbit_chunk(self: &Reader) -> &[u8];
        fn output_color_type(self: &Reader) -> ColorType;
        fn output_bits_per_component(self: &Reader) -> u8;
        fn next_frame_info(self: &mut Reader) -> DecodingResult;
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
        unsafe fn read_row(self: &mut Reader, output_buffer: &mut [u8]) -> DecodingResult;

        fn new_writer(
            output: UniquePtr<WriteTrait>,
            width: u32,
            height: u32,
            color: ColorType,
            bits_per_component: u8,
            compression: Compression,
            icc_profile: &[u8],
        ) -> Box<ResultOfWriter>;

        type ResultOfWriter;
        fn err(self: &ResultOfWriter) -> EncodingResult;
        fn unwrap(self: &mut ResultOfWriter) -> Box<Writer>;

        type Writer;
        fn write_text_chunk(self: &mut Writer, keyword: &[u8], text: &[u8]) -> EncodingResult;
        fn convert_writer_into_stream_writer(writer: Box<Writer>) -> Box<ResultOfStreamWriter>;

        type ResultOfStreamWriter;
        fn err(self: &ResultOfStreamWriter) -> EncodingResult;
        fn unwrap(self: &mut ResultOfStreamWriter) -> Box<StreamWriter>;

        type StreamWriter;
        fn write(self: &mut StreamWriter, data: &[u8]) -> EncodingResult;
        fn finish_encoding(stream_writer: Box<StreamWriter>) -> EncodingResult;
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

impl Into<png::ColorType> for ffi::ColorType {
    fn into(self) -> png::ColorType {
        match self {
            Self::Grayscale => png::ColorType::Grayscale,
            Self::Rgb => png::ColorType::Rgb,
            Self::GrayscaleAlpha => png::ColorType::GrayscaleAlpha,
            Self::Rgba => png::ColorType::Rgba,

            // `SkPngRustEncoderImpl` only uses the color types above.
            _ => unreachable!(),
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
                        Self::OtherIoError
                    }
                }
                png::DecodingError::Format(_) => Self::FormatError,
                png::DecodingError::Parameter(_) => Self::ParameterError,
                png::DecodingError::LimitsExceeded => Self::LimitsExceededError,
            },
        }
    }
}

impl ffi::Compression {
    fn apply<'a, W: Write>(&self, encoder: &mut png::Encoder<'a, W>) {
        match self {
            &Self::Level1WithUpFilter => {
                encoder.set_deflate_compression(png::DeflateCompression::Level(1));
                encoder.set_filter(png::Filter::Up);
            }
            &Self::Fastest => encoder.set_compression(png::Compression::Fastest),
            &Self::Fast => encoder.set_compression(png::Compression::Fast),
            &Self::Balanced => encoder.set_compression(png::Compression::Balanced),
            &Self::High => encoder.set_compression(png::Compression::High),
            _ => unreachable!(),
        }
    }
}

impl From<Option<&png::EncodingError>> for ffi::EncodingResult {
    fn from(option: Option<&png::EncodingError>) -> Self {
        match option {
            None => Self::Success,
            Some(encoding_error) => match encoding_error {
                png::EncodingError::IoError(_) => Self::IoError,
                png::EncodingError::Format(_) => Self::FormatError,
                png::EncodingError::Parameter(_) => Self::ParameterError,
                png::EncodingError::LimitsExceeded => Self::LimitsExceededError,
            },
        }
    }
}

impl<'a> Write for Pin<&'a mut ffi::WriteTrait> {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        if self.as_mut().write(buf) {
            Ok(buf.len())
        } else {
            Err(ErrorKind::Other.into())
        }
    }

    fn flush(&mut self) -> std::io::Result<()> {
        self.as_mut().flush();
        Ok(())
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

fn compute_transformations(info: &png::Info) -> png::Transformations {
    // There are 2 scenarios where `EXPAND` transformation may be needed:
    //
    // * `SkSwizzler` can handle low-bit-depth `ColorType::Indexed`, but it may not
    //   support other inputs with low bit depth (e.g. `kGray_Color` with bpp=4). We
    //   use `EXPAND` to ask the `png` crate to expand such low-bpp images to at
    //   least 8 bits.
    // * We may need to inject an alpha channel from the `tRNS` chunk if present.
    //   Note that we can't check `info.trns.is_some()` because at this point we
    //   have not yet read beyond the `IHDR` chunk.
    //
    // We avoid using `EXPAND` for `ColorType::Indexed` because this results in some
    // performance gains - see https://crbug.com/356882657 for more details.
    let mut result = match info.color_type {
        // Work around bpp<8 limitations of `SkSwizzler`
        png::ColorType::Rgba | png::ColorType::GrayscaleAlpha if (info.bit_depth as u8) < 8 => {
            png::Transformations::EXPAND
        }

        // Handle `tRNS` expansion + work around bpp<8 limitations of `SkSwizzler`
        png::ColorType::Rgb | png::ColorType::Grayscale => png::Transformations::EXPAND,

        // Otherwise there is no need to `EXPAND`.
        png::ColorType::Indexed | png::ColorType::Rgba | png::ColorType::GrayscaleAlpha => {
            png::Transformations::IDENTITY
        }
    };

    // We mimic how the `libpng`-based `SkPngCodec` handles G16 and GA16.
    //
    // TODO(https://crbug.com/359245096): Avoid stripping least signinficant 8 bits in G16 and
    // GA16 images.
    result = {
        #[cfg(not(feature = "skia_png_new_gray16_behavior_for_crbug359245096"))]
        {
            if info.bit_depth == png::BitDepth::Sixteen {
                if matches!(
                    info.color_type,
                    png::ColorType::Grayscale | png::ColorType::GrayscaleAlpha
                ) {
                    result |= png::Transformations::STRIP_16;
                }
            }

            result
        }

        #[cfg(feature = "skia_png_new_gray16_behavior_for_crbug359245096")]
        {
            result
        }
    };

    result
}

/// FFI-friendly wrapper around `png::Reader<R>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct Reader {
    reader: png::Reader<BufReader<cxx::UniquePtr<ffi::SkStreamAdapter>>>,
    last_interlace_info: Option<png::InterlaceInfo>,
}

impl Reader {
    fn new(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, png::DecodingError> {
        // The magic value of `BUF_CAPACITY` is based on `CHUNK_BUFFER_SIZE` which was
        // used in `BufReader::with_capacity` calls by `png` crate up to version
        // 0.17.16 - see: https://github.com/image-rs/image-png/pull/558/files#diff-c28833b65510e37441203b4256b74068f191d29ea34b6e753442e644d3a316b8L28
        // and
        // https://github.com/image-rs/image-png/blob/eb9b5d7f371b88f15aaca6a8d21c58b86c400d76/src/decoder/stream.rs#L21
        const BUF_CAPACITY: usize = 32 * 1024;
        // TODO(https://crbug.com/399894620): Consider instead implementing `BufRead` on top of
        // `SkStream` API when/if possible in the future.
        let input = BufReader::with_capacity(BUF_CAPACITY, input);

        let mut decoder = {
            // By default, `DecodeOptions` cap the memory usage at using 64 Mib.
            let mut options = png::DecodeOptions::default();
            if cfg!(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) {
                options.set_ignore_checksums(true);
            }
            options.set_ignore_text_chunk(true);
            png::Decoder::new_with_options(input, options)
        };

        let info = decoder.read_header_info()?;
        let transformations = compute_transformations(info);
        decoder.set_transformations(transformations);

        Ok(Self {
            reader: decoder.read_info()?,
            last_interlace_info: None,
        })
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
    fn try_get_chrm(&self, chrm: &mut ffi::ColorSpacePrimaries) -> bool {
        fn copy_channel(channel: &(png::ScaledFloat, png::ScaledFloat), x: &mut f32, y: &mut f32) {
            *x = png_u32_into_f32(channel.0);
            *y = png_u32_into_f32(channel.1);
        }

        match self.reader.info().chrm_chunk.as_ref() {
            None => false,
            Some(png_chrm) => {
                copy_channel(&png_chrm.white, &mut chrm.fWX, &mut chrm.fWY);
                copy_channel(&png_chrm.red, &mut chrm.fRX, &mut chrm.fRY);
                copy_channel(&png_chrm.green, &mut chrm.fGX, &mut chrm.fGY);
                copy_channel(&png_chrm.blue, &mut chrm.fBX, &mut chrm.fBY);
                true
            }
        }
    }

    /// If the decoded PNG image contained a `cICP` chunk then
    /// `try_get_cicp_chunk` returns `true` and populates the out
    /// parameters.  Otherwise, returns `false`.
    fn try_get_cicp_chunk(
        &self,
        primaries_id: &mut u8,
        transfer_id: &mut u8,
        matrix_id: &mut u8,
        is_full_range: &mut bool,
    ) -> bool {
        match self.reader.info().coding_independent_code_points.as_ref() {
            None => false,
            Some(cicp) => {
                *primaries_id = cicp.color_primaries;
                *transfer_id = cicp.transfer_function;
                *matrix_id = cicp.matrix_coefficients;
                *is_full_range = cicp.is_video_full_range_image;
                true
            }
        }
    }

    /// If the decoded PNG image contained a `mDCV` chunk then
    /// `try_get_mdcv_chunk` returns `true` and populates the out parameters
    /// as values that are CIE 1931 xy coordinates or values in cd/m^2.
    /// Otherwise, returns `false`.
    fn try_get_mdcv_chunk(self: &Reader, mdcv: &mut ffi::MasteringDisplayColorVolume) -> bool {
        match self.reader.info().mastering_display_color_volume.as_ref() {
            None => false,
            Some(png_mdcv) => {
                *mdcv = ffi::MasteringDisplayColorVolume {
                    fDisplayPrimaries: ffi::ColorSpacePrimaries {
                        fRX: png_mdcv.chromaticities.red.0.into_value(),
                        fRY: png_mdcv.chromaticities.red.1.into_value(),
                        fGX: png_mdcv.chromaticities.green.0.into_value(),
                        fGY: png_mdcv.chromaticities.green.1.into_value(),
                        fBX: png_mdcv.chromaticities.blue.0.into_value(),
                        fBY: png_mdcv.chromaticities.blue.1.into_value(),
                        fWX: png_mdcv.chromaticities.white.0.into_value(),
                        fWY: png_mdcv.chromaticities.white.1.into_value(),
                    },
                    fMaximumDisplayMasteringLuminance: png_mdcv.max_luminance as f32 / 10_000.0,
                    fMinimumDisplayMasteringLuminance: png_mdcv.min_luminance as f32 / 10_000.0,
                };
                true
            }
        }
    }

    /// If the decoded PNG image contained a `cLLI` chunk then
    /// `try_get_clli_chunk` returns `true` and populates the out
    /// parameters as values in cd/m^2.  Otherwise, returns `false`.
    fn try_get_clli_chunk(&self, clli: &mut ffi::ContentLightLevelInfo) -> bool {
        match self.reader.info().content_light_level.as_ref() {
            None => false,
            Some(png_clli) => {
                *clli = ffi::ContentLightLevelInfo {
                    fMaxCLL: png_clli.max_content_light_level as f32 / 10_000.0,
                    fMaxFALL: png_clli.max_frame_average_light_level as f32 / 10_000.0,
                };
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
            Some(&scaled_float) => {
                *gamma = png_u32_into_f32(scaled_float);
                true
            }
        }
    }

    /// Returns whether the `eXIf` chunk exists.
    fn has_exif_chunk(&self) -> bool {
        self.reader.info().exif_metadata.is_some()
    }

    /// Returns contents of the `eXIf` chunk.  Panics if there is no `eXIf`
    /// chunk.
    fn get_exif_chunk(&self) -> &[u8] {
        self.reader.info().exif_metadata.as_ref().unwrap().as_ref()
    }

    /// Returns whether the `iCCP` chunk exists.
    fn has_iccp_chunk(&self) -> bool {
        self.reader.info().icc_profile.is_some()
    }

    /// Returns contents of the `iCCP` chunk.  Panics if there is no `iCCP`
    /// chunk.
    fn get_iccp_chunk(&self) -> &[u8] {
        self.reader.info().icc_profile.as_ref().unwrap().as_ref()
    }

    /// Returns whether the `tRNS` chunk exists.
    fn has_trns_chunk(&self) -> bool {
        self.reader.info().trns.is_some()
    }

    /// Returns contents of the `tRNS` chunk.  Panics if there is no `tRNS`
    /// chunk.
    fn get_trns_chunk(&self) -> &[u8] {
        self.reader.info().trns.as_ref().unwrap().as_ref()
    }

    /// Returns whether the `PLTE` chunk exists.
    fn has_plte_chunk(&self) -> bool {
        self.reader.info().palette.is_some()
    }

    /// Returns contents of the `PLTE` chunk.  Panics if there is no `PLTE`
    /// chunk.
    fn get_plte_chunk(&self) -> &[u8] {
        self.reader.info().palette.as_ref().unwrap().as_ref()
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
        self.reader
            .info()
            .animation_control
            .as_ref()
            .unwrap()
            .num_frames
    }

    /// Returns `num_plays` from the `acTL` chunk.  Panics if there is no `acTL`
    /// chunk.
    ///
    /// `0` indicates that the animation should play indefinitely. See
    /// <https://wiki.mozilla.org/APNG_Specification#.60acTL.60:_The_Animation_Control_Chunk>.
    fn get_actl_num_plays(&self) -> u32 {
        self.reader
            .info()
            .animation_control
            .as_ref()
            .unwrap()
            .num_plays
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
        &self,
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

    /// Returns whether the `sBIT` chunk exists.
    fn has_sbit_chunk(&self) -> bool {
        self.reader.info().sbit.is_some()
    }

    /// Returns contents of the `sBIT` chunk.  Panics if there is no `sBIT`
    /// chunk.
    fn get_sbit_chunk(&self) -> &[u8] {
        self.reader.info().sbit.as_ref().unwrap().as_ref()
    }

    fn output_color_type(&self) -> ffi::ColorType {
        self.reader.output_color_type().0.into()
    }

    fn output_bits_per_component(&self) -> u8 {
        self.reader.output_color_type().1 as u8
    }

    fn next_frame_info(&mut self) -> ffi::DecodingResult {
        self.reader.next_frame_info().as_ref().err().into()
    }

    /// Decodes the next row - see
    /// https://docs.rs/png/latest/png/struct.Reader.html#method.next_interlaced_row
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

    /// Decodes the next row directly into a caller-provided buffer - see
    /// https://docs.rs/png/0.18.0-rc.3/png/struct.Reader.html#method.read_row
    fn read_row(&mut self, output_buffer: &mut [u8]) -> ffi::DecodingResult {
        match self.reader.read_row(output_buffer) {
            Ok(Some(info)) => {
                self.last_interlace_info = Some(info);
                ffi::DecodingResult::Success
            }
            Ok(None) => ffi::DecodingResult::EndOfFrame,
            Err(e) => ffi::DecodingResult::from(Some(&e)),
        }
    }
}

fn png_u32_into_f32(v: png::ScaledFloat) -> f32 {
    // This uses `0.00001_f32 * (v.into_scaled() as f32)` instead of just
    // `v.into_value()` for compatibility with the legacy implementation
    // of `ReadColorProfile` in
    // `.../blink/renderer/platform/image-decoders/png/png_image_decoder.cc`.
    0.00001_f32 * (v.into_scaled() as f32)
}

/// This provides a public C++ API for decoding a PNG image.
fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<ResultOfReader> {
    Box::new(ResultOfReader(Reader::new(input)))
}

/// FFI-friendly wrapper around `Result<T, E>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct ResultOfWriter(Result<Writer, png::EncodingError>);

impl ResultOfWriter {
    fn err(&self) -> ffi::EncodingResult {
        self.0.as_ref().err().into()
    }

    fn unwrap(&mut self) -> Box<Writer> {
        // Leaving `self` in a C++-friendly "moved-away" state.
        let mut result = Err(png::EncodingError::LimitsExceeded);
        std::mem::swap(&mut self.0, &mut result);

        Box::new(result.unwrap())
    }
}

/// FFI-friendly wrapper around `png::Writer` (`cxx` can't handle
/// arbitrary generics, so we manually monomorphize here, but still expose a
/// minimal, somewhat tweaked API of the original type).
struct Writer(png::Writer<cxx::UniquePtr<ffi::WriteTrait>>);

impl Writer {
    fn new(
        output: cxx::UniquePtr<ffi::WriteTrait>,
        width: u32,
        height: u32,
        color: ffi::ColorType,
        bits_per_component: u8,
        compression: ffi::Compression,
        icc_profile: &[u8],
    ) -> Result<Self, png::EncodingError> {
        let mut info = png::Info::with_size(width, height);
        info.color_type = color.into();
        info.bit_depth = match bits_per_component {
            8 => png::BitDepth::Eight,
            16 => png::BitDepth::Sixteen,

            // `SkPngRustEncoderImpl` only encodes 8-bit or 16-bit images.
            _ => unreachable!(),
        };
        if !icc_profile.is_empty() {
            info.icc_profile = Some(Cow::Owned(icc_profile.to_owned()));
        }
        let mut encoder = png::Encoder::with_info(output, info)?;
        compression.apply(&mut encoder);

        let writer = encoder.write_header()?;
        Ok(Self(writer))
    }

    /// FFI-friendly wrapper around `png::Writer::write_text_chunk`.
    ///
    /// `keyword` and `text` are treated as strings encoded as Latin-1 (i.e.
    /// ISO-8859-1).
    ///
    /// `ffi::EncodingResult::Parameter` error will be returned if `keyword` or
    /// `text` don't meet the requirements of the PNG spec.  `text` may have
    /// any length and contain any of the 191 Latin-1 characters (and/or the
    /// linefeed character), but `keyword`'s length is restricted to at most
    /// 79 characters and it can't contain a non-breaking space character.
    ///
    /// See also https://docs.rs/png/latest/png/struct.Writer.html#method.write_text_chunk
    fn write_text_chunk(&mut self, keyword: &[u8], text: &[u8]) -> ffi::EncodingResult {
        // https://www.w3.org/TR/png-3/#11tEXt says that "`text` is interpreted according to the
        // Latin-1 character set [ISO_8859-1]. The text string may contain any Latin-1
        // character."
        let is_latin1_byte = |b| (0x20..=0x7E).contains(b) || (0xA0..=0xFF).contains(b);
        let is_nbsp_byte = |&b: &u8| b == 0xA0;
        let is_linefeed_byte = |&b: &u8| b == 10;
        if !text
            .iter()
            .all(|b| is_latin1_byte(b) || is_linefeed_byte(b))
        {
            return ffi::EncodingResult::ParameterError;
        }
        fn latin1_bytes_into_string(bytes: &[u8]) -> String {
            bytes.iter().map(|&b| b as char).collect()
        }
        let text = latin1_bytes_into_string(text);

        // https://www.w3.org/TR/png-3/#11keywords says that "keywords shall contain only printable
        // Latin-1 [ISO_8859-1] characters and spaces; that is, only code points 0x20-7E
        // and 0xA1-FF are allowed."
        if !keyword
            .iter()
            .all(|b| is_latin1_byte(b) && !is_nbsp_byte(b))
        {
            return ffi::EncodingResult::ParameterError;
        }
        let keyword = latin1_bytes_into_string(keyword);

        let chunk = png::text_metadata::TEXtChunk { keyword, text };
        let result = self.0.write_text_chunk(&chunk);
        result.as_ref().err().into()
    }
}

/// FFI-friendly wrapper around `png::Writer::into_stream_writer`.
///
/// See also https://docs.rs/png/latest/png/struct.Writer.html#method.into_stream_writer
fn convert_writer_into_stream_writer(writer: Box<Writer>) -> Box<ResultOfStreamWriter> {
    Box::new(ResultOfStreamWriter(
        writer.0.into_stream_writer().map(StreamWriter),
    ))
}

/// FFI-friendly wrapper around `Result<T, E>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct ResultOfStreamWriter(Result<StreamWriter, png::EncodingError>);

impl ResultOfStreamWriter {
    fn err(&self) -> ffi::EncodingResult {
        self.0.as_ref().err().into()
    }

    fn unwrap(&mut self) -> Box<StreamWriter> {
        // Leaving `self` in a C++-friendly "moved-away" state.
        let mut result = Err(png::EncodingError::LimitsExceeded);
        std::mem::swap(&mut self.0, &mut result);

        Box::new(result.unwrap())
    }
}

/// FFI-friendly wrapper around `png::StreamWriter` (`cxx` can't handle
/// arbitrary generics, so we manually monomorphize here, but still expose a
/// minimal, somewhat tweaked API of the original type).
struct StreamWriter(png::StreamWriter<'static, cxx::UniquePtr<ffi::WriteTrait>>);

impl StreamWriter {
    /// FFI-friendly wrapper around `Write::write` implementation of
    /// `png::StreamWriter`.
    ///
    /// See also https://docs.rs/png/latest/png/struct.StreamWriter.html#method.write
    pub fn write(&mut self, data: &[u8]) -> ffi::EncodingResult {
        let io_result = self.0.write(data);
        let encoding_result = io_result.map_err(|err| png::EncodingError::IoError(err));
        encoding_result.as_ref().err().into()
    }
}

/// This provides a public C++ API for encoding a PNG image.
///
/// `icc_profile` set to an empty slice acts as null / `None`.
fn new_writer(
    output: cxx::UniquePtr<ffi::WriteTrait>,
    width: u32,
    height: u32,
    color: ffi::ColorType,
    bits_per_component: u8,
    compression: ffi::Compression,
    icc_profile: &[u8],
) -> Box<ResultOfWriter> {
    Box::new(ResultOfWriter(Writer::new(
        output,
        width,
        height,
        color,
        bits_per_component,
        compression,
        icc_profile,
    )))
}

/// FFI-friendly wrapper around `png::StreamWriter::finish`.
///
/// See also https://docs.rs/png/latest/png/struct.StreamWriter.html#method.finish
fn finish_encoding(stream_writer: Box<StreamWriter>) -> ffi::EncodingResult {
    stream_writer.0.finish().as_ref().err().into()
}
