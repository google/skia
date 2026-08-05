// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! JPEG Multi-Picture Format (MPF) parser.
//!
//! Parses the MPF structure from APP2 segments per CIPA DC-x007-2009.
//! Extracts image offsets and sizes from the MP Index IFD.

use crate::jpeg_segment_scan::{get_segment_params, jpeg_marker, ScannedSegment};
use crate::tiff_ifd;

/// MPF segment signature: "MPF\0".
const MPF_SIG: &[u8] = b"MPF\0";
/// Size of one MP Entry in the Index IFD: 16 bytes.
const MPF_ENTRY_SIZE: usize = 16;
/// MPF version we accept: "0100".
const MPF_VERSION_EXPECTED: &[u8] = b"0100";

// MPF IFD tag constants (from CIPA DC-x007-2009).
const MPF_TAG_VERSION: u16 = 0xB000;
const MPF_TAG_NUMBER_OF_IMAGES: u16 = 0xB001;
const MPF_TAG_MP_ENTRY: u16 = 0xB002;
const MPF_TAG_INDIVIDUAL_IMAGE_UNIQUE_ID: u16 = 0xB003;
const MPF_TAG_TOTAL_FRAMES: u16 = 0xB004;

// MP Entry attribute masks.
const MP_ENTRY_ATTR_FORMAT_MASK: u32 = 0x7000000;
const MP_ENTRY_ATTR_TYPE_MASK: u32 = 0xFFFFFF;
const MP_ENTRY_ATTR_TYPE_PRIMARY: u32 = 0x030000;

/// A single image listed in MPF.
#[derive(Clone, Debug)]
pub struct MpfImage {
    pub size: u32,
    pub data_offset: u32,
}

/// Result of MPF parsing: list of images + offset of the MPF segment.
#[derive(Clone, Debug)]
pub struct MpfResult {
    pub images: Vec<MpfImage>,
    /// Byte offset of the MPF APP2 segment's 0xFF marker byte.
    pub segment_offset: u32,
}

/// Parse MPF metadata from segment parameter data (after stripping the
/// marker and length field).  Returns `None` if the data is not valid MPF.
pub fn parse_mpf_segment(param_data: &[u8]) -> Option<Vec<MpfImage>> {
    if param_data.len() < MPF_SIG.len() {
        return None;
    }
    if &param_data[..MPF_SIG.len()] != MPF_SIG {
        return None;
    }

    let tiff_data = &param_data[MPF_SIG.len()..];
    let (little_endian, ifd_offset) = tiff_ifd::parse_header(tiff_data)?;
    let entries = tiff_ifd::parse_ifd(tiff_data, ifd_offset, little_endian)?;

    let mut number_of_images: u32 = 0;
    let mut mp_entries_data: Option<&[u8]> = None;
    let mut previous_tag: u16 = 0;

    for entry in &entries {
        if previous_tag >= entry.tag {
            return None;
        }
        previous_tag = entry.tag;

        match entry.tag {
            MPF_TAG_VERSION => {
                let data = tiff_ifd::get_entry_data(tiff_data, entry, little_endian)?;
                if data != MPF_VERSION_EXPECTED {
                    return None;
                }
            }
            MPF_TAG_NUMBER_OF_IMAGES => {
                number_of_images =
                    tiff_ifd::read_entry_unsigned_long(tiff_data, entry, little_endian)?;
                if number_of_images < 1 {
                    return None;
                }
            }
            MPF_TAG_MP_ENTRY => {
                let data = tiff_ifd::get_entry_data(tiff_data, entry, little_endian)?;
                let expected_size = MPF_ENTRY_SIZE.checked_mul(number_of_images as usize)?;
                if data.len() != expected_size {
                    return None;
                }
                mp_entries_data = Some(data);
            }
            MPF_TAG_INDIVIDUAL_IMAGE_UNIQUE_ID => {
                let data = tiff_ifd::get_entry_data(tiff_data, entry, little_endian)?;
                let expected = 33usize.checked_mul(number_of_images as usize)?;
                if data.len() != expected {
                    return None;
                }
            }
            MPF_TAG_TOTAL_FRAMES => {}
            // Skip unknown tags (e.g. vendor extensions) rather than
            // rejecting the entire MPF block, matching C++ behavior.
            _ => {}
        }
    }

    if number_of_images == 0 {
        return None;
    }
    let mp_entries_data = mp_entries_data?;

    let mut images = Vec::with_capacity(number_of_images as usize);
    for i in 0..number_of_images as usize {
        let e = &mp_entries_data[i * MPF_ENTRY_SIZE..];
        let attribute = tiff_ifd::read_u32(e, 0, little_endian)?;
        let size = tiff_ifd::read_u32(e, 4, little_endian)?;
        let data_offset = tiff_ifd::read_u32(e, 8, little_endian)?;

        let is_primary = (attribute & MP_ENTRY_ATTR_TYPE_MASK) == MP_ENTRY_ATTR_TYPE_PRIMARY;
        let is_jpeg = (attribute & MP_ENTRY_ATTR_FORMAT_MASK) == 0;

        if is_primary != (i == 0) {
            return None;
        }
        if !is_jpeg {
            return None;
        }
        if i == 0 && data_offset != 0 {
            return None;
        }

        images.push(MpfImage { size, data_offset });
    }

    Some(images)
}

/// Search scanned segments for MPF metadata, returning the parsed result.
pub fn find_mpf_in_segments(raw_data: &[u8], segments: &[ScannedSegment]) -> Option<MpfResult> {
    for seg in segments {
        if seg.marker != jpeg_marker::APP0 + 2 {
            continue;
        }
        let param_data = get_segment_params(raw_data, seg);
        if let Some(images) = parse_mpf_segment(param_data) {
            return Some(MpfResult {
                images,
                segment_offset: seg.offset,
            });
        }
    }
    None
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::jpeg_segment_scan::SegmentScanner;

    /// Build a minimal MPF APP2 segment parameter payload (big-endian).
    fn make_mpf_param_data() -> Vec<u8> {
        let mut d = Vec::new();
        d.extend_from_slice(MPF_SIG);
        d.extend_from_slice(&[b'M', b'M', 0x00, 0x2A]);
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x08]);

        let tiff_start = MPF_SIG.len();
        d.extend_from_slice(&[0x00, 0x03]); // 3 entries

        // Entry 1: Version
        d.extend_from_slice(&[0xB0, 0x00]);
        d.extend_from_slice(&[0x00, 0x07]);
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x04]);
        d.extend_from_slice(b"0100");

        // Entry 2: NumberOfImages = 2
        d.extend_from_slice(&[0xB0, 0x01]);
        d.extend_from_slice(&[0x00, 0x04]);
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x01]);
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x02]);

        // Entry 3: MPEntry (out-of-line, 32 bytes)
        d.extend_from_slice(&[0xB0, 0x02]);
        d.extend_from_slice(&[0x00, 0x07]);
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x20]);
        let mp_entry_data_offset = d.len() - tiff_start + 4 + 4;
        d.extend_from_slice(&(mp_entry_data_offset as u32).to_be_bytes());

        // Next IFD offset = 0
        d.extend_from_slice(&[0x00, 0x00, 0x00, 0x00]);

        // MP Entry 0: primary, size=1000, offset=0
        d.extend_from_slice(&0x030000u32.to_be_bytes());
        d.extend_from_slice(&1000u32.to_be_bytes());
        d.extend_from_slice(&0u32.to_be_bytes());
        d.extend_from_slice(&[0x00; 4]);

        // MP Entry 1: secondary, size=500, offset=2000
        d.extend_from_slice(&0u32.to_be_bytes());
        d.extend_from_slice(&500u32.to_be_bytes());
        d.extend_from_slice(&2000u32.to_be_bytes());
        d.extend_from_slice(&[0x00; 4]);

        d
    }

    #[test]
    fn test_parse_mpf_segment_basic() {
        let mpf_data = make_mpf_param_data();
        let images = parse_mpf_segment(&mpf_data).unwrap();
        assert_eq!(images.len(), 2);
        assert_eq!(images[0].size, 1000);
        assert_eq!(images[0].data_offset, 0);
        assert_eq!(images[1].size, 500);
        assert_eq!(images[1].data_offset, 2000);
    }

    #[test]
    fn test_parse_mpf_segment_no_signature() {
        assert!(parse_mpf_segment(b"NOT_MPF_DATA").is_none());
    }

    #[test]
    fn test_parse_mpf_segment_empty() {
        assert!(parse_mpf_segment(&[]).is_none());
    }

    #[test]
    fn test_find_mpf_in_segments() {
        let mpf_params = make_mpf_param_data();
        let mut jpeg = Vec::new();
        jpeg.extend_from_slice(&[0xFF, 0xD8]);
        jpeg.extend_from_slice(&[0xFF, 0xE2]);
        let param_len = (mpf_params.len() as u16) + 2;
        jpeg.extend_from_slice(&param_len.to_be_bytes());
        jpeg.extend_from_slice(&mpf_params);
        jpeg.extend_from_slice(&[0xFF, 0xDA]);
        jpeg.extend_from_slice(&[0x00, 0x02]);

        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&jpeg);
        assert!(scanner.is_done());

        let mpf = find_mpf_in_segments(&jpeg, &scanner.segments).unwrap();
        assert_eq!(mpf.images.len(), 2);
        assert_eq!(mpf.images[1].data_offset, 2000);
        assert_eq!(mpf.segment_offset, 2);
    }
}
