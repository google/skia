// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for ICO decoder using image-rs crate.
//!
//! This module provides FFI for ICO format detection, directory parsing,
//! and decoding of embedded PNG and BMP images.
//!
//! BMP-in-ICO format has specific differences from standalone BMP files:
//! 1. No "BM" file header (starts directly with DIB/BITMAPINFOHEADER)
//! 2. Height in DIB header is doubled (image height + AND mask height)
//! 3. 32-bit BMPs use embedded alpha channel
//! 4. Other bit depths store transparency in AND mask after pixel data
//!
//! Note: image-rs provides `BmpDecoder::new_with_ico_format()` that handles
//! these differences, but it's `pub(crate)`. We manually handle the doubled
//! height by patching the DIB header before passing to `new_without_file_header()`.

use std::io::Read;
use std::pin::Pin;

#[cxx::bridge(namespace = "rust_ico")]
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
    enum DirectoryParseResult {
        Success,
        InsufficientData,
        InvalidSignature,
        NoImages,
        TruncatedDirectory,
    }

    #[derive(Debug, Clone, Copy, PartialEq)]
    enum EmbeddedFormat {
        Png,
        Bmp,
    }

    /// ICO color type matching SkEncodedInfo::Color values.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum IcoColor {
        RGBA = 6,
    }

    /// ICO alpha type matching SkEncodedInfo::Alpha values.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum IcoAlpha {
        Opaque = 0,
        Unpremul = 1,
        Binary = 2,
    }

    /// ICO file type - either icon or cursor.
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum IcoFileType {
        Icon,
        Cursor,
    }

    /// Information about a single image entry in the ICO directory.
    #[derive(Debug, Clone, Copy)]
    struct IcoEntry {
        /// Offset from start of file to the embedded image data.
        pub offset: u32,
        /// Size of the embedded image data in bytes.
        pub size: u32,
        /// Whether this entry contains PNG or BMP data.
        pub format: EmbeddedFormat,
        /// Hotspot X coordinate (only valid for CUR files).
        pub hotspot_x: u16,
        /// Hotspot Y coordinate (only valid for CUR files).
        pub hotspot_y: u16,
        /// Width of the image (0 means 256).
        pub width: u8,
        /// Height of the image (0 means 256).
        pub height: u8,
        /// Bits per pixel (may be 0 meaning unspecified).
        pub bit_count: u16,
    }

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        // Reference the SkStreamAdapter type from skia_rust_common.
        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        /// Returns true if the stream starts with an ICO or CUR signature.
        /// Reads up to 4 bytes from the stream.
        fn is_ico(input: Pin<&mut SkStreamAdapter>) -> bool;

        /// Parse the ICO directory from a stream.
        fn parse_directory(input: Pin<&mut SkStreamAdapter>) -> Box<DirectoryResult>;

        type DirectoryResult;
        /// Returns the parse result status.
        fn status(self: &DirectoryResult) -> DirectoryParseResult;
        /// Returns the file type (Icon or Cursor).
        fn file_type(self: &DirectoryResult) -> IcoFileType;
        /// Returns the number of images in the directory (0 if parse failed).
        fn image_count(self: &DirectoryResult) -> u32;
        /// Returns the entry at the given index. Panics if index >= image_count().
        fn get_entry(self: &DirectoryResult, index: u32) -> IcoEntry;

        /// Create a new ICO image reader for a specific entry.
        /// `input` is the stream positioned at the embedded image data.
        /// `format` specifies whether it's PNG or BMP.
        /// `entry_size` is the total size from directory (used for AND mask calculation).
        fn new_reader(
            input: Pin<&mut SkStreamAdapter>,
            format: EmbeddedFormat,
            entry_size: u32,
        ) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> IcoColor;
        fn alpha(self: &Reader) -> IcoAlpha;
        fn next_row(self: &mut Reader, buffer: &mut [u8]) -> DecodingResult;
        fn reset_decode_state(self: &mut Reader);

        /// Apply the AND mask from BMP-in-ICO entry data to decoded RGBA pixels.
        /// For non-32-bit BMPs, the AND mask follows pixel data in the ICO entry
        /// and defines which pixels are transparent.
        /// `pixels` is the decoded output buffer (4 bytes per pixel).
        /// `entry_data` is the raw BMP entry data from the ICO file.
        /// Transparent pixels have all 4 bytes set to 0.
        fn apply_and_mask(
            pixels: &mut [u8],
            entry_data: &[u8],
            width: u32,
            height: u32,
            bytes_per_pixel: u32,
        );
    }
}

pub use ffi::*;

// ============================================================================
// Error handling
// ============================================================================

#[derive(Debug)]
enum IcoError {
    InsufficientData,
    InvalidData,
    UnsupportedFeature,
    InvalidHeader,
    IoError,
    FormatError,
}

impl From<image::ImageError> for IcoError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => IcoError::FormatError,
            image::ImageError::Encoding(_) => IcoError::FormatError,
            image::ImageError::Parameter(_) => {
                panic!(
                    "Internal bug in how Skia Rust ICO calls image crate: {:?}",
                    err
                )
            }
            image::ImageError::Limits(_) => IcoError::UnsupportedFeature,
            image::ImageError::Unsupported(_) => IcoError::UnsupportedFeature,
            image::ImageError::IoError(_) => IcoError::IoError,
        }
    }
}

impl From<std::io::Error> for IcoError {
    fn from(err: std::io::Error) -> Self {
        use std::io::ErrorKind;
        match err.kind() {
            ErrorKind::UnexpectedEof => IcoError::InsufficientData,
            _ => IcoError::IoError,
        }
    }
}

// ============================================================================
// Reader implementation
// ============================================================================

pub struct ResultOfReader {
    result: Result<Reader, IcoError>,
}

impl ResultOfReader {
    pub fn err(&self) -> DecodingResult {
        match &self.result {
            Ok(_) => DecodingResult::Success,
            Err(IcoError::InvalidHeader) => DecodingResult::FormatError,
            Err(IcoError::InsufficientData) => DecodingResult::IncompleteInput,
            Err(IcoError::UnsupportedFeature) => DecodingResult::UnsupportedFeature,
            Err(IcoError::InvalidData) => DecodingResult::FormatError,
            Err(IcoError::IoError) => DecodingResult::OtherError,
            Err(IcoError::FormatError) => DecodingResult::FormatError,
        }
    }

    pub fn unwrap(&mut self) -> Box<Reader> {
        match std::mem::replace(&mut self.result, Err(IcoError::InvalidHeader)) {
            Ok(reader) => Box::new(reader),
            Err(_) => panic!("Called unwrap on an error ResultOfReader"),
        }
    }
}

/// ICO image reader. All ICO images are decoded to RGBA with appropriate alpha.
pub struct Reader {
    pixels: Vec<u8>,
    width: u32,
    height: u32,
    alpha: IcoAlpha,
    current_row: u32,
}

impl Reader {
    /// Create a reader for a PNG embedded in ICO from a stream.
    fn new_png_from_stream<R: Read>(mut input: R) -> Result<Self, IcoError> {
        // Read all data from the stream
        let mut data = Vec::new();
        input.read_to_end(&mut data)?;

        Self::new_png(&data)
    }

    /// Create a reader for a PNG embedded in ICO.
    fn new_png(data: &[u8]) -> Result<Self, IcoError> {
        use std::io::Cursor;

        let cursor = Cursor::new(data);
        let decoder =
            image::codecs::png::PngDecoder::new(cursor).map_err(|_| IcoError::InvalidHeader)?;

        let (width, height) = image::ImageDecoder::dimensions(&decoder);
        let color_type = image::ImageDecoder::color_type(&decoder);

        // PNG in ICO must be RGBA8 per spec
        // https://blogs.msdn.microsoft.com/oldnewthing/20101022-00/?p=12473
        if color_type != image::ColorType::Rgba8 {
            return Err(IcoError::UnsupportedFeature);
        }

        let total_bytes = image::ImageDecoder::total_bytes(&decoder) as usize;
        let mut pixels = vec![0u8; total_bytes];
        image::ImageDecoder::read_image(decoder, &mut pixels)
            .map_err(|_| IcoError::InvalidData)?;

        Ok(Reader {
            pixels,
            width,
            height,
            alpha: IcoAlpha::Unpremul, // PNG in ICO has unpremul alpha
            current_row: 0,
        })
    }

    /// Create a reader for a BMP embedded in ICO from a stream.
    /// BMP-in-ICO format differences from standalone BMP:
    /// 1. No "BM" file header (starts directly with DIB header)
    /// 2. Height in DIB header is doubled (includes AND mask)
    /// 3. 32-bit BMPs have alpha channel embedded
    /// 4. Other bit depths use AND mask for transparency
    fn new_bmp_from_stream<R: Read>(mut input: R, entry_size: u32) -> Result<Self, IcoError> {
        // Read all data from the stream
        let mut data = Vec::new();
        input.read_to_end(&mut data)?;

        Self::new_bmp(&data, entry_size)
    }

    /// Create a reader for a BMP embedded in ICO.
    /// BMP-in-ICO format differences from standalone BMP:
    /// 1. No "BM" file header (starts directly with DIB header)
    /// 2. Height in DIB header is doubled (includes AND mask)
    /// 3. 32-bit BMPs have alpha channel embedded
    /// 4. Other bit depths use AND mask for transparency
    fn new_bmp(data: &[u8], _entry_size: u32) -> Result<Self, IcoError> {
        use std::io::Cursor;

        // BMP-in-ICO starts with DIB header, first 4 bytes are header size
        if data.len() < 40 {
            return Err(IcoError::InvalidHeader);
        }

        // Read DIB header size to validate format
        let header_size = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
        if header_size < 40 {
            return Err(IcoError::InvalidHeader);
        }

        // Read dimensions from DIB header (BITMAPINFOHEADER layout):
        // offset 4-7: width (i32)
        // offset 8-11: height (i32) - DOUBLED in ICO!
        // offset 12-13: planes (u16)
        // offset 14-15: bit_count (u16)
        // offset 16-19: compression (u32)
        let width = i32::from_le_bytes([data[4], data[5], data[6], data[7]]);
        let doubled_height = i32::from_le_bytes([data[8], data[9], data[10], data[11]]);
        let bit_count = u16::from_le_bytes([data[14], data[15]]);

        if width <= 0 || doubled_height <= 0 {
            return Err(IcoError::InvalidHeader);
        }

        // ICO stores doubled height (image + AND mask)
        let height = (doubled_height.abs() / 2) as u32;
        let width = width as u32;

        // Create modified data with correct height for BmpDecoder
        // We need to patch the height field to be the actual image height
        let mut patched_data = data.to_vec();
        let actual_height_bytes = (height as i32).to_le_bytes();
        patched_data[8..12].copy_from_slice(&actual_height_bytes);

        let cursor = Cursor::new(patched_data);

        // Use new_without_file_header since BMP-in-ICO has no "BM" header
        let decoder = image::codecs::bmp::BmpDecoder::new_without_file_header(cursor)
            .map_err(|_| IcoError::InvalidHeader)?;

        let color_type = image::ImageDecoder::color_type(&decoder);

        // Decode pixels
        let total_bytes = image::ImageDecoder::total_bytes(&decoder) as usize;
        let mut decoded_pixels = vec![0u8; total_bytes];
        image::ImageDecoder::read_image(decoder, &mut decoded_pixels)
            .map_err(|_| IcoError::InvalidData)?;

        // Convert to RGBA and apply AND mask if needed
        let (pixels, alpha) = match color_type {
            image::ColorType::Rgba8 => {
                // 32-bit BMP already has alpha channel embedded
                (decoded_pixels, IcoAlpha::Unpremul)
            }
            image::ColorType::Rgb8 => {
                // 24-bit BMP needs AND mask for transparency
                let rgba_pixels = Self::convert_rgb_to_rgba_with_and_mask(
                    &decoded_pixels,
                    data,
                    width,
                    height,
                    bit_count,
                )?;
                (rgba_pixels, IcoAlpha::Binary)
            }
            _ => {
                // Other formats (paletted, etc.) - need conversion + AND mask
                // For now, unsupported - could be expanded later
                return Err(IcoError::UnsupportedFeature);
            }
        };

        Ok(Reader {
            pixels,
            width,
            height,
            alpha,
            current_row: 0,
        })
    }

    /// Convert RGB pixels to RGBA and apply AND mask from BMP-in-ICO.
    /// The AND mask follows the pixel data in the ICO entry.
    ///
    /// Note: image-rs outputs decoded pixels in top-to-bottom order, but the
    /// AND mask in the original file is stored bottom-up (like BMP pixel data).
    fn convert_rgb_to_rgba_with_and_mask(
        rgb_pixels: &[u8],
        original_data: &[u8],
        width: u32,
        height: u32,
        bit_count: u16,
    ) -> Result<Vec<u8>, IcoError> {
        let pixel_count = (width * height) as usize;
        let mut rgba_pixels = Vec::with_capacity(pixel_count * 4);

        // Convert RGB to RGBA with opaque alpha
        for chunk in rgb_pixels.chunks(3) {
            if chunk.len() == 3 {
                rgba_pixels.push(chunk[0]); // R
                rgba_pixels.push(chunk[1]); // G
                rgba_pixels.push(chunk[2]); // B
                rgba_pixels.push(255);      // A (opaque)
            }
        }

        // Calculate where the AND mask starts in the original data
        // DIB header size + color table + pixel data
        let header_size = u32::from_le_bytes([
            original_data[0],
            original_data[1],
            original_data[2],
            original_data[3],
        ]) as usize;

        // Color table size (for bit depths <= 8)
        let color_table_size = if bit_count <= 8 {
            (1 << bit_count) * 4 // 4 bytes per color entry (RGBQUAD)
        } else {
            0
        };

        // Row stride for pixel data (4-byte aligned)
        let bits_per_row = width * (bit_count as u32);
        let bytes_per_row = ((bits_per_row + 31) / 32) * 4;
        let pixel_data_size = (bytes_per_row * height) as usize;

        // AND mask starts after header + color table + pixel data
        let and_mask_offset = header_size + color_table_size + pixel_data_size;

        // AND mask row stride (1 bit per pixel, 4-byte aligned)
        let mask_bits_per_row = width;
        let mask_bytes_per_row = ((mask_bits_per_row + 31) / 32) * 4;

        // Apply AND mask if present
        if and_mask_offset < original_data.len() {
            let and_mask = &original_data[and_mask_offset..];

            for y in 0..height {
                // Decoded pixels are top-down (y=0 is top row)
                // AND mask is stored bottom-up, so row 0 in mask = bottom row of image
                // For top-down pixel y, we need mask row (height - 1 - y)
                let mask_row_offset = ((height - 1 - y) * mask_bytes_per_row) as usize;

                for x in 0..width {
                    let byte_index = mask_row_offset + (x / 8) as usize;
                    let bit_index = 7 - (x % 8);

                    if byte_index < and_mask.len() {
                        let mask_bit = (and_mask[byte_index] >> bit_index) & 1;
                        if mask_bit == 1 {
                            // AND mask bit 1 = transparent
                            let pixel_index = ((y * width + x) * 4 + 3) as usize;
                            if pixel_index < rgba_pixels.len() {
                                rgba_pixels[pixel_index] = 0;
                            }
                        }
                    }
                }
            }
        }

        Ok(rgba_pixels)
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color(&self) -> IcoColor {
        IcoColor::RGBA // All ICO images are decoded to RGBA
    }

    pub fn alpha(&self) -> IcoAlpha {
        self.alpha
    }

    pub fn reset_decode_state(&mut self) {
        self.current_row = 0;
    }

    pub fn next_row(&mut self, buffer: &mut [u8]) -> DecodingResult {
        if self.current_row >= self.height {
            return DecodingResult::Success;
        }

        let bytes_per_pixel = 4u32; // RGBA
        let row_bytes = (self.width * bytes_per_pixel) as usize;

        if buffer.len() < row_bytes {
            return DecodingResult::ParameterError;
        }

        let row_offset = (self.current_row as usize) * row_bytes;
        buffer[..row_bytes].copy_from_slice(&self.pixels[row_offset..row_offset + row_bytes]);

        self.current_row += 1;
        DecodingResult::Success
    }
}

/// Create a new reader for an ICO entry from a stream.
pub fn new_reader(
    input: Pin<&mut ffi::SkStreamAdapter>,
    format: EmbeddedFormat,
    entry_size: u32,
) -> Box<ResultOfReader> {
    let result = match format {
        EmbeddedFormat::Png => Reader::new_png_from_stream(input),
        EmbeddedFormat::Bmp => Reader::new_bmp_from_stream(input, entry_size),
        // Handle any future variants or invalid values
        _ => Err(IcoError::UnsupportedFeature),
    };
    Box::new(ResultOfReader { result })
}

/// Apply the AND mask from BMP-in-ICO entry data to a decoded pixel buffer.
///
/// For non-32-bit BMPs embedded in ICO, a 1bpp AND mask follows the pixel data.
/// A set bit (1) means the pixel is transparent. This function zeroes out all
/// bytes of transparent pixels (works correctly for both premul and unpremul).
///
/// `pixels` is the decoded output buffer (bytes_per_pixel bytes per pixel).
/// `entry_data` is the raw BMP entry data (starting at the DIB header).
/// `width` and `height` are the image dimensions.
/// `bytes_per_pixel` is the number of bytes per pixel in the output buffer.
pub fn apply_and_mask(
    pixels: &mut [u8],
    entry_data: &[u8],
    width: u32,
    height: u32,
    bytes_per_pixel: u32,
) {
    if entry_data.len() < 16 {
        return;
    }

    // Read bit_count from the DIB header (offset 14-15)
    let bit_count = u16::from_le_bytes([entry_data[14], entry_data[15]]);

    // 32-bit BMPs have alpha embedded — no AND mask to apply
    if bit_count == 32 {
        return;
    }

    let header_size = u32::from_le_bytes([
        entry_data[0], entry_data[1], entry_data[2], entry_data[3],
    ]) as usize;

    // Color table size (for bit depths <= 8)
    let color_table_size = if bit_count <= 8 {
        (1usize << bit_count) * 4
    } else {
        0
    };

    // Row stride for pixel data (4-byte aligned)
    let bits_per_row = width * (bit_count as u32);
    let bytes_per_row = ((bits_per_row + 31) / 32) * 4;
    let pixel_data_size = (bytes_per_row * height) as usize;

    let and_mask_offset = header_size + color_table_size + pixel_data_size;

    if and_mask_offset >= entry_data.len() {
        return;
    }

    let and_mask = &entry_data[and_mask_offset..];

    // AND mask row stride (1 bit per pixel, 4-byte aligned)
    let mask_bytes_per_row = ((width + 31) / 32) * 4;

    let bpp = bytes_per_pixel as usize;

    for y in 0..height {
        // Decoded pixels are top-down (y=0 is top row).
        // AND mask is stored bottom-up, so mask row 0 = bottom row of image.
        let mask_row_offset = ((height - 1 - y) * mask_bytes_per_row) as usize;

        for x in 0..width {
            let byte_index = mask_row_offset + (x / 8) as usize;
            let bit_index = 7 - (x % 8);

            if byte_index < and_mask.len() {
                let mask_bit = (and_mask[byte_index] >> bit_index) & 1;
                if mask_bit == 1 {
                    // Transparent pixel — zero all bytes (works for premul and unpremul)
                    let pixel_offset = (y * width + x) as usize * bpp;
                    let end = pixel_offset + bpp;
                    if end <= pixels.len() {
                        pixels[pixel_offset..end].fill(0);
                    }
                }
            }
        }
    }
}

// ============================================================================
// Directory parsing (unchanged)
// ============================================================================

/// PNG file signature (first 8 bytes).
const PNG_SIGNATURE: [u8; 8] = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A];

/// ICO directory header size in bytes.
const ICO_DIRECTORY_BYTES: usize = 6;

/// ICO directory entry size in bytes.
const ICO_DIR_ENTRY_BYTES: usize = 16;

/// Checks if the stream starts with an ICO or CUR file signature.
/// Reads up to 4 bytes from the stream.
///
/// ICO signature: 00 00 01 00
/// CUR signature: 00 00 02 00
pub fn is_ico(mut input: Pin<&mut ffi::SkStreamAdapter>) -> bool {
    let mut header = [0u8; 4];
    if input.read_exact(&mut header).is_err() {
        return false;
    }
    is_ico_data(&header)
}

/// Checks if the data starts with an ICO or CUR file signature.
/// Internal helper used by is_ico and tests.
fn is_ico_data(data: &[u8]) -> bool {
    if data.len() < 4 {
        return false;
    }
    // ICO: 00 00 01 00
    // CUR: 00 00 02 00
    data[0] == 0x00 && data[1] == 0x00 && (data[2] == 0x01 || data[2] == 0x02) && data[3] == 0x00
}

/// Checks if the data starts with a PNG signature.
fn is_png_data(data: &[u8]) -> bool {
    data.len() >= PNG_SIGNATURE.len() && data[..PNG_SIGNATURE.len()] == PNG_SIGNATURE
}

/// Read a little-endian u16 from the data at the given offset.
fn read_u16_le(data: &[u8], offset: usize) -> u16 {
    u16::from_le_bytes([data[offset], data[offset + 1]])
}

/// Read a little-endian u32 from the data at the given offset.
fn read_u32_le(data: &[u8], offset: usize) -> u32 {
    u32::from_le_bytes([
        data[offset],
        data[offset + 1],
        data[offset + 2],
        data[offset + 3],
    ])
}

/// Result of parsing an ICO directory.
pub struct DirectoryResult {
    status: DirectoryParseResult,
    file_type: IcoFileType,
    entries: Vec<IcoEntry>,
}

impl DirectoryResult {
    fn error(status: DirectoryParseResult) -> Box<Self> {
        Box::new(Self { status, file_type: IcoFileType::Icon, entries: Vec::new() })
    }

    fn error_for(status: DirectoryParseResult, file_type: IcoFileType) -> Box<Self> {
        Box::new(Self { status, file_type, entries: Vec::new() })
    }

    pub fn status(&self) -> DirectoryParseResult {
        self.status
    }

    pub fn file_type(&self) -> IcoFileType {
        self.file_type
    }

    pub fn image_count(&self) -> u32 {
        self.entries.len() as u32
    }

    pub fn get_entry(&self, index: u32) -> IcoEntry {
        self.entries[index as usize]
    }
}

/// Parse the ICO directory from a stream.
///
/// This parses the ICO header and directory entries, determining the offset,
/// size, and format (PNG or BMP) of each embedded image.
pub fn parse_directory(mut input: Pin<&mut ffi::SkStreamAdapter>) -> Box<DirectoryResult> {
    // Read all data from the stream into memory
    let mut data = Vec::new();
    if input.read_to_end(&mut data).is_err() {
        return DirectoryResult::error(DirectoryParseResult::InsufficientData);
    }

    parse_directory_from_data(&data)
}

/// Parse the ICO directory from the given data.
///
/// This parses the ICO header and directory entries, determining the offset,
/// size, and format (PNG or BMP) of each embedded image.
fn parse_directory_from_data(data: &[u8]) -> Box<DirectoryResult> {
    // Check minimum size for header
    if data.len() < ICO_DIRECTORY_BYTES {
        return DirectoryResult::error(DirectoryParseResult::InsufficientData);
    }

    // Validate ICO/CUR signature
    if !is_ico_data(data) {
        return DirectoryResult::error(DirectoryParseResult::InvalidSignature);
    }

    // Determine file type from header (offset 2, 2 bytes LE)
    // 1 = ICO (icon), 2 = CUR (cursor)
    let file_type_value = read_u16_le(data, 2);
    let file_type = if file_type_value == 2 {
        IcoFileType::Cursor
    } else {
        IcoFileType::Icon
    };

    // Read number of images from header (offset 4, 2 bytes LE)
    let num_images = read_u16_le(data, 4) as usize;

    if num_images == 0 {
        return DirectoryResult::error_for(DirectoryParseResult::NoImages, file_type);
    }

    // Check if all directory entries fit in the data
    let directory_end = ICO_DIRECTORY_BYTES + num_images * ICO_DIR_ENTRY_BYTES;
    if data.len() < directory_end {
        return DirectoryResult::error_for(DirectoryParseResult::TruncatedDirectory, file_type);
    }

    // Parse each directory entry
    let mut entries = Vec::with_capacity(num_images);
    for i in 0..num_images {
        let entry_offset = ICO_DIRECTORY_BYTES + i * ICO_DIR_ENTRY_BYTES;

        // Read width and height (offset 0 and 1 within entry)
        // 0 means 256 for both
        let width = data[entry_offset];
        let height = data[entry_offset + 1];

        // Read hotspot/planes (offset 4 within entry) and hotspot/bpp (offset 6 within entry)
        // For CUR files, these are the hotspot X and Y coordinates.
        // For ICO files, these are color planes and bits per pixel.
        let hotspot_x = read_u16_le(data, entry_offset + 4);
        let hotspot_y_or_bpp = read_u16_le(data, entry_offset + 6);

        // For ICO files, offset 6 is bits per pixel. For CUR, it's hotspot_y.
        let bit_count = if file_type == IcoFileType::Cursor {
            0 // Not used for sorting CUR files
        } else {
            hotspot_y_or_bpp
        };
        let hotspot_y = hotspot_y_or_bpp;

        // Read size (offset 8 within entry) and offset (offset 12 within entry)
        let size = read_u32_le(data, entry_offset + 8);
        let offset = read_u32_le(data, entry_offset + 12);

        // Determine format by checking if the embedded data starts with PNG signature
        let format = if (offset as usize) < data.len() {
            let embedded_start = offset as usize;
            let available = data.len() - embedded_start;
            if available >= PNG_SIGNATURE.len() && is_png_data(&data[embedded_start..]) {
                EmbeddedFormat::Png
            } else {
                EmbeddedFormat::Bmp
            }
        } else {
            // Can't determine, assume BMP (will fail later if invalid)
            EmbeddedFormat::Bmp
        };

        entries.push(IcoEntry {
            offset,
            size,
            format,
            hotspot_x,
            hotspot_y,
            width,
            height,
            bit_count,
        });
    }

    // Sort entries by offset for proper sequential reading.
    // Note: C++ ICOImageDecoder sorts by size (largest first) for quality selection,
    // but SkIcoCodec sorts by offset. We follow SkIcoCodec's approach here.
    entries.sort_by_key(|e| e.offset);

    Box::new(DirectoryResult {
        status: DirectoryParseResult::Success,
        file_type,
        entries,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_ico_data() {
        // Valid ICO signature
        let valid_ico = vec![0x00, 0x00, 0x01, 0x00, 0x01, 0x00];
        assert!(is_ico_data(&valid_ico));

        // Valid CUR signature
        let valid_cur = vec![0x00, 0x00, 0x02, 0x00, 0x01, 0x00];
        assert!(is_ico_data(&valid_cur));

        // Invalid signature (PNG)
        let png_sig = vec![0x89, 0x50, 0x4E, 0x47];
        assert!(!is_ico_data(&png_sig));

        // Invalid signature (BMP)
        let bmp_sig = vec![0x42, 0x4D, 0x00, 0x00];
        assert!(!is_ico_data(&bmp_sig));

        // Too short
        let too_short = vec![0x00, 0x00, 0x01];
        assert!(!is_ico_data(&too_short));

        // Empty
        assert!(!is_ico_data(&[]));
    }

    #[test]
    fn test_parse_directory_insufficient_data() {
        let result = parse_directory_from_data(&[0x00, 0x00, 0x01]);
        assert!(matches!(
            result.status(),
            DirectoryParseResult::InsufficientData
        ));
        assert_eq!(result.image_count(), 0);
    }

    #[test]
    fn test_parse_directory_invalid_signature() {
        let data = vec![0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A]; // PNG header
        let result = parse_directory_from_data(&data);
        assert!(matches!(
            result.status(),
            DirectoryParseResult::InvalidSignature
        ));
    }

    #[test]
    fn test_parse_directory_no_images() {
        // Valid ICO header but 0 images
        let data = vec![0x00, 0x00, 0x01, 0x00, 0x00, 0x00];
        let result = parse_directory_from_data(&data);
        assert!(matches!(result.status(), DirectoryParseResult::NoImages));
    }

    #[test]
    fn test_parse_directory_truncated() {
        // ICO header claiming 1 image but no directory entry data
        let data = vec![0x00, 0x00, 0x01, 0x00, 0x01, 0x00];
        let result = parse_directory_from_data(&data);
        assert!(matches!(
            result.status(),
            DirectoryParseResult::TruncatedDirectory
        ));
    }

    #[test]
    fn test_parse_directory_single_bmp() {
        // ICO with 1 BMP image
        // Header: 00 00 01 00 01 00 (ICO, 1 image)
        // Entry: width(1), height(1), colors(1), reserved(1), planes(2), bpp(2), size(4), offset(4)
        let mut data = vec![
            0x00, 0x00, 0x01, 0x00, // ICO signature
            0x01, 0x00, // 1 image
            // Directory entry (16 bytes)
            0x10, // width = 16 (0 means 256)
            0x10, // height = 16
            0x00, // color count
            0x00, // reserved
            0x01, 0x00, // planes
            0x20, 0x00, // bits per pixel
            0x00, 0x01, 0x00, 0x00, // size = 256
            0x16, 0x00, 0x00, 0x00, // offset = 22 (right after header+entry)
        ];
        // Add some BMP data (not PNG signature)
        data.extend_from_slice(&[0x28, 0x00, 0x00, 0x00]); // BITMAPINFOHEADER

        let result = parse_directory_from_data(&data);
        assert!(matches!(result.status(), DirectoryParseResult::Success));
        assert_eq!(result.image_count(), 1);

        let entry = result.get_entry(0);
        assert_eq!(entry.offset, 22);
        assert_eq!(entry.size, 256);
        assert_eq!(entry.format, EmbeddedFormat::Bmp);
    }

    #[test]
    fn test_parse_directory_single_png() {
        // ICO with 1 PNG image
        let mut data = vec![
            0x00, 0x00, 0x01, 0x00, // ICO signature
            0x01, 0x00, // 1 image
            // Directory entry (16 bytes)
            0x10, 0x10, 0x00, 0x00, // width, height, colors, reserved
            0x01, 0x00, 0x20, 0x00, // planes, bpp
            0x00, 0x01, 0x00, 0x00, // size = 256
            0x16, 0x00, 0x00, 0x00, // offset = 22
        ];
        // Add PNG signature
        data.extend_from_slice(&PNG_SIGNATURE);

        let result = parse_directory_from_data(&data);
        assert!(matches!(result.status(), DirectoryParseResult::Success));
        assert_eq!(result.image_count(), 1);

        let entry = result.get_entry(0);
        assert_eq!(entry.format, EmbeddedFormat::Png);
    }
}