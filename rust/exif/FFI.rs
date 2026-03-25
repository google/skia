// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! EXIF metadata parser FFI bindings.
//!
//! Provides C++ bindings for parsing EXIF metadata. All parsing happens in
//! Rust for memory safety, then the result is passed to the C++ side for
//! conversion to SkExif::Metadata.

mod exif_parse;

#[cxx::bridge(namespace = "rust_exif")]
mod ffi {
    /// Parsed EXIF metadata fields.
    ///
    /// CXX does not support Rust's `Option<T>`, so each field is represented
    /// as a `has_*` boolean paired with a value field. The value is only valid
    /// when `has_*` is true.
    struct ExifMetadata {
        /// Image orientation (SkEncodedOrigin), values 1-8 per EXIF tag 0x0112.
        has_origin: bool,
        origin: u32,

        /// Apple HDR headroom (computed from MakerNote tags 33 and 48).
        /// See: https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
        has_hdr_headroom: bool,
        hdr_headroom: f32,

        /// Resolution unit (EXIF tag 0x0128): 2 = inch, 3 = centimeter.
        has_resolution_unit: bool,
        resolution_unit: u16,

        /// X resolution (EXIF tag 0x011a).
        has_x_resolution: bool,
        x_resolution: f32,

        /// Y resolution (EXIF tag 0x011b).
        has_y_resolution: bool,
        y_resolution: f32,

        /// Image width in pixels (EXIF tag 0xa002, can be SHORT or LONG).
        has_pixel_x_dimension: bool,
        pixel_x_dimension: u32,

        /// Image height in pixels (EXIF tag 0xa003, can be SHORT or LONG).
        has_pixel_y_dimension: bool,
        pixel_y_dimension: u32,
    }

    extern "Rust" {
        /// Parses raw EXIF data from `data`. Returns `true` on success and
        /// populates `out`; individual missing fields are shown by `has_*` flags.
        fn parse_exif(data: &[u8], out: &mut ExifMetadata) -> bool;
    }
}

/// Parses raw EXIF bytes into `out`. Returns `false` if the data cannot be
/// parsed at all; returns `true` and sets appropriate `has_*` flags otherwise.
pub fn parse_exif(data: &[u8], out: &mut ffi::ExifMetadata) -> bool {
    *out = ffi::ExifMetadata {
        has_origin: false,
        origin: 0,
        has_hdr_headroom: false,
        hdr_headroom: 0.0,
        has_resolution_unit: false,
        resolution_unit: 0,
        has_x_resolution: false,
        x_resolution: 0.0,
        has_y_resolution: false,
        y_resolution: 0.0,
        has_pixel_x_dimension: false,
        pixel_x_dimension: 0,
        has_pixel_y_dimension: false,
        pixel_y_dimension: 0,
    };

    /// Maximum EXIF data size (4 MiB). Prevents unbounded heap allocation
    /// for malformed or adversarially crafted inputs.
    const MAX_EXIF_SIZE: usize = 4 * 1024 * 1024;

    if data.is_empty() || data.len() > MAX_EXIF_SIZE {
        return false;
    }

    exif_parse::parse(data, out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn empty_metadata() -> ffi::ExifMetadata {
        ffi::ExifMetadata {
            has_origin: false,
            origin: 0,
            has_hdr_headroom: false,
            hdr_headroom: 0.0,
            has_resolution_unit: false,
            resolution_unit: 0,
            has_x_resolution: false,
            x_resolution: 0.0,
            has_y_resolution: false,
            y_resolution: 0.0,
            has_pixel_x_dimension: false,
            pixel_x_dimension: 0,
            has_pixel_y_dimension: false,
            pixel_y_dimension: 0,
        }
    }

    #[test]
    fn test_empty_data_returns_false() {
        let mut out = empty_metadata();
        assert!(!parse_exif(&[], &mut out));
    }

    #[test]
    fn test_invalid_data_returns_false() {
        let mut out = empty_metadata();
        assert!(!parse_exif(b"not valid exif data at all", &mut out));
        assert!(!parse_exif(b"MM\x00\x2a", &mut out));
    }
}
