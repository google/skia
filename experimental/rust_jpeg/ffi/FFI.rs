// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for JPEG decoder (zune-jpeg) and encoder (jpeg-encoder).
//! Includes a memory-safe JPEG segment scanner for metadata extraction.

#[cxx::bridge(namespace = "rust_jpeg")]
mod ffi {
    #[derive(Debug, Clone, Copy)]
    enum DecodingResult {
        Success,
        FormatError,
        ParameterError,
        UnsupportedFeature,
        IncompleteInput,
        MemoryError,
        OtherError,
    }

    #[derive(Debug, Clone, Copy)]
    enum OutputStatus {
        Ready,
        NeedMoreInput,
        Complete,
        Error,
    }

    #[derive(Debug, Clone, Copy, PartialEq)]
    enum PackedOutputFormat {
        Source,
        RGBA,
        BGRA,
    }

    #[derive(Debug, Clone, Copy, Default)]
    struct RawPlaneInfo {
        component_id: u8,
        horizontal_sampling_factor: u32,
        vertical_sampling_factor: u32,
        width: u32,
        height: u32,
        row_bytes: u32,
    }

    struct RawOutputResult {
        status: OutputStatus,
        result: DecodingResult,
        y_rows: u32,
        u_rows: u32,
        v_rows: u32,
    }

    struct ScanlineOutputResult {
        status: OutputStatus,
        result: DecodingResult,
        rows: u32,
    }

    /// JPEG color type matching SkEncodedInfo::Color values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum JpegColor {
        Grayscale = 0,
        RGB = 5,
        InvertedCMYK = 12,
    }

    /// JPEG alpha type matching SkEncodedInfo::Alpha values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum JpegAlpha {
        Opaque = 0,
    }

    /// Information about decoded rows.
    struct DecodedRowsInfo {
        /// The destination row index where the buffer should start being copied.
        dst_row_start: u32,
        row_count: u32,
        /// Whether these rows replace a previous progressive preview rather than
        /// extending the stable final-output prefix.
        is_preview: bool,
    }

    /// A scanned JPEG segment's header information.
    struct JpegSegmentInfo {
        /// The second byte of the marker code (e.g. 0xE1 for APP1).
        marker: u8,
        /// Offset in bytes from start of JPEG data to the marker's 0xFF byte.
        offset: u32,
        /// Length of the segment parameters (including the 2-byte length field).
        parameter_length: u16,
    }

    /// An image entry from Multi-Picture Format (MPF) metadata.
    struct MpfImageEntry {
        /// Offset of the image data relative to the MP Endian field
        /// in the MPF segment.  For the first (primary) image this is 0.
        data_offset: u32,
        /// Size of the image in bytes.
        size: u32,
    }

    // Encoder types
    #[derive(Debug, Clone, Copy)]
    enum EncodingResult {
        Success,
        ParameterError,
        OtherError,
    }

    #[derive(Debug, Clone, Copy)]
    enum JpegEncodeColor {
        RGB,
        RGBA,
        BGRA,
        Grayscale,
    }

    /// Alpha handling for JPEG encoding (JPEG is always opaque).
    #[derive(Debug, Clone, Copy)]
    enum JpegEncodeAlpha {
        /// Ignore alpha channel entirely.
        Ignore,
        /// Blend onto black background before encoding.
        BlendOnBlack,
    }

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        fn is_jpeg_data(data: &[u8]) -> bool;

        fn new_reader(
            input: UniquePtr<SkStreamAdapter>,
            stream_length: u64,
            input_complete: bool,
        ) -> Box<Reader>;

        type Reader;
        fn initialization_result(self: &Reader) -> DecodingResult;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> JpegColor;
        fn alpha(self: &Reader) -> JpegAlpha;
        fn metadata_loaded(self: &Reader) -> bool;
        fn mcu_width(self: &Reader) -> u32;
        fn mcu_height(self: &Reader) -> u32;

        fn read_metadata(self: &mut Reader) -> DecodingResult;
        fn read_image_data(self: &mut Reader) -> DecodingResult;
        fn read_incremental_image_data(self: &mut Reader) -> DecodingResult;
        fn image_data_loaded(self: &Reader) -> bool;

        /// # Safety
        /// CXX requires `unsafe` for functions with explicit lifetimes.
        unsafe fn get_next_rows<'a>(self: &'a mut Reader, buffer: &mut &'a [u8])
            -> DecodedRowsInfo;

        fn row_bytes(self: &Reader) -> u32;
        /// Select full-size (1) or reduced-IDCT output (2, 4, 8) after resetting
        /// decode state. Coded width/height and raw plane geometry stay unchanged.
        fn set_output_scale(self: &mut Reader, denominator: u8) -> DecodingResult;
        fn set_output_scale_ratio(
            self: &mut Reader,
            numerator: u8,
            denominator: u8,
        ) -> DecodingResult;
        fn output_height(self: &Reader) -> u32;
        fn supports_streaming_full_decode(self: &Reader) -> bool;
        fn supports_direct_packed_output(self: &Reader) -> bool;
        fn full_image_buffer_capacity_for_testing(self: &Reader) -> u64;
        fn reset_decode_state(self: &mut Reader);

        fn raw_component_count(self: &Reader) -> u32;
        fn raw_plane_info(self: &Reader, index: u32) -> RawPlaneInfo;
        fn input_complete(self: &Reader) -> bool;
        fn read_raw_imcu_row(
            self: &mut Reader,
            y: &mut [u8],
            y_row_bytes: u32,
            u: &mut [u8],
            u_row_bytes: u32,
            v: &mut [u8],
            v_row_bytes: u32,
        ) -> RawOutputResult;

        fn start_scanline_output(
            self: &mut Reader,
            format: PackedOutputFormat,
        ) -> ScanlineOutputResult;
        fn finish_scanline_output(self: &mut Reader) -> ScanlineOutputResult;
        fn read_scanlines(
            self: &mut Reader,
            output: &mut [u8],
            row_bytes: u32,
        ) -> ScanlineOutputResult;
        fn skip_scanlines(self: &mut Reader, rows: u32) -> ScanlineOutputResult;

        fn icc_profile(self: &Reader) -> Vec<u8>;
        fn exif_data(self: &Reader) -> Vec<u8>;

        fn segment_count(self: &Reader) -> u32;
        fn segment_info(self: &Reader, index: u32) -> JpegSegmentInfo;
        /// Return segment parameter data (excluding marker and length bytes).
        fn segment_data(self: &Reader, index: u32) -> Vec<u8>;
        fn segments_scanned(self: &Reader) -> bool;

        // MPF — parsed from scanned APP2 segments entirely in Rust.
        fn has_mpf(self: &Reader) -> bool;
        fn mpf_image_count(self: &Reader) -> u32;
        fn mpf_image_entry(self: &Reader, index: u32) -> MpfImageEntry;
        /// Byte offset of the MPF APP2 marker within the JPEG data.
        fn mpf_segment_offset(self: &Reader) -> u32;

        // Embedded JPEG scanner (for Multi-Picture / gainmap images).
        type EmbeddedJpegScanner;
        fn scan_embedded_jpeg(data: &[u8]) -> Box<EmbeddedJpegScanner>;
        fn embedded_had_error(self: &EmbeddedJpegScanner) -> bool;
        fn embedded_segment_count(self: &EmbeddedJpegScanner) -> u32;
        fn embedded_segment_info(self: &EmbeddedJpegScanner, index: u32) -> JpegSegmentInfo;
        fn embedded_segment_data(self: &EmbeddedJpegScanner, index: u32) -> Vec<u8>;

        // Encoder
        fn encode_jpeg(
            pixels: &[u8],
            width: u32,
            height: u32,
            row_bytes: u32,
            color_type: JpegEncodeColor,
            alpha_option: JpegEncodeAlpha,
            quality: u32,
            output: &mut Vec<u8>,
        ) -> EncodingResult;
    }
}

pub use ffi::*;

mod jpeg_metadata;
mod jpeg_multipicture;
pub(crate) mod jpeg_segment_scan;
pub(crate) mod tiff_ifd;

use jpeg_metadata::{extract_exif_data, extract_icc_profile};
use jpeg_multipicture::{find_mpf_in_segments, MpfResult};
use jpeg_segment_scan::jpeg_marker;
use jpeg_segment_scan::{get_segment_params, ScannedSegment, SegmentScanner};

use std::cell::RefCell;
use std::rc::Rc;

use zune_core::bytestream::{ZByteIoError, ZByteReaderTrait, ZSeekFrom};
use zune_core::colorspace::ColorSpace as ZuneColorSpace;
use zune_jpeg::{RawImcuRowStatus, ScanlineReadStatus, ScanlineStatus};

const MAX_DECODE_IMAGE_BYTES: usize = 512 * 1024 * 1024;

#[derive(Clone)]
struct GrowingJpegCursor {
    data: Rc<RefCell<Vec<u8>>>,
    position: usize,
}

impl GrowingJpegCursor {
    fn new(data: Rc<RefCell<Vec<u8>>>) -> Self {
        Self { data, position: 0 }
    }

    fn available_from_position(&self) -> usize {
        self.data.borrow().len().saturating_sub(self.position)
    }

    fn add_signed_offset(base: usize, offset: i64) -> Option<usize> {
        if offset >= 0 {
            base.checked_add(usize::try_from(offset).ok()?)
        } else {
            base.checked_sub(usize::try_from(offset.checked_neg()?).ok()?)
        }
    }
}

impl ZByteReaderTrait for GrowingJpegCursor {
    fn read_byte_no_error(&mut self) -> u8 {
        let byte = *self.data.borrow().get(self.position).unwrap_or(&0);
        self.position = self.position.wrapping_add(1);
        byte
    }

    fn read_exact_bytes(&mut self, buf: &mut [u8]) -> Result<(), ZByteIoError> {
        let available = self.available_from_position();
        if available < buf.len() {
            return Err(ZByteIoError::NotEnoughBytes(available, buf.len()));
        }

        let data = self.data.borrow();
        let end = self.position + buf.len();
        buf.copy_from_slice(&data[self.position..end]);
        self.position = end;
        Ok(())
    }

    fn read_bytes(&mut self, buf: &mut [u8]) -> Result<usize, ZByteIoError> {
        let bytes_to_read = self.available_from_position().min(buf.len());
        let data = self.data.borrow();
        let end = self.position + bytes_to_read;
        buf[..bytes_to_read].copy_from_slice(&data[self.position..end]);
        self.position = end;
        Ok(bytes_to_read)
    }

    fn peek_bytes(&mut self, buf: &mut [u8]) -> Result<usize, ZByteIoError> {
        let bytes_to_read = self.available_from_position().min(buf.len());
        let data = self.data.borrow();
        let end = self.position + bytes_to_read;
        buf[..bytes_to_read].copy_from_slice(&data[self.position..end]);
        Ok(bytes_to_read)
    }

    fn peek_exact_bytes(&mut self, buf: &mut [u8]) -> Result<(), ZByteIoError> {
        let available = self.available_from_position();
        if available < buf.len() {
            return Err(ZByteIoError::NotEnoughBytes(available, buf.len()));
        }

        let data = self.data.borrow();
        let end = self.position + buf.len();
        buf.copy_from_slice(&data[self.position..end]);
        Ok(())
    }

    fn z_seek(&mut self, from: ZSeekFrom) -> Result<u64, ZByteIoError> {
        let new_position = match from {
            ZSeekFrom::Start(position) => usize::try_from(position)?,
            ZSeekFrom::End(offset) => {
                let len = self.data.borrow().len();
                Self::add_signed_offset(len, offset)
                    .ok_or(ZByteIoError::SeekError("Negative seek"))?
            }
            ZSeekFrom::Current(offset) => Self::add_signed_offset(self.position, offset)
                .ok_or(ZByteIoError::SeekError("Negative seek"))?,
        };
        self.position = new_position;
        Ok(u64::try_from(self.position)?)
    }

    fn is_eof(&mut self) -> Result<bool, ZByteIoError> {
        Ok(self.position >= self.data.borrow().len())
    }

    fn z_position(&mut self) -> Result<u64, ZByteIoError> {
        Ok(u64::try_from(self.position)?)
    }

    fn read_remaining(&mut self, sink: &mut Vec<u8>) -> Result<usize, ZByteIoError> {
        let data = self.data.borrow();
        let start = self.position.min(data.len());
        sink.extend_from_slice(&data[start..]);
        let bytes_read = data.len() - start;
        self.position = data.len();
        Ok(bytes_read)
    }
}

type IncrementalJpegDecoder = zune_jpeg::JpegDecoder<GrowingJpegCursor>;

fn scanned_segment_info(segments: &[ScannedSegment], index: u32) -> JpegSegmentInfo {
    if let Some(seg) = segments.get(index as usize) {
        JpegSegmentInfo {
            marker: seg.marker,
            offset: seg.offset,
            parameter_length: seg.parameter_length,
        }
    } else {
        JpegSegmentInfo {
            marker: 0,
            offset: 0,
            parameter_length: 0,
        }
    }
}

fn scanned_segment_data(data: &[u8], segments: &[ScannedSegment], index: u32) -> Vec<u8> {
    if let Some(seg) = segments.get(index as usize) {
        get_segment_params(data, seg).to_vec()
    } else {
        Vec::new()
    }
}

/// Scanner for an embedded JPEG image (e.g. MPF gainmap).
///
/// Methods use the `embedded_` prefix because CXX exposes extern Rust
/// methods in a flat namespace — the prefix disambiguates from Reader
/// methods with similar names.
pub struct EmbeddedJpegScanner {
    scanner: SegmentScanner,
    // TODO: this copies the embedded image bytes so segment_data() can
    // reference them.  Consider having C++ pass the data slice on each
    // call instead, to avoid the allocation.
    data: Vec<u8>,
}

pub fn scan_embedded_jpeg(data: &[u8]) -> Box<EmbeddedJpegScanner> {
    let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
    scanner.on_bytes(data);
    Box::new(EmbeddedJpegScanner {
        scanner,
        data: data.to_vec(),
    })
}

impl EmbeddedJpegScanner {
    pub fn embedded_had_error(&self) -> bool {
        self.scanner.had_error()
    }

    pub fn embedded_segment_count(&self) -> u32 {
        self.scanner.segments.len() as u32
    }

    pub fn embedded_segment_info(&self, index: u32) -> JpegSegmentInfo {
        scanned_segment_info(&self.scanner.segments, index)
    }

    pub fn embedded_segment_data(&self, index: u32) -> Vec<u8> {
        scanned_segment_data(&self.data, &self.scanner.segments, index)
    }
}

/// Read all available bytes from a SkStreamAdapter.
/// Does not block — reads whatever is available.
fn read_available_from_stream(
    input: &mut cxx::UniquePtr<ffi::SkStreamAdapter>,
    sink: &mut Vec<u8>,
) -> Result<usize, ()> {
    let Some(mut stream) = input.as_mut() else {
        return Err(());
    };
    let initial_sink_len = sink.len();
    let mut buffer = [0u8; 8192];

    loop {
        let bytes_read = stream.as_mut().read(&mut buffer);
        if bytes_read == 0 {
            break;
        }
        sink.extend_from_slice(&buffer[..bytes_read]);
    }

    Ok(sink.len() - initial_sink_len)
}

/// Map zune-jpeg errors to our FFI DecodingResult.
fn map_zune_error(err: &zune_jpeg::errors::DecodeErrors) -> DecodingResult {
    use zune_jpeg::errors::DecodeErrors::*;
    match err {
        Unsupported(_) => DecodingResult::UnsupportedFeature,
        LargeDimensions(_) => DecodingResult::ParameterError,
        _ => DecodingResult::FormatError,
    }
}

/// Map a zune-core ColorSpace to our FFI JpegColor.
fn map_color_space(cs: ZuneColorSpace) -> (JpegColor, u32) {
    match cs {
        ZuneColorSpace::Luma | ZuneColorSpace::LumaA => (JpegColor::Grayscale, 1),
        // JPEG CMYK/YCCK is stored as inverted CMYK (255 = no ink).
        // We pass raw inverted CMYK to C++ where SkSwizzler handles the
        // CMYK→RGB conversion, optionally using a CMYK ICC profile.
        ZuneColorSpace::CMYK | ZuneColorSpace::YCCK => (JpegColor::InvertedCMYK, 4),
        _ => (JpegColor::RGB, 3),
    }
}

const JPEG_SIGNATURE: [u8; 2] = [0xFF, jpeg_marker::SOI];

fn classify_jpeg_signature(data: &[u8]) -> DecodingResult {
    if data.len() < JPEG_SIGNATURE.len() {
        return if JPEG_SIGNATURE.starts_with(data) {
            DecodingResult::IncompleteInput
        } else {
            DecodingResult::FormatError
        };
    }
    if data.starts_with(&JPEG_SIGNATURE) {
        DecodingResult::Success
    } else {
        DecodingResult::FormatError
    }
}

pub fn is_jpeg_data(data: &[u8]) -> bool {
    matches!(classify_jpeg_signature(data), DecodingResult::Success)
}

/// Streaming JPEG reader wrapping zune-jpeg with Rust-side segment scanning.
pub struct Reader {
    stream: Option<cxx::UniquePtr<ffi::SkStreamAdapter>>,
    initialization_result: DecodingResult,
    raw_data: Rc<RefCell<Vec<u8>>>,
    stream_exhausted: bool,
    orig_color_space: ZuneColorSpace,
    metadata_loaded: bool,
    width: u32,
    height: u32,
    color: JpegColor,
    alpha: JpegAlpha,
    bytes_per_pixel: u32,
    output_format: PackedOutputFormat,
    supports_streaming_full_decode: bool,
    image_data: Vec<u8>,
    image_data_loaded: bool,
    /// Append-only stable output prefix. May be less than height for truncated images.
    valid_rows: u32,
    /// Stable rows already returned by `get_next_rows`.
    last_consumed_row_count: u32,
    /// Progressive previews are replaceable full-frame renders. Track the
    /// completed scan represented by the current preview separately from the
    /// append-only stable row prefix.
    preview_rows: u32,
    preview_scan_count: usize,
    /// Last preview generation returned by `get_next_rows`.
    last_consumed_preview_scan_count: usize,
    // RawDecodeSession and ScanlineDecodeSession are short-lived borrow guards.
    // This decoder owns their pull/checkpoint state across FFI calls; no caller
    // slice or pointer is retained after a bridge method returns.
    incremental_decoder: Option<IncrementalJpegDecoder>,
    output_scale: Option<zune_jpeg::JpegScale>,
    scaled_decoder: Option<zune_jpeg::ScaledJpegDecoder<GrowingJpegCursor>>,
    scaled_scanline_row: u32,
    raw_component_count: u32,
    raw_plane_info: [RawPlaneInfo; 4],
    mcu_width: u32,
    mcu_height: u32,
    /// Constructor-owned source finality, independent of temporary stream EOF.
    input_complete: bool,
    output_sequence: OutputSequence,

    scanner: SegmentScanner,
    scanner_bytes_consumed: usize,
    cached_icc: Option<Vec<u8>>,
    cached_exif: Option<Vec<u8>>,
    /// `None` = not yet scanned; `Some(None)` = scanned, no MPF found.
    cached_mpf: Option<Option<MpfResult>>,
}

#[derive(Clone, Copy, PartialEq)]
enum OutputSequence {
    Idle,
    Raw { complete: bool },
    Scanline { complete: bool },
    Incremental,
    FullImage,
}

fn decoded_row_slice(
    image_data: &[u8],
    row_start: u32,
    row_count: u32,
    row_bytes: usize,
) -> Option<&[u8]> {
    let start = (row_start as usize).checked_mul(row_bytes)?;
    let size = (row_count as usize).checked_mul(row_bytes)?;
    let end = start.checked_add(size)?;
    image_data.get(start..end)
}

fn validate_stream_length(stream_length: u64) -> DecodingResult {
    match usize::try_from(stream_length) {
        Ok(capacity) if capacity <= MAX_DECODE_IMAGE_BYTES => DecodingResult::Success,
        _ => DecodingResult::MemoryError,
    }
}

impl Reader {
    /// Create a new reader without reading any data from the stream.
    /// The stream is stored for later incremental reads.
    fn new(
        input: cxx::UniquePtr<ffi::SkStreamAdapter>,
        stream_length: u64,
        input_complete: bool,
    ) -> Self {
        Reader {
            stream: Some(input),
            initialization_result: validate_stream_length(stream_length),
            raw_data: Rc::new(RefCell::new(Vec::new())),
            stream_exhausted: false,
            orig_color_space: ZuneColorSpace::RGB,
            metadata_loaded: false,
            width: 0,
            height: 0,
            color: JpegColor::RGB,
            alpha: JpegAlpha::Opaque,
            bytes_per_pixel: 3,
            output_format: PackedOutputFormat::Source,
            supports_streaming_full_decode: false,
            image_data: Vec::new(),
            image_data_loaded: false,
            valid_rows: 0,
            last_consumed_row_count: 0,
            preview_rows: 0,
            preview_scan_count: 0,
            last_consumed_preview_scan_count: 0,
            incremental_decoder: None,
            output_scale: None,
            scaled_decoder: None,
            scaled_scanline_row: 0,
            raw_component_count: 0,
            raw_plane_info: [RawPlaneInfo::default(); 4],
            mcu_width: 0,
            mcu_height: 0,
            input_complete,
            output_sequence: OutputSequence::Idle,
            // Stop at SOS — we only need header segments for metadata.
            scanner: SegmentScanner::new(jpeg_marker::SOS),
            scanner_bytes_consumed: 0,
            cached_icc: None,
            cached_exif: None,
            cached_mpf: None,
        }
    }

    /// Try to read more data from the stream into raw_data, and feed
    /// any new bytes to the segment scanner.
    fn try_read_more(&mut self) {
        if self.stream_exhausted {
            return;
        }
        if let Some(ref mut stream) = self.stream {
            match read_available_from_stream(stream, &mut self.raw_data.borrow_mut()) {
                Ok(bytes_read) => {
                    if bytes_read == 0 {
                        self.stream_exhausted = true;
                    }
                }
                Err(()) => {
                    self.stream_exhausted = true;
                }
            }
        }

        // Feed any new bytes to the segment scanner.
        self.advance_scanner();
    }

    fn advance_scanner(&mut self) {
        if self.scanner.is_done() || self.scanner.had_error() {
            return;
        }
        let raw_data = self.raw_data.borrow();
        if self.scanner_bytes_consumed < raw_data.len() {
            let new_bytes = &raw_data[self.scanner_bytes_consumed..];
            self.scanner.on_bytes(new_bytes);
            self.scanner_bytes_consumed = raw_data.len();
        }
        // Populate caches once scanning completes.
        if self.scanner.is_done() && self.cached_icc.is_none() {
            self.cached_icc = Some(extract_icc_profile(&raw_data, &self.scanner.segments));
            if self.cached_exif.is_none() {
                self.cached_exif = Some(extract_exif_data(&raw_data, &self.scanner.segments));
            }
            if self.cached_mpf.is_none() {
                self.cached_mpf = Some(find_mpf_in_segments(&raw_data, &self.scanner.segments));
            }
        }
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color(&self) -> JpegColor {
        self.color
    }

    pub fn alpha(&self) -> JpegAlpha {
        self.alpha
    }

    pub fn metadata_loaded(&self) -> bool {
        self.metadata_loaded
    }

    pub fn initialization_result(&self) -> DecodingResult {
        self.initialization_result
    }

    pub fn read_metadata(&mut self) -> DecodingResult {
        if self.metadata_loaded {
            return DecodingResult::Success;
        }

        self.try_read_more();

        let raw_data = self.raw_data.borrow();
        match classify_jpeg_signature(&raw_data) {
            DecodingResult::Success => {}
            result => return result,
        }
        drop(raw_data);

        let options = zune_core::options::DecoderOptions::default().set_strict_mode(false);
        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            GrowingJpegCursor::new(Rc::clone(&self.raw_data)),
            options,
        );
        decoder.set_incremental_mode(!self.input_complete);

        match decoder.decode_headers() {
            Ok(()) => {}
            Err(ref e) if e.is_recoverable_eof() => {
                return DecodingResult::IncompleteInput;
            }
            Err(ref e) => {
                return map_zune_error(e);
            }
        }

        let (width, height) = match decoder.dimensions() {
            Some(dims) => dims,
            None => return DecodingResult::FormatError,
        };
        let supports_streaming_full_decode = decoder
            .info()
            // DNL dimensions are unknown until entropy decoding, so SkCodec
            // cannot advertise or allocate the scanline-backed full decode.
            .is_some_and(|info| info.sof.is_sequential_dct() && info.height != 0);

        let cs = decoder.input_colorspace().unwrap_or(ZuneColorSpace::RGB);
        let mut raw_component_count = 0;
        let mut raw_plane_info = [RawPlaneInfo::default(); 4];
        {
            let raw = decoder.raw_output();
            // Source MCU geometry also applies to grayscale, RGB, CMYK and
            // sampling layouts that SkCodec's YUV API cannot represent.
            let Some(layout) = raw.layout() else {
                return DecodingResult::FormatError;
            };
            let max_h = layout
                .iter()
                .map(|plane| plane.horizontal_sampling_factor)
                .max();
            let max_v = layout
                .iter()
                .map(|plane| plane.vertical_sampling_factor)
                .max();
            let mcu_size = max_h
                .zip(max_v)
                .and_then(|(h, v)| Some((u32::try_from(h).ok()?, u32::try_from(v).ok()?)))
                .and_then(|(h, v)| Some((h.checked_mul(8)?, v.checked_mul(8)?)));
            let Some((mcu_width, mcu_height)) = mcu_size else {
                return DecodingResult::FormatError;
            };
            if mcu_width == 0 || mcu_height == 0 {
                return DecodingResult::FormatError;
            }
            self.mcu_width = mcu_width;
            self.mcu_height = mcu_height;
            if let (Some(count), Some(ids), Some(layout)) =
                (raw.num_components(), raw.component_ids(), raw.layout())
            {
                raw_component_count = u32::try_from(count).unwrap_or(0);
                for index in 0..count
                    .min(raw_plane_info.len())
                    .min(ids.len())
                    .min(layout.len())
                {
                    let plane = layout[index];
                    let converted = (
                        u32::try_from(plane.horizontal_sampling_factor),
                        u32::try_from(plane.vertical_sampling_factor),
                        u32::try_from(plane.width),
                        u32::try_from(plane.height),
                        u32::try_from(plane.stride),
                    );
                    if let (Ok(h), Ok(v), Ok(width), Ok(height), Ok(row_bytes)) = converted {
                        raw_plane_info[index] = RawPlaneInfo {
                            component_id: ids[index],
                            horizontal_sampling_factor: h,
                            vertical_sampling_factor: v,
                            width,
                            height,
                            row_bytes,
                        };
                    } else {
                        raw_component_count = 0;
                        raw_plane_info = [RawPlaneInfo::default(); 4];
                        break;
                    }
                }
            }
        }
        let (color, bytes_per_pixel) = map_color_space(cs);
        let (width, height) = match (u32::try_from(width), u32::try_from(height)) {
            (Ok(width), Ok(height)) => (width, height),
            _ => return DecodingResult::MemoryError,
        };
        if width.checked_mul(bytes_per_pixel).is_none() {
            return DecodingResult::MemoryError;
        }

        self.width = width;
        self.height = height;
        self.color = color;
        self.alpha = JpegAlpha::Opaque;
        self.bytes_per_pixel = bytes_per_pixel;
        self.orig_color_space = cs;
        decoder.set_options(self.output_decoder_options());
        self.incremental_decoder = Some(decoder);
        self.supports_streaming_full_decode = supports_streaming_full_decode;
        if matches!(cs, ZuneColorSpace::YCbCr) {
            self.raw_component_count = raw_component_count;
            self.raw_plane_info = raw_plane_info;
        }
        self.metadata_loaded = true;

        DecodingResult::Success
    }

    pub fn read_image_data(&mut self) -> DecodingResult {
        if !matches!(
            self.output_sequence,
            OutputSequence::Idle | OutputSequence::FullImage
        ) {
            return DecodingResult::ParameterError;
        }
        if self.image_data_loaded {
            return DecodingResult::Success;
        }

        if !self.metadata_loaded {
            return DecodingResult::ParameterError;
        }
        self.output_sequence = OutputSequence::FullImage;

        self.stream_exhausted = false;
        self.try_read_more();

        if self.output_scale.is_some() {
            return self.read_scaled_image_data();
        }

        let target_cs = match self.orig_color_space {
            ZuneColorSpace::Luma | ZuneColorSpace::LumaA => ZuneColorSpace::Luma,
            // Output raw CMYK so C++ can swizzle with ICC profile support.
            ZuneColorSpace::CMYK | ZuneColorSpace::YCCK => ZuneColorSpace::CMYK,
            _ => ZuneColorSpace::RGB,
        };

        // zune-jpeg defaults to a 100-scan limit for progressive JPEGs
        // (matching Blink's ProgressMonitor), so we don't need to set it.
        let options = zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .jpeg_set_out_colorspace(target_cs);

        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            GrowingJpegCursor::new(Rc::clone(&self.raw_data)),
            options,
        );
        let total_bytes = match (self.width as usize)
            .checked_mul(self.height as usize)
            .and_then(|v| v.checked_mul(self.bytes_per_pixel as usize))
        {
            Some(n) => n,
            None => return DecodingResult::MemoryError,
        };
        if total_bytes > MAX_DECODE_IMAGE_BYTES {
            return DecodingResult::MemoryError;
        }

        let mut image_data = Vec::new();
        if image_data.try_reserve_exact(total_bytes).is_err() {
            return DecodingResult::MemoryError;
        }
        image_data.resize(total_bytes, 0);
        self.image_data = image_data;

        match decoder.decode_into(&mut self.image_data) {
            Ok(()) => {
                self.valid_rows = self.height;
                self.image_data_loaded = true;
                DecodingResult::Success
            }
            Err(ref e) => {
                if !self.stream_exhausted {
                    self.image_data.clear();
                    return DecodingResult::IncompleteInput;
                }
                let rows = decoder
                    .decoded_scanlines()
                    .and_then(|rows| u32::try_from(rows).ok())
                    .unwrap_or(0)
                    .min(self.height);
                if rows > 0 {
                    self.valid_rows = rows;
                    self.image_data_loaded = true;
                    return DecodingResult::Success;
                }
                self.image_data.clear();
                map_zune_error(e)
            }
        }
    }

    pub fn read_incremental_image_data(&mut self) -> DecodingResult {
        if !matches!(
            self.output_sequence,
            OutputSequence::Idle | OutputSequence::Incremental
        ) {
            return DecodingResult::ParameterError;
        }
        if self.image_data_loaded {
            return DecodingResult::Success;
        }

        if !self.metadata_loaded {
            return DecodingResult::ParameterError;
        }
        self.output_sequence = OutputSequence::Incremental;

        // A zero-byte read during incremental decode only means that no new
        // bytes are visible for this attempt.  The SkCodec caller may make
        // more bytes visible and call us again.
        self.stream_exhausted = false;
        self.try_read_more();

        if self.output_scale.is_some() {
            return self.read_scaled_image_data();
        }

        let total_bytes = match (self.width as usize)
            .checked_mul(self.height as usize)
            .and_then(|v| v.checked_mul(self.bytes_per_pixel as usize))
        {
            Some(n) => n,
            None => return DecodingResult::MemoryError,
        };
        if total_bytes > MAX_DECODE_IMAGE_BYTES {
            return DecodingResult::MemoryError;
        }
        if self.image_data.len() != total_bytes {
            let mut image_data = Vec::new();
            if image_data.try_reserve_exact(total_bytes).is_err() {
                return DecodingResult::MemoryError;
            }
            image_data.resize(total_bytes, 0);
            self.image_data = image_data;
        }

        if self.incremental_decoder.is_none() {
            let target_cs = match self.orig_color_space {
                ZuneColorSpace::Luma | ZuneColorSpace::LumaA => ZuneColorSpace::Luma,
                ZuneColorSpace::CMYK | ZuneColorSpace::YCCK => ZuneColorSpace::CMYK,
                _ => ZuneColorSpace::RGB,
            };
            let options = zune_core::options::DecoderOptions::default()
                .set_strict_mode(false)
                .jpeg_set_out_colorspace(target_cs);
            let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
                GrowingJpegCursor::new(Rc::clone(&self.raw_data)),
                options,
            );
            decoder.set_incremental_mode(!self.input_complete);
            self.incremental_decoder = Some(decoder);
        }

        let decoder = self.incremental_decoder.as_mut().unwrap();
        match decoder.decode_into(&mut self.image_data) {
            Ok(()) => {
                let rows = decoder.buffered_output_rows();
                self.finish_buffered_image(rows)
            }
            Err(ref e) if e.is_recoverable_eof() => {
                self.valid_rows = decoder
                    .decoded_scanlines()
                    .and_then(|rows| u32::try_from(rows).ok())
                    .unwrap_or(0)
                    .min(self.height);
                self.preview_rows = decoder
                    .decoded_preview_scanlines()
                    .and_then(|rows| u32::try_from(rows).ok())
                    .unwrap_or(0)
                    .min(self.height);
                self.preview_scan_count = decoder.decoded_scans().unwrap_or(0);
                DecodingResult::IncompleteInput
            }
            Err(ref e) => {
                self.image_data.clear();
                map_zune_error(e)
            }
        }
    }

    pub fn image_data_loaded(&self) -> bool {
        self.image_data_loaded
    }

    fn finish_buffered_image(&mut self, buffered_rows: Option<usize>) -> DecodingResult {
        // Full-frame reads retain best-effort output. Incremental reads expose
        // only the valid buffered prefix, including zero rows for tainted
        // buffered sequential images.
        let height = self.output_height();
        self.valid_rows = if self.output_sequence == OutputSequence::Incremental {
            buffered_rows.map_or(height, |rows| {
                let coded_rows = rows.min(self.height as usize) as u32;
                self.output_dimension(coded_rows).min(height)
            })
        } else {
            height
        };
        self.preview_rows = 0;
        self.preview_scan_count = 0;
        self.image_data_loaded = self.valid_rows == height;
        if self.image_data_loaded {
            DecodingResult::Success
        } else {
            DecodingResult::IncompleteInput
        }
    }

    pub fn get_next_rows<'a>(&'a mut self, buffer: &mut &'a [u8]) -> DecodedRowsInfo {
        // Successful output is final even when the last incremental attempt
        // previously exposed a progressive preview.
        if !self.image_data_loaded
            && self.preview_rows > 0
            && self.preview_scan_count > self.last_consumed_preview_scan_count
        {
            let Some(preview) = decoded_row_slice(
                &self.image_data,
                0,
                self.preview_rows,
                self.row_bytes() as usize,
            ) else {
                *buffer = &[];
                return DecodedRowsInfo {
                    dst_row_start: 0,
                    row_count: 0,
                    is_preview: false,
                };
            };
            self.last_consumed_preview_scan_count = self.preview_scan_count;
            *buffer = preview;
            return DecodedRowsInfo {
                dst_row_start: 0,
                row_count: self.preview_rows,
                is_preview: true,
            };
        }

        let already_consumed = self.last_consumed_row_count;
        let row_bytes = self.row_bytes() as usize;

        // `decoded_scanlines()` is an append-only stable prefix and is useful
        // before the complete image has loaded (notably for baseline JPEGs).
        let current_rows = self.valid_rows;

        if current_rows <= already_consumed {
            *buffer = &[];
            return DecodedRowsInfo {
                dst_row_start: 0,
                row_count: 0,
                is_preview: false,
            };
        }

        let new_row_count = current_rows - already_consumed;
        let Some(rows) =
            decoded_row_slice(&self.image_data, already_consumed, new_row_count, row_bytes)
        else {
            *buffer = &[];
            return DecodedRowsInfo {
                dst_row_start: 0,
                row_count: 0,
                is_preview: false,
            };
        };
        self.last_consumed_row_count = current_rows;
        *buffer = rows;

        DecodedRowsInfo {
            dst_row_start: already_consumed,
            row_count: new_row_count,
            is_preview: false,
        }
    }

    fn read_scaled_image_data(&mut self) -> DecodingResult {
        let Some(scale) = self.output_scale else {
            return DecodingResult::ParameterError;
        };
        if self.scaled_decoder.is_none() {
            let decoder = zune_jpeg::JpegDecoder::new_with_options(
                GrowingJpegCursor::new(Rc::clone(&self.raw_data)),
                self.output_decoder_options(),
            );
            let mut decoder = match decoder.into_scaled(scale) {
                Ok(decoder) => decoder,
                Err(error) if error.is_recoverable_eof() => {
                    return DecodingResult::IncompleteInput;
                }
                Err(error) => return map_zune_error(&error),
            };
            // One-shot decoding retains legacy best effort. Growing incremental
            // and scanline reads must not finalize temporary EOF.
            decoder.set_incremental_mode(
                self.output_sequence != OutputSequence::FullImage && !self.input_complete,
            );
            self.scaled_decoder = Some(decoder);
        }
        let decoder = self.scaled_decoder.as_mut().unwrap();
        let Some(length) = decoder.output_buffer_size() else {
            return DecodingResult::MemoryError;
        };
        if length > MAX_DECODE_IMAGE_BYTES {
            return DecodingResult::MemoryError;
        }
        if self.image_data.len() != length {
            let mut image_data = Vec::new();
            if image_data.try_reserve_exact(length).is_err() {
                return DecodingResult::MemoryError;
            }
            image_data.resize(length, 0);
            self.image_data = image_data;
        }
        match decoder.decode_into(&mut self.image_data) {
            Ok(()) => {
                let rows = decoder.buffered_output_rows();
                self.finish_buffered_image(rows)
            }
            Err(error) if error.is_recoverable_eof() => DecodingResult::IncompleteInput,
            Err(error) => map_zune_error(&error),
        }
    }

    pub fn set_output_scale(&mut self, denominator: u8) -> DecodingResult {
        self.set_output_scale_ratio(1, denominator)
    }

    pub fn set_output_scale_ratio(&mut self, numerator: u8, denominator: u8) -> DecodingResult {
        if self.output_sequence != OutputSequence::Idle {
            return DecodingResult::ParameterError;
        }
        if denominator == 0 || numerator == 0 || numerator > denominator {
            return DecodingResult::ParameterError;
        }
        let numerator = u16::from(numerator);
        let denominator = u16::from(denominator);
        self.output_scale = match (numerator * 8 / denominator, numerator * 8 % denominator) {
            (8, 0) => None,
            (7, 0) => Some(zune_jpeg::JpegScale::SevenEighths),
            (6, 0) => Some(zune_jpeg::JpegScale::ThreeQuarters),
            (5, 0) => Some(zune_jpeg::JpegScale::FiveEighths),
            (4, 0) => Some(zune_jpeg::JpegScale::Half),
            (3, 0) => Some(zune_jpeg::JpegScale::ThreeEighths),
            (2, 0) => Some(zune_jpeg::JpegScale::Quarter),
            (1, 0) => Some(zune_jpeg::JpegScale::Eighth),
            _ => return DecodingResult::ParameterError,
        };
        self.scaled_decoder = None;
        DecodingResult::Success
    }

    pub fn output_height(&self) -> u32 {
        self.scaled_decoder.as_ref().map_or_else(
            || self.output_dimension(self.height),
            |decoder| decoder.dimensions().1 as u32,
        )
    }

    fn output_denominator(&self) -> u32 {
        self.output_scale
            .map_or(1, |scale| scale.denominator() as u32)
    }

    fn output_dimension(&self, dimension: u32) -> u32 {
        let numerator = self
            .output_scale
            .map_or(1, |scale| scale.numerator() as u32);
        // Every supported ratio is at most one, so the result fits in u32.
        (u64::from(dimension) * u64::from(numerator)).div_ceil(u64::from(self.output_denominator()))
            as u32
    }

    pub fn row_bytes(&self) -> u32 {
        let bytes_per_pixel = match self.output_format {
            PackedOutputFormat::RGBA | PackedOutputFormat::BGRA => 4,
            PackedOutputFormat::Source => self.bytes_per_pixel,
            _ => return 0,
        };
        let width = self.scaled_decoder.as_ref().map_or_else(
            || self.output_dimension(self.width),
            |decoder| decoder.dimensions().0 as u32,
        );
        width.checked_mul(bytes_per_pixel).unwrap_or(0)
    }

    pub fn supports_streaming_full_decode(&self) -> bool {
        self.supports_streaming_full_decode && self.output_scale.is_none()
    }

    pub fn supports_direct_packed_output(&self) -> bool {
        matches!(self.orig_color_space, ZuneColorSpace::YCbCr)
    }

    pub fn full_image_buffer_capacity_for_testing(&self) -> u64 {
        u64::try_from(self.image_data.capacity()).unwrap_or(u64::MAX)
    }

    pub fn reset_decode_state(&mut self) {
        self.output_scale = None;
        self.scaled_decoder = None;
        self.scaled_scanline_row = 0;
        self.image_data.clear();
        self.image_data_loaded = false;
        self.valid_rows = 0;
        self.last_consumed_row_count = 0;
        self.preview_rows = 0;
        self.preview_scan_count = 0;
        self.last_consumed_preview_scan_count = 0;
        self.output_sequence = OutputSequence::Idle;
        self.output_format = PackedOutputFormat::Source;
        let options = self.output_decoder_options();
        if let Some(decoder) = self.incremental_decoder.as_mut() {
            decoder.set_options(options);
        }
        self.stream_exhausted = false;
    }

    fn output_decoder_options(&self) -> zune_core::options::DecoderOptions {
        let target_cs = match self.output_format {
            PackedOutputFormat::RGBA => ZuneColorSpace::RGBA,
            PackedOutputFormat::BGRA => ZuneColorSpace::BGRA,
            PackedOutputFormat::Source => match self.orig_color_space {
                ZuneColorSpace::Luma | ZuneColorSpace::LumaA => ZuneColorSpace::Luma,
                ZuneColorSpace::CMYK | ZuneColorSpace::YCCK => ZuneColorSpace::CMYK,
                _ => ZuneColorSpace::RGB,
            },
            _ => ZuneColorSpace::RGB,
        };
        zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .jpeg_set_out_colorspace(target_cs)
    }

    fn prepare_output_decoder(&mut self) {
        self.stream_exhausted = false;
        self.try_read_more();
        if self.incremental_decoder.is_none() {
            let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
                GrowingJpegCursor::new(Rc::clone(&self.raw_data)),
                self.output_decoder_options(),
            );
            decoder.set_incremental_mode(!self.input_complete);
            self.incremental_decoder = Some(decoder);
        }
    }

    pub fn raw_component_count(&self) -> u32 {
        self.raw_component_count
    }

    pub fn raw_plane_info(&self, index: u32) -> RawPlaneInfo {
        self.raw_plane_info
            .get(index as usize)
            .copied()
            .unwrap_or_default()
    }

    pub fn mcu_width(&self) -> u32 {
        self.mcu_width
    }

    pub fn mcu_height(&self) -> u32 {
        self.mcu_height
    }

    pub fn input_complete(&self) -> bool {
        self.input_complete
    }

    pub fn read_raw_imcu_row(
        &mut self,
        y: &mut [u8],
        y_row_bytes: u32,
        u: &mut [u8],
        u_row_bytes: u32,
        v: &mut [u8],
        v_row_bytes: u32,
    ) -> RawOutputResult {
        let error = |result| RawOutputResult {
            status: OutputStatus::Error,
            result,
            y_rows: 0,
            u_rows: 0,
            v_rows: 0,
        };
        if self.raw_component_count != 3 {
            return error(DecodingResult::ParameterError);
        }
        match self.output_sequence {
            OutputSequence::Idle => {
                self.output_sequence = OutputSequence::Raw { complete: false };
            }
            OutputSequence::Raw { complete: false } => {}
            OutputSequence::Raw { complete: true } => {
                return RawOutputResult {
                    status: OutputStatus::Complete,
                    result: DecodingResult::Success,
                    y_rows: 0,
                    u_rows: 0,
                    v_rows: 0,
                };
            }
            _ => return error(DecodingResult::ParameterError),
        }
        self.prepare_output_decoder();
        let Some(decoder) = self.incremental_decoder.as_mut() else {
            return error(DecodingResult::OtherError);
        };
        if let Err(ref decode_error) = decoder.decode_headers() {
            if decode_error.is_recoverable_eof() {
                return RawOutputResult {
                    status: OutputStatus::NeedMoreInput,
                    result: DecodingResult::IncompleteInput,
                    y_rows: 0,
                    u_rows: 0,
                    v_rows: 0,
                };
            }
            return error(map_zune_error(decode_error));
        }

        let strides = [
            y_row_bytes as usize,
            u_row_bytes as usize,
            v_row_bytes as usize,
        ];
        let mut planes = [y, u, v];
        let status = {
            let mut raw = decoder.raw_output();
            if self.input_complete {
                raw.set_input_complete();
            }
            raw.decode_next_imcu_row(&mut planes, &strides)
        };
        let rows_written = match status {
            Ok(RawImcuRowStatus::RowReady { rows_written }) => rows_written,
            Ok(RawImcuRowStatus::NeedMoreInput) => {
                return RawOutputResult {
                    status: OutputStatus::NeedMoreInput,
                    result: DecodingResult::IncompleteInput,
                    y_rows: 0,
                    u_rows: 0,
                    v_rows: 0,
                }
            }
            Ok(RawImcuRowStatus::Complete) => {
                self.output_sequence = OutputSequence::Raw { complete: true };
                return RawOutputResult {
                    status: OutputStatus::Complete,
                    result: DecodingResult::Success,
                    y_rows: 0,
                    u_rows: 0,
                    v_rows: 0,
                };
            }
            Ok(_) => return error(DecodingResult::OtherError),
            Err(ref decode_error) if decode_error.is_recoverable_eof() => {
                return error(DecodingResult::IncompleteInput)
            }
            Err(ref decode_error) => return error(map_zune_error(decode_error)),
        };
        // Recovered component buffers do not make a corrupt sequential source publishable.
        // Keep raw recovery intact, but do not report these planes as successfully decoded.
        if decoder.decoded_scans().is_none() && decoder.buffered_output_rows() == Some(0) {
            return error(DecodingResult::IncompleteInput);
        }
        RawOutputResult {
            status: OutputStatus::Ready,
            result: DecodingResult::Success,
            y_rows: u32::try_from(rows_written[0]).unwrap_or(0),
            u_rows: u32::try_from(rows_written[1]).unwrap_or(0),
            v_rows: u32::try_from(rows_written[2]).unwrap_or(0),
        }
    }

    fn start_scaled_scanline_output(&mut self, format: PackedOutputFormat) -> ScanlineOutputResult {
        if !self.metadata_loaded || format != PackedOutputFormat::Source {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::ParameterError,
                rows: 0,
            };
        }
        match self.output_sequence {
            OutputSequence::Idle => {
                self.output_sequence = OutputSequence::Scanline { complete: false };
                self.scaled_scanline_row = 0;
            }
            OutputSequence::Scanline { complete: true } => {
                return self.map_scanline_read_status(Ok(ScanlineReadStatus::Complete));
            }
            OutputSequence::Scanline { complete: false } => {}
            _ => {
                return ScanlineOutputResult {
                    status: OutputStatus::Error,
                    result: DecodingResult::ParameterError,
                    rows: 0,
                };
            }
        }
        if !self.image_data_loaded {
            self.stream_exhausted = false;
            self.try_read_more();
            let result = self.read_scaled_image_data();
            if result != DecodingResult::Success {
                return ScanlineOutputResult {
                    status: if result == DecodingResult::IncompleteInput {
                        OutputStatus::NeedMoreInput
                    } else {
                        OutputStatus::Error
                    },
                    result,
                    rows: 0,
                };
            }
        }
        if self.scaled_scanline_row == self.output_height() {
            return self.map_scanline_read_status(Ok(ScanlineReadStatus::Complete));
        }
        ScanlineOutputResult {
            status: OutputStatus::Ready,
            result: DecodingResult::Success,
            rows: 0,
        }
    }

    fn read_scaled_scanlines(&mut self, output: &mut [u8], row_bytes: u32) -> ScanlineOutputResult {
        let ready = self.start_scaled_scanline_output(self.output_format);
        if ready.status != OutputStatus::Ready {
            return ready;
        }
        let source_bytes = self.row_bytes() as usize;
        let row_bytes = row_bytes as usize;
        if row_bytes < source_bytes || source_bytes == 0 || output.len() < source_bytes {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::ParameterError,
                rows: 0,
            };
        }
        let capacity = 1 + (output.len() - source_bytes) / row_bytes;
        let rows = capacity.min((self.output_height() - self.scaled_scanline_row) as usize);
        let Some(source) = decoded_row_slice(
            &self.image_data,
            self.scaled_scanline_row,
            rows as u32,
            source_bytes,
        ) else {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::OtherError,
                rows: 0,
            };
        };
        for row in 0..rows {
            output[row * row_bytes..row * row_bytes + source_bytes]
                .copy_from_slice(&source[row * source_bytes..(row + 1) * source_bytes]);
        }
        self.scaled_scanline_row += rows as u32;
        self.map_scanline_read_status(Ok(ScanlineReadStatus::RowsProcessed { rows }))
    }

    pub fn start_scanline_output(&mut self, format: PackedOutputFormat) -> ScanlineOutputResult {
        if self.output_scale.is_some() {
            return self.start_scaled_scanline_output(format);
        }
        // A loaded image is necessarily in Incremental or FullImage sequence state and is rejected
        // below. reset_decode_state() clears both the sequence and image_data_loaded together.
        let complete = match self.output_sequence {
            OutputSequence::Idle => {
                if format != PackedOutputFormat::Source && !self.supports_direct_packed_output() {
                    return ScanlineOutputResult {
                        status: OutputStatus::Error,
                        result: DecodingResult::UnsupportedFeature,
                        rows: 0,
                    };
                }
                self.output_format = format;
                let options = self.output_decoder_options();
                if let Some(decoder) = self.incremental_decoder.as_mut() {
                    decoder.set_options(options);
                }
                self.output_sequence = OutputSequence::Scanline { complete: false };
                false
            }
            OutputSequence::Scanline { complete } => {
                if self.output_format != format {
                    return ScanlineOutputResult {
                        status: OutputStatus::Error,
                        result: DecodingResult::ParameterError,
                        rows: 0,
                    };
                }
                complete
            }
            _ => {
                return ScanlineOutputResult {
                    status: OutputStatus::Error,
                    result: DecodingResult::ParameterError,
                    rows: 0,
                };
            }
        };
        if complete {
            return ScanlineOutputResult {
                status: OutputStatus::Complete,
                result: DecodingResult::Success,
                rows: 0,
            };
        }
        self.prepare_output_decoder();
        let Some(decoder) = self.incremental_decoder.as_mut() else {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::OtherError,
                rows: 0,
            };
        };
        let status = {
            let mut scanlines = decoder.scanline_output();
            scanlines.start()
        };
        match status {
            Ok(ScanlineStatus::Ready) => ScanlineOutputResult {
                status: OutputStatus::Ready,
                result: DecodingResult::Success,
                rows: 0,
            },
            Ok(ScanlineStatus::NeedMoreInput) => ScanlineOutputResult {
                status: OutputStatus::NeedMoreInput,
                result: DecodingResult::IncompleteInput,
                rows: 0,
            },
            Ok(ScanlineStatus::Complete) => {
                self.output_sequence = OutputSequence::Scanline { complete: true };
                ScanlineOutputResult {
                    status: OutputStatus::Complete,
                    result: DecodingResult::Success,
                    rows: 0,
                }
            }
            Ok(_) => unreachable!(),
            Err(ref decode_error) => ScanlineOutputResult {
                status: OutputStatus::Error,
                result: map_zune_error(decode_error),
                rows: 0,
            },
        }
    }

    pub fn read_scanlines(&mut self, output: &mut [u8], row_bytes: u32) -> ScanlineOutputResult {
        if !matches!(self.output_sequence, OutputSequence::Scanline { .. }) {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::ParameterError,
                rows: 0,
            };
        }
        if self.output_scale.is_some() {
            return self.read_scaled_scanlines(output, row_bytes);
        }
        self.prepare_output_decoder();
        let Some(decoder) = self.incremental_decoder.as_mut() else {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::OtherError,
                rows: 0,
            };
        };
        let status = {
            let mut scanlines = decoder.scanline_output();
            scanlines.read_scanlines(output, row_bytes as usize)
        };
        self.map_scanline_read_status(status)
    }

    pub fn finish_scanline_output(&mut self) -> ScanlineOutputResult {
        if !matches!(self.output_sequence, OutputSequence::Scanline { .. }) {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::ParameterError,
                rows: 0,
            };
        }
        if self.output_scale.is_some() {
            let ready = self.start_scaled_scanline_output(self.output_format);
            if ready.status != OutputStatus::Ready {
                return ready;
            }
            self.scaled_scanline_row = self.output_height();
            return self.map_scanline_read_status(Ok(ScanlineReadStatus::Complete));
        }
        self.prepare_output_decoder();
        let Some(decoder) = self.incremental_decoder.as_mut() else {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::OtherError,
                rows: 0,
            };
        };
        let status = {
            let mut scanlines = decoder.scanline_output();
            scanlines.finish()
        };
        match status {
            Ok(ScanlineStatus::Complete) => {
                self.output_sequence = OutputSequence::Scanline { complete: true };
                ScanlineOutputResult {
                    status: OutputStatus::Complete,
                    result: DecodingResult::Success,
                    rows: 0,
                }
            }
            Ok(ScanlineStatus::NeedMoreInput) => ScanlineOutputResult {
                status: OutputStatus::NeedMoreInput,
                result: DecodingResult::IncompleteInput,
                rows: 0,
            },
            Ok(ScanlineStatus::Ready) => ScanlineOutputResult {
                status: OutputStatus::Ready,
                result: DecodingResult::Success,
                rows: 0,
            },
            Ok(_) => unreachable!(),
            Err(ref decode_error) => ScanlineOutputResult {
                status: OutputStatus::Error,
                result: map_zune_error(decode_error),
                rows: 0,
            },
        }
    }

    pub fn skip_scanlines(&mut self, rows: u32) -> ScanlineOutputResult {
        if !matches!(self.output_sequence, OutputSequence::Scanline { .. }) {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::ParameterError,
                rows: 0,
            };
        }
        if self.output_scale.is_some() {
            let ready = self.start_scaled_scanline_output(self.output_format);
            if ready.status != OutputStatus::Ready {
                return ready;
            }
            if rows > self.output_height() - self.scaled_scanline_row {
                return ScanlineOutputResult {
                    status: OutputStatus::Error,
                    result: DecodingResult::ParameterError,
                    rows: 0,
                };
            }
            self.scaled_scanline_row += rows;
            return self.map_scanline_read_status(Ok(ScanlineReadStatus::RowsProcessed {
                rows: rows as usize,
            }));
        }
        self.prepare_output_decoder();
        let Some(decoder) = self.incremental_decoder.as_mut() else {
            return ScanlineOutputResult {
                status: OutputStatus::Error,
                result: DecodingResult::OtherError,
                rows: 0,
            };
        };
        let status = {
            let mut scanlines = decoder.scanline_output();
            scanlines.skip_scanlines(rows as usize)
        };
        self.map_scanline_read_status(status)
    }

    fn map_scanline_read_status(
        &mut self,
        status: Result<ScanlineReadStatus, zune_jpeg::errors::DecodeErrors>,
    ) -> ScanlineOutputResult {
        match status {
            Ok(ScanlineReadStatus::RowsProcessed { rows }) => ScanlineOutputResult {
                status: OutputStatus::Ready,
                result: DecodingResult::Success,
                rows: u32::try_from(rows).unwrap_or(0),
            },
            Ok(ScanlineReadStatus::NeedMoreInput) => ScanlineOutputResult {
                status: OutputStatus::NeedMoreInput,
                result: DecodingResult::IncompleteInput,
                rows: 0,
            },
            Ok(ScanlineReadStatus::Complete) => {
                self.output_sequence = OutputSequence::Scanline { complete: true };
                ScanlineOutputResult {
                    status: OutputStatus::Complete,
                    result: DecodingResult::Success,
                    rows: 0,
                }
            }
            Ok(_) => unreachable!(),
            Err(ref decode_error) => ScanlineOutputResult {
                status: OutputStatus::Error,
                result: map_zune_error(decode_error),
                rows: 0,
            },
        }
    }

    pub fn icc_profile(&self) -> Vec<u8> {
        if let Some(ref cached) = self.cached_icc {
            return cached.clone();
        }
        extract_icc_profile(&self.raw_data.borrow(), &self.scanner.segments)
    }

    pub fn exif_data(&self) -> Vec<u8> {
        if let Some(ref cached) = self.cached_exif {
            return cached.clone();
        }
        extract_exif_data(&self.raw_data.borrow(), &self.scanner.segments)
    }

    pub fn segment_count(&self) -> u32 {
        self.scanner.segments.len() as u32
    }

    pub fn segment_info(&self, index: u32) -> JpegSegmentInfo {
        scanned_segment_info(&self.scanner.segments, index)
    }

    pub fn segment_data(&self, index: u32) -> Vec<u8> {
        scanned_segment_data(&self.raw_data.borrow(), &self.scanner.segments, index)
    }

    pub fn segments_scanned(&self) -> bool {
        self.scanner.is_done()
    }

    fn mpf(&self) -> Option<&MpfResult> {
        match self.cached_mpf {
            Some(ref opt) => opt.as_ref(),
            None => None,
        }
    }

    pub fn has_mpf(&self) -> bool {
        self.mpf().is_some()
    }

    pub fn mpf_image_count(&self) -> u32 {
        self.mpf().map_or(0, |m| m.images.len() as u32)
    }

    pub fn mpf_image_entry(&self, index: u32) -> MpfImageEntry {
        self.mpf()
            .and_then(|m| m.images.get(index as usize))
            .map_or(
                MpfImageEntry {
                    data_offset: 0,
                    size: 0,
                },
                |img| MpfImageEntry {
                    data_offset: img.data_offset,
                    size: img.size,
                },
            )
    }

    pub fn mpf_segment_offset(&self) -> u32 {
        self.mpf().map_or(0, |m| m.segment_offset)
    }
}

fn new_reader(
    input: cxx::UniquePtr<ffi::SkStreamAdapter>,
    stream_length: u64,
    input_complete: bool,
) -> Box<Reader> {
    Box::new(Reader::new(input, stream_length, input_complete))
}

pub fn encode_jpeg(
    pixels: &[u8],
    width: u32,
    height: u32,
    row_bytes: u32,
    color_type: JpegEncodeColor,
    alpha_option: JpegEncodeAlpha,
    quality: u32,
    output: &mut Vec<u8>,
) -> EncodingResult {
    let src_bpp: usize = match color_type {
        JpegEncodeColor::RGB => 3,
        JpegEncodeColor::RGBA | JpegEncodeColor::BGRA => 4,
        JpegEncodeColor::Grayscale => 1,
        _ => return EncodingResult::ParameterError,
    };

    let row_bytes = row_bytes as usize;
    let w = width as usize;
    let h = height as usize;
    if w == 0 || h == 0 {
        return EncodingResult::ParameterError;
    }

    let min_row = match w.checked_mul(src_bpp) {
        Some(v) => v,
        None => return EncodingResult::ParameterError,
    };

    if row_bytes < min_row {
        return EncodingResult::ParameterError;
    }
    if let Some(total) = h.checked_mul(row_bytes) {
        if pixels.len() < total {
            return EncodingResult::ParameterError;
        }
    } else {
        return EncodingResult::ParameterError;
    }

    let (w16, h16) = match (u16::try_from(width), u16::try_from(height)) {
        (Ok(w), Ok(h)) => (w, h),
        _ => return EncodingResult::ParameterError,
    };

    let (rgb_data, jpeg_color) = match color_type {
        JpegEncodeColor::Grayscale => {
            let mut data = Vec::with_capacity(w * h);
            for y in 0..h {
                let row_start = y * row_bytes;
                data.extend_from_slice(&pixels[row_start..row_start + w]);
            }
            (data, jpeg_encoder::ColorType::Luma)
        }
        JpegEncodeColor::RGB => {
            let mut data = Vec::with_capacity(w * h * 3);
            for y in 0..h {
                let row_start = y * row_bytes;
                data.extend_from_slice(&pixels[row_start..row_start + w * 3]);
            }
            (data, jpeg_encoder::ColorType::Rgb)
        }
        JpegEncodeColor::RGBA | JpegEncodeColor::BGRA => {
            let swap_rb = matches!(color_type, JpegEncodeColor::BGRA);
            let blend_on_black = matches!(alpha_option, JpegEncodeAlpha::BlendOnBlack);
            let mut data = Vec::with_capacity(w * h * 3);
            for y in 0..h {
                let row_start = y * row_bytes;
                for x in 0..w {
                    let px = row_start + x * 4;
                    let (mut r, g, mut b, a) =
                        (pixels[px], pixels[px + 1], pixels[px + 2], pixels[px + 3]);
                    if swap_rb {
                        std::mem::swap(&mut r, &mut b);
                    }
                    if blend_on_black && a < 255 {
                        let af = a as u32;
                        data.push(((r as u32 * af + 127) / 255) as u8);
                        data.push(((g as u32 * af + 127) / 255) as u8);
                        data.push(((b as u32 * af + 127) / 255) as u8);
                    } else {
                        data.push(r);
                        data.push(g);
                        data.push(b);
                    }
                }
            }
            (data, jpeg_encoder::ColorType::Rgb)
        }
        _ => return EncodingResult::ParameterError,
    };

    let quality = quality.min(100).max(1) as u8;

    let mut buf: Vec<u8> = Vec::new();
    let mut encoder = jpeg_encoder::Encoder::new(&mut buf, quality);
    encoder.set_chroma_subsampling_method(jpeg_encoder::ChromaSubsamplingMethod::Average);
    encoder.set_exact_quantization(true);
    encoder.set_optimized_huffman_tables(true);

    match encoder.encode(&rgb_data, w16, h16, jpeg_color) {
        Ok(()) => {
            *output = buf;
            EncodingResult::Success
        }
        Err(_) => EncodingResult::OtherError,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn dc_only_jpeg() -> Vec<u8> {
        // Three independent DC scans of an 8x8 constant YCbCr image. The
        // final Cr scan has no following marker, deliberately including no EOI.
        let mut jpeg = vec![0xFF, 0xD8, 0xFF, 0xDB, 0, 67, 0];
        jpeg.extend_from_slice(&[1; 64]);
        jpeg.extend_from_slice(&[
            0xFF, 0xC2, 0, 17, 8, 0, 8, 0, 8, 3, 1, 0x11, 0, 2, 0x11, 0, 3, 0x11, 0, 0xFF, 0xC4, 0,
            21, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0xFF, 0xDA, 0, 8, 1, 1,
            0, 0, 0, 0, 0x7F, 0xFF, 0xDA, 0, 8, 1, 2, 0, 0, 0, 0, 0x7F, 0xFF, 0xDA, 0, 8, 1, 3, 0,
            0, 0, 0, 0xA0, 0x0F,
        ]);
        jpeg
    }

    fn reader_with_bytes(bytes: &[u8], input_complete: bool) -> Box<Reader> {
        let mut reader = new_reader(cxx::UniquePtr::null(), 0, input_complete);
        reader.stream = None;
        reader.raw_data.borrow_mut().extend_from_slice(bytes);
        let result = reader.read_metadata();
        assert!(
            matches!(result, DecodingResult::Success),
            "{result:?}: {bytes:?}"
        );
        reader
    }

    #[test]
    fn test_source_mcu_geometry_without_yuv_restrictions() {
        let cases: &[(&[u8], u32, u32, bool, bool)] = &[
            (&[0x11], 8, 8, false, false),
            (&[0x21], 16, 8, false, false),
            (&[0x22, 0x11, 0x11], 16, 16, false, true),
            (&[0x42, 0x11, 0x11], 32, 16, false, true),
            (&[0x24, 0x11, 0x11], 16, 32, false, true),
            (&[0x11, 0x22, 0x11], 16, 16, false, true),
            (&[0x21, 0x21, 0x21], 16, 8, true, false),
            (&[0x11, 0x11, 0x11, 0x22], 16, 16, false, false),
        ];
        for &(sampling, mcu_width, mcu_height, rgb, raw_ycbcr) in cases {
            for partial in [0, 1] {
                let width = mcu_width * 2 + partial;
                let height = mcu_height * 2 + partial;
                let mut jpeg = vec![0xFF, 0xD8, 0xFF, 0xDB, 0, 67, 0];
                jpeg.extend_from_slice(&[1; 64]);
                if rgb {
                    jpeg.extend_from_slice(&[
                        0xFF, 0xEE, 0, 14, b'A', b'd', b'o', b'b', b'e', 0, 100, 0, 0, 0, 0, 0,
                    ]);
                }
                jpeg.extend_from_slice(&[
                    0xFF,
                    0xC2,
                    0,
                    (8 + sampling.len() * 3) as u8,
                    8,
                    (height >> 8) as u8,
                    height as u8,
                    (width >> 8) as u8,
                    width as u8,
                    sampling.len() as u8,
                ]);
                for (index, &factor) in sampling.iter().enumerate() {
                    jpeg.extend_from_slice(&[(index + 1) as u8, factor, 0]);
                }
                jpeg.extend_from_slice(&[0xFF, 0xDA, 0, 8, 1, 1, 0, 0, 0, 0]);
                let reader = reader_with_bytes(&jpeg, false);
                assert_eq!(
                    (reader.mcu_width(), reader.mcu_height()),
                    (mcu_width, mcu_height)
                );
                assert_eq!((reader.width(), reader.height()), (width, height));
                assert_eq!(reader.raw_component_count() != 0, raw_ycbcr);
            }
        }
    }

    #[test]
    fn test_packed_final_input_without_eoi() {
        let jpeg = dc_only_jpeg();
        for incremental in [false, true] {
            for denominator in [1, 2, 4, 8] {
                let mut reader = reader_with_bytes(&jpeg, true);
                for _ in 0..2 {
                    reader.reset_decode_state();
                    assert!(matches!(
                        reader.set_output_scale(denominator),
                        DecodingResult::Success
                    ));
                    let result = if incremental {
                        reader.read_incremental_image_data()
                    } else {
                        reader.read_image_data()
                    };
                    assert_eq!(
                        matches!(result, DecodingResult::Success),
                        !incremental,
                        "{result:?}"
                    );
                    assert_eq!(reader.image_data_loaded(), !incremental);
                    let mut rows = &[][..];
                    let info = reader.get_next_rows(&mut rows);
                    assert_eq!(
                        info.row_count,
                        if incremental {
                            0
                        } else {
                            8 / u32::from(denominator)
                        }
                    );
                    assert!(!info.is_preview);
                    if incremental {
                        assert!(matches!(result, DecodingResult::IncompleteInput));
                        assert!(rows.is_empty());
                    } else {
                        assert!(rows.chunks_exact(3).all(|pixel| pixel == [128, 128, 128]));
                    }
                }
            }
        }

        // Full-frame reads preserve legacy best effort. Incremental retries
        // must not finalize a genuinely growing source.
        for incremental in [false, true] {
            for denominator in [1, 2, 4, 8] {
                let mut reader = reader_with_bytes(&jpeg, false);
                assert!(matches!(
                    reader.set_output_scale(denominator),
                    DecodingResult::Success
                ));
                for _ in 0..2 {
                    let result = if incremental {
                        reader.read_incremental_image_data()
                    } else {
                        reader.read_image_data()
                    };
                    assert_eq!(
                        matches!(result, DecodingResult::IncompleteInput),
                        incremental,
                        "{result:?}"
                    );
                    assert_eq!(reader.image_data_loaded(), !incremental);
                    if !incremental {
                        assert_eq!(&reader.image_data[..3], &[128, 128, 128]);
                    }
                }
                reader
                    .raw_data
                    .borrow_mut()
                    .extend_from_slice(&[0xFF, 0xD9]);
                if !incremental {
                    reader.reset_decode_state();
                    assert!(matches!(
                        reader.set_output_scale(denominator),
                        DecodingResult::Success
                    ));
                }
                let result = if incremental {
                    reader.read_incremental_image_data()
                } else {
                    reader.read_image_data()
                };
                assert!(matches!(result, DecodingResult::Success), "{result:?}");
                assert_eq!(&reader.image_data[..3], &[218, 82, 128]);
            }
        }
    }

    #[test]
    fn test_scaled_scanline_rows_padding_skip_and_reset() {
        let mut jpeg = dc_only_jpeg();
        jpeg.extend_from_slice(&[0xFF, 0xD9]);
        for numerator in 1..8 {
            for input_complete in [false, true] {
                let mut reader = reader_with_bytes(&jpeg, input_complete);
                for _ in 0..2 {
                    reader.reset_decode_state();
                    assert!(matches!(
                        reader.set_output_scale_ratio(numerator, 8),
                        DecodingResult::Success
                    ));
                    assert_eq!((reader.width(), reader.height()), (8, 8));
                    assert_eq!(reader.output_height(), u32::from(numerator));
                    assert_eq!(reader.row_bytes(), u32::from(numerator) * 3);
                    assert!(matches!(
                        reader
                            .start_scanline_output(PackedOutputFormat::RGBA)
                            .status,
                        OutputStatus::Error
                    ));
                    assert!(matches!(
                        reader
                            .start_scanline_output(PackedOutputFormat::Source)
                            .status,
                        OutputStatus::Ready
                    ));
                    let row_bytes = reader.row_bytes();
                    let mut short = vec![0xCD; row_bytes as usize - 1];
                    assert!(matches!(
                        reader.read_scanlines(&mut short, row_bytes).status,
                        OutputStatus::Error
                    ));
                    assert!(short.iter().all(|&value| value == 0xCD));
                    let stride = row_bytes + 5;
                    for y in 0..numerator {
                        if y % 2 == 1 {
                            let result = reader.skip_scanlines(1);
                            assert!(matches!(result.status, OutputStatus::Ready));
                            assert_eq!(result.rows, 1);
                        } else {
                            let mut row = vec![0xCD; stride as usize];
                            let result = reader.read_scanlines(&mut row, stride);
                            assert!(matches!(result.status, OutputStatus::Ready));
                            assert_eq!(result.rows, 1);
                            assert!(row[..row_bytes as usize]
                                .chunks_exact(3)
                                .all(|p| p == [218, 82, 128]));
                            assert!(row[row_bytes as usize..].iter().all(|&v| v == 0xCD));
                        }
                    }
                    let mut row = vec![0xCD; stride as usize];
                    assert!(matches!(
                        reader.read_scanlines(&mut row, stride).status,
                        OutputStatus::Complete
                    ));
                    assert!(row.iter().all(|&value| value == 0xCD));
                    assert_eq!(reader.input_complete(), input_complete);
                }
            }
        }
    }

    #[test]
    fn test_scaled_scanline_finality_and_resume() {
        for numerator in 1..8 {
            for input_complete in [false, true] {
                let mut reader = reader_with_bytes(&dc_only_jpeg(), input_complete);
                assert!(matches!(
                    reader.set_output_scale_ratio(numerator, 8),
                    DecodingResult::Success
                ));
                let result = reader.start_scanline_output(PackedOutputFormat::Source);
                if input_complete {
                    assert!(matches!(result.status, OutputStatus::Ready));
                } else {
                    assert!(matches!(result.status, OutputStatus::NeedMoreInput));
                    assert!(!reader.image_data_loaded());
                    assert!(matches!(
                        reader
                            .start_scanline_output(PackedOutputFormat::Source)
                            .status,
                        OutputStatus::NeedMoreInput
                    ));
                    reader
                        .raw_data
                        .borrow_mut()
                        .extend_from_slice(&[0xFF, 0xD9]);
                    assert!(matches!(
                        reader
                            .start_scanline_output(PackedOutputFormat::Source)
                            .status,
                        OutputStatus::Ready
                    ));
                }
                let row_bytes = reader.row_bytes();
                let mut row = vec![0; row_bytes as usize];
                assert_eq!(reader.read_scanlines(&mut row, row_bytes).rows, 1);
                let expected = if input_complete {
                    [128, 128, 128]
                } else {
                    [218, 82, 128]
                };
                assert!(row.chunks_exact(3).all(|p| p == expected));
                assert!(matches!(
                    reader.finish_scanline_output().status,
                    OutputStatus::Complete
                ));
            }
        }
    }

    #[test]
    fn test_buffered_prefix_only_limits_incremental_output() {
        for input_complete in [false, true] {
            for incremental in [false, true] {
                for denominator in [1, 2, 4, 8] {
                    for coded_rows in [None, Some(0), Some(24), Some(usize::MAX)] {
                        let mut reader = new_reader(cxx::UniquePtr::null(), 0, input_complete);
                        reader.width = 17;
                        reader.height = 65;
                        assert!(matches!(
                            reader.set_output_scale(denominator),
                            DecodingResult::Success
                        ));
                        reader.output_sequence = if incremental {
                            OutputSequence::Incremental
                        } else {
                            OutputSequence::FullImage
                        };
                        reader.preview_rows = 65;
                        reader.preview_scan_count = 3;
                        let height = reader.output_height();
                        let expected = if incremental {
                            coded_rows.map_or(height, |rows| {
                                (rows.min(65) as u32).div_ceil(u32::from(denominator))
                            })
                        } else {
                            height
                        };
                        let result = reader.finish_buffered_image(coded_rows);
                        assert_eq!(reader.valid_rows, expected);
                        assert_eq!(reader.image_data_loaded(), expected == height);
                        assert_eq!(reader.preview_rows, 0);
                        assert_eq!(reader.preview_scan_count, 0);
                        assert_eq!(
                            matches!(result, DecodingResult::Success),
                            expected == height
                        );
                        assert_eq!(reader.width(), 17);
                        assert_eq!(reader.height(), 65);
                        assert_eq!(reader.input_complete(), input_complete);
                    }
                }
            }
        }
    }

    #[test]
    fn test_output_scale_keeps_coded_geometry_and_resets() {
        let mut reader = new_reader(cxx::UniquePtr::null(), 0, true);
        reader.width = 17;
        reader.height = 19;
        reader.supports_streaming_full_decode = true;
        for denominator in [1_u8, 2, 4, 8] {
            reader.reset_decode_state();
            assert!(matches!(
                reader.set_output_scale(denominator),
                DecodingResult::Success
            ));
            assert_eq!(reader.width(), 17);
            assert_eq!(reader.height(), 19);
            assert_eq!(reader.row_bytes(), 17_u32.div_ceil(denominator.into()) * 3);
            assert_eq!(reader.output_height(), 19_u32.div_ceil(denominator.into()));
            assert_eq!(reader.supports_streaming_full_decode(), denominator == 1);
            assert!(reader.input_complete());
        }
        reader.reset_decode_state();
        assert_eq!(reader.row_bytes(), 17 * 3);
        assert_eq!(reader.output_height(), 19);
        assert!(reader.supports_streaming_full_decode());
        assert!(matches!(
            reader.set_output_scale(3),
            DecodingResult::ParameterError
        ));
        assert_eq!(reader.row_bytes(), 17 * 3);
        reader.output_sequence = OutputSequence::Incremental;
        assert!(matches!(
            reader.set_output_scale(2),
            DecodingResult::ParameterError
        ));
        assert_eq!(reader.row_bytes(), 17 * 3);
    }

    #[test]
    fn test_scaled_decode_retries_and_output_sequence() {
        let width = 65_u32;
        let height = 67_u32;
        let pixels: Vec<u8> = (0..width * height * 3).map(|i| (i * 13) as u8).collect();
        let mut jpeg = Vec::new();
        assert_eq!(
            encode_jpeg(
                &pixels,
                width,
                height,
                width * 3,
                JpegEncodeColor::RGB,
                JpegEncodeAlpha::Ignore,
                90,
                &mut jpeg,
            ),
            EncodingResult::Success
        );
        for denominator in [2, 4, 8] {
            let mut reader = new_reader(cxx::UniquePtr::null(), 0, false);
            reader.stream = None;
            let mut decoded = false;
            let mut suspended = false;
            for chunk in jpeg.chunks(23) {
                reader.raw_data.borrow_mut().extend_from_slice(chunk);
                if !reader.metadata_loaded() {
                    match reader.read_metadata() {
                        DecodingResult::Success => {
                            assert!(matches!(
                                reader.set_output_scale(denominator),
                                DecodingResult::Success
                            ));
                        }
                        DecodingResult::IncompleteInput => continue,
                        result => panic!("metadata failed: {result:?}"),
                    }
                }
                match reader.read_incremental_image_data() {
                    DecodingResult::Success => {
                        decoded = true;
                        break;
                    }
                    DecodingResult::IncompleteInput => {
                        suspended = true;
                        let mut rows = &[][..];
                        assert_eq!(reader.get_next_rows(&mut rows).row_count, 0);
                        assert!(rows.is_empty());
                        assert!(matches!(
                            reader.read_incremental_image_data(),
                            DecodingResult::IncompleteInput
                        ));
                        assert!(matches!(
                            reader.set_output_scale(1),
                            DecodingResult::ParameterError
                        ));
                    }
                    result => panic!("scaled decode failed: {result:?}"),
                }
            }
            assert!(suspended && decoded);
            assert_eq!(reader.width(), width);
            assert_eq!(reader.height(), height);
            let expected_height = height.div_ceil(denominator.into());
            let expected_row_bytes = width.div_ceil(denominator.into()) * 3;
            assert_eq!(reader.output_height(), expected_height);
            assert_eq!(reader.row_bytes(), expected_row_bytes);
            assert_eq!(
                reader.image_data.len(),
                (expected_height * expected_row_bytes) as usize
            );
            let incremental_pixels = reader.image_data.clone();
            reader.reset_decode_state();
            *reader.raw_data.borrow_mut() = jpeg.clone();
            assert!(matches!(
                reader.set_output_scale(denominator),
                DecodingResult::Success
            ));
            assert!(matches!(reader.read_image_data(), DecodingResult::Success));
            assert_eq!(reader.image_data, incremental_pixels);
        }
    }

    #[test]
    fn test_raw_input_finality_is_preserved_on_decode_reset() {
        for input_complete in [false, true] {
            let mut reader = new_reader(cxx::UniquePtr::null(), 0, input_complete);
            assert_eq!(reader.input_complete(), input_complete);
            reader.reset_decode_state();
            assert_eq!(reader.input_complete(), input_complete);
        }
    }

    #[test]
    fn test_validate_stream_length_rejects_oversized_length() {
        assert!(matches!(
            validate_stream_length(MAX_DECODE_IMAGE_BYTES as u64),
            DecodingResult::Success
        ));
        assert!(matches!(
            validate_stream_length(MAX_DECODE_IMAGE_BYTES as u64 + 1),
            DecodingResult::MemoryError
        ));
        assert!(matches!(
            validate_stream_length(u64::MAX),
            DecodingResult::MemoryError
        ));
    }

    #[test]
    fn test_decoded_row_slice_checks_bounds() {
        let pixels: Vec<u8> = (0..24).collect();
        assert_eq!(decoded_row_slice(&pixels, 1, 2, 6), Some(&pixels[6..18]));
        assert_eq!(decoded_row_slice(&pixels, 3, 2, 6), None);
        assert_eq!(
            decoded_row_slice(&pixels, u32::MAX, u32::MAX, usize::MAX),
            None
        );
    }

    #[test]
    fn test_is_jpeg_data() {
        assert!(is_jpeg_data(&[0xFF, 0xD8, 0xFF, 0xE0]));
        assert!(!is_jpeg_data(&[0x89, b'P', b'N', b'G']));
        assert!(!is_jpeg_data(&[b'B', b'M']));
        assert!(!is_jpeg_data(&[0xFF]));
        assert!(!is_jpeg_data(&[]));
    }

    #[test]
    fn test_incomplete_jpeg_signature() {
        assert!(matches!(
            classify_jpeg_signature(&[]),
            DecodingResult::IncompleteInput
        ));
        assert!(matches!(
            classify_jpeg_signature(&[0xFF]),
            DecodingResult::IncompleteInput
        ));
    }

    #[test]
    fn test_invalid_jpeg_signature() {
        assert!(matches!(
            classify_jpeg_signature(&[0x00]),
            DecodingResult::FormatError
        ));
        assert!(matches!(
            classify_jpeg_signature(&[0xFF, 0x00]),
            DecodingResult::FormatError
        ));
    }

    #[test]
    fn test_encode_jpeg_rgb_roundtrip() {
        let pixels: Vec<u8> = vec![255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            2,
            2,
            6,
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::Success);
        assert!(is_jpeg_data(&output));
    }

    #[test]
    fn test_encode_jpeg_uses_optimized_huffman_tables() {
        let pixels = [0, 255, 0].repeat(100 * 100);

        let mut output = Vec::new();
        assert_eq!(
            encode_jpeg(
                &pixels,
                100,
                100,
                100 * 3,
                JpegEncodeColor::RGB,
                JpegEncodeAlpha::Ignore,
                30,
                &mut output,
            ),
            EncodingResult::Success
        );

        let mut default_output = Vec::new();
        jpeg_encoder::Encoder::new(&mut default_output, 30)
            .encode(&pixels, 100, 100, jpeg_encoder::ColorType::Rgb)
            .unwrap();

        assert!(output.len() < default_output.len());
    }

    #[test]
    fn test_encode_jpeg_uses_average_subsampling_and_exact_quantization() {
        let row = [0u8, 0, 0, 246, 255, 255, 255, 255];
        let mut pixels = Vec::with_capacity(8 * 8);
        for _ in 0..8 {
            pixels.extend_from_slice(&row);
        }

        let mut encoded = Vec::new();
        assert_eq!(
            encode_jpeg(
                &pixels,
                8,
                8,
                8,
                JpegEncodeColor::Grayscale,
                JpegEncodeAlpha::Ignore,
                85,
                &mut encoded,
            ),
            EncodingResult::Success
        );

        let options =
            zune_core::options::DecoderOptions::default().jpeg_set_out_colorspace(ZuneColorSpace::Luma);
        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            zune_core::bytestream::ZCursor::new(&encoded),
            options,
        );
        let decoded = decoder.decode().unwrap();
        assert_eq!(&decoded[..8], &[0, 1, 0, 246, 254, 255, 255, 255]);
    }

    #[test]
    fn test_encode_jpeg_rejects_invalid_row_bytes() {
        let pixels = vec![0u8; 12];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            2,
            2,
            1,
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::ParameterError);
    }

    #[test]
    fn test_encode_jpeg_rejects_oversize_dimensions() {
        let pixels = vec![0u8; 3];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            u32::MAX,
            1,
            u32::MAX,
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::ParameterError);
    }

    #[test]
    fn test_growing_cursor_supports_zune_incremental_retries() {
        let width = 16u32;
        let height = 16u32;
        let mut pixels = Vec::new();
        for y in 0..height {
            for x in 0..width {
                pixels.push((x * 13) as u8);
                pixels.push((y * 11) as u8);
                pixels.push(((x + y) * 7) as u8);
            }
        }

        let mut jpeg = Vec::new();
        assert_eq!(
            encode_jpeg(
                &pixels,
                width,
                height,
                width * 3,
                JpegEncodeColor::RGB,
                JpegEncodeAlpha::Ignore,
                90,
                &mut jpeg,
            ),
            EncodingResult::Success
        );

        let input = Rc::new(RefCell::new(Vec::new()));
        let options = zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .jpeg_set_out_colorspace(ZuneColorSpace::RGB);
        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            GrowingJpegCursor::new(Rc::clone(&input)),
            options,
        );
        decoder.set_incremental_mode(true);

        let mut output = Vec::new();
        let mut headers_done = false;
        let mut decoded = false;

        for chunk in jpeg.chunks(23) {
            input.borrow_mut().extend_from_slice(chunk);

            if !headers_done {
                match decoder.decode_headers() {
                    Ok(()) => {
                        headers_done = true;
                        output.resize(decoder.output_buffer_size().unwrap(), 0);
                    }
                    Err(ref e) if e.is_recoverable_eof() => continue,
                    Err(e) => panic!("unexpected header error: {e:?}"),
                }
            }

            match decoder.decode_into(&mut output) {
                Ok(()) => {
                    decoded = true;
                    break;
                }
                Err(ref e) if e.is_recoverable_eof() => {
                    let stable_scanlines = decoder.decoded_scanlines();
                    assert!(stable_scanlines.is_some());

                    match decoder.decode_into(&mut output) {
                        Err(ref e) if e.is_recoverable_eof() => {}
                        other => panic!("expected retryable EOF without new bytes, got {other:?}"),
                    }
                    assert_eq!(decoder.decoded_scanlines(), stable_scanlines);
                }
                Err(e) => panic!("unexpected decode error: {e:?}"),
            }
        }

        if !decoded {
            decoder.decode_into(&mut output).unwrap();
        }
        assert_eq!(decoder.decoded_scanlines(), Some(height as usize));
    }

    fn make_minimal_jpeg_with_app0() -> Vec<u8> {
        jpeg_segment_scan::tests::make_minimal_jpeg_with_app0()
    }

    #[test]
    fn test_scan_embedded_jpeg_basic() {
        let data = make_minimal_jpeg_with_app0();
        let scanner = scan_embedded_jpeg(&data);

        assert!(!scanner.embedded_had_error());
        assert_eq!(scanner.embedded_segment_count(), 3);

        let seg1 = scanner.embedded_segment_info(1);
        assert_eq!(seg1.marker, 0xE0);
        assert_eq!(seg1.parameter_length, 6);

        let seg_data = scanner.embedded_segment_data(1);
        assert_eq!(seg_data, b"Hi\x00\x00");
    }

    #[test]
    fn test_scan_embedded_jpeg_bad_data() {
        let scanner = scan_embedded_jpeg(b"not a jpeg");
        assert!(scanner.embedded_had_error());
        assert_eq!(scanner.embedded_segment_count(), 0);
    }

    #[test]
    fn test_scan_embedded_jpeg_empty() {
        let scanner = scan_embedded_jpeg(&[]);
        assert_eq!(scanner.embedded_segment_count(), 0);
    }

    #[test]
    fn test_scan_embedded_jpeg_out_of_range_index() {
        let data = make_minimal_jpeg_with_app0();
        let scanner = scan_embedded_jpeg(&data);
        let bad = scanner.embedded_segment_info(999);
        assert_eq!(bad.marker, 0);
        assert!(scanner.embedded_segment_data(999).is_empty());
    }
}
