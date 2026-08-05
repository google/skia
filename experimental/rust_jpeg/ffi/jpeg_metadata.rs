// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! JPEG metadata extraction from scanned segments.
//!
//! Extracts ICC profiles and EXIF data from JPEG APP marker segments that
//! have been parsed by the segment scanner.

use crate::jpeg_segment_scan::{get_segment_params, jpeg_marker, ScannedSegment};

/// JPEG ICC profile signature: "ICC_PROFILE\0".
const ICC_SIG: &[u8] = b"ICC_PROFILE\0";
/// Size of the ICC marker header: signature (12) + sequence number (1) +
/// total count (1).
const ICC_HEADER_SIZE: usize = 14;

/// EXIF APP1 signature: "Exif\0\0".
const EXIF_SIG: &[u8] = b"Exif\0\0";

/// Extract and reassemble a multi-segment ICC profile from scanned APP2
/// segments.
pub fn extract_icc_profile(raw_data: &[u8], segments: &[ScannedSegment]) -> Vec<u8> {
    let mut parts: Vec<Option<Vec<u8>>> = Vec::new();
    let mut found_part_count = 0usize;
    let mut expected_part_count = 0u8;

    for seg in segments {
        if seg.marker != jpeg_marker::APP0 + 2 {
            continue;
        }
        let param_data = get_segment_params(raw_data, seg);
        if param_data.len() <= ICC_HEADER_SIZE {
            continue;
        }
        if &param_data[..ICC_SIG.len()] != ICC_SIG {
            continue;
        }
        let seq_num = param_data[12];
        let part_count = param_data[13];
        if part_count == 0 || seq_num == 0 || seq_num > part_count {
            return Vec::new();
        }

        if expected_part_count == 0 {
            expected_part_count = part_count;
            parts.resize_with(expected_part_count as usize, || None);
        } else if part_count != expected_part_count {
            return Vec::new();
        }

        let part_index = (seq_num - 1) as usize;
        if parts[part_index].is_some() {
            return Vec::new();
        }

        let payload = &param_data[ICC_HEADER_SIZE..];
        parts[part_index] = Some(payload.to_vec());
        found_part_count += 1;

        if found_part_count == expected_part_count as usize {
            break;
        }
    }

    if expected_part_count == 0 || found_part_count != expected_part_count as usize {
        return Vec::new();
    }

    let mut result = Vec::with_capacity(parts.iter().flatten().map(Vec::len).sum());
    for part in parts {
        let Some(data) = part else {
            return Vec::new();
        };
        result.extend_from_slice(&data);
    }
    result
}

/// Extract raw EXIF data (the TIFF portion after the "Exif\0\0" prefix)
/// from the first APP1 segment with an EXIF signature.
pub fn extract_exif_data(raw_data: &[u8], segments: &[ScannedSegment]) -> Vec<u8> {
    for seg in segments {
        if seg.marker != jpeg_marker::APP0 + 1 {
            continue;
        }
        let param_data = get_segment_params(raw_data, seg);
        if param_data.len() < EXIF_SIG.len() {
            continue;
        }
        if &param_data[..EXIF_SIG.len()] == EXIF_SIG {
            return param_data[EXIF_SIG.len()..].to_vec();
        }
    }
    Vec::new()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::jpeg_segment_scan::SegmentScanner;

    fn append_icc_segment(data: &mut Vec<u8>, seq_num: u8, part_count: u8, payload: &[u8]) {
        data.extend_from_slice(&[0xFF, 0xE2]); // APP2
        let param_len = (ICC_HEADER_SIZE + payload.len()) as u16 + 2;
        data.extend_from_slice(&param_len.to_be_bytes());
        data.extend_from_slice(ICC_SIG);
        data.push(seq_num);
        data.push(part_count);
        data.extend_from_slice(payload);
    }

    fn build_icc_jpeg(parts: &[(u8, u8, &[u8])]) -> Vec<u8> {
        let mut data = Vec::new();
        data.extend_from_slice(&[0xFF, 0xD8]); // SOI
        for (seq_num, part_count, payload) in parts {
            append_icc_segment(&mut data, *seq_num, *part_count, payload);
        }
        data
    }

    #[test]
    fn test_extract_icc_profile_basic() {
        let mut data = Vec::new();
        data.extend_from_slice(&[0xFF, 0xD8]); // SOI
        data.extend_from_slice(&[0xFF, 0xE2]); // APP2
        let payload = b"test_icc_data";
        let param_len = (ICC_HEADER_SIZE + payload.len()) as u16 + 2;
        data.extend_from_slice(&param_len.to_be_bytes());
        data.extend_from_slice(ICC_SIG);
        data.push(1); // sequence number
        data.push(1); // total count
        data.extend_from_slice(payload);

        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        let icc = extract_icc_profile(&data, &scanner.segments);
        assert_eq!(icc, payload);
    }

    #[test]
    fn test_extract_exif_data_basic() {
        let mut data = Vec::new();
        data.extend_from_slice(&[0xFF, 0xD8]); // SOI
        data.extend_from_slice(&[0xFF, 0xE1]); // APP1
        let tiff_data = b"some_tiff_content";
        let param_len = (EXIF_SIG.len() + tiff_data.len()) as u16 + 2;
        data.extend_from_slice(&param_len.to_be_bytes());
        data.extend_from_slice(EXIF_SIG);
        data.extend_from_slice(tiff_data);

        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        let exif = extract_exif_data(&data, &scanner.segments);
        assert_eq!(exif, tiff_data);
    }

    #[test]
    fn test_extract_icc_profile_multi_segment() {
        // Build a JPEG with two APP2 ICC segments (out of order) that must
        // be reassembled by sequence number.
        let part1 = b"FIRST_CHUNK_";
        let part2 = b"SECOND_CHUNK";

        let mut data = Vec::new();
        data.extend_from_slice(&[0xFF, 0xD8]); // SOI

        // Segment with sequence number 2 (arrives first in the stream).
        data.extend_from_slice(&[0xFF, 0xE2]); // APP2
        let param_len2 = (ICC_HEADER_SIZE + part2.len()) as u16 + 2;
        data.extend_from_slice(&param_len2.to_be_bytes());
        data.extend_from_slice(ICC_SIG);
        data.push(2); // sequence number
        data.push(2); // total count
        data.extend_from_slice(part2);

        // Segment with sequence number 1 (arrives second in the stream).
        data.extend_from_slice(&[0xFF, 0xE2]); // APP2
        let param_len1 = (ICC_HEADER_SIZE + part1.len()) as u16 + 2;
        data.extend_from_slice(&param_len1.to_be_bytes());
        data.extend_from_slice(ICC_SIG);
        data.push(1); // sequence number
        data.push(2); // total count
        data.extend_from_slice(part1);

        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        let icc = extract_icc_profile(&data, &scanner.segments);
        // Segments are sorted by sequence number, so part1 comes first.
        let mut expected = Vec::new();
        expected.extend_from_slice(part1);
        expected.extend_from_slice(part2);
        assert_eq!(icc, expected);
    }

    #[test]
    fn test_extract_icc_profile_rejects_invalid_multipart_sets() {
        for data in [
            build_icc_jpeg(&[(1, 0, b"part")]),
            build_icc_jpeg(&[(0, 1, b"part")]),
            build_icc_jpeg(&[(2, 1, b"part")]),
            build_icc_jpeg(&[(1, 2, b"one"), (1, 2, b"duplicate")]),
            build_icc_jpeg(&[(1, 2, b"one"), (2, 3, b"two")]),
            build_icc_jpeg(&[(1, 2, b"one")]),
        ] {
            let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
            scanner.on_bytes(&data);

            assert!(extract_icc_profile(&data, &scanner.segments).is_empty());
        }
    }
}
