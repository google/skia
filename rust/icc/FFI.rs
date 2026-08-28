// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser FFI bindings.
//!
//! Provides C++ bindings for parsing ICC profiles. All parsing happens in
//! Rust for memory safety, populating data structures for use by skcms / Skia codecs.

mod icc_parse;

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    extern "C++" {
        include!("modules/skcms/skcms.h");
        include!("rust/icc/FFI.h");
    }

    /// 3x3 matrix for color space transforms (matches `skcms_Matrix3x3`).
    #[derive(Clone, Copy)]
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }

    /// Transfer function parameters (matches `skcms_TransferFunction`).
    #[derive(Clone, Copy, Default)]
    struct TransferFunction {
        g: f32,
        a: f32,
        b: f32,
        c: f32,
        d: f32,
        e: f32,
        f: f32,
    }

    /// CICP color metadata (matches `skcms_CICP` and `CICP_Layout`).
    #[derive(Clone, Copy)]
    struct Cicp {
        color_primaries: u8,
        transfer_characteristics: u8,
        matrix_coefficients: u8,
        video_full_range_flag: u8,
    }

    /// Table data byte width / element format (matches `table_8` vs `table_16`
    /// and `grid_8` vs `grid_16`).
    enum TableFormat {
        U8,
        U16,
    }

    /// Curve for LUT transforms - parametric or table-based (matches `skcms_Curve`).
    #[derive(Clone)]
    struct Curve {
        table_entries: u32, // 0 = parametric, >0 = table
        parametric: TransferFunction,
        table_data: Vec<u8>, // raw table bytes
        table_format: TableFormat,
    }

    /// Multi-stage device <-> PCS transform (matches `skcms_A2B` and `skcms_B2A`).
    struct A2BOrB2ATransform {
        input_curves: Vec<Curve>,
        input_channels: u32,
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        grid_format: TableFormat,
        matrix_curves: Vec<Curve>,
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_channels: u32,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    /// Parsed ICC profile data (matches `skcms_ICCProfile`).
    struct IccProfile {
        data_color_space: u32,
        connection_space: u32,
        to_xyzd50: Matrix3x3,
        has_to_xyzd50: bool,
        /// Transfer curves for R, G, B channels (or gray replicated x3).
        trc: [Curve; 3],
        has_trc: bool,
        cicp: Cicp,
        has_cicp: bool,
        a2b: A2BOrB2ATransform,
        has_a2b: bool,
        b2a: A2BOrB2ATransform,
        has_b2a: bool,
    }

    extern "Rust" {
        /// Parses ICC profile from `data`. If successful, returns `true`
        /// and writes result to `out`. If failure, returns `false`.
        /// (Matches `skcms_Parse`).
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;

        /// Parses ICC profile from `data` with caller-specified A2B / B2A priority.
        /// (Matches `skcms_ParseWithA2BPriority`).
        fn parse_icc_profile_with_a2b_priority(
            data: &[u8],
            priority: &[i32],
            out: &mut IccProfile,
        ) -> bool;
    }
}

impl ffi::TableFormat {
    #[inline]
    pub fn byte_width(self) -> usize {
        match self {
            ffi::TableFormat::U8 => 1,
            ffi::TableFormat::U16 => 2,
            _ => 1,
        }
    }
}

// =============================================================================
// FFI Interface Implementation
// =============================================================================

/// Parses ICC profile using default priority [0, 1] (Perceptual, then Relative Colorimetric).
/// (Matches `skcms_Parse`).
pub fn parse_icc_profile(data: &[u8], out: &mut ffi::IccProfile) -> bool {
    const DEFAULT_PRIORITY: [i32; 2] = [0, 1];
    parse_icc_profile_with_a2b_priority(data, &DEFAULT_PRIORITY, out)
}

/// Parses ICC profile from `data` with caller-specified A2B / B2A priority.
/// `priority` contains indices in 0..=2:
///   0 = Perceptual (A2B0 / B2A0)
///   1 = Relative Colorimetric (A2B1 / B2A1)
///   2 = Saturation (A2B2 / B2A2)
/// (Matches `skcms_ParseWithA2BPriority`).
pub fn parse_icc_profile_with_a2b_priority(
    data: &[u8],
    priority: &[i32],
    out: &mut ffi::IccProfile,
) -> bool {
    icc_parse::parse_icc_profile(data, priority, out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn empty_curve() -> ffi::Curve {
        ffi::Curve {
            table_entries: 0,
            parametric: ffi::TransferFunction::default(),
            table_data: Vec::new(),
            table_format: ffi::TableFormat::U8,
        }
    }

    fn empty_lut() -> ffi::A2BOrB2ATransform {
        ffi::A2BOrB2ATransform {
            input_curves: Vec::new(),
            input_channels: 0,
            grid_points: [0; 4],
            grid_data: Vec::new(),
            grid_format: ffi::TableFormat::U8,
            matrix_curves: Vec::new(),
            matrix: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            matrix_bias: [0.0; 3],
            matrix_channels: 0,
            output_curves: Vec::new(),
            output_channels: 0,
        }
    }

    fn empty_icc_profile() -> ffi::IccProfile {
        ffi::IccProfile {
            data_color_space: u32::from_be_bytes(*b"RGB "),
            connection_space: u32::from_be_bytes(*b"XYZ "),
            to_xyzd50: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            has_to_xyzd50: false,
            trc: [empty_curve(), empty_curve(), empty_curve()],
            has_trc: false,
            cicp: ffi::Cicp {
                color_primaries: 0,
                transfer_characteristics: 0,
                matrix_coefficients: 0,
                video_full_range_flag: 0,
            },
            has_cicp: false,
            a2b: empty_lut(),
            has_a2b: false,
            b2a: empty_lut(),
            has_b2a: false,
        }
    }

    #[test]
    fn test_empty_profile() {
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&[], &mut out);
        assert!(!result);
    }

    #[test]
    fn test_too_short() {
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&[0; 39], &mut out);
        assert!(!result);
    }

    #[test]
    fn test_invalid_signature() {
        let mut data = vec![0u8; 132];
        data[36..40].copy_from_slice(b"badd");
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }

    #[test]
    fn test_valid_signature_invalid_profile() {
        let mut data = vec![0u8; 132];
        data[36..40].copy_from_slice(b"acsp");
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }
}
