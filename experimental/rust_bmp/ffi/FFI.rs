// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for BMP decoder using the image-rs crate.

#[cxx::bridge(namespace = "rust_bmp")]
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

    /// BMP color type matching SkEncodedInfo::Color values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum BmpColor {
        RGB = 5,
        RGBA = 6,
        BGR = 7,
        BGRA = 9,
    }

    /// BMP alpha type matching SkEncodedInfo::Alpha values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum BmpAlpha {
        Opaque = 0,
        Unpremul = 1,
    }

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        fn is_bmp_data(data: &[u8]) -> bool;

        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> BmpColor;
        fn alpha(self: &Reader) -> BmpAlpha;
        fn next_row(self: &mut Reader, buffer: &mut [u8]) -> DecodingResult;
        fn reset_decode_state(self: &mut Reader);
    }
}

pub use ffi::*;

#[derive(Debug)]
enum BmpError {
    InsufficientData,
    InvalidData,
    UnsupportedFeature,
    InvalidHeader,
    IoError,
    FormatError,
}

impl From<image::ImageError> for BmpError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => BmpError::FormatError,
            image::ImageError::Encoding(_) => BmpError::FormatError,
            image::ImageError::Parameter(_) => {
                panic!(
                    "Internal bug in how Skia Rust BMP calls image crate: {:?}",
                    err
                )
            }
            image::ImageError::Limits(_) => BmpError::UnsupportedFeature,
            image::ImageError::Unsupported(_) => BmpError::UnsupportedFeature,
            image::ImageError::IoError(_) => BmpError::IoError,
        }
    }
}

impl From<std::io::Error> for BmpError {
    fn from(err: std::io::Error) -> Self {
        use std::io::ErrorKind;
        match err.kind() {
            ErrorKind::UnexpectedEof => BmpError::InsufficientData,
            _ => BmpError::IoError,
        }
    }
}

pub fn is_bmp_data(data: &[u8]) -> bool {
    // "IC", "PT", "CI", "CP", "BA" are not yet supported by image crate.
    data.len() >= 2 && data[0] == b'B' && data[1] == b'M'
}

pub struct ResultOfReader {
    result: Result<Reader, BmpError>,
}

impl ResultOfReader {
    pub fn err(&self) -> DecodingResult {
        match &self.result {
            Ok(_) => DecodingResult::Success,
            Err(BmpError::InvalidHeader) => DecodingResult::FormatError,
            Err(BmpError::InsufficientData) => DecodingResult::IncompleteInput,
            Err(BmpError::UnsupportedFeature) => DecodingResult::UnsupportedFeature,
            Err(BmpError::InvalidData) => DecodingResult::FormatError,
            Err(BmpError::IoError) => DecodingResult::OtherError,
            Err(BmpError::FormatError) => DecodingResult::FormatError,
        }
    }

    pub fn unwrap(&mut self) -> Box<Reader> {
        match std::mem::replace(&mut self.result, Err(BmpError::InvalidHeader)) {
            Ok(reader) => Box::new(reader),
            Err(_) => panic!("Called unwrap on an error ResultOfReader"),
        }
    }
}

/// Image-rs BMP decoder doesn't support true row-by-row streaming, so this
/// Reader decodes the entire image upfront and provides row access via next_row().
pub struct Reader {
    pixels: Vec<u8>,
    width: u32,
    height: u32,
    color: BmpColor,
    alpha: BmpAlpha,
    bytes_per_pixel: u32,
    current_row: u32,
}

impl Reader {
    fn new(mut input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, BmpError> {
        use std::io::{Cursor, Read};

        // TODO(crbug.com/452666425): Support incremental decoding for incomplete data. Currently we
        // read the entire stream upfront because image-rs BmpDecoder requires
        // seekable input (std::io::Seek). This means if the stream has incomplete
        // data, we fail during MakeFromStream and the caller must retry from scratch.
        // For better streaming: buffer data incrementally and retry BmpDecoder::new()
        // when more data arrives (similar to SkPngRustCodec).

        let mut data = Vec::new();
        input
            .as_mut()
            .expect("input should not be null")
            .read_to_end(&mut data)
            .map_err(|_| BmpError::InsufficientData)?;

        let mut cursor = Cursor::new(&data);
        let decoder = image::codecs::bmp::BmpDecoder::new(&mut cursor)
            .map_err(|_| BmpError::InvalidHeader)?;

        let (width, height) = image::ImageDecoder::dimensions(&decoder);
        let image_color_type = image::ImageDecoder::color_type(&decoder);

        // image-rs converts BGR→RGB internally. BmpColor also supports BGR for
        // future decoders that might preserve native order.
        let (color, alpha, bytes_per_pixel) = match image_color_type {
            image::ColorType::Rgb8 => (BmpColor::RGB, BmpAlpha::Opaque, 3),
            image::ColorType::Rgba8 => (BmpColor::RGBA, BmpAlpha::Unpremul, 4),
            _ => {
                return Err(BmpError::UnsupportedFeature);
            }
        };

        // TODO(crbug.com/452666425): Extract ICC profile for color-managed decoding. BMP files can
        // embed ICC profiles in BITMAPV5HEADER format at offset 0x7A, but
        // image-rs doesn't expose this. Once rust/icc module lands, parse the
        // BMP header manually to extract profile bytes, validate with
        // rust/icc::parse_icc_profile(), and pass to C++ via ResultOfReader
        // for SkEncodedInfo integration.

        let total_bytes = image::ImageDecoder::total_bytes(&decoder) as usize;
        let mut pixels = vec![0u8; total_bytes];
        image::ImageDecoder::read_image(decoder, &mut pixels).map_err(|_| BmpError::InvalidData)?;

        Ok(Reader {
            pixels,
            width,
            height,
            color,
            alpha,
            bytes_per_pixel,
            current_row: 0,
        })
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color(&self) -> BmpColor {
        self.color
    }

    pub fn alpha(&self) -> BmpAlpha {
        self.alpha
    }

    pub fn reset_decode_state(&mut self) {
        self.current_row = 0;
    }

    pub fn next_row(&mut self, buffer: &mut [u8]) -> DecodingResult {
        if self.current_row >= self.height {
            return DecodingResult::Success;
        }

        let row_bytes = (self.width * self.bytes_per_pixel) as usize;

        if buffer.len() < row_bytes {
            return DecodingResult::ParameterError;
        }

        let row_offset = (self.current_row as usize) * row_bytes;
        buffer[..row_bytes].copy_from_slice(&self.pixels[row_offset..row_offset + row_bytes]);

        self.current_row += 1;
        DecodingResult::Success
    }
}

pub fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<ResultOfReader> {
    let result = Reader::new(input);
    Box::new(ResultOfReader { result })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_bmp_data() {
        // Valid BMP signature (BM)
        let valid_bmp = vec![b'B', b'M', 0x00, 0x00];
        assert!(is_bmp_data(&valid_bmp));

        // BA signature not yet supported by image crate
        let valid_ba = vec![b'B', b'A', 0x00, 0x00];
        assert!(!is_bmp_data(&valid_ba));

        // Invalid signature
        let invalid = vec![0x89, b'P', b'N', b'G']; // PNG signature
        assert!(!is_bmp_data(&invalid));

        // Too short
        let too_short = vec![b'B'];
        assert!(!is_bmp_data(&too_short));

        // Empty
        assert!(!is_bmp_data(&[]));
    }
}
