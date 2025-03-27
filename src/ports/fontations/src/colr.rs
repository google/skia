// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use std::pin::Pin;

use crate::{
    base::{BridgeFontRef, BridgeNormalizedCoords},
    ffi::{
        ClipBox, ColorPainterWrapper, ColorStop, FillLinearParams, FillRadialParams,
        FillSweepParams, PaletteOverride,
    },
};

use font_types::{BoundingBox, GlyphId};
use read_fonts::{
    tables::{colr::CompositeMode, cpal::Cpal},
    TableProvider,
};
use skrifa::{
    color::{Brush, ColorGlyphFormat, ColorPainter, Transform},
    prelude::Size,
    MetadataProvider,
};

pub struct BridgeColorStops<'a> {
    pub stops_iterator: Box<dyn Iterator<Item = &'a skrifa::color::ColorStop> + 'a>,
    pub num_stops: usize,
}

struct ColorPainterImpl<'a> {
    color_painter_wrapper: Pin<&'a mut ColorPainterWrapper>,
    clip_level: usize,
}

impl<'a> ColorPainter for ColorPainterImpl<'a> {
    fn push_transform(&mut self, transform: Transform) {
        if self.clip_level > 0 {
            return;
        }
        self.color_painter_wrapper
            .as_mut()
            .push_transform(&crate::ffi::Transform {
                xx: transform.xx,
                xy: transform.xy,
                yx: transform.yx,
                yy: transform.yy,
                dx: transform.dx,
                dy: transform.dy,
            });
    }

    fn pop_transform(&mut self) {
        if self.clip_level > 0 {
            return;
        }
        self.color_painter_wrapper.as_mut().pop_transform();
    }

    fn push_clip_glyph(&mut self, glyph: GlyphId) {
        if self.clip_level == 0 {
            // TODO(drott): Handle large glyph ids in clip operation.
            self.color_painter_wrapper
                .as_mut()
                .push_clip_glyph(glyph.to_u32().try_into().ok().unwrap_or_default());
        }
        if self.color_painter_wrapper.as_mut().is_bounds_mode() {
            self.clip_level += 1;
        }
    }

    fn push_clip_box(&mut self, clip_box: BoundingBox<f32>) {
        if self.clip_level == 0 {
            self.color_painter_wrapper.as_mut().push_clip_rectangle(
                clip_box.x_min,
                clip_box.y_min,
                clip_box.x_max,
                clip_box.y_max,
            );
        }
        if self.color_painter_wrapper.as_mut().is_bounds_mode() {
            self.clip_level += 1;
        }
    }

    fn pop_clip(&mut self) {
        if self.color_painter_wrapper.as_mut().is_bounds_mode() {
            self.clip_level -= 1;
        }
        if self.clip_level == 0 {
            self.color_painter_wrapper.as_mut().pop_clip();
        }
    }

    fn fill(&mut self, fill_type: Brush) {
        if self.clip_level > 0 {
            return;
        }
        let color_painter = self.color_painter_wrapper.as_mut();
        match fill_type {
            Brush::Solid {
                palette_index,
                alpha,
            } => {
                color_painter.fill_solid(palette_index, alpha);
            }

            Brush::LinearGradient {
                p0,
                p1,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_linear(
                    &FillLinearParams {
                        x0: p0.x,
                        y0: p0.y,
                        x1: p1.x,
                        y1: p1.y,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
            Brush::RadialGradient {
                c0,
                r0,
                c1,
                r1,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_radial(
                    &FillRadialParams {
                        x0: c0.x,
                        y0: c0.y,
                        r0,
                        x1: c1.x,
                        y1: c1.y,
                        r1,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
            Brush::SweepGradient {
                c0,
                start_angle,
                end_angle,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_sweep(
                    &FillSweepParams {
                        x0: c0.x,
                        y0: c0.y,
                        start_angle,
                        end_angle,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
        }
    }

    fn fill_glyph(&mut self, glyph: GlyphId, brush_transform: Option<Transform>, brush: Brush) {
        if self.color_painter_wrapper.as_mut().is_bounds_mode() {
            self.push_clip_glyph(glyph);
            self.pop_clip();
            return;
        }

        let color_painter = self.color_painter_wrapper.as_mut();
        let brush_transform = brush_transform.unwrap_or_default();
        match brush {
            Brush::Solid {
                palette_index,
                alpha,
            } => {
                // TODO(drott): Handle large glyph ids in fill glyph operation.
                color_painter.fill_glyph_solid(
                    glyph.to_u32().try_into().ok().unwrap_or_default(),
                    palette_index,
                    alpha,
                );
            }
            Brush::LinearGradient {
                p0,
                p1,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_glyph_linear(
                    // TODO(drott): Handle large glyph ids in fill glyph operation.
                    glyph.to_u32().try_into().ok().unwrap_or_default(),
                    &crate::ffi::Transform {
                        xx: brush_transform.xx,
                        xy: brush_transform.xy,
                        yx: brush_transform.yx,
                        yy: brush_transform.yy,
                        dx: brush_transform.dx,
                        dy: brush_transform.dy,
                    },
                    &FillLinearParams {
                        x0: p0.x,
                        y0: p0.y,
                        x1: p1.x,
                        y1: p1.y,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
            Brush::RadialGradient {
                c0,
                r0,
                c1,
                r1,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_glyph_radial(
                    // TODO(drott): Handle large glyph ids in fill glyph operation.
                    glyph.to_u32().try_into().ok().unwrap_or_default(),
                    &crate::ffi::Transform {
                        xx: brush_transform.xx,
                        xy: brush_transform.xy,
                        yx: brush_transform.yx,
                        yy: brush_transform.yy,
                        dx: brush_transform.dx,
                        dy: brush_transform.dy,
                    },
                    &FillRadialParams {
                        x0: c0.x,
                        y0: c0.y,
                        r0,
                        x1: c1.x,
                        y1: c1.y,
                        r1,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
            Brush::SweepGradient {
                c0,
                start_angle,
                end_angle,
                color_stops,
                extend,
            } => {
                let mut bridge_color_stops = BridgeColorStops {
                    stops_iterator: Box::new(color_stops.iter()),
                    num_stops: color_stops.len(),
                };
                color_painter.fill_glyph_sweep(
                    // TODO(drott): Handle large glyph ids in fill glyph operation.
                    glyph.to_u32().try_into().ok().unwrap_or_default(),
                    &crate::ffi::Transform {
                        xx: brush_transform.xx,
                        xy: brush_transform.xy,
                        yx: brush_transform.yx,
                        yy: brush_transform.yy,
                        dx: brush_transform.dx,
                        dy: brush_transform.dy,
                    },
                    &FillSweepParams {
                        x0: c0.x,
                        y0: c0.y,
                        start_angle,
                        end_angle,
                    },
                    &mut bridge_color_stops,
                    extend as u8,
                );
            }
        }
    }

    fn push_layer(&mut self, composite_mode: CompositeMode) {
        if self.clip_level > 0 {
            return;
        }
        self.color_painter_wrapper
            .as_mut()
            .push_layer(composite_mode as u8);
    }
    fn pop_layer(&mut self) {
        if self.clip_level > 0 {
            return;
        }
        self.color_painter_wrapper.as_mut().pop_layer();
    }
}

pub fn resolve_palette(
    font_ref: &BridgeFontRef,
    base_palette: u16,
    palette_overrides: &[PaletteOverride],
) -> Vec<u32> {
    let cpal_to_vector = |cpal: &Cpal, palette_index| -> Option<Vec<u32>> {
        let start_index: usize = cpal
            .color_record_indices()
            .get(usize::from(palette_index))?
            .get()
            .into();
        let num_entries: usize = cpal.num_palette_entries().into();
        let color_records = cpal.color_records_array()?.ok()?;
        Some(
            color_records
                .get(start_index..start_index + num_entries)?
                .iter()
                .map(|record| {
                    u32::from_be_bytes([record.alpha, record.red, record.green, record.blue])
                })
                .collect(),
        )
    };

    font_ref
        .with_font(|f| {
            let cpal = f.cpal().ok()?;

            let mut palette = cpal_to_vector(&cpal, base_palette).or(cpal_to_vector(&cpal, 0))?;

            for override_entry in palette_overrides {
                let index = override_entry.index as usize;
                if index < palette.len() {
                    palette[index] = override_entry.color_8888;
                }
            }
            Some(palette)
        })
        .unwrap_or_default()
}

pub fn has_colr_glyph(font_ref: &BridgeFontRef, format: ColorGlyphFormat, glyph_id: u16) -> bool {
    font_ref
        .with_font(|f| {
            let colrv1_paintable = f
                .color_glyphs()
                .get_with_format(GlyphId::from(glyph_id), format);
            Some(colrv1_paintable.is_some())
        })
        .unwrap_or_default()
}

pub fn has_colrv1_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    has_colr_glyph(font_ref, ColorGlyphFormat::ColrV1, glyph_id)
}

pub fn has_colrv0_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    has_colr_glyph(font_ref, ColorGlyphFormat::ColrV0, glyph_id)
}

pub fn get_colrv1_clip_box(
    font_ref: &BridgeFontRef,
    coords: &BridgeNormalizedCoords,
    glyph_id: u16,
    size: f32,
    clip_box: &mut ClipBox,
) -> bool {
    let size = match size {
        x if x == 0.0 => {
            return false;
        }
        _ => Size::new(size),
    };
    font_ref
        .with_font(|f| {
            match f
                .color_glyphs()
                .get_with_format(GlyphId::from(glyph_id), ColorGlyphFormat::ColrV1)?
                .bounding_box(coords.normalized_coords.coords(), size)
            {
                Some(bounding_box) => {
                    *clip_box = ClipBox {
                        x_min: bounding_box.x_min,
                        y_min: bounding_box.y_min,
                        x_max: bounding_box.x_max,
                        y_max: bounding_box.y_max,
                    };
                    Some(true)
                }
                _ => None,
            }
        })
        .unwrap_or_default()
}

pub fn draw_colr_glyph(
    font_ref: &BridgeFontRef,
    coords: &BridgeNormalizedCoords,
    glyph_id: u16,
    color_painter: Pin<&mut ColorPainterWrapper>,
) -> bool {
    let mut color_painter_impl = ColorPainterImpl {
        color_painter_wrapper: color_painter,
        // In bounds mode, we do not need to track or forward to the client anything below the
        // first clip layer, as the bounds cannot grow after that.
        clip_level: 0,
    };
    font_ref
        .with_font(|f| {
            let paintable = f.color_glyphs().get(GlyphId::from(glyph_id))?;
            paintable
                .paint(coords.normalized_coords.coords(), &mut color_painter_impl)
                .ok()
        })
        .is_some()
}

pub fn next_color_stop(color_stops: &mut BridgeColorStops, out_stop: &mut ColorStop) -> bool {
    if let Some(color_stop) = color_stops.stops_iterator.next() {
        out_stop.alpha = color_stop.alpha;
        out_stop.stop = color_stop.offset;
        out_stop.palette_index = color_stop.palette_index;
        true
    } else {
        false
    }
}

pub fn num_color_stops(color_stops: &BridgeColorStops) -> usize {
    color_stops.num_stops
}

#[cfg(test)]
mod test {
    use std::fs;

    use crate::{colr::PaletteOverride, font_ref_is_valid, make_font_ref, resolve_palette};

    const TEST_FONT_FILENAME: &str = "resources/fonts/test_glyphs-glyf_colr_1_variable.ttf";

    #[test]
    fn test_palette_override() {
        let file_buffer =
            fs::read(TEST_FONT_FILENAME).expect("COLRv0/v1 test font could not be opened.");
        let font_ref = make_font_ref(&file_buffer, 0);
        assert!(font_ref_is_valid(&font_ref));

        let override_color = 0xFFEEEEEE;
        let valid_overrides = [
            PaletteOverride {
                index: 9,
                color_8888: override_color,
            },
            PaletteOverride {
                index: 10,
                color_8888: override_color,
            },
            PaletteOverride {
                index: 11,
                color_8888: override_color,
            },
        ];

        let palette = resolve_palette(&font_ref, 0, &valid_overrides);

        assert_eq!(palette.len(), 14);
        assert_eq!(palette[9], override_color);
        assert_eq!(palette[10], override_color);
        assert_eq!(palette[11], override_color);

        let out_of_bounds_overrides = [
            PaletteOverride {
                index: 15,
                color_8888: override_color,
            },
            PaletteOverride {
                index: 16,
                color_8888: override_color,
            },
            PaletteOverride {
                index: 17,
                color_8888: override_color,
            },
        ];

        let palette = resolve_palette(&font_ref, 0, &out_of_bounds_overrides);

        assert_eq!(palette.len(), 14);
        assert_eq!(
            (palette[11], palette[12], palette[13],),
            (0xff68c7e8, 0xffffdc01, 0xff808080)
        );
    }

    #[test]
    fn test_default_palette_for_invalid_index() {
        let file_buffer =
            fs::read(TEST_FONT_FILENAME).expect("COLRv0/v1 test font could not be opened.");
        let font_ref = make_font_ref(&file_buffer, 0);
        assert!(font_ref_is_valid(&font_ref));
        let palette = resolve_palette(&font_ref, 65535, &[]);
        assert_eq!(palette.len(), 14);
        assert_eq!(
            (palette[0], palette[6], palette[13],),
            (0xFFFF0000, 0xFFEE82EE, 0xFF808080)
        );
    }
}
