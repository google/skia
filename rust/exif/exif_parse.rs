// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Custom TIFF/EXIF parser for the SkExif metadata fields.
//!
//! Handles both big-endian (Motorola, "MM") and little-endian (Intel, "II")
//! TIFF byte orders. Follows the ExifIFD sub-IFD (tag 0x8769) from the root
//! IFD to match the behavior of SkExif::Parse / SkTiff::ImageFileDirectory.

use crate::ffi::ExifMetadata;

#[derive(Clone, Copy)]
enum Endianness {
    Little,
    Big,
}

// Tag numbers parsed by this module.
const TAG_ORIENTATION: u16 = 0x0112;
const TAG_X_RESOLUTION: u16 = 0x011a;
const TAG_Y_RESOLUTION: u16 = 0x011b;
const TAG_RESOLUTION_UNIT: u16 = 0x0128;
const TAG_MAKER_NOTE: u16 = 0x927c;
const TAG_EXIF_IFD: u16 = 0x8769;
const TAG_PIXEL_X_DIMENSION: u16 = 0xa002;
const TAG_PIXEL_Y_DIMENSION: u16 = 0xa003;

// TIFF type codes.
const TYPE_SHORT: u16 = 3;
const TYPE_LONG: u16 = 4;
const TYPE_RATIONAL: u16 = 5;
const TYPE_UNDEFINED: u16 = 7;

// Apple MakerNote signature (14 bytes).
const APPLE_MAKER_NOTE_SIG: &[u8] = b"Apple iOS\x00\x00\x01MM";

fn read_u16(data: &[u8], off: usize, endianness: Endianness) -> Option<u16> {
    let bytes: [u8; 2] = data.get(off..off + 2)?.try_into().unwrap();
    Some(match endianness {
        Endianness::Little => u16::from_le_bytes(bytes),
        Endianness::Big => u16::from_be_bytes(bytes),
    })
}

fn read_u32(data: &[u8], off: usize, endianness: Endianness) -> Option<u32> {
    let bytes: [u8; 4] = data.get(off..off + 4)?.try_into().unwrap();
    Some(match endianness {
        Endianness::Little => u32::from_le_bytes(bytes),
        Endianness::Big => u32::from_be_bytes(bytes),
    })
}

fn read_i32(data: &[u8], off: usize, endianness: Endianness) -> Option<i32> {
    let bytes: [u8; 4] = data.get(off..off + 4)?.try_into().unwrap();
    Some(match endianness {
        Endianness::Little => i32::from_le_bytes(bytes),
        Endianness::Big => i32::from_be_bytes(bytes),
    })
}

/// Computes Apple HDR headroom from the raw MakerNote IFD bytes.
/// Formula: <https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos>
fn parse_apple_hdr_headroom(data: &[u8]) -> Option<f32> {
    // The Apple MakerNote is big-endian only. No little-endian images using
    // this data have been observed (matches the comment in SkExif.cpp).
    if !data.starts_with(APPLE_MAKER_NOTE_SIG) {
        return None;
    }

    let ifd_start = APPLE_MAKER_NOTE_SIG.len();
    let entry_count = read_u16(data, ifd_start, Endianness::Big)? as usize;
    let entries_start = ifd_start + 2;
    const ENTRY_SIZE: usize = 12;

    let mut maker33: Option<f32> = None;
    let mut maker48: Option<f32> = None;

    for i in 0..entry_count {
        let off = entries_start + i * ENTRY_SIZE;
        let entry = match data.get(off..off + ENTRY_SIZE) {
            Some(e) => e,
            None => break,
        };

        let tag = read_u16(entry, 0, Endianness::Big).unwrap();
        let type_ = read_u16(entry, 2, Endianness::Big).unwrap();
        let count = read_u32(entry, 4, Endianness::Big).unwrap();

        // Only process tags 33 and 48, type SRATIONAL (10), count 1.
        if (tag == 33 || tag == 48) && type_ == 10 && count == 1 {
            let value_off = read_u32(entry, 8, Endianness::Big).unwrap() as usize;
            let numer = match read_i32(data, value_off, Endianness::Big) {
                Some(v) => v,
                None => continue,
            };
            let denom = match read_i32(data, value_off + 4, Endianness::Big) {
                Some(v) => v,
                None => continue,
            };
            let value = if denom != 0 {
                numer as f32 / denom as f32
            } else {
                0.0
            };
            match tag {
                33 if maker33.is_none() => maker33 = Some(value),
                48 if maker48.is_none() => maker48 = Some(value),
                _ => {}
            }
        }
    }

    let maker33 = maker33?;
    let maker48 = maker48.unwrap_or(0.0);

    let stops = if maker33 < 1.0 {
        if maker48 <= 0.01 {
            -20.0 * maker48 + 1.8
        } else {
            -0.101 * maker48 + 1.601
        }
    } else {
        if maker48 <= 0.01 {
            -70.0 * maker48 + 3.0
        } else {
            -0.303 * maker48 + 2.303
        }
    };
    Some(f32::powf(2.0, stops.max(0.0)))
}

fn parse_ifd(
    data: &[u8],
    ifd_off: usize,
    endianness: Endianness,
    is_root: bool,
    out: &mut ExifMetadata,
) {
    let count = match read_u16(data, ifd_off, endianness) {
        Some(c) => c as usize,
        None => return,
    };
    const ENTRY_SIZE: usize = 12;
    let entries_start = ifd_off + 2;

    for i in 0..count {
        let e = entries_start + i * ENTRY_SIZE;

        // Read the four fields of the 12-byte IFD entry.
        let tag = match read_u16(data, e, endianness) {
            Some(v) => v,
            None => break,
        };
        let type_ = match read_u16(data, e + 2, endianness) {
            Some(v) => v,
            None => break,
        };
        let cnt = match read_u32(data, e + 4, endianness) {
            Some(v) => v,
            None => break,
        };
        // The last 4 bytes are either an inline value (if total ≤ 4 bytes) or
        // a file offset to the value. We read it as u32 here regardless and
        // treat it as an offset only when total > 4.
        let val_field = match read_u32(data, e + 8, endianness) {
            Some(v) => v,
            None => break,
        };

        match tag {
            // Orientation: SHORT, count=1, 2 bytes → inline at e+8.
            TAG_ORIENTATION if !out.has_origin && type_ == TYPE_SHORT && cnt == 1 => {
                if let Some(v) = read_u16(data, e + 8, endianness) {
                    if v >= 1 && v <= 8 {
                        out.has_origin = true;
                        out.origin = v as u32;
                    }
                }
            }
            // ResolutionUnit: SHORT, count=1, 2 bytes → inline at e+8.
            TAG_RESOLUTION_UNIT if !out.has_resolution_unit && type_ == TYPE_SHORT && cnt == 1 => {
                if let Some(v) = read_u16(data, e + 8, endianness) {
                    out.has_resolution_unit = true;
                    out.resolution_unit = v;
                }
            }
            // XResolution: RATIONAL, count=1, 8 bytes → val_field is a file offset.
            TAG_X_RESOLUTION if !out.has_x_resolution && type_ == TYPE_RATIONAL && cnt == 1 => {
                let off = val_field as usize;
                if let (Some(n), Some(d)) = (
                    read_u32(data, off, endianness),
                    read_u32(data, off + 4, endianness),
                ) {
                    out.has_x_resolution = true;
                    out.x_resolution = if d != 0 { n as f32 / d as f32 } else { 0.0 };
                }
            }
            // YResolution: RATIONAL, count=1, 8 bytes → val_field is a file offset.
            TAG_Y_RESOLUTION if !out.has_y_resolution && type_ == TYPE_RATIONAL && cnt == 1 => {
                let off = val_field as usize;
                if let (Some(n), Some(d)) = (
                    read_u32(data, off, endianness),
                    read_u32(data, off + 4, endianness),
                ) {
                    out.has_y_resolution = true;
                    out.y_resolution = if d != 0 { n as f32 / d as f32 } else { 0.0 };
                }
            }
            // PixelXDimension: SHORT or LONG per Exif 2.3 spec, count=1 → inline at e+8.
            TAG_PIXEL_X_DIMENSION if !out.has_pixel_x_dimension && cnt == 1 => {
                if type_ == TYPE_SHORT {
                    if let Some(v) = read_u16(data, e + 8, endianness) {
                        out.has_pixel_x_dimension = true;
                        out.pixel_x_dimension = v as u32;
                    }
                } else if type_ == TYPE_LONG {
                    if let Some(v) = read_u32(data, e + 8, endianness) {
                        out.has_pixel_x_dimension = true;
                        out.pixel_x_dimension = v;
                    }
                }
            }
            // PixelYDimension: SHORT or LONG per Exif 2.3 spec, count=1 → inline at e+8.
            TAG_PIXEL_Y_DIMENSION if !out.has_pixel_y_dimension && cnt == 1 => {
                if type_ == TYPE_SHORT {
                    if let Some(v) = read_u16(data, e + 8, endianness) {
                        out.has_pixel_y_dimension = true;
                        out.pixel_y_dimension = v as u32;
                    }
                } else if type_ == TYPE_LONG {
                    if let Some(v) = read_u32(data, e + 8, endianness) {
                        out.has_pixel_y_dimension = true;
                        out.pixel_y_dimension = v;
                    }
                }
            }
            // MakerNote: UNDEFINED bytes, typically large → val_field is a file offset.
            TAG_MAKER_NOTE if !out.has_hdr_headroom && type_ == TYPE_UNDEFINED && cnt > 4 => {
                let off = val_field as usize;
                let len = cnt as usize;
                if let Some(bytes) = data.get(off..off + len) {
                    if let Some(h) = parse_apple_hdr_headroom(bytes) {
                        out.has_hdr_headroom = true;
                        out.hdr_headroom = h;
                    }
                }
            }
            // ExifIFD pointer: LONG, count=1 → val_field is the sub-IFD offset.
            // Only follow from the root IFD to match SkExif.cpp behavior.
            TAG_EXIF_IFD if is_root && type_ == TYPE_LONG && cnt == 1 => {
                parse_ifd(data, val_field as usize, endianness, false, out);
            }
            _ => {}
        }
    }
}

/// Parses a raw TIFF-formatted EXIF blob into `out`.
/// Returns `true` if the header is valid; `false` otherwise.
/// Missing individual fields are indicated by their `has_*` flags.
pub fn parse(data: &[u8], out: &mut ExifMetadata) -> bool {
    if data.len() < 8 {
        return false;
    }

    let endianness = match data.get(0..2) {
        Some(b"MM") => Endianness::Big,
        Some(b"II") => Endianness::Little,
        _ => return false,
    };

    // TIFF magic: 42 (0x002A).
    if read_u16(data, 2, endianness) != Some(0x002A) {
        return false;
    }

    let ifd_off = match read_u32(data, 4, endianness) {
        Some(off) => off as usize,
        None => return false,
    };

    parse_ifd(data, ifd_off, endianness, true, out);
    true
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read_u16() {
        let data = [0x01, 0x02, 0x03, 0x04];
        assert_eq!(read_u16(&data, 0, Endianness::Big), Some(0x0102));
        assert_eq!(read_u16(&data, 0, Endianness::Little), Some(0x0201));
        assert_eq!(read_u16(&data, 2, Endianness::Big), Some(0x0304));
        assert_eq!(read_u16(&data, 2, Endianness::Little), Some(0x0403));
        assert_eq!(read_u16(&data, 3, Endianness::Big), None); // out of bounds
    }

    #[test]
    fn test_read_u32() {
        let data = [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08];
        assert_eq!(read_u32(&data, 0, Endianness::Big), Some(0x01020304));
        assert_eq!(read_u32(&data, 0, Endianness::Little), Some(0x04030201));
        assert_eq!(read_u32(&data, 4, Endianness::Big), Some(0x05060708));
        assert_eq!(read_u32(&data, 4, Endianness::Little), Some(0x08070605));
        assert_eq!(read_u32(&data, 5, Endianness::Big), None); // out of bounds
    }

    #[test]
    fn test_read_i32() {
        // 0xFF000001 is -16777215 in two's complement big-endian.
        let data = [0xFF, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0xFF];
        assert_eq!(read_i32(&data, 0, Endianness::Big), Some(-16777215));
        assert_eq!(
            read_i32(&data, 0, Endianness::Little),
            Some(0x010000FF_u32 as i32)
        );
        assert_eq!(
            read_i32(&data, 4, Endianness::Big),
            Some(0x010000FF_u32 as i32)
        );
        assert_eq!(read_i32(&data, 4, Endianness::Little), Some(-16777215));
        assert_eq!(read_i32(&data, 5, Endianness::Big), None); // out of bounds
    }

    #[test]
    fn test_apple_hdr_headroom_sig_mismatch() {
        assert!(parse_apple_hdr_headroom(b"").is_none());
        assert!(parse_apple_hdr_headroom(b"Not Apple MakerNote data").is_none());
    }

    #[test]
    fn test_apple_hdr_headroom_truncated() {
        // Signature present but no IFD data following it.
        assert!(parse_apple_hdr_headroom(APPLE_MAKER_NOTE_SIG).is_none());
    }

    #[test]
    fn test_apple_hdr_headroom_valid() {
        // Build a minimal Apple MakerNote (56 bytes):
        //   14-byte signature | 2-byte count | 2×12-byte IFD entries | 2×8-byte SRATIONAL values
        const IFD_START: usize = APPLE_MAKER_NOTE_SIG.len(); // 14
        const ENTRIES_START: usize = IFD_START + 2; // 16
        const SRATIONAL_0: usize = ENTRIES_START + 24; // 40 (after 2 entries)
        const SRATIONAL_1: usize = SRATIONAL_0 + 8; // 48

        let mut data = vec![0u8; SRATIONAL_1 + 8];
        data[..IFD_START].copy_from_slice(APPLE_MAKER_NOTE_SIG);
        data[IFD_START..IFD_START + 2].copy_from_slice(&2u16.to_be_bytes()); // count=2

        // Entry 0: tag=33, type=10 (SRATIONAL), count=1, offset → SRATIONAL_0.
        let e0 = ENTRIES_START;
        data[e0..e0 + 2].copy_from_slice(&33u16.to_be_bytes());
        data[e0 + 2..e0 + 4].copy_from_slice(&10u16.to_be_bytes());
        data[e0 + 4..e0 + 8].copy_from_slice(&1u32.to_be_bytes());
        data[e0 + 8..e0 + 12].copy_from_slice(&(SRATIONAL_0 as u32).to_be_bytes());

        // Entry 1: tag=48, type=10 (SRATIONAL), count=1, offset → SRATIONAL_1.
        let e1 = e0 + 12;
        data[e1..e1 + 2].copy_from_slice(&48u16.to_be_bytes());
        data[e1 + 2..e1 + 4].copy_from_slice(&10u16.to_be_bytes());
        data[e1 + 4..e1 + 8].copy_from_slice(&1u32.to_be_bytes());
        data[e1 + 8..e1 + 12].copy_from_slice(&(SRATIONAL_1 as u32).to_be_bytes());

        // SRATIONAL for tag 33: 1/1 = 1.0 (maker33 ≥ 1.0 branch).
        data[SRATIONAL_0..SRATIONAL_0 + 4].copy_from_slice(&1i32.to_be_bytes());
        data[SRATIONAL_0 + 4..SRATIONAL_0 + 8].copy_from_slice(&1i32.to_be_bytes());

        // SRATIONAL for tag 48: 1/10 = 0.1 (maker48 > 0.01 branch).
        data[SRATIONAL_1..SRATIONAL_1 + 4].copy_from_slice(&1i32.to_be_bytes());
        data[SRATIONAL_1 + 4..SRATIONAL_1 + 8].copy_from_slice(&10i32.to_be_bytes());

        // maker33=1.0 ≥ 1.0, maker48=0.1 > 0.01:
        //   stops = -0.303 * 0.1 + 2.303 = 2.2727
        //   headroom = 2^2.2727 ≈ 4.82
        let headroom = parse_apple_hdr_headroom(&data);
        assert!(headroom.is_some(), "expected a valid headroom value");
        let h = headroom.unwrap();
        assert!(
            (h - 4.82_f32).abs() < 0.05,
            "expected headroom ≈ 4.82, got {h}"
        );
    }
}
