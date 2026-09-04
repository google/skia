// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser implementation mirroring skcms.
//!
//! Provides pure Rust parsing logic and constants for ICC profiles, populating
//! the FFI data structures for consumption across the CXX bridge.

use crate::ffi::{
    A2BOrB2ATransform, Cicp, Curve, IccProfile, Matrix3x3, TableFormat, TransferFunction,
};

// =============================================================================
// ICC signature values that are used internally
// =============================================================================

// File signature
const SKCMS_SIGNATURE_ACSP: u32 = u32::from_be_bytes(*b"acsp");

// Tag signatures
const SKCMS_SIGNATURE_R_TRC: u32 = u32::from_be_bytes(*b"rTRC");
const SKCMS_SIGNATURE_G_TRC: u32 = u32::from_be_bytes(*b"gTRC");
const SKCMS_SIGNATURE_B_TRC: u32 = u32::from_be_bytes(*b"bTRC");
const SKCMS_SIGNATURE_K_TRC: u32 = u32::from_be_bytes(*b"kTRC");

const SKCMS_SIGNATURE_R_XYZ: u32 = u32::from_be_bytes(*b"rXYZ");
const SKCMS_SIGNATURE_G_XYZ: u32 = u32::from_be_bytes(*b"gXYZ");
const SKCMS_SIGNATURE_B_XYZ: u32 = u32::from_be_bytes(*b"bXYZ");

const SKCMS_SIGNATURE_A2B0: u32 = u32::from_be_bytes(*b"A2B0");
const SKCMS_SIGNATURE_B2A0: u32 = u32::from_be_bytes(*b"B2A0");

const SKCMS_SIGNATURE_CICP: u32 = u32::from_be_bytes(*b"cicp");
const SKCMS_SIGNATURE_HAGC: u32 = u32::from_be_bytes(*b"HAGC");

// Type signatures
const SKCMS_SIGNATURE_CURV: u32 = u32::from_be_bytes(*b"curv");
const SKCMS_SIGNATURE_HAGC_TYPE: u32 = u32::from_be_bytes(*b"hagc");
const SKCMS_SIGNATURE_MFT1: u32 = u32::from_be_bytes(*b"mft1");
const SKCMS_SIGNATURE_MFT2: u32 = u32::from_be_bytes(*b"mft2");
const SKCMS_SIGNATURE_M_AB: u32 = u32::from_be_bytes(*b"mAB ");
const SKCMS_SIGNATURE_M_BA: u32 = u32::from_be_bytes(*b"mBA ");
const SKCMS_SIGNATURE_PARA: u32 = u32::from_be_bytes(*b"para");

// Color space and PCS signatures
const SKCMS_SIGNATURE_XYZ: u32 = u32::from_be_bytes(*b"XYZ ");
const SKCMS_SIGNATURE_LAB: u32 = u32::from_be_bytes(*b"Lab ");
const SKCMS_SIGNATURE_GRAY: u32 = u32::from_be_bytes(*b"GRAY");

// Parametric curve function types (enum { kG = 0, kGAB = 1, kGABC = 2, kGABCD = 3, kGABCDEF = 4 };)
const K_G: u16 = 0;
const K_GAB: u16 = 1;
const K_GABC: u16 = 2;
const K_GABCD: u16 = 3;
const K_GABCDEF: u16 = 4;

// Limits (mirroring header_Layout, tag_Layout)
// Maps to header_Layout: SAFE_SIZEOF(header_Layout) == 132 (including tag_count)
const SAFE_HEADER_SIZE: usize = 132;
// Maps to tag_Layout: SAFE_SIZEOF(tag_Layout) == 12
const SAFE_TAG_ENTRY_SIZE: usize = 12;

// =============================================================================
// Byte / Buffer Reader (View and Cursor Tracking)
// =============================================================================

/// Cursor-based buffer reader that tracks read offsets and provides scoped sub-views.
/// Maximum allocation sizes are implicitly bounded by the view size (`data.len()`).
#[derive(Clone, Copy)]
struct IccReader<'a> {
    data: &'a [u8],
    cursor: usize,
}

impl<'a> IccReader<'a> {
    #[inline]
    fn new(data: &'a [u8]) -> Self {
        Self { data, cursor: 0 }
    }

    /// Creates an isolated sub-view for a tag or nested block.
    #[inline]
    fn sub_reader(&self, offset: usize, len: usize) -> Option<IccReader<'a>> {
        let end = offset.checked_add(len)?;
        let slice = self.data.get(offset..end)?;
        Some(IccReader::new(slice))
    }

    /// Creates a sub-view from `offset` to the end of the current view.
    #[inline]
    fn sub_reader_from(&self, offset: usize) -> Option<IccReader<'a>> {
        let slice = self.data.get(offset..)?;
        Some(IccReader::new(slice))
    }

    #[inline]
    fn read_u8(&mut self) -> Option<u8> {
        let slice = self.read_bytes(1)?;
        Some(slice[0])
    }

    #[inline]
    fn read_u16(&mut self) -> Option<u16> {
        let slice = self.read_bytes(2)?;
        Some(u16::from_be_bytes([slice[0], slice[1]]))
    }

    #[inline]
    fn read_u32(&mut self) -> Option<u32> {
        let slice = self.read_bytes(4)?;
        Some(u32::from_be_bytes([slice[0], slice[1], slice[2], slice[3]]))
    }

    #[inline]
    fn read_i32(&mut self) -> Option<i32> {
        let slice = self.read_bytes(4)?;
        Some(i32::from_be_bytes([slice[0], slice[1], slice[2], slice[3]]))
    }

    #[inline]
    fn read_fixed(&mut self) -> Option<f32> {
        let raw = self.read_i32()?;
        Some((raw as f32) * (1.0 / 65536.0))
    }

    #[inline]
    fn read_bytes(&mut self, len: usize) -> Option<&'a [u8]> {
        let next_cursor = self.cursor.checked_add(len)?;
        let slice = self.data.get(self.cursor..next_cursor)?;
        self.cursor = next_cursor;
        Some(slice)
    }

    #[inline]
    fn skip(&mut self, len: usize) -> Option<()> {
        self.read_bytes(len).map(|_| ())
    }

    /// Skips padding bytes to align `self.cursor` to a multiple of `alignment`.
    #[inline]
    fn skip_to_alignment(&mut self, alignment: usize) -> Option<()> {
        let remainder = self.cursor % alignment;
        if remainder != 0 {
            self.skip(alignment - remainder)?;
        }
        Some(())
    }

    /// Reads `num_elements * format.byte_width()` bytes into an allocated buffer with at least
    /// 2 extra bytes of zero-padding aligned to a 4-byte boundary (for SIMD gather overread
    /// safety).
    #[inline]
    fn read_vec(&mut self, num_elements: usize, format: TableFormat) -> Option<Vec<u8>> {
        let len = num_elements.checked_mul(format.byte_width())?;
        let padded_len = (len.checked_add(2)?.checked_add(3)?) & !3;
        let slice = self.read_bytes(len)?;
        let mut vec = Vec::with_capacity(padded_len);
        vec.extend_from_slice(slice);
        vec.resize(padded_len, 0);
        Some(vec)
    }
}

/// Represents an ICC tag entry in the profile (matches `skcms_ICCTag` and `tag_Layout`).
struct IccTag<'a> {
    tag_type: u32,
    reader: IccReader<'a>,
}

/// Looks up an ICC tag by its signature (matches `skcms_GetTagBySignature`).
fn get_tag_by_signature<'a>(
    data: &'a [u8],
    profile_size: usize,
    tag_count: usize,
    sig: u32,
) -> Option<IccTag<'a>> {
    let profile_reader = IccReader::new(data.get(..profile_size)?);
    let mut tag_table_reader = profile_reader.sub_reader(
        SAFE_HEADER_SIZE,
        tag_count.checked_mul(SAFE_TAG_ENTRY_SIZE)?,
    )?;

    for _ in 0..tag_count {
        let tag_sig = tag_table_reader.read_u32()?;
        let offset = tag_table_reader.read_u32()? as usize;
        let size = tag_table_reader.read_u32()? as usize;
        if tag_sig == sig {
            let mut tag_reader = profile_reader.sub_reader(offset, size)?;
            let tag_type = tag_reader.read_u32()?;
            return Some(IccTag {
                tag_type,
                reader: tag_reader,
            });
        }
    }
    None
}

// =============================================================================
// XYZ and Colorant Matrix Parsing
// =============================================================================

/// Reads XYZ tag data (matches `XYZ_Layout` and `read_tag_xyz`).
///
/// XYZType is technically variable sized, holding N XYZ triples. However, the only valid uses of
/// the type are for tags/data that store exactly one triple.
fn read_tag_xyz(tag: &IccTag) -> Option<[f32; 3]> {
    if tag.tag_type != SKCMS_SIGNATURE_XYZ {
        return None;
    }
    let mut r = tag.reader;
    r.skip(4)?; // reserved
    Some([r.read_fixed()?, r.read_fixed()?, r.read_fixed()?])
}

/// Reads rXYZ, gXYZ, bXYZ tags into a 3x3 toXYZD50 matrix (matches `read_to_XYZD50`).
fn read_to_xyzd50(r_tag: &IccTag, g_tag: &IccTag, b_tag: &IccTag) -> Option<Matrix3x3> {
    let r = read_tag_xyz(r_tag)?;
    let g = read_tag_xyz(g_tag)?;
    let b = read_tag_xyz(b_tag)?;
    Some(Matrix3x3 {
        vals: [
            [r[0], g[0], b[0]],
            [r[1], g[1], b[1]],
            [r[2], g[2], b[2]],
        ],
    })
}

// =============================================================================
// Curve Parsing ('para' and 'curv')
// =============================================================================

/// Basic soundness checks for sRGBish transfer functions
/// (matches `skcms_TransferFunction_isSRGBish`).
fn is_srgb_ish(tf: &TransferFunction) -> bool {
    let sum = tf.a + tf.b + tf.c + tf.d + tf.e + tf.f + tf.g;
    // a,c,d,g should be non-negative to make any sense.
    // Raising a negative value to a fractional tf->g produces complex numbers.
    sum.is_finite()
        && tf.a >= 0.0
        && tf.c >= 0.0
        && tf.d >= 0.0
        && tf.g >= 0.0
        && (tf.a * tf.d + tf.b) >= 0.0
}

/// Parses parametric curve data (matches `read_curve_para` and `para_Layout`).
/// 1, 3, 4, 5, or 7 s15.16, depending on function_type.
fn read_curve_para(r: &mut IccReader) -> Option<Curve> {
    r.skip(4)?; // reserved
    let function_type = r.read_u16()?;
    r.skip(2)?; // reserved

    if function_type > K_GABCDEF {
        return None;
    }

    let mut tf = TransferFunction {
        a: 1.0,
        b: 0.0,
        c: 0.0,
        d: 0.0,
        e: 0.0,
        f: 0.0,
        g: r.read_fixed()?,
    };

    match function_type {
        K_G => {}
        K_GAB => {
            tf.a = r.read_fixed()?;
            tf.b = r.read_fixed()?;
            if tf.a == 0.0 {
                return None;
            }
            tf.d = -tf.b / tf.a;
        }
        K_GABC => {
            tf.a = r.read_fixed()?;
            tf.b = r.read_fixed()?;
            tf.e = r.read_fixed()?;
            if tf.a == 0.0 {
                return None;
            }
            tf.d = -tf.b / tf.a;
            tf.f = tf.e;
        }
        K_GABCD => {
            tf.a = r.read_fixed()?;
            tf.b = r.read_fixed()?;
            tf.c = r.read_fixed()?;
            tf.d = r.read_fixed()?;
        }
        K_GABCDEF => {
            tf.a = r.read_fixed()?;
            tf.b = r.read_fixed()?;
            tf.c = r.read_fixed()?;
            tf.d = r.read_fixed()?;
            tf.e = r.read_fixed()?;
            tf.f = r.read_fixed()?;
        }
        _ => return None,
    }

    if !is_srgb_ish(&tf) {
        return None;
    }

    Some(Curve {
        table_entries: 0,
        parametric: tf,
        table_data: Vec::new(),
        table_format: TableFormat::U8,
    })
}

/// Parses table or simple gamma curve data (matches `read_curve_curv` and `curv_Layout`).
/// value_count, 8.8 if 1, uint16 (n*65535) if > 1.
fn read_curve_curv(r: &mut IccReader) -> Option<Curve> {
    r.skip(4)?; // reserved
    let value_count = r.read_u32()? as usize;

    if value_count < 2 {
        let g = if value_count == 0 {
            // Empty tables are a shorthand for an identity curve
            1.0
        } else {
            // Single entry tables are a shorthand for simple gamma
            (r.read_u16()? as f32) * (1.0 / 256.0)
        };
        return Some(Curve {
            table_entries: 0,
            parametric: TransferFunction {
                g,
                a: 1.0,
                b: 0.0,
                c: 0.0,
                d: 0.0,
                e: 0.0,
                f: 0.0,
            },
            table_data: Vec::new(),
            table_format: TableFormat::U8,
        });
    }

    let table_data = r.read_vec(value_count, TableFormat::U16)?;

    Some(Curve {
        table_entries: value_count as u32,
        parametric: TransferFunction::default(),
        table_data,
        table_format: TableFormat::U16,
    })
}

/// Parses both curveType and parametricCurveType data (matches `read_curve`).
fn read_curve(r: &mut IccReader) -> Option<Curve> {
    let tag_type = r.read_u32()?;
    if tag_type == SKCMS_SIGNATURE_PARA {
        read_curve_para(r)
    } else if tag_type == SKCMS_SIGNATURE_CURV {
        read_curve_curv(r)
    } else {
        None
    }
}

/// Parses a Curve from an ICC tag whose signature was already read.
fn read_curve_from_tag(tag: &IccTag) -> Option<Curve> {
    let mut r = tag.reader;
    if tag.tag_type == SKCMS_SIGNATURE_PARA {
        read_curve_para(&mut r)
    } else if tag.tag_type == SKCMS_SIGNATURE_CURV {
        read_curve_curv(&mut r)
    } else {
        None
    }
}

/// Reads a sequence of N curves, taking 4-byte padding into account (matches `read_curves`).
fn read_curves(mut r: IccReader, num_curves: usize) -> Option<Vec<Curve>> {
    let mut curves = Vec::with_capacity(num_curves);
    for _ in 0..num_curves {
        curves.push(read_curve(&mut r)?);
        r.skip_to_alignment(4)?;
    }
    Some(curves)
}

// =============================================================================
// Multi-Stage Transforms (mft1, mft2, mAB, mBA)
// =============================================================================

/// Direction for LUT-based color transforms.
#[derive(Copy, Clone)]
enum LutDirection {
    A2B,
    B2A,
}

fn empty_transform() -> A2BOrB2ATransform {
    A2BOrB2ATransform {
        input_curves: Vec::new(),
        input_channels: 0,
        grid_points: [0; 4],
        grid_data: Vec::new(),
        grid_format: TableFormat::U8,
        matrix_curves: Vec::new(),
        matrix: Matrix3x3 {
            vals: [[0.0; 3]; 3],
        },
        matrix_bias: [0.0; 3],
        matrix_channels: 0,
        output_curves: Vec::new(),
        output_channels: 0,
    }
}

/// Extracts and validates channels, grid dimensions, and axis limits
/// (matches `mft_CommonLayout` and `read_mft_common`).
fn read_mft_common(
    mut r: IccReader,
    direction: LutDirection,
    out: &mut A2BOrB2ATransform,
) -> Option<()> {
    r.skip(4)?; // reserved
    let input_channels = r.read_u8()? as u32;
    let output_channels = r.read_u8()? as u32;
    let grid_point = r.read_u8()?;

    match direction {
        LutDirection::A2B => {
            // We require exactly three (ie XYZ/Lab/RGB) output channels
            // We require at least one, and no more than four (ie CMYK) input channels
            // The grid only makes sense with at least two points along each axis
            if output_channels != 3 || input_channels < 1 || input_channels > 4 || grid_point < 2 {
                return None;
            }
        }
        LutDirection::B2A => {
            // For B2A, exactly 3 input channels (XYZ) and 3 (RGB) or 4 (CMYK) output channels.
            if input_channels != 3 || output_channels < 3 || output_channels > 4 || grid_point < 2 {
                return None;
            }
        }
    }

    out.grid_points[0..input_channels as usize].fill(grid_point);
    out.input_channels = input_channels;
    out.output_channels = output_channels;
    out.matrix_channels = 0;
    Some(())
}

/// Reads 8-bit or 16-bit input curves, CLUT grid data, and output curves (matches `init_tables`).
fn init_tables(
    mut r: IccReader,
    format: TableFormat,
    input_entries: usize,
    output_entries: usize,
    out: &mut A2BOrB2ATransform,
) -> Option<()> {
    let input_channels = out.input_channels as usize;
    let output_channels = out.output_channels as usize;

    let mut input_curves = Vec::with_capacity(input_channels);
    for _ in 0..input_channels {
        let table_data = r.read_vec(input_entries, format)?;
        input_curves.push(Curve {
            table_entries: input_entries as u32,
            parametric: TransferFunction::default(),
            table_data,
            table_format: format,
        });
    }
    out.input_curves = input_curves;

    let mut grid_points_count = 1usize;
    for i in 0..input_channels {
        grid_points_count = grid_points_count.checked_mul(out.grid_points[i] as usize)?;
    }
    let total_samples = grid_points_count.checked_mul(output_channels)?;

    out.grid_format = format;
    out.grid_data = r.read_vec(total_samples, format)?;

    let mut output_curves = Vec::with_capacity(output_channels);
    for _ in 0..output_channels {
        let table_data = r.read_vec(output_entries, format)?;
        output_curves.push(Curve {
            table_entries: output_entries as u32,
            parametric: TransferFunction::default(),
            table_data,
            table_format: format,
        });
    }
    out.output_curves = output_curves;

    Some(())
}

/// Parses mft1 tag data (matches `mft1_Layout` and `read_tag_mft1`).
///
/// MFT matrices are applied before the first set of curves, but must be identity unless the
/// input is PCSXYZ. We don't support PCSXYZ profiles, so we ignore this matrix.
fn read_tag_mft1(tag: &IccTag, direction: LutDirection) -> Option<A2BOrB2ATransform> {
    let mut out = empty_transform();
    read_mft_common(tag.reader, direction, &mut out)?;
    let tables_reader = tag.reader.sub_reader_from(48)?;
    init_tables(tables_reader, TableFormat::U8, 256, 256, &mut out)?;
    Some(out)
}

/// Parses mft2 tag data (matches `mft2_Layout` and `read_tag_mft2`).
fn read_tag_mft2(tag: &IccTag, direction: LutDirection) -> Option<A2BOrB2ATransform> {
    let mut out = empty_transform();
    read_mft_common(tag.reader, direction, &mut out)?;
    let mut r = tag.reader.sub_reader_from(48)?;
    let in_entries = r.read_u16()? as usize;
    let out_entries = r.read_u16()? as usize;
    // ICC spec mandates that 2 <= table_entries <= 4096
    if in_entries < 2 || in_entries > 4096 || out_entries < 2 || out_entries > 4096 {
        return None;
    }
    let tables_reader = tag.reader.sub_reader_from(52)?;
    init_tables(tables_reader, TableFormat::U16, in_entries, out_entries, &mut out)?;
    Some(out)
}

/// Parses mAB or mBA tag data (matches `mAB_or_mBA_Layout`, `CLUT_Layout`, `read_tag_mab`,
/// and `read_tag_mba`).
/// mAB and mBA tags use the same encoding, including color lookup tables.
fn read_tag_mab_or_mba(
    tag: &IccTag,
    pcs_is_xyz: bool,
    direction: LutDirection,
) -> Option<A2BOrB2ATransform> {
    let mut r = tag.reader;
    r.skip(4)?; // reserved
    let input_channels = r.read_u8()? as u32;
    let output_channels = r.read_u8()? as u32;
    r.skip(2)?; // padding

    match direction {
        LutDirection::A2B => {
            // We require exactly three (ie XYZ/Lab/RGB) output channels
            // We require at least one, and no more than four (ie CMYK) input channels
            if output_channels != 3 || input_channels < 1 || input_channels > 4 {
                return None;
            }
        }
        LutDirection::B2A => {
            // Require exactly 3 inputs (XYZ) and 3 (RGB) or 4 (CMYK) outputs.
            if input_channels != 3 || output_channels < 3 || output_channels > 4 {
                return None;
            }
        }
    }

    let b_curve_offset = r.read_u32()? as usize;
    let matrix_offset = r.read_u32()? as usize;
    let m_curve_offset = r.read_u32()? as usize;
    let clut_offset = r.read_u32()? as usize;
    let a_curve_offset = r.read_u32()? as usize;

    // "B" curves must be present
    if b_curve_offset == 0 {
        return None;
    }

    // For A2B, "B" curves are output (PCS); for B2A, "B" curves are input (PCS).
    let b_curves_count = match direction {
        LutDirection::A2B => output_channels as usize,
        LutDirection::B2A => input_channels as usize,
    };
    let b_curves = read_curves(tag.reader.sub_reader_from(b_curve_offset)?, b_curves_count)?;

    let (matrix_channels, matrix_curves, matrix, matrix_bias) = if m_curve_offset != 0 {
        if matrix_offset == 0 {
            return None;
        }
        let mut mtx_reader = tag.reader.sub_reader_from(matrix_offset)?;
        // For A2B, matrix channels is tied to output_channels (3);
        // for B2A, matrix channels is tied to input_channels (3).
        let matrix_channels = match direction {
            LutDirection::A2B => output_channels,
            LutDirection::B2A => input_channels,
        };
        let matrix_curves =
            read_curves(tag.reader.sub_reader_from(m_curve_offset)?, matrix_channels as usize)?;

        let encoding_factor = if pcs_is_xyz {
            match direction {
                LutDirection::A2B => 65535.0 / 32768.0,
                LutDirection::B2A => 32768.0 / 65535.0,
            }
        } else {
            1.0
        };
        let matrix = Matrix3x3 {
            vals: [
                [
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                ],
                [
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                ],
                [
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                    encoding_factor * mtx_reader.read_fixed()?,
                ],
            ],
        };
        let matrix_bias = [
            encoding_factor * mtx_reader.read_fixed()?,
            encoding_factor * mtx_reader.read_fixed()?,
            encoding_factor * mtx_reader.read_fixed()?,
        ];
        (matrix_channels, matrix_curves, matrix, matrix_bias)
    } else {
        if matrix_offset != 0 {
            return None;
        }
        (
            0,
            Vec::new(),
            Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            [0.0; 3],
        )
    };

    // "A" curves and CLUT must be used together
    let (final_channels, a_curves, grid_points, grid_data, grid_format) =
        if a_curve_offset != 0 {
            if clut_offset == 0 {
                return None;
            }
            let mut clut_reader = tag.reader.sub_reader_from(clut_offset)?;

            // For A2B, "A" curves are input (Device); for B2A, "A" curves are output (Device).
            let a_curves_count = match direction {
                LutDirection::A2B => input_channels as usize,
                LutDirection::B2A => output_channels as usize,
            };
            let a_curves =
                read_curves(tag.reader.sub_reader_from(a_curve_offset)?, a_curves_count)?;

            let mut grid_points = [0u8; 4];
            let mut total_grid_points = 1usize;
            for i in 0..16 {
                let gp = clut_reader.read_u8()?;
                if i < input_channels as usize {
                    grid_points[i] = gp;
                    // The grid only makes sense with at least two points along each axis
                    if gp < 2 {
                        return None;
                    }
                    total_grid_points = total_grid_points.checked_mul(gp as usize)?;
                }
            }

            let grid_format = match clut_reader.read_u8()? {
                1 => TableFormat::U8,
                2 => TableFormat::U16,
                _ => return None,
            };
            clut_reader.skip(3)?; // reserved padding

            let total_samples = total_grid_points.checked_mul(output_channels as usize)?;
            let grid_data = clut_reader.read_vec(total_samples, grid_format)?;

            (
                match direction {
                    LutDirection::A2B => input_channels,
                    LutDirection::B2A => output_channels,
                },
                a_curves,
                grid_points,
                grid_data,
                grid_format,
            )
        } else {
            if clut_offset != 0 {
                return None;
            }

            // If there is no CLUT, the number of input and output channels must match
            if input_channels != output_channels {
                return None;
            }

            // Zero out channels to signal that we're skipping this stage
            // (input_channels for A2B, output_channels for B2A)
            (0, Vec::new(), [0; 4], Vec::new(), TableFormat::U8)
        };

    let (input_curves, final_input_channels, output_curves, final_output_channels) =
        match direction {
            LutDirection::A2B => (a_curves, final_channels, b_curves, output_channels),
            LutDirection::B2A => (b_curves, input_channels, a_curves, final_channels),
        };

    Some(A2BOrB2ATransform {
        input_curves,
        input_channels: final_input_channels,
        grid_points,
        grid_data,
        grid_format,
        matrix_curves,
        matrix,
        matrix_bias,
        matrix_channels,
        output_curves,
        output_channels: final_output_channels,
    })
}

/// Dispatches LUT tag reading based on tag type (matches `read_a2b` and `read_b2a`).
fn read_lut(
    tag: &IccTag,
    pcs_is_xyz: bool,
    direction: LutDirection,
) -> Option<A2BOrB2ATransform> {
    match (tag.tag_type, direction) {
        (SKCMS_SIGNATURE_MFT1, _) => read_tag_mft1(tag, direction),
        (SKCMS_SIGNATURE_MFT2, _) => read_tag_mft2(tag, direction),
        (SKCMS_SIGNATURE_M_AB, LutDirection::A2B) | (SKCMS_SIGNATURE_M_BA, LutDirection::B2A) => {
            read_tag_mab_or_mba(tag, pcs_is_xyz, direction)
        }
        _ => None,
    }
}

/// Parses CICP metadata (matches `read_cicp` and `CICP_Layout`).
fn read_cicp(tag: &IccTag) -> Option<Cicp> {
    if tag.tag_type != SKCMS_SIGNATURE_CICP {
        return None;
    }
    let mut r = tag.reader;
    r.skip(4)?; // reserved
    Some(Cicp {
        color_primaries: r.read_u8()?,
        transfer_characteristics: r.read_u8()?,
        matrix_coefficients: r.read_u8()?,
        video_full_range_flag: r.read_u8()?,
    })
}

/// Parses HAGC metadata (matches `read_hagc` and `HAGC_Layout`).
fn read_hagc(tag: &IccTag) -> Option<Vec<u8>> {
    if tag.tag_type != SKCMS_SIGNATURE_HAGC_TYPE {
        return None;
    }
    let mut r = tag.reader;
    r.skip(4)?; // reserved
    let size = r.read_u32()? as usize;
    let bytes = r.read_bytes(size)?;
    Some(bytes.to_vec())
}

/// Validates whether a profile is usable as a source profile (matches `usable_as_src`).
fn usable_as_src(profile: &IccProfile) -> bool {
    profile.has_a2b || (profile.has_trc && profile.has_to_xyzd50)
}

// =============================================================================
// Top-Level Profile Parser Implementation
// =============================================================================

/// Parses ICC profile from `data` with caller-specified A2B / B2A priority.
/// `priority` contains indices in 0..=2:
///   0 = Perceptual (A2B0 / B2A0)
///   1 = Relative Colorimetric (A2B1 / B2A1)
///   2 = Saturation (A2B2 / B2A2)
/// (Matches `skcms_ParseWithA2BPriority`).
fn parse_icc_profile_impl(
    data: &[u8],
    priority: &[i32],
    out: &mut IccProfile,
) -> Option<()> {
    if data.len() < SAFE_HEADER_SIZE {
        return None;
    }

    let mut r = IccReader::new(data);
    let size = r.read_u32()? as usize;
    r.skip(4)?; // cmm_type
    let version = r.read_u32()?;
    r.skip(4)?; // profile_class
    let data_color_space = r.read_u32()?;
    let pcs = r.read_u32()?;
    r.skip(12)?; // date_time (12)
    let signature = r.read_u32()?;
    r.skip(24)?; // platform (4), flags (4), manufacturer (4), model (4), attributes (8) = 24 bytes
    r.skip(4)?; // rendering intent (4)
    let illuminant_x = r.read_fixed()?;
    let illuminant_y = r.read_fixed()?;
    let illuminant_z = r.read_fixed()?;
    r.skip(48)?; // creator (4), profile id (16), reserved (28) = 48 bytes -> takes us to byte 128
    let tag_count = r.read_u32()? as usize;
    let tag_table_size = tag_count.checked_mul(SAFE_TAG_ENTRY_SIZE)?;
    let min_size = SAFE_HEADER_SIZE.checked_add(tag_table_size)?;

    // Validate signature, size (smaller than buffer, large enough to hold tag table),
    // and major version
    if signature != SKCMS_SIGNATURE_ACSP
        || size > data.len()
        || size < min_size
        || (version >> 24) > 4
    {
        return None;
    }

    // Validate that illuminant is D50 white
    if (illuminant_x - 0.9642).abs() > 0.0100
        || (illuminant_y - 1.0000).abs() > 0.0100
        || (illuminant_z - 0.8249).abs() > 0.0100
    {
        return None;
    }

    // Validate that all tag entries have sane offset + size
    let mut tag_table = r.sub_reader(SAFE_HEADER_SIZE, tag_table_size)?;
    for _ in 0..tag_count {
        tag_table.skip(4)?;
        let tag_offset = tag_table.read_u32()? as usize;
        let tag_size = tag_table.read_u32()? as usize;
        let tag_end = tag_offset.checked_add(tag_size)?;
        if tag_size < 4 || tag_end > size {
            return None;
        }
    }

    if pcs != SKCMS_SIGNATURE_XYZ && pcs != SKCMS_SIGNATURE_LAB {
        return None;
    }

    let pcs_is_xyz = match pcs {
        SKCMS_SIGNATURE_XYZ => true,
        SKCMS_SIGNATURE_LAB => false,
        _ => return None,
    };

    out.data_color_space = data_color_space;
    out.connection_space = pcs;

    out.has_to_xyzd50 = false;
    out.has_trc = false;

    // Pre-parse commonly used tags.
    if data_color_space == SKCMS_SIGNATURE_GRAY {
        if let Some(ktrc) = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_K_TRC) {
            let curve = read_curve_from_tag(&ktrc)?;
            out.trc = [curve.clone(), curve.clone(), curve];
            out.has_trc = true;

            if pcs_is_xyz {
                out.to_xyzd50 = Matrix3x3 {
                    vals: [
                        [illuminant_x, 0.0, 0.0],
                        [0.0, illuminant_y, 0.0],
                        [0.0, 0.0, illuminant_z],
                    ],
                };
                out.has_to_xyzd50 = true;
            }
        }
    } else {
        let r_trc = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_R_TRC);
        let g_trc = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_G_TRC);
        let b_trc = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_B_TRC);
        if let (Some(r), Some(g), Some(b)) = (r_trc, g_trc, b_trc) {
            let r_curve = read_curve_from_tag(&r)?;
            let g_curve = read_curve_from_tag(&g)?;
            let b_curve = read_curve_from_tag(&b)?;
            out.trc = [r_curve, g_curve, b_curve];
            out.has_trc = true;
        }

        let r_xyz = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_R_XYZ);
        let g_xyz = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_G_XYZ);
        let b_xyz = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_B_XYZ);
        if let (Some(r), Some(g), Some(b)) = (r_xyz, g_xyz, b_xyz) {
            out.to_xyzd50 = read_to_xyzd50(&r, &g, &b)?;
            out.has_to_xyzd50 = true;
        }
    }

    // enum { perceptual, relative_colormetric, saturation }
    out.has_a2b = false;
    for &p in priority {
        if p < 0 || p > 2 {
            return None;
        }
        let sig = SKCMS_SIGNATURE_A2B0 + p as u32;
        if let Some(tag) = get_tag_by_signature(data, size, tag_count, sig) {
            out.a2b = read_lut(&tag, pcs_is_xyz, LutDirection::A2B)?;
            out.has_a2b = true;
            break;
        }
    }

    out.has_b2a = false;
    for &p in priority {
        if p < 0 || p > 2 {
            return None;
        }
        let sig = SKCMS_SIGNATURE_B2A0 + p as u32;
        if let Some(tag) = get_tag_by_signature(data, size, tag_count, sig) {
            out.b2a = read_lut(&tag, pcs_is_xyz, LutDirection::B2A)?;
            out.has_b2a = true;
            break;
        }
    }

    out.has_cicp = false;
    if let Some(tag) = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_CICP) {
        out.cicp = read_cicp(&tag)?;
        out.has_cicp = true;
    }

    out.has_hagc = false;
    out.hagc.clear();
    if let Some(tag) = get_tag_by_signature(data, size, tag_count, SKCMS_SIGNATURE_HAGC) {
        out.hagc = read_hagc(&tag)?;
        out.has_hagc = true;
    }

    if !usable_as_src(out) {
        return None;
    }

    Some(())
}

/// Parses ICC profile from `data` with caller-specified A2B / B2A priority.
/// `priority` contains indices in 0..=2:
///   0 = Perceptual (A2B0 / B2A0)
///   1 = Relative Colorimetric (A2B1 / B2A1)
///   2 = Saturation (A2B2 / B2A2)
/// (Matches `skcms_ParseWithA2BPriority`).
pub fn parse_icc_profile(
    data: &[u8],
    priority: &[i32],
    out: &mut IccProfile,
) -> bool {
    parse_icc_profile_impl(data, priority, out).is_some()
}
