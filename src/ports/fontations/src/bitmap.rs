// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use read_fonts::{
    tables::{
        bitmap::{BitmapContent, BitmapData, BitmapDataFormat, BitmapMetrics, BitmapSize},
        sbix::GlyphData,
    },
    FontRef, TableProvider,
};

use font_types::{BoundingBox, GlyphId};
use skrifa::{
    bitmap::MaskData,
    instance::{LocationRef, Size},
    metrics::GlyphMetrics,
};

use crate::{
    ffi::{AlphaBitmapSize, BitmapMetrics as FfiBitmapMetrics},
    BridgeFontRef,
};

pub enum BitmapPixelData<'a> {
    PngData(&'a [u8]),
    AlphaMask {
        data: Vec<u8>,
        width: u32,
        height: u32,
    },
}

struct BitmapTableGlyph<'a> {
    bitmap_data: BitmapData<'a>,
    ppem_x: u8,
    ppem_y: u8,
    bit_depth: u8,
}

struct SbixGlyph<'a> {
    glyph_data: GlyphData<'a>,
    ppem: u16,
}

#[derive(Default)]
pub struct BridgeBitmapGlyph<'a> {
    pub data: Option<BitmapPixelData<'a>>,
    pub metrics: FfiBitmapMetrics,
}

trait StrikeSizeRetrievable {
    fn strike_size(&self) -> f32;
}

impl StrikeSizeRetrievable for &BitmapSize {
    fn strike_size(&self) -> f32 {
        self.ppem_y() as f32
    }
}

impl StrikeSizeRetrievable for SbixGlyph<'_> {
    fn strike_size(&self) -> f32 {
        self.ppem as f32
    }
}

// Find the nearest larger strike size, or if no larger one is available, the nearest smaller.
fn best_strike_size<T>(strikes: impl Iterator<Item = T>, font_size: f32) -> Option<T>
where
    T: StrikeSizeRetrievable,
{
    // After a bigger strike size is found, the order of strike sizes smaller
    // than the requested font size does not matter anymore. A new strike size
    // is only an improvement if it gets closer to the requested font size (and
    // is smaller than the current best, but bigger than font size). And vice
    // versa: As long as we have found only smaller ones so far, only any strike
    // size matters that is bigger than the current best.
    strikes.reduce(|best, entry| {
        let entry_size = entry.strike_size();
        if (entry_size >= font_size && entry_size < best.strike_size())
            || (best.strike_size() < font_size && entry_size > best.strike_size())
        {
            entry
        } else {
            best
        }
    })
}

fn sbix_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<SbixGlyph<'a>> {
    let sbix = font_ref.sbix().ok()?;
    let mut strikes = sbix.strikes().iter().filter_map(|strike| {
        let strike = strike.ok()?;
        Some(SbixGlyph {
            ppem: strike.ppem(),
            glyph_data: strike.glyph_data(glyph_id).ok()??,
        })
    });

    match font_size {
        Some(size) => best_strike_size(strikes, size),
        _ => strikes.next(),
    }
}

fn cblc_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<BitmapTableGlyph<'a>> {
    let cblc = font_ref.cblc().ok()?;
    let cbdt = font_ref.cbdt().ok()?;

    let strikes = &cblc.bitmap_sizes();
    let best_strike = font_size
        .and_then(|size| best_strike_size(strikes.iter(), size))
        .or(strikes.get(0))?;

    let location = best_strike.location(cblc.offset_data(), glyph_id).ok()?;

    Some(BitmapTableGlyph {
        bitmap_data: cbdt.data(&location).ok()?,
        ppem_x: best_strike.ppem_x,
        ppem_y: best_strike.ppem_y,
        bit_depth: best_strike.bit_depth,
    })
}

fn eblc_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<BitmapTableGlyph<'a>> {
    let eblc = font_ref.eblc().ok()?;
    let ebdt = font_ref.ebdt().ok()?;

    let strikes = &eblc.bitmap_sizes();
    let best_strike = font_size
        .and_then(|size| best_strike_size(strikes.iter(), size))
        .or(strikes.get(0))?;

    let location = best_strike.location(eblc.offset_data(), glyph_id).ok()?;

    Some(BitmapTableGlyph {
        bitmap_data: ebdt.data(&location).ok()?,
        ppem_x: best_strike.ppem_x,
        ppem_y: best_strike.ppem_y,
        bit_depth: best_strike.bit_depth,
    })
}

fn bitmap_table_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<BitmapTableGlyph<'a>> {
    cblc_glyph(font_ref, glyph_id, font_size)
        .or_else(|| eblc_glyph(font_ref, glyph_id, font_size))
}

struct AlphaBitmapDimensions {
    width: u32,
    height: u32,
}

fn bitmap_dimensions(metrics: &BitmapMetrics) -> Option<AlphaBitmapDimensions> {
    let (width, height) = match metrics {
        BitmapMetrics::Small(m) => (m.width as u32, m.height as u32),
        BitmapMetrics::Big(m) => (m.width as u32, m.height as u32),
    };
    if width == 0 || height == 0 {
        return None;
    }
    Some(AlphaBitmapDimensions { width, height })
}

pub fn has_bitmap_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    font_ref
        .with_font(|font| {
            let glyph_id = GlyphId::from(glyph_id);
            let has_sbix = sbix_glyph(font, glyph_id, None).is_some();
            let has_blc = bitmap_table_glyph(font, glyph_id, None).is_some();
            Some(has_sbix || has_blc)
        })
        .unwrap_or_default()
}

fn glyf_bounds(font_ref: &FontRef, glyph_id: GlyphId) -> Option<BoundingBox<i16>> {
    let glyf_table = font_ref.glyf().ok()?;
    let glyph = font_ref
        .loca(None)
        .ok()?
        .get_glyf(glyph_id, &glyf_table)
        .ok()??;
    Some(BoundingBox {
        x_min: glyph.x_min(),
        y_min: glyph.y_min(),
        x_max: glyph.x_max(),
        y_max: glyph.y_max(),
    })
}

pub unsafe fn bitmap_glyph<'a>(
    font_ref: &'a BridgeFontRef,
    glyph_id: u16,
    font_size: f32,
) -> Box<BridgeBitmapGlyph<'a>> {
    let glyph_id = GlyphId::from(glyph_id);
    font_ref
        .with_font(|font| {
            if let Some(sbix_glyph) = sbix_glyph(font, glyph_id, Some(font_size)) {
                // https://learn.microsoft.com/en-us/typography/opentype/spec/sbix
                // "If there is a glyph contour, the glyph design space
                // origin for the graphic is placed at the lower left corner
                // of the glyph bounding box (xMin, yMin)."
                let glyf_bb = glyf_bounds(font, glyph_id).unwrap_or_default();
                let glyf_left_side_bearing =
                    GlyphMetrics::new(font, Size::unscaled(), LocationRef::default())
                        .left_side_bearing(glyph_id)
                        .unwrap_or_default();

                return Some(Box::new(BridgeBitmapGlyph {
                    data: Some(BitmapPixelData::PngData(sbix_glyph.glyph_data.data())),
                    metrics: FfiBitmapMetrics {
                        bearing_x: glyf_left_side_bearing,
                        bearing_y: glyf_bb.y_min as f32,
                        inner_bearing_x: sbix_glyph.glyph_data.origin_offset_x() as f32,
                        inner_bearing_y: sbix_glyph.glyph_data.origin_offset_y() as f32,
                        ppem_x: sbix_glyph.ppem as f32,
                        ppem_y: sbix_glyph.ppem as f32,
                        placement_origin_bottom_left: true,
                        advance: f32::NAN,
                    },
                }));
            } else if let Some(blc_glyph) = bitmap_table_glyph(font, glyph_id, Some(font_size)) {
                let (bearing_x, bearing_y, advance) = match blc_glyph.bitmap_data.metrics {
                    BitmapMetrics::Small(small_metrics) => (
                        small_metrics.bearing_x() as f32,
                        small_metrics.bearing_y() as f32,
                        small_metrics.advance as f32,
                    ),
                    BitmapMetrics::Big(big_metrics) => (
                        big_metrics.hori_bearing_x() as f32,
                        big_metrics.hori_bearing_y() as f32,
                        big_metrics.hori_advance as f32,
                    ),
                };

                let metrics = FfiBitmapMetrics {
                    bearing_x: 0.0,
                    bearing_y: 0.0,
                    inner_bearing_x: bearing_x,
                    inner_bearing_y: bearing_y,
                    ppem_x: blc_glyph.ppem_x as f32,
                    ppem_y: blc_glyph.ppem_y as f32,
                    placement_origin_bottom_left: false,
                    advance: advance,
                };
                match blc_glyph.bitmap_data.content {
                    BitmapContent::Data(BitmapDataFormat::Png, png_buffer) => {
                        return Some(Box::new(BridgeBitmapGlyph {
                            data: Some(BitmapPixelData::PngData(png_buffer)),
                            metrics,
                        }));
                    }
                    BitmapContent::Data(format, raw_data) => {
                        let dims = bitmap_dimensions(&blc_glyph.bitmap_data.metrics)?;
                        let is_packed = match format {
                            BitmapDataFormat::BitAligned => true,
                            BitmapDataFormat::ByteAligned => false,
                            _ => return None,
                        };
                        let mask = MaskData {
                            bpp: blc_glyph.bit_depth,
                            is_packed,
                            data: raw_data,
                        };
                        if let Ok(alpha_data) = mask.decode(dims.width, dims.height) {
                            return Some(Box::new(BridgeBitmapGlyph {
                                data: Some(BitmapPixelData::AlphaMask {
                                    data: alpha_data,
                                    width: dims.width,
                                    height: dims.height,
                                }),
                                metrics,
                            }));
                        }
                    }
                    _ => {}
                }
            }
            None
        })
        .unwrap_or_default()
}

pub unsafe fn png_data<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a [u8] {
    match bitmap_glyph.data {
        Some(BitmapPixelData::PngData(glyph_data)) => glyph_data,
        _ => &[],
    }
}

pub unsafe fn bitmap_metrics<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a FfiBitmapMetrics {
    &bitmap_glyph.metrics
}

pub fn has_alpha_mask(bitmap_glyph: &BridgeBitmapGlyph) -> bool {
    matches!(&bitmap_glyph.data, Some(BitmapPixelData::AlphaMask { .. }))
}

pub unsafe fn alpha_mask_data<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a [u8] {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::AlphaMask { data, .. }) => data.as_slice(),
        _ => &[],
    }
}

pub fn alpha_mask_size(bitmap_glyph: &BridgeBitmapGlyph) -> AlphaBitmapSize {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::AlphaMask { width, height, .. }) => AlphaBitmapSize {
            width: *width,
            height: *height,
        },
        _ => AlphaBitmapSize::default(),
    }
}

#[cfg(test)]
mod test {
    use std::fs;

    use font_types::{GlyphId, Tag};
    use read_fonts::{FontRef, Offset, TableProvider};

    use crate::{
        bitmap::{bitmap_glyph, bitmap_metrics, has_bitmap_glyph, png_data},
        font_ref_is_valid, make_font_ref,
    };

    const TEST_SBIX_FILENAME: &str = "resources/fonts/sbix.ttf";
    const LAST_GLYPH_ID: u16 = 3;

    // Zeroes out the last glyph's data range in `strike_index` by setting its
    // end offset equal to its start offset in `glyphDataOffsets`.
    fn clear_last_sbix_glyph_in_strike(font_buffer: &mut [u8], strike_index: usize) {
        let (start_offset, end_offset_pos) = {
            let font = FontRef::new(font_buffer).unwrap();
            let sbix_record = font
                .table_directory
                .table_records()
                .iter()
                .find(|r| r.tag() == Tag::new(b"sbix"))
                .unwrap();
            let sbix_offset = sbix_record.offset() as usize;
            let sbix = font.sbix().unwrap();
            let strike_offset =
                sbix_offset + sbix.strike_offsets()[strike_index].get().to_usize();
            let strike = sbix.strikes().get(strike_index).unwrap();
            let offsets = strike.glyph_data_offsets();
            let last_glyph_index = offsets.len() - 2;
            assert_eq!(last_glyph_index, LAST_GLYPH_ID as usize);
            let start = offsets[last_glyph_index].get();
            // Strike header is ppem (u16) + ppi (u16) = 4 bytes, followed by
            // glyphDataOffsets: [Offset32; numGlyphs + 1].
            let end_pos = strike_offset + 4 + (last_glyph_index + 1) * 4;
            (start, end_pos)
        };
        font_buffer[end_offset_pos..end_offset_pos + 4]
            .copy_from_slice(&start_offset.to_be_bytes());

        let font = FontRef::new(font_buffer).unwrap();
        let strike = font.sbix().unwrap().strikes().get(strike_index).unwrap();
        assert!(strike
            .glyph_data(GlyphId::from(LAST_GLYPH_ID))
            .unwrap()
            .is_none());
    }

    #[test]
    fn test_sbix_sparse_strikes() {
        let base_buffer =
            fs::read(TEST_SBIX_FILENAME).expect("sbix test font could not be opened.");

        // Strike 0 (16 ppem) is empty for LAST_GLYPH_ID; strikes 1 (64 ppem)
        // and 2 (128 ppem) are populated.
        let mut missing_first_strike = base_buffer.clone();
        clear_last_sbix_glyph_in_strike(&mut missing_first_strike, 0);
        let font_ref = make_font_ref(&missing_first_strike, 0);
        assert!(font_ref_is_valid(&font_ref));
        assert!(has_bitmap_glyph(&font_ref, LAST_GLYPH_ID));
        unsafe {
            let glyph_at_16 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 16.0);
            assert!(!png_data(&glyph_at_16).is_empty());
            assert_eq!(bitmap_metrics(&glyph_at_16).ppem_x, 64.0);
            assert_eq!(bitmap_metrics(&glyph_at_16).ppem_y, 64.0);

            let glyph_at_64 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 64.0);
            assert!(!png_data(&glyph_at_64).is_empty());
            assert_eq!(bitmap_metrics(&glyph_at_64).ppem_x, 64.0);

            let glyph_at_128 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 128.0);
            assert!(!png_data(&glyph_at_128).is_empty());
            assert_eq!(bitmap_metrics(&glyph_at_128).ppem_x, 128.0);
        }

        // Strike 1 (64 ppem) is empty for LAST_GLYPH_ID; strikes 0 (16 ppem)
        // and 2 (128 ppem) are populated. Requesting 64 ppem should select the
        // nearest larger populated strike (128 ppem).
        let mut missing_middle_strike = base_buffer;
        clear_last_sbix_glyph_in_strike(&mut missing_middle_strike, 1);
        let font_ref = make_font_ref(&missing_middle_strike, 0);
        assert!(has_bitmap_glyph(&font_ref, LAST_GLYPH_ID));
        unsafe {
            let glyph_at_64 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 64.0);
            assert!(!png_data(&glyph_at_64).is_empty());
            assert_eq!(bitmap_metrics(&glyph_at_64).ppem_x, 128.0);
        }

        // Also clear strike 2 (128 ppem), leaving only strike 0 (16 ppem).
        // Requesting 64 ppem should fall back to the smaller populated strike.
        clear_last_sbix_glyph_in_strike(&mut missing_middle_strike, 2);
        let font_ref = make_font_ref(&missing_middle_strike, 0);
        assert!(has_bitmap_glyph(&font_ref, LAST_GLYPH_ID));
        unsafe {
            let glyph_at_64 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 64.0);
            assert!(!png_data(&glyph_at_64).is_empty());
            assert_eq!(bitmap_metrics(&glyph_at_64).ppem_x, 16.0);
        }

        // Clear the remaining strike so no strikes contain data for LAST_GLYPH_ID.
        clear_last_sbix_glyph_in_strike(&mut missing_middle_strike, 0);
        let font_ref = make_font_ref(&missing_middle_strike, 0);
        assert!(!has_bitmap_glyph(&font_ref, LAST_GLYPH_ID));
        unsafe {
            let glyph_at_16 = bitmap_glyph(&font_ref, LAST_GLYPH_ID, 16.0);
            assert!(png_data(&glyph_at_16).is_empty());
        }
    }
}
