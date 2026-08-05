// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Incremental JPEG segment scanner (Rust port of `SkJpegSegmentScanner`).

/// JPEG marker constants matching `src/codec/SkJpegConstants.h`.
pub mod jpeg_marker {
    /// Start of Image.
    pub const SOI: u8 = 0xD8;
    /// End of Image.
    #[allow(dead_code)]
    pub const EOI: u8 = 0xD9;
    /// Start of Scan — marks the beginning of entropy-coded data.
    pub const SOS: u8 = 0xDA;
    /// APP0 marker base; APPn = APP0 + n.
    pub const APP0: u8 = 0xE0;
    /// Marker prefix byte (all JPEG markers begin with 0xFF).
    pub const PREFIX: u8 = 0xFF;

    /// Returns true if this marker "stands alone" (has no parameter payload).
    /// These are TEM (0x01), RST0–RST7 (0xD0–0xD7), SOI (0xD8), EOI (0xD9).
    pub fn stands_alone(marker: u8) -> bool {
        marker == 0x01 || (marker >= 0xD0 && marker <= 0xD9)
    }
}

/// A scanned JPEG segment: marker byte, offset, and parameter length.
#[derive(Clone, Debug)]
pub struct ScannedSegment {
    /// Second byte of the marker code (e.g. 0xE1 for APP1).
    pub marker: u8,
    /// Offset in bytes from start of data to the marker's 0xFF byte.
    pub offset: u32,
    /// Length of the segment parameters (including the 2-byte length field).
    pub parameter_length: u16,
}

/// State machine for incrementally scanning JPEG segment structure.
pub struct SegmentScanner {
    state: ScanState,
    /// Marker byte at which to stop scanning.
    stop_marker: u8,
    /// Number of bytes processed so far.
    offset: usize,
    /// Accumulated segments.
    pub segments: Vec<ScannedSegment>,

    // In-progress segment tracking.
    current_marker: u8,
    current_offset: usize,
    param_length_byte0: u8,
    param_bytes_remaining: usize,
}

#[derive(Clone, Copy, Debug, PartialEq)]
enum ScanState {
    StartOfImageByte0,
    StartOfImageByte1,
    SecondMarkerByte0,
    SecondMarkerByte1,
    SegmentParamLengthByte0,
    SegmentParamLengthByte1,
    SegmentParam,
    EntropyCodedData,
    EntropyCodedDataSentinel,
    PostEntropyCodedDataFill,
    Done,
    Error,
}

impl SegmentScanner {
    pub fn new(stop_marker: u8) -> Self {
        SegmentScanner {
            state: ScanState::StartOfImageByte0,
            stop_marker,
            offset: 0,
            segments: Vec::new(),
            current_marker: 0,
            current_offset: 0,
            param_length_byte0: 0,
            param_bytes_remaining: 0,
        }
    }

    pub fn is_done(&self) -> bool {
        self.state == ScanState::Done
    }

    pub fn had_error(&self) -> bool {
        self.state == ScanState::Error
    }

    /// Feed more bytes into the state machine.
    pub fn on_bytes(&mut self, data: &[u8]) {
        let mut pos = 0;
        while pos < data.len() {
            let remaining = data.len() - pos;
            match self.state {
                ScanState::SegmentParam => {
                    let skip = remaining.min(self.param_bytes_remaining);
                    self.param_bytes_remaining -= skip;
                    if self.param_bytes_remaining == 0 {
                        self.state = ScanState::EntropyCodedData;
                    }
                    self.offset += skip;
                    pos += skip;
                }
                ScanState::EntropyCodedData => {
                    if let Some(idx) = memchr(jpeg_marker::PREFIX, &data[pos..]) {
                        let skip = idx + 1;
                        self.offset += skip;
                        pos += skip;
                        self.state = ScanState::EntropyCodedDataSentinel;
                    } else {
                        self.offset += remaining;
                        pos += remaining;
                    }
                }
                ScanState::Done => {
                    self.offset += remaining;
                    pos += remaining;
                }
                _ => {
                    self.on_byte(data[pos]);
                    self.offset += 1;
                    pos += 1;
                }
            }
        }
    }

    fn save_current_segment(&mut self, length: u16) -> bool {
        let Ok(offset) = u32::try_from(self.current_offset) else {
            self.state = ScanState::Error;
            return false;
        };
        self.segments.push(ScannedSegment {
            marker: self.current_marker,
            offset,
            parameter_length: length,
        });
        self.current_marker = 0;
        self.current_offset = 0;
        true
    }

    fn on_marker_second_byte(&mut self, byte: u8) {
        self.current_marker = byte;
        self.current_offset = self.offset - 1;

        if byte == self.stop_marker {
            if !self.save_current_segment(0) {
                return;
            }
            self.state = ScanState::Done;
        } else if byte == jpeg_marker::SOI {
            if !self.save_current_segment(0) {
                return;
            }
            self.state = ScanState::SecondMarkerByte0;
        } else if jpeg_marker::stands_alone(byte) {
            if !self.save_current_segment(0) {
                return;
            }
            self.state = ScanState::EntropyCodedData;
        } else {
            self.state = ScanState::SegmentParamLengthByte0;
        }
    }

    fn on_byte(&mut self, byte: u8) {
        match self.state {
            ScanState::StartOfImageByte0 => {
                if byte != jpeg_marker::PREFIX {
                    self.state = ScanState::Error;
                    return;
                }
                self.state = ScanState::StartOfImageByte1;
            }
            ScanState::StartOfImageByte1 => {
                if byte != jpeg_marker::SOI {
                    self.state = ScanState::Error;
                    return;
                }
                self.on_marker_second_byte(byte);
            }
            ScanState::SecondMarkerByte0 => {
                if byte != jpeg_marker::PREFIX {
                    self.state = ScanState::Error;
                    return;
                }
                self.state = ScanState::SecondMarkerByte1;
            }
            ScanState::SecondMarkerByte1 => {
                if byte == jpeg_marker::PREFIX {
                    return;
                }
                if byte == 0x00 {
                    self.state = ScanState::Error;
                    return;
                }
                self.on_marker_second_byte(byte);
            }
            ScanState::SegmentParamLengthByte0 => {
                self.param_length_byte0 = byte;
                self.state = ScanState::SegmentParamLengthByte1;
            }
            ScanState::SegmentParamLengthByte1 => {
                let param_length = 256u16 * (self.param_length_byte0 as u16) + (byte as u16);
                self.param_length_byte0 = 0;
                if param_length < 2 {
                    self.state = ScanState::Error;
                    return;
                }
                if !self.save_current_segment(param_length) {
                    return;
                }
                self.param_bytes_remaining = (param_length - 2) as usize;
                if self.param_bytes_remaining > 0 {
                    self.state = ScanState::SegmentParam;
                } else {
                    self.state = ScanState::EntropyCodedData;
                }
            }
            ScanState::SegmentParam => {
                self.param_bytes_remaining -= 1;
                if self.param_bytes_remaining == 0 {
                    self.state = ScanState::EntropyCodedData;
                }
            }
            ScanState::EntropyCodedData => {
                if byte == jpeg_marker::PREFIX {
                    self.state = ScanState::EntropyCodedDataSentinel;
                }
            }
            ScanState::EntropyCodedDataSentinel => {
                if byte == 0x00 {
                    self.state = ScanState::EntropyCodedData;
                } else if byte == jpeg_marker::PREFIX {
                    self.state = ScanState::PostEntropyCodedDataFill;
                } else {
                    self.on_marker_second_byte(byte);
                }
            }
            ScanState::PostEntropyCodedDataFill => {
                if byte == jpeg_marker::PREFIX {
                    // Still fill bytes; stay in this state.
                } else {
                    self.on_marker_second_byte(byte);
                }
            }
            ScanState::Done | ScanState::Error => {}
        }
    }
}

fn memchr(needle: u8, haystack: &[u8]) -> Option<usize> {
    haystack.iter().position(|&b| b == needle)
}

/// Get the parameter payload bytes (excluding marker and length field)
/// for a scanned segment.
pub fn get_segment_params<'a>(raw_data: &'a [u8], seg: &ScannedSegment) -> &'a [u8] {
    // Segment layout: [0xFF] [marker] [length_hi] [length_lo] [params...]
    // parameter_length includes the 2-byte length field.
    let Some(param_start) = (seg.offset as usize)
        .checked_add(2 /* marker */)
        .and_then(|offset| offset.checked_add(2 /* length field */))
    else {
        return &[];
    };
    let Some(param_len) = (seg.parameter_length as usize).checked_sub(2) else {
        return &[];
    };
    let Some(end) = param_start.checked_add(param_len) else {
        return &[];
    };
    if param_start <= raw_data.len() && end <= raw_data.len() {
        &raw_data[param_start..end]
    } else {
        &[]
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    /// Build a minimal JPEG byte sequence: SOI + APP0 segment + SOS.
    pub fn make_minimal_jpeg_with_app0() -> Vec<u8> {
        let mut data = Vec::new();
        data.extend_from_slice(&[0xFF, 0xD8]);
        data.extend_from_slice(&[0xFF, 0xE0]); // APP0 marker
        data.extend_from_slice(&[0x00, 0x06]); // param length = 6
        data.extend_from_slice(b"Hi\x00\x00"); // 4 bytes of parameters
        data.extend_from_slice(&[0xFF, 0xDA]); // SOS marker
        data.extend_from_slice(&[0x00, 0x02]); // param length = 2
        data
    }

    #[test]
    fn test_segment_scanner_basic() {
        let data = make_minimal_jpeg_with_app0();
        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        assert!(scanner.is_done());
        assert!(!scanner.had_error());
        assert_eq!(scanner.segments.len(), 3);

        assert_eq!(scanner.segments[0].marker, jpeg_marker::SOI);
        assert_eq!(scanner.segments[0].parameter_length, 0);
        assert_eq!(scanner.segments[1].marker, 0xE0);
        assert_eq!(scanner.segments[1].parameter_length, 6);
        assert_eq!(scanner.segments[2].marker, jpeg_marker::SOS);
    }

    #[test]
    fn test_segment_scanner_incremental() {
        let data = make_minimal_jpeg_with_app0();
        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        for &b in &data {
            scanner.on_bytes(&[b]);
        }
        assert!(scanner.is_done());
        assert_eq!(scanner.segments.len(), 3);
    }

    #[test]
    fn test_segment_scanner_bad_signature() {
        let data = [0x00, 0x01, 0x02];
        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);
        assert!(scanner.had_error());
    }

    #[test]
    fn test_segment_scanner_accepts_fill_bytes_before_marker() {
        let data = [
            0xFF, 0xD8, // SOI
            0xFF, 0xFF, 0xFF, 0xE1, // Fill bytes before APP1 marker
            0x00, 0x02, // param length = 2
            0xFF, 0xDA, // SOS marker
            0x00, 0x02, // param length = 2
        ];
        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        assert!(scanner.is_done());
        assert!(!scanner.had_error());
        assert_eq!(scanner.segments[1].marker, 0xE1);
    }

    #[test]
    fn test_get_segment_params() {
        let data = make_minimal_jpeg_with_app0();
        let mut scanner = SegmentScanner::new(jpeg_marker::SOS);
        scanner.on_bytes(&data);

        let params = get_segment_params(&data, &scanner.segments[1]);
        assert_eq!(params, b"Hi\x00\x00");
    }

    #[test]
    fn test_get_segment_params_rejects_invalid_ranges() {
        let data = make_minimal_jpeg_with_app0();

        assert_eq!(
            get_segment_params(
                &data,
                &ScannedSegment {
                    marker: 0xE0,
                    offset: u32::MAX,
                    parameter_length: 6,
                },
            ),
            b""
        );

        assert_eq!(
            get_segment_params(
                &data,
                &ScannedSegment {
                    marker: 0xE0,
                    offset: 2,
                    parameter_length: 1,
                },
            ),
            b""
        );
    }
}
