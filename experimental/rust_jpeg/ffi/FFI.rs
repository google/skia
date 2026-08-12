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

        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> JpegColor;
        fn alpha(self: &Reader) -> JpegAlpha;
        fn metadata_loaded(self: &Reader) -> bool;

        fn read_metadata(self: &mut Reader) -> DecodingResult;
        fn read_image_data(self: &mut Reader) -> DecodingResult;
        fn read_incremental_image_data(self: &mut Reader) -> DecodingResult;
        fn image_data_loaded(self: &Reader) -> bool;

        /// # Safety
        /// CXX requires `unsafe` for functions with explicit lifetimes.
        unsafe fn get_next_rows<'a>(self: &'a mut Reader, buffer: &mut &'a [u8])
            -> DecodedRowsInfo;

        fn row_bytes(self: &Reader) -> u32;
        fn reset_decode_state(self: &mut Reader);

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

use zune_core::bytestream::{ZByteIoError, ZByteReaderTrait, ZCursor, ZSeekFrom};
use zune_core::colorspace::ColorSpace as ZuneColorSpace;

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

pub fn is_jpeg_data(data: &[u8]) -> bool {
    data.len() >= 2 && data[0] == 0xFF && data[1] == jpeg_marker::SOI
}

/// Streaming JPEG reader wrapping zune-jpeg with Rust-side segment scanning.
pub struct Reader {
    stream: Option<cxx::UniquePtr<ffi::SkStreamAdapter>>,
    raw_data: Rc<RefCell<Vec<u8>>>,
    stream_exhausted: bool,
    orig_color_space: ZuneColorSpace,
    metadata_loaded: bool,
    width: u32,
    height: u32,
    color: JpegColor,
    alpha: JpegAlpha,
    bytes_per_pixel: u32,
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
    incremental_decoder: Option<IncrementalJpegDecoder>,

    scanner: SegmentScanner,
    scanner_bytes_consumed: usize,
    cached_icc: Option<Vec<u8>>,
    cached_exif: Option<Vec<u8>>,
    /// `None` = not yet scanned; `Some(None)` = scanned, no MPF found.
    cached_mpf: Option<Option<MpfResult>>,
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

impl Reader {
    /// Create a new reader without reading any data from the stream.
    /// The stream is stored for later incremental reads.
    fn new(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, DecodingResult> {
        Ok(Reader {
            stream: Some(input),
            raw_data: Rc::new(RefCell::new(Vec::new())),
            stream_exhausted: false,
            orig_color_space: ZuneColorSpace::RGB,
            metadata_loaded: false,
            width: 0,
            height: 0,
            color: JpegColor::RGB,
            alpha: JpegAlpha::Opaque,
            bytes_per_pixel: 3,
            image_data: Vec::new(),
            image_data_loaded: false,
            valid_rows: 0,
            last_consumed_row_count: 0,
            preview_rows: 0,
            preview_scan_count: 0,
            last_consumed_preview_scan_count: 0,
            incremental_decoder: None,
            // Stop at SOS — we only need header segments for metadata.
            scanner: SegmentScanner::new(jpeg_marker::SOS),
            scanner_bytes_consumed: 0,
            cached_icc: None,
            cached_exif: None,
            cached_mpf: None,
        })
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

    pub fn read_metadata(&mut self) -> DecodingResult {
        if self.metadata_loaded {
            return DecodingResult::Success;
        }

        self.try_read_more();

        let raw_data = self.raw_data.borrow();
        if !is_jpeg_data(&raw_data) {
            if raw_data.len() < 2 && !self.stream_exhausted {
                return DecodingResult::IncompleteInput;
            }
            return DecodingResult::FormatError;
        }

        let options = zune_core::options::DecoderOptions::default().set_strict_mode(false);

        let mut decoder =
            zune_jpeg::JpegDecoder::new_with_options(ZCursor::new(raw_data.as_slice()), options);

        match decoder.decode_headers() {
            Ok(()) => {}
            Err(ref e) => {
                if !self.stream_exhausted {
                    return DecodingResult::IncompleteInput;
                }
                return map_zune_error(e);
            }
        }

        let (width, height) = match decoder.dimensions() {
            Some(dims) => dims,
            None => return DecodingResult::FormatError,
        };

        let cs = decoder.input_colorspace().unwrap_or(ZuneColorSpace::RGB);
        let (color, bytes_per_pixel) = map_color_space(cs);
        drop(decoder);
        drop(raw_data);
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
        self.metadata_loaded = true;

        DecodingResult::Success
    }

    pub fn read_image_data(&mut self) -> DecodingResult {
        if self.image_data_loaded {
            return DecodingResult::Success;
        }

        if !self.metadata_loaded {
            return DecodingResult::ParameterError;
        }

        self.try_read_more();

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

        let raw_data = self.raw_data.borrow();
        let mut decoder =
            zune_jpeg::JpegDecoder::new_with_options(ZCursor::new(raw_data.as_slice()), options);

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
        if self.image_data_loaded {
            return DecodingResult::Success;
        }

        if !self.metadata_loaded {
            return DecodingResult::ParameterError;
        }

        // A zero-byte read during incremental decode only means that no new
        // bytes are visible for this attempt.  The SkCodec caller may make
        // more bytes visible and call us again.
        self.stream_exhausted = false;
        self.try_read_more();

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
            decoder.set_incremental_mode(true);
            self.incremental_decoder = Some(decoder);
        }

        let decoder = self.incremental_decoder.as_mut().unwrap();
        match decoder.decode_into(&mut self.image_data) {
            Ok(()) => {
                self.valid_rows = self.height;
                self.image_data_loaded = true;
                DecodingResult::Success
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
        let Some(rows) = decoded_row_slice(
            &self.image_data,
            already_consumed,
            new_row_count,
            row_bytes,
        ) else {
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

    pub fn row_bytes(&self) -> u32 {
        self.width.checked_mul(self.bytes_per_pixel).unwrap_or(0)
    }

    pub fn reset_decode_state(&mut self) {
        self.image_data.clear();
        self.image_data_loaded = false;
        self.valid_rows = 0;
        self.last_consumed_row_count = 0;
        self.preview_rows = 0;
        self.preview_scan_count = 0;
        self.last_consumed_preview_scan_count = 0;
        self.incremental_decoder = None;
        self.stream_exhausted = false;
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

pub fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<Reader> {
    // Reader construction is infallible today.
    Box::new(Reader::new(input).expect("Reader::new should be infallible"))
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
    let encoder = jpeg_encoder::Encoder::new(&mut buf, quality);

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

    #[test]
    fn test_decoded_row_slice_checks_bounds() {
        let pixels: Vec<u8> = (0..24).collect();
        assert_eq!(decoded_row_slice(&pixels, 1, 2, 6), Some(&pixels[6..18]));
        assert_eq!(decoded_row_slice(&pixels, 3, 2, 6), None);
        assert_eq!(decoded_row_slice(&pixels, u32::MAX, u32::MAX, usize::MAX), None);
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
