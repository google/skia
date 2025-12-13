// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser FFI bindings.
//!
//! Provides C++ bindings for parsing ICC profiles using moxcms. All parsing
//! happens in Rust for memory safety, then validated data is converted to
//! skcms-compatible structures for color transformations.

// No `use moxcms::...` nor `use ffi::...` because we want the code to explicitly
// spell out if it means types from the moxcms crate vs types from the ffi module.

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    /// Color space types (matches skcms_Signature enum values).
    #[repr(u32)]
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum skcms_Signature {
        skcms_Signature_CMYK = 0x434D594B,
        skcms_Signature_Gray = 0x47524159,
        skcms_Signature_RGB = 0x52474220,
        skcms_Signature_Lab = 0x4C616220,
        skcms_Signature_XYZ = 0x58595A20,
        skcms_Signature_CIELUV = 0x4C757620,
        skcms_Signature_YCbCr = 0x59436272,
        skcms_Signature_CIEYxy = 0x59787920,
        skcms_Signature_HSV = 0x48535620,
        skcms_Signature_HLS = 0x484C5320,
        skcms_Signature_CMY = 0x434D5920,
        skcms_Signature_2CLR = 0x32434C52,
        skcms_Signature_3CLR = 0x33434C52,
        skcms_Signature_4CLR = 0x34434C52,
        skcms_Signature_5CLR = 0x35434C52,
        skcms_Signature_6CLR = 0x36434C52,
        skcms_Signature_7CLR = 0x37434C52,
        skcms_Signature_8CLR = 0x38434C52,
        skcms_Signature_9CLR = 0x39434C52,
        skcms_Signature_10CLR = 0x41434C52,
        skcms_Signature_11CLR = 0x42434C52,
        skcms_Signature_12CLR = 0x43434C52,
        skcms_Signature_13CLR = 0x44434C52,
        skcms_Signature_14CLR = 0x45434C52,
        skcms_Signature_15CLR = 0x46434C52,
    }

    // Extern enum definition to assure that CXX will generate static
    // assertions to verify that the enum values match between Rust and C++.
    extern "C++" {
        include!("modules/skcms/skcms.h");

        #[namespace = ""]
        type skcms_Signature;
    }

    // The types below re-define C structs defined in `skcms_public.h`.
    // The type definitions below have to be manually kept in-sync with the ones
    // in `skcms_public.h`.  Some guardrails exist, but they are not 100% accurate:
    //
    // * `static_assert`s in `rust/icc/FFI.cpp` verify that the 2 types are
    //   `std::is_layout_compatible` (this won't catch if the order of two
    //   field definitions is swapped for fields of the same type)
    // * Tests in `tests/RustIccTest.cpp` verify round-tripping
    //   (this should catch issues with order of *known* fields)
    //
    // TODO(https://crbug.com/462751628): If all Skia clients using Rust codecs
    // support `bindgen`, then it may be possible to avoid duplicating / redefining
    // the types below.

    /// 3x3 matrix for color space transforms (matches skcms_Matrix3x3).
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }

    /// Transfer function parameters (matches skcms_TransferFunction).
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

    /// CICP color metadata (matches skcms_CICP).
    #[derive(Clone, Copy)]
    struct Cicp {
        color_primaries: u8,
        transfer_characteristics: u8,
        matrix_coefficients: u8,
        video_full_range_flag: u8,
    }

    /// Curve for LUT transforms - parametric or table-based (matches skcms_Curve).
    struct Curve {
        table_entries: u32, // 0 = parametric, >0 = table
        parametric: TransferFunction,
        table_data: Vec<u8>, // u16 values as bytes (little-endian) for tables
    }

    /// Device-to-PCS transform (matches skcms_A2B).
    struct A2B {
        input_curves: Vec<Curve>,
        input_channels: u32,
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        is_16bit_grid: bool,
        matrix_curves: Vec<Curve>,
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_channels: u32,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    /// PCS-to-device transform (matches skcms_B2A).
    struct B2A {
        /// Required: 3 1D "B" curves. Always present.
        input_curves: Vec<Curve>,
        input_channels: u32,

        /// Optional: 3x4 matrix followed by 3 1D "M" curves.
        /// If matrix_channels == 0, matrix and curves are skipped.
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_curves: Vec<Curve>,
        matrix_channels: u32,

        /// Optional: N-D CLUT followed by N 1D "A" curves.
        /// If output_channels == 0, CLUT and curves are skipped.
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        is_16bit_grid: bool,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    /// Parsed ICC profile data.
    struct IccProfile {
        data_color_space: skcms_Signature,
        connection_space: skcms_Signature,
        to_xyzd50: Matrix3x3,
        has_to_xyzd50: bool,
        trc: [TransferFunction; 3],
        has_trc: bool,
        cicp: Cicp,
        has_cicp: bool,
        a2b: A2B,
        has_a2b: bool,
        b2a: B2A,
        has_b2a: bool,
    }

    // C++ wrapper for skcms_ApproximateCurve.
    // Using cxx::bridge for compile-time type safety.
    // The C++ side (FFI.cpp) wraps skcms_ApproximateCurve, so any signature changes
    // in skcms will cause a compile error rather than silent ABI breakage.
    unsafe extern "C++" {
        include!("rust/icc/FFI.h");

        #[cxx_name = "ApproximateCurveWrapper"]
        fn approximate_curve_wrapper(
            table: &[u16],
            out_approx: &mut TransferFunction,
            out_max_error: &mut f32,
        ) -> bool;
    }

    extern "Rust" {
        /// Parses ICC profile from `data`. If successful, returns `true`
        /// and writes result to `out`. If failure, returns `false`.
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}

/// Parses ICC profile from `data`. If successful, returns `true`
/// and writes result to `out`. If failure, returns `false`.
pub fn parse_icc_profile(data: &[u8], out: &mut ffi::IccProfile) -> bool {
    // Parse with moxcms (it validates size, signature, etc.)
    let Ok(profile) = moxcms::ColorProfile::new_from_slice(data) else {
        return false;
    };

    // Convert moxcms DataColorSpace to our skcms_Signature enum
    // NOTE: This match is intentionally exhaustive (no catch-all pattern).
    // If moxcms adds new DataColorSpace variants, this will fail to compile,
    // forcing us to consciously decide how to map the new variant.
    out.data_color_space = match profile.color_space {
        moxcms::DataColorSpace::Xyz => ffi::skcms_Signature::skcms_Signature_XYZ,
        moxcms::DataColorSpace::Lab => ffi::skcms_Signature::skcms_Signature_Lab,
        moxcms::DataColorSpace::Luv => ffi::skcms_Signature::skcms_Signature_CIELUV,
        moxcms::DataColorSpace::YCbr => ffi::skcms_Signature::skcms_Signature_YCbCr,
        moxcms::DataColorSpace::Yxy => ffi::skcms_Signature::skcms_Signature_CIEYxy,
        moxcms::DataColorSpace::Rgb => ffi::skcms_Signature::skcms_Signature_RGB,
        moxcms::DataColorSpace::Gray => ffi::skcms_Signature::skcms_Signature_Gray,
        moxcms::DataColorSpace::Hsv => ffi::skcms_Signature::skcms_Signature_HSV,
        moxcms::DataColorSpace::Hls => ffi::skcms_Signature::skcms_Signature_HLS,
        moxcms::DataColorSpace::Cmyk => ffi::skcms_Signature::skcms_Signature_CMYK,
        moxcms::DataColorSpace::Cmy => ffi::skcms_Signature::skcms_Signature_CMY,
        // Multi-channel color spaces recognized by skcms
        moxcms::DataColorSpace::Color2 => ffi::skcms_Signature::skcms_Signature_2CLR,
        moxcms::DataColorSpace::Color3 => ffi::skcms_Signature::skcms_Signature_3CLR,
        moxcms::DataColorSpace::Color4 => ffi::skcms_Signature::skcms_Signature_4CLR,
        moxcms::DataColorSpace::Color5 => ffi::skcms_Signature::skcms_Signature_5CLR,
        moxcms::DataColorSpace::Color6 => ffi::skcms_Signature::skcms_Signature_6CLR,
        moxcms::DataColorSpace::Color7 => ffi::skcms_Signature::skcms_Signature_7CLR,
        moxcms::DataColorSpace::Color8 => ffi::skcms_Signature::skcms_Signature_8CLR,
        moxcms::DataColorSpace::Color9 => ffi::skcms_Signature::skcms_Signature_9CLR,
        moxcms::DataColorSpace::Color10 => ffi::skcms_Signature::skcms_Signature_10CLR,
        moxcms::DataColorSpace::Color11 => ffi::skcms_Signature::skcms_Signature_11CLR,
        moxcms::DataColorSpace::Color12 => ffi::skcms_Signature::skcms_Signature_12CLR,
        moxcms::DataColorSpace::Color13 => ffi::skcms_Signature::skcms_Signature_13CLR,
        moxcms::DataColorSpace::Color14 => ffi::skcms_Signature::skcms_Signature_14CLR,
        moxcms::DataColorSpace::Color15 => ffi::skcms_Signature::skcms_Signature_15CLR,
    };

    // Profile Connection Space (PCS) must be XYZ or Lab per ICC spec.
    // skcms only supports these two PCS values. Reject profiles with other values
    // to match skcms_ParseWithA2BPriority behavior.
    out.connection_space = match profile.pcs {
        moxcms::DataColorSpace::Xyz => ffi::skcms_Signature::skcms_Signature_XYZ,
        moxcms::DataColorSpace::Lab => ffi::skcms_Signature::skcms_Signature_Lab,
        _ => return false, // Reject unsupported PCS
    };

    let matrix = profile.colorant_matrix();
    out.to_xyzd50 = matrix3d_to_ffi(&matrix);
    // Validate colorant matrix: each column must have at least one non-zero value.
    // Each column represents a colorant's contribution to XYZ (red, green, blue).
    // An all-zero column would mean that colorant contributes nothing, which is invalid.
    // moxcms returns a zero matrix when colorant tags (rXYZ, gXYZ, bXYZ) are missing.
    let red_valid = matrix.v[0][0] != 0.0 || matrix.v[1][0] != 0.0 || matrix.v[2][0] != 0.0;
    let green_valid = matrix.v[0][1] != 0.0 || matrix.v[1][1] != 0.0 || matrix.v[2][1] != 0.0;
    let blue_valid = matrix.v[0][2] != 0.0 || matrix.v[1][2] != 0.0 || matrix.v[2][2] != 0.0;
    out.has_to_xyzd50 = red_valid && green_valid && blue_valid;

    out.has_trc = false;
    if let Some(gray_trc) = &profile.gray_trc {
        if let Some(tf) = convert_trc_to_transfer_function(gray_trc) {
            out.trc = [tf, tf, tf];
            out.has_trc = true;
        }
    } else if let (Some(r_trc), Some(g_trc), Some(b_trc)) =
        (&profile.red_trc, &profile.green_trc, &profile.blue_trc)
    {
        if let (Some(r_tf), Some(g_tf), Some(b_tf)) = (
            convert_trc_to_transfer_function(r_trc),
            convert_trc_to_transfer_function(g_trc),
            convert_trc_to_transfer_function(b_trc),
        ) {
            out.trc = [r_tf, g_tf, b_tf];
            out.has_trc = true;
        }
    }

    out.has_cicp = false;
    if let Some(cicp) = &profile.cicp {
        out.cicp = ffi::Cicp {
            color_primaries: cicp.color_primaries as u8,
            transfer_characteristics: cicp.transfer_characteristics as u8,
            matrix_coefficients: cicp.matrix_coefficients as u8,
            video_full_range_flag: if cicp.full_range { 1 } else { 0 },
        };
        out.has_cicp = true;
    }

    // Extract A2B transform (device-to-PCS with LUTs)
    // Prefer perceptual, then colorimetric, then saturation
    out.has_a2b = false;
    if let Some(ref lut) = profile
        .lut_a_to_b_perceptual
        .as_ref()
        .or(profile.lut_a_to_b_colorimetric.as_ref())
        .or(profile.lut_a_to_b_saturation.as_ref())
    {
        if let Some(a2b) = convert_to_a2b(lut) {
            out.a2b = a2b;
            out.has_a2b = true;
        }
    }

    out.has_b2a = false;
    if let Some(ref lut) = profile
        .lut_b_to_a_perceptual
        .as_ref()
        .or(profile.lut_b_to_a_colorimetric.as_ref())
        .or(profile.lut_b_to_a_saturation.as_ref())
    {
        if let Some(a2b_data) = convert_to_a2b(lut) {
            out.b2a = a2b_to_b2a(a2b_data);
            out.has_b2a = true;
        }
    }

    true
}

/// Convert Vec<u16> to Vec<u8> in little-endian byte order.
fn u16_vec_to_bytes(values: &[u16]) -> Vec<u8> {
    let mut bytes = Vec::with_capacity(values.len() * 2);
    for value in values {
        bytes.extend(value.to_le_bytes());
    }
    bytes
}

/// Convert moxcms Matrix3d to FFI Matrix3x3.
fn matrix3d_to_ffi(matrix: &moxcms::Matrix3d) -> ffi::Matrix3x3 {
    ffi::Matrix3x3 {
        vals: [
            [
                matrix.v[0][0] as f32,
                matrix.v[0][1] as f32,
                matrix.v[0][2] as f32,
            ],
            [
                matrix.v[1][0] as f32,
                matrix.v[1][1] as f32,
                matrix.v[1][2] as f32,
            ],
            [
                matrix.v[2][0] as f32,
                matrix.v[2][1] as f32,
                matrix.v[2][2] as f32,
            ],
        ],
    }
}

/// Convert LutStore to Vec<u16>, scaling 8-bit values to 16-bit range if needed.
fn lut_store_to_u16(store: &moxcms::LutStore) -> Vec<u16> {
    match store {
        moxcms::LutStore::Store8(data) => {
            // Convert u8 to u16 (scale up to 16-bit range)
            data.iter().map(|&v| (v as u16) << 8).collect()
        }
        moxcms::LutStore::Store16(data) => data.clone(),
    }
}

/// Split a flat table into per-channel curves.
/// Returns empty Vec if table is too small for the expected layout.
fn split_table_to_curves(
    table_data: &[u16],
    entries_per_channel: usize,
    num_channels: usize,
) -> Vec<ffi::Curve> {
    if table_data.is_empty() || table_data.len() < entries_per_channel * num_channels {
        return Vec::new();
    }

    (0..num_channels)
        .map(|ch| {
            let start = ch * entries_per_channel;
            let end = start + entries_per_channel;
            let channel_table = &table_data[start..end];

            ffi::Curve {
                table_entries: entries_per_channel as u32,
                parametric: ffi::TransferFunction::default(),
                table_data: u16_vec_to_bytes(channel_table),
            }
        })
        .collect()
}

/// Convert A2B structure to B2A by reversing the transform direction.
/// B2A is the inverse of A2B: output curves become input, input curves become output.
fn a2b_to_b2a(a2b: ffi::A2B) -> ffi::B2A {
    ffi::B2A {
        input_curves: a2b.output_curves,
        input_channels: a2b.output_channels,
        matrix: a2b.matrix,
        matrix_bias: a2b.matrix_bias,
        matrix_curves: a2b.matrix_curves,
        matrix_channels: a2b.matrix_channels,
        grid_points: a2b.grid_points,
        grid_data: a2b.grid_data,
        is_16bit_grid: a2b.is_16bit_grid,
        output_curves: a2b.input_curves,
        output_channels: a2b.input_channels,
    }
}

/// Convert moxcms parametric curve parameters to TransferFunction.
/// Returns None if there are fewer than 7 parameters.
fn params_to_transfer_function(params: &[f32]) -> Option<ffi::TransferFunction> {
    if params.len() >= 7 {
        Some(ffi::TransferFunction {
            g: params[0],
            a: params[1],
            b: params[2],
            c: params[3],
            d: params[4],
            e: params[5],
            f: params[6],
        })
    } else {
        None
    }
}

/// Convert moxcms ToneReprCurve to FFI Curve structure.
/// Supports both parametric and table-based curves.
/// Returns None if the curve is empty or invalid.
fn convert_to_curve(trc: &moxcms::ToneReprCurve) -> Option<ffi::Curve> {
    use moxcms::ToneReprCurve;

    match trc {
        ToneReprCurve::Parametric(params) => {
            params_to_transfer_function(params).map(|tf| ffi::Curve {
                table_entries: 0, // 0 indicates parametric
                parametric: tf,
                table_data: Vec::new(),
            })
        }
        ToneReprCurve::Lut(table) => {
            if table.is_empty() {
                return None;
            }

            Some(ffi::Curve {
                table_entries: table.len() as u32,
                parametric: ffi::TransferFunction::default(),
                table_data: u16_vec_to_bytes(table),
            })
        }
    }
}

/// Convert moxcms LutWarehouse to A2B structure.
/// Returns None if the LUT cannot be converted.
fn convert_to_a2b(lut: &moxcms::LutWarehouse) -> Option<ffi::A2B> {
    use moxcms::{LutStore, LutWarehouse};

    match lut {
        LutWarehouse::Multidimensional(mdt) => {
            // Convert input curves (A curves)
            let input_curves: Vec<ffi::Curve> = mdt
                .a_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();

            // Grid data
            let (grid_data, is_16bit_grid) = if let Some(ref clut) = mdt.clut {
                match clut {
                    LutStore::Store8(data) => (data.clone(), false),
                    LutStore::Store16(data) => (u16_vec_to_bytes(data), true),
                }
            } else {
                (Vec::new(), false)
            };

            // Matrix curves (M curves)
            let matrix_curves: Vec<ffi::Curve> = mdt
                .m_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();

            // Matrix (3x3 part) and bias
            let matrix = matrix3d_to_ffi(&mdt.matrix);
            let matrix_bias = [
                mdt.bias.v[0] as f32,
                mdt.bias.v[1] as f32,
                mdt.bias.v[2] as f32,
            ];

            // Output curves (B curves)
            let output_curves: Vec<ffi::Curve> = mdt
                .b_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();

            // Copy grid_points (first 4 elements)
            let grid_points: [u8; 4] = mdt.grid_points[..4].try_into().unwrap();

            let matrix_channels = if matrix_curves.is_empty() { 0 } else { 3 };

            Some(ffi::A2B {
                input_curves,
                input_channels: mdt.num_input_channels as u32,
                grid_points,
                grid_data,
                is_16bit_grid,
                matrix_curves,
                matrix,
                matrix_bias,
                matrix_channels,
                output_curves,
                output_channels: mdt.num_output_channels as u32,
            })
        }
        LutWarehouse::Lut(ldt) => {
            // Legacy Lut8Type/Lut16Type (mft1/mft2 tags)
            // Similar structure to Multidimensional, but uses uniform grid size

            // Convert input table curves
            let input_curves: Vec<ffi::Curve> = {
                let curve_data = lut_store_to_u16(&ldt.input_table);
                split_table_to_curves(
                    &curve_data,
                    ldt.num_input_table_entries as usize,
                    ldt.num_input_channels as usize,
                )
            };

            // CLUT grid data
            let (grid_data, is_16bit_grid) = match &ldt.clut_table {
                LutStore::Store8(data) => (data.clone(), false),
                LutStore::Store16(data) => (u16_vec_to_bytes(data), true),
            };

            // Legacy LUT uses uniform grid size for all dimensions
            let grid_size = ldt.num_clut_grid_points;
            let grid_points: [u8; 4] = match ldt.num_input_channels {
                3 => [grid_size, grid_size, grid_size, 0],
                4 => [grid_size, grid_size, grid_size, grid_size],
                _ => [grid_size, 0, 0, 0], // 1D or 2D case
            };

            // Matrix (3x3 part) and bias
            let matrix = matrix3d_to_ffi(&ldt.matrix);
            // Legacy LUT matrix is typically applied post-CLUT, so bias is zero
            let matrix_bias = [0.0, 0.0, 0.0];

            // Convert output table curves
            let output_curves: Vec<ffi::Curve> = {
                let curve_data = lut_store_to_u16(&ldt.output_table);
                split_table_to_curves(
                    &curve_data,
                    ldt.num_output_table_entries as usize,
                    ldt.num_output_channels as usize,
                )
            };

            // Legacy LUT doesn't have separate matrix curves (M curves)
            let matrix_curves: Vec<ffi::Curve> = Vec::new();
            let matrix_channels = 0;

            Some(ffi::A2B {
                input_curves,
                input_channels: ldt.num_input_channels as u32,
                grid_points,
                grid_data,
                is_16bit_grid,
                matrix_curves,
                matrix,
                matrix_bias,
                matrix_channels,
                output_curves,
                output_channels: ldt.num_output_channels as u32,
            })
        }
    }
}

/// Convert moxcms ToneReprCurve to skcms-compatible TransferFunction.
/// Returns None if the curve cannot be represented as a parametric function.
fn convert_trc_to_transfer_function(trc: &moxcms::ToneReprCurve) -> Option<ffi::TransferFunction> {
    use moxcms::ToneReprCurve;

    match trc {
        ToneReprCurve::Parametric(params) => {
            // moxcms parametric curve: Vec<f32> with 7 parameters (g, a, b, c, d, e, f)
            // Matches skcms transfer function layout
            params_to_transfer_function(params)
        }
        ToneReprCurve::Lut(table) => {
            // Handle simple gamma curves (single-entry LUT)
            // In ICC, a single u16 value represents gamma in 8.8 fixed-point
            if table.len() == 1 {
                let gamma = table[0] as f32 / 256.0;
                // Simple power curve: f(x) = x^gamma
                Some(ffi::TransferFunction {
                    g: gamma,
                    a: 1.0,
                    b: 0.0,
                    c: 0.0,
                    d: 0.0,
                    e: 0.0,
                    f: 0.0,
                })
            } else if !table.is_empty() {
                // Multi-entry table: use skcms_ApproximateCurve to fit a parametric function
                let mut approx = ffi::TransferFunction::default();
                let mut max_error: f32 = 0.0;

                if ffi::approximate_curve_wrapper(table, &mut approx, &mut max_error) {
                    Some(approx)
                } else {
                    None
                }
            } else {
                None
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Helper to create an empty IccProfile for testing
    fn empty_icc_profile() -> ffi::IccProfile {
        ffi::IccProfile {
            data_color_space: ffi::skcms_Signature::skcms_Signature_RGB,
            connection_space: ffi::skcms_Signature::skcms_Signature_XYZ,
            to_xyzd50: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            has_to_xyzd50: false,
            trc: [ffi::TransferFunction::default(); 3],
            has_trc: false,
            cicp: ffi::Cicp {
                color_primaries: 0,
                transfer_characteristics: 0,
                matrix_coefficients: 0,
                video_full_range_flag: 0,
            },
            has_cicp: false,
            a2b: empty_a2b(),
            has_a2b: false,
            b2a: empty_b2a(),
            has_b2a: false,
        }
    }

    fn empty_a2b() -> ffi::A2B {
        ffi::A2B {
            input_curves: Vec::new(),
            input_channels: 0,
            grid_points: [0; 4],
            grid_data: Vec::new(),
            is_16bit_grid: false,
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

    fn empty_b2a() -> ffi::B2A {
        ffi::B2A {
            input_curves: Vec::new(),
            input_channels: 0,
            matrix: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            matrix_bias: [0.0; 3],
            matrix_curves: Vec::new(),
            matrix_channels: 0,
            grid_points: [0; 4],
            grid_data: Vec::new(),
            is_16bit_grid: false,
            output_curves: Vec::new(),
            output_channels: 0,
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
        let mut data = vec![0u8; 128];
        // Put wrong signature at offset 36
        data[36..40].copy_from_slice(b"badd");

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }

    #[test]
    fn test_valid_signature_invalid_profile() {
        let mut data = vec![0u8; 128];
        // Put correct ICC signature "acsp" at offset 36
        data[36..40].copy_from_slice(b"acsp");

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        // This will fail moxcms parsing since it's not a real ICC profile
        assert!(!result);
    }

    #[test]
    fn test_valid_profile() {
        // Sample from AdobeRGB1998.icc - a real 560-byte ICC profile
        let data: [u8; 560] = [
            0x00, 0x00, 0x02, 0x30, 0x41, 0x44, 0x42, 0x45, 0x02, 0x10, 0x00, 0x00, 0x6d, 0x6e,
            0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20, 0x07, 0xd0, 0x00, 0x08,
            0x00, 0x0b, 0x00, 0x13, 0x00, 0x33, 0x00, 0x3b, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50,
            0x50, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6f, 0x6e, 0x65, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 0x41, 0x44, 0x42, 0x45,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x00, 0xfc,
            0x00, 0x00, 0x00, 0x32, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00,
            0x00, 0x6b, 0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x14,
            0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x14, 0x72, 0x54,
            0x52, 0x43, 0x00, 0x00, 0x01, 0xc4, 0x00, 0x00, 0x00, 0x0e, 0x67, 0x54, 0x52, 0x43,
            0x00, 0x00, 0x01, 0xd4, 0x00, 0x00, 0x00, 0x0e, 0x62, 0x54, 0x52, 0x43, 0x00, 0x00,
            0x01, 0xe4, 0x00, 0x00, 0x00, 0x0e, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xf4,
            0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
            0x00, 0x14, 0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x14,
            0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69,
            0x67, 0x68, 0x74, 0x20, 0x32, 0x30, 0x30, 0x30, 0x20, 0x41, 0x64, 0x6f, 0x62, 0x65,
            0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73, 0x20, 0x49, 0x6e, 0x63, 0x6f, 0x72,
            0x70, 0x6f, 0x72, 0x61, 0x74, 0x65, 0x64, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x41, 0x64, 0x6f, 0x62, 0x65, 0x20,
            0x52, 0x47, 0x42, 0x20, 0x28, 0x31, 0x39, 0x39, 0x38, 0x29, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x16, 0xcc, 0x58, 0x59,
            0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x18, 0x00, 0x00, 0x4f, 0xa5, 0x00, 0x00,
            0x04, 0xfc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x8d,
            0x00, 0x00, 0xa0, 0x2c, 0x00, 0x00, 0x0f, 0x95, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x26, 0x31, 0x00, 0x00, 0x10, 0x2f, 0x00, 0x00, 0xbe, 0x9c,
        ];

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(result, "Failed to parse valid ICC profile");
        assert_eq!(
            out.data_color_space,
            ffi::skcms_Signature::skcms_Signature_RGB
        );
        assert_eq!(
            out.connection_space,
            ffi::skcms_Signature::skcms_Signature_XYZ
        );

        // AdobeRGB has toXYZD50 matrix and TRCs
        assert!(out.has_to_xyzd50, "AdobeRGB should have toXYZD50 matrix");
        assert!(out.has_trc, "AdobeRGB should have TRCs");

        // Verify the TRCs are gamma 2.2 (encoded as 563 in 8.8 fixed point = 2.199...)
        assert!(
            (out.trc[0].g - 2.2).abs() < 0.01,
            "Red TRC should be ~2.2, got {}",
            out.trc[0].g
        );
    }

    #[test]
    fn test_approximate_curve_linear() {
        // Test approximating a linear curve (identity function)
        // Values from 0 to 65535 in steps
        let table: Vec<u16> = (0..=255).map(|i| (i as u16) * 257).collect();

        let mut approx = ffi::TransferFunction::default();
        let mut max_error: f32 = 0.0;

        // For testing, use the C++ wrapper directly
        let success = ffi::approximate_curve_wrapper(&table, &mut approx, &mut max_error);
        assert!(success, "Should successfully approximate linear curve");

        // Linear curve should have g ≈ 1.0, a ≈ 1.0, b ≈ 0.0
        assert!(
            (approx.g - 1.0).abs() < 0.1,
            "Linear curve gamma should be ~1.0, got {}",
            approx.g
        );
        assert!(max_error < 0.01, "Linear curve error should be very small");
    }

    #[test]
    fn test_approximate_curve_gamma() {
        // Test approximating a gamma 2.2 curve
        let table: Vec<u16> = (0..=255)
            .map(|i| {
                let x = i as f32 / 255.0;
                let y = x.powf(2.2);
                (y * 65535.0) as u16
            })
            .collect();

        let mut approx = ffi::TransferFunction::default();
        let mut max_error: f32 = 0.0;

        // For testing, use the C++ wrapper directly
        let success = ffi::approximate_curve_wrapper(&table, &mut approx, &mut max_error);
        assert!(success, "Should successfully approximate gamma curve");

        // skcms_ApproximateCurve may not fit gamma curves perfectly
        // Allow a reasonable tolerance since approximation trades off accuracy
        assert!(
            (approx.g - 2.2).abs() < 0.5,
            "Gamma curve should be roughly ~2.2, got {}",
            approx.g
        );
    }

    #[test]
    fn test_convert_legacy_lut_basic() {
        use moxcms::{LutDataType, LutStore, LutType, LutWarehouse, Matrix3d};

        // Create a simple legacy LUT with 3 input channels, 3 output channels
        let ldt = LutDataType {
            num_input_channels: 3,
            num_output_channels: 3,
            num_clut_grid_points: 2, // 2x2x2 grid
            matrix: Matrix3d {
                v: [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
            },
            num_input_table_entries: 256,
            num_output_table_entries: 256,
            // Create identity input tables (3 channels * 256 entries)
            input_table: LutStore::Store16(
                (0..3)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            // Create simple CLUT (2^3 * 3 = 24 values)
            clut_table: LutStore::Store16(vec![
                0, 0, 0, // [0,0,0] -> [0,0,0]
                65535, 0, 0, // [1,0,0] -> [1,0,0]
                0, 65535, 0, // [0,1,0] -> [0,1,0]
                65535, 65535, 0, // [1,1,0] -> [1,1,0]
                0, 0, 65535, // [0,0,1] -> [0,0,1]
                65535, 0, 65535, // [1,0,1] -> [1,0,1]
                0, 65535, 65535, // [0,1,1] -> [0,1,1]
                65535, 65535, 65535, // [1,1,1] -> [1,1,1]
            ]),
            // Create identity output tables (3 channels * 256 entries)
            output_table: LutStore::Store16(
                (0..3)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            lut_type: LutType::Lut16,
        };

        let lut_warehouse = LutWarehouse::Lut(ldt);
        let result = convert_to_a2b(&lut_warehouse);

        assert!(result.is_some(), "Should successfully convert legacy LUT");
        let a2b = result.unwrap();

        // Verify channel counts
        assert_eq!(a2b.input_channels, 3);
        assert_eq!(a2b.output_channels, 3);

        // Verify grid points (uniform 2x2x2 for 3 input channels)
        assert_eq!(a2b.grid_points, [2, 2, 2, 0]);

        // Verify 3 input curves (one per channel)
        assert_eq!(a2b.input_curves.len(), 3);
        assert_eq!(a2b.input_curves[0].table_entries, 256);

        // Verify 3 output curves (one per channel)
        assert_eq!(a2b.output_curves.len(), 3);
        assert_eq!(a2b.output_curves[0].table_entries, 256);

        // Verify no matrix curves (legacy LUT doesn't have M curves)
        assert_eq!(a2b.matrix_curves.len(), 0);
        assert_eq!(a2b.matrix_channels, 0);

        // Verify grid data is present and 16-bit
        assert!(!a2b.grid_data.is_empty());
        assert!(a2b.is_16bit_grid);

        // Verify matrix is identity
        assert_eq!(a2b.matrix.vals[0][0], 1.0);
        assert_eq!(a2b.matrix.vals[1][1], 1.0);
        assert_eq!(a2b.matrix.vals[2][2], 1.0);

        // Verify bias is zero
        assert_eq!(a2b.matrix_bias, [0.0, 0.0, 0.0]);
    }

    #[test]
    fn test_convert_legacy_lut_8bit() {
        use moxcms::{LutDataType, LutStore, LutType, LutWarehouse, Matrix3d};

        // Create legacy 8-bit LUT
        let ldt = LutDataType {
            num_input_channels: 3,
            num_output_channels: 3,
            num_clut_grid_points: 3, // 3x3x3 grid
            matrix: Matrix3d {
                v: [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
            },
            num_input_table_entries: 256,
            num_output_table_entries: 256,
            // Use 8-bit storage
            input_table: LutStore::Store8((0..3).flat_map(|_| (0..256).map(|i| i as u8)).collect()),
            clut_table: LutStore::Store8(vec![128; 3 * 3 * 3 * 3]), // 3^3 entries * 3 channels
            output_table: LutStore::Store8(
                (0..3).flat_map(|_| (0..256).map(|i| i as u8)).collect(),
            ),
            lut_type: LutType::Lut8,
        };

        let lut_warehouse = LutWarehouse::Lut(ldt);
        let result = convert_to_a2b(&lut_warehouse);

        assert!(
            result.is_some(),
            "Should successfully convert 8-bit legacy LUT"
        );
        let a2b = result.unwrap();

        // Verify grid data is 8-bit
        assert!(!a2b.is_16bit_grid);
        assert!(!a2b.grid_data.is_empty());

        // Verify input/output curves converted from 8-bit to 16-bit
        assert_eq!(a2b.input_curves.len(), 3);
        assert_eq!(a2b.output_curves.len(), 3);

        // Check that table data is in 16-bit format (u16 values as bytes)
        // Each curve should have 256 entries * 2 bytes per entry
        assert_eq!(a2b.input_curves[0].table_data.len(), 256 * 2);
    }

    #[test]
    fn test_convert_legacy_lut_4channel() {
        use moxcms::{LutDataType, LutStore, LutType, LutWarehouse, Matrix3d};

        // Create 4-channel (CMYK) legacy LUT
        let ldt = LutDataType {
            num_input_channels: 4,
            num_output_channels: 3,
            num_clut_grid_points: 2, // 2x2x2x2 grid for 4 channels
            matrix: Matrix3d {
                v: [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
            },
            num_input_table_entries: 256,
            num_output_table_entries: 256,
            input_table: LutStore::Store16(
                (0..4)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            // 2^4 * 3 = 48 values
            clut_table: LutStore::Store16(vec![32768; 16 * 3]),
            output_table: LutStore::Store16(
                (0..3)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            lut_type: LutType::Lut16,
        };

        let lut_warehouse = LutWarehouse::Lut(ldt);
        let result = convert_to_a2b(&lut_warehouse);

        assert!(
            result.is_some(),
            "Should successfully convert 4-channel legacy LUT"
        );
        let a2b = result.unwrap();

        // Verify 4 input channels
        assert_eq!(a2b.input_channels, 4);
        assert_eq!(a2b.input_curves.len(), 4);

        // Verify grid points for 4D CLUT
        assert_eq!(a2b.grid_points, [2, 2, 2, 2]);

        // Verify 3 output channels (to XYZ/Lab)
        assert_eq!(a2b.output_channels, 3);
        assert_eq!(a2b.output_curves.len(), 3);
    }

    #[test]
    fn test_convert_legacy_lut_empty_tables() {
        use moxcms::{LutDataType, LutStore, LutType, LutWarehouse, Matrix3d};

        // Create LUT with empty input/output tables
        let ldt = LutDataType {
            num_input_channels: 3,
            num_output_channels: 3,
            num_clut_grid_points: 2,
            matrix: Matrix3d {
                v: [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
            },
            num_input_table_entries: 256,
            num_output_table_entries: 256,
            input_table: LutStore::Store16(Vec::new()), // Empty
            clut_table: LutStore::Store16(vec![0; 8 * 3]),
            output_table: LutStore::Store16(Vec::new()), // Empty
            lut_type: LutType::Lut16,
        };

        let lut_warehouse = LutWarehouse::Lut(ldt);
        let result = convert_to_a2b(&lut_warehouse);

        assert!(result.is_some(), "Should handle empty tables gracefully");
        let a2b = result.unwrap();

        // Should have empty curve vectors
        assert_eq!(a2b.input_curves.len(), 0);
        assert_eq!(a2b.output_curves.len(), 0);

        // But still valid channel counts and grid
        assert_eq!(a2b.input_channels, 3);
        assert_eq!(a2b.output_channels, 3);
        assert_eq!(a2b.grid_points, [2, 2, 2, 0]);
    }

    #[test]
    fn test_convert_legacy_lut_non_identity_matrix() {
        use moxcms::{LutDataType, LutStore, LutType, LutWarehouse, Matrix3d};

        // Create LUT with non-identity matrix
        let ldt = LutDataType {
            num_input_channels: 3,
            num_output_channels: 3,
            num_clut_grid_points: 2,
            matrix: Matrix3d {
                v: [
                    [0.4124, 0.3576, 0.1805],
                    [0.2126, 0.7152, 0.0722],
                    [0.0193, 0.1192, 0.9505],
                ],
            },
            num_input_table_entries: 256,
            num_output_table_entries: 256,
            input_table: LutStore::Store16(
                (0..3)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            clut_table: LutStore::Store16(vec![32768; 8 * 3]),
            output_table: LutStore::Store16(
                (0..3)
                    .flat_map(|_| (0..256).map(|i| (i as u16) * 257))
                    .collect(),
            ),
            lut_type: LutType::Lut16,
        };

        let lut_warehouse = LutWarehouse::Lut(ldt);
        let result = convert_to_a2b(&lut_warehouse);

        assert!(result.is_some(), "Should convert LUT with custom matrix");
        let a2b = result.unwrap();

        // Verify matrix values (sRGB to XYZ-like)
        assert!((a2b.matrix.vals[0][0] - 0.4124).abs() < 0.001);
        assert!((a2b.matrix.vals[1][1] - 0.7152).abs() < 0.001);
        assert!((a2b.matrix.vals[2][2] - 0.9505).abs() < 0.001);

        // Bias should still be zero for legacy LUT
        assert_eq!(a2b.matrix_bias, [0.0, 0.0, 0.0]);
    }

    #[test]
    fn test_u16_vec_to_bytes() {
        // Test the helper function for little-endian conversion
        let values: Vec<u16> = vec![0x0000, 0x00FF, 0xFF00, 0xFFFF];
        let bytes = u16_vec_to_bytes(&values);

        assert_eq!(bytes.len(), 8); // 4 values * 2 bytes each

        // Verify little-endian encoding
        assert_eq!(bytes[0..2], [0x00, 0x00]); // 0x0000
        assert_eq!(bytes[2..4], [0xFF, 0x00]); // 0x00FF
        assert_eq!(bytes[4..6], [0x00, 0xFF]); // 0xFF00
        assert_eq!(bytes[6..8], [0xFF, 0xFF]); // 0xFFFF
    }

    #[test]
    fn test_params_to_transfer_function() {
        // Test with valid 7 parameters
        let params = vec![2.2, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0];
        let result = params_to_transfer_function(&params);
        assert!(result.is_some());
        let tf = result.unwrap();
        assert_eq!(tf.g, 2.2);
        assert_eq!(tf.a, 1.0);

        // Test with too few parameters
        let params_short = vec![2.2, 1.0];
        let result_short = params_to_transfer_function(&params_short);
        assert!(result_short.is_none());

        // Test with more than 7 parameters (should still work)
        let params_long = vec![2.2, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 99.0, 99.0];
        let result_long = params_to_transfer_function(&params_long);
        assert!(result_long.is_some());
    }
}
