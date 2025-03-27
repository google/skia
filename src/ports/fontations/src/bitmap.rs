// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use read_fonts::{
    tables::{
        bitmap::{BitmapContent, BitmapData, BitmapDataFormat, BitmapMetrics, BitmapSize},
        sbix::{GlyphData, Strike},
    },
    FontRef, TableProvider,
};

use font_types::{BoundingBox, GlyphId};
use skrifa::{
    instance::{LocationRef, Size},
    metrics::GlyphMetrics,
};

use crate::{ffi::BitmapMetrics as FfiBitmapMetrics, BridgeFontRef};

pub enum BitmapPixelData<'a> {
    PngData(&'a [u8]),
}

struct CblcGlyph<'a> {
    bitmap_data: BitmapData<'a>,
    ppem_x: u8,
    ppem_y: u8,
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

impl StrikeSizeRetrievable for Strike<'_> {
    fn strike_size(&self) -> f32 {
        self.ppem() as f32
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
    let mut strikes = sbix.strikes().iter().filter_map(|strike| strike.ok());

    let best_strike = match font_size {
        Some(size) => best_strike_size(strikes, size),
        _ => strikes.next(),
    }?;

    Some(SbixGlyph {
        ppem: best_strike.ppem(),
        glyph_data: best_strike.glyph_data(glyph_id).ok()??,
    })
}

fn cblc_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<CblcGlyph<'a>> {
    let cblc = font_ref.cblc().ok()?;
    let cbdt = font_ref.cbdt().ok()?;

    let strikes = &cblc.bitmap_sizes();
    let best_strike = font_size
        .and_then(|size| best_strike_size(strikes.iter(), size))
        .or(strikes.get(0))?;

    let location = best_strike.location(cblc.offset_data(), glyph_id).ok()?;

    Some(CblcGlyph {
        bitmap_data: cbdt.data(&location).ok()?,
        ppem_x: best_strike.ppem_x,
        ppem_y: best_strike.ppem_y,
    })
}

pub fn has_bitmap_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    font_ref
        .with_font(|font| {
            let glyph_id = GlyphId::from(glyph_id);
            let has_sbix = sbix_glyph(font, glyph_id, None).is_some();
            let has_cblc = cblc_glyph(font, glyph_id, None).is_some();
            Some(has_sbix || has_cblc)
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
            } else if let Some(cblc_glyph) = cblc_glyph(font, glyph_id, Some(font_size)) {
                let (bearing_x, bearing_y, advance) = match cblc_glyph.bitmap_data.metrics {
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
                if let BitmapContent::Data(BitmapDataFormat::Png, png_buffer) =
                    cblc_glyph.bitmap_data.content
                {
                    return Some(Box::new(BridgeBitmapGlyph {
                        data: Some(BitmapPixelData::PngData(png_buffer)),
                        metrics: FfiBitmapMetrics {
                            bearing_x: 0.0,
                            bearing_y: 0.0,
                            inner_bearing_x: bearing_x,
                            inner_bearing_y: bearing_y,
                            ppem_x: cblc_glyph.ppem_x as f32,
                            ppem_y: cblc_glyph.ppem_y as f32,
                            placement_origin_bottom_left: false,
                            advance: advance,
                        },
                    }));
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
