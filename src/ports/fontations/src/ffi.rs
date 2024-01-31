use ffi::{FillLinearParams, FillRadialParams};
// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
use font_types::{BoundingBox, GlyphId, Pen};
use read_fonts::{tables::colr::CompositeMode, FileRef, FontRef, ReadError, TableProvider};
use skrifa::{
    color::{Brush, ColorGlyphFormat, ColorPainter, Transform},
    instance::{Location, Size},
    metrics::{GlyphMetrics, Metrics},
    outline::DrawSettings,
    setting::VariationSetting,
    string::{LocalizedStrings, StringId},
    MetadataProvider, OutlineGlyphCollection, Tag,
};
use std::pin::Pin;

use crate::ffi::{
    AxisWrapper, BridgeScalerMetrics, ColorPainterWrapper, ColorStop, PaletteOverride, PathWrapper,
    SkiaDesignCoordinate,
};

fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16 {
    font_ref
        .with_font(|f| Some(f.charmap().map(codepoint)?.to_u16()))
        .unwrap_or_default()
}

fn num_glyphs(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| Some(f.maxp().ok()?.num_glyphs()))
        .unwrap_or_default()
}

struct PathWrapperPen<'a> {
    path_wrapper: Pin<&'a mut ffi::PathWrapper>,
}

// We need to wrap ffi::PathWrapper in PathWrapperPen and forward the path
// recording calls to the path wrapper as we can't define trait implementations
// inside the cxx::bridge section.
impl<'a> Pen for PathWrapperPen<'a> {
    fn move_to(&mut self, x: f32, y: f32) {
        self.path_wrapper.as_mut().move_to(x, -y);
    }

    fn line_to(&mut self, x: f32, y: f32) {
        self.path_wrapper.as_mut().line_to(x, -y);
    }

    fn quad_to(&mut self, cx0: f32, cy0: f32, x: f32, y: f32) {
        self.path_wrapper.as_mut().quad_to(cx0, -cy0, x, -y);
    }

    fn curve_to(&mut self, cx0: f32, cy0: f32, cx1: f32, cy1: f32, x: f32, y: f32) {
        self.path_wrapper
            .as_mut()
            .curve_to(cx0, -cy0, cx1, cy1, x, -y);
    }

    fn close(&mut self) {
        self.path_wrapper.as_mut().close();
    }
}

struct ColorPainterImpl<'a> {
    color_painter_wrapper: Pin<&'a mut ffi::ColorPainterWrapper>,
}

impl<'a> ColorPainter for ColorPainterImpl<'a> {
    fn push_transform(&mut self, transform: Transform) {
        self.color_painter_wrapper
            .as_mut()
            .push_transform(&ffi::Transform {
                xx: transform.xx,
                xy: transform.xy,
                yx: transform.yx,
                yy: transform.yy,
                dx: transform.dx,
                dy: transform.dy,
            });
    }

    fn pop_transform(&mut self) {
        self.color_painter_wrapper.as_mut().pop_transform();
    }

    fn push_clip_glyph(&mut self, glyph: GlyphId) {
        self.color_painter_wrapper
            .as_mut()
            .push_clip_glyph(glyph.to_u16());
    }

    fn push_clip_box(&mut self, clip_box: BoundingBox<f32>) {
        self.color_painter_wrapper.as_mut().push_clip_rectangle(
            clip_box.x_min,
            clip_box.y_min,
            clip_box.x_max,
            clip_box.y_max,
        );
    }

    fn pop_clip(&mut self) {
        self.color_painter_wrapper.as_mut().pop_clip();
    }

    fn fill(&mut self, fill_type: Brush) {
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
                        r0: r0,
                        x1: c1.x,
                        y1: c1.y,
                        r1: r1,
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
                    &ffi::FillSweepParams {
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
        let color_painter = self.color_painter_wrapper.as_mut();
        let brush_transform = brush_transform.unwrap_or_default();
        match brush {
            Brush::Solid {
                palette_index,
                alpha,
            } => {
                color_painter.fill_glyph_solid(glyph.to_u16(), palette_index, alpha);
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
                    glyph.to_u16(),
                    &ffi::Transform {
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
                    glyph.to_u16(),
                    &ffi::Transform {
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
                        r0: r0,
                        x1: c1.x,
                        y1: c1.y,
                        r1: r1,
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
                    glyph.to_u16(),
                    &ffi::Transform {
                        xx: brush_transform.xx,
                        xy: brush_transform.xy,
                        yx: brush_transform.yx,
                        yy: brush_transform.yy,
                        dx: brush_transform.dx,
                        dy: brush_transform.dy,
                    },
                    &ffi::FillSweepParams {
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
        self.color_painter_wrapper
            .as_mut()
            .push_layer(composite_mode as u8);
    }
    fn pop_layer(&mut self) {
        self.color_painter_wrapper.as_mut().pop_layer();
    }
}

fn get_path(
    outlines: &BridgeOutlineCollection,
    glyph_id: u16,
    size: f32,
    coords: &BridgeNormalizedCoords,
    path_wrapper: Pin<&mut PathWrapper>,
    scaler_metrics: &mut BridgeScalerMetrics,
) -> bool {
    outlines
        .0
        .as_ref()
        .map(|outlines| {
            let glyph = outlines.get(GlyphId::new(glyph_id))?;
            let draw_settings = DrawSettings::unhinted(Size::new(size), &coords.normalized_coords);

            let mut pen_dump = PathWrapperPen {
                path_wrapper: path_wrapper,
            };
            match glyph.draw(draw_settings, &mut pen_dump) {
                Err(_) => None,
                Ok(metrics) => {
                    scaler_metrics.has_overlaps = metrics.has_overlaps;
                    Some(())
                }
            }
        })
        .is_some()
}

fn advance_width_or_zero(
    font_ref: &BridgeFontRef,
    size: f32,
    coords: &BridgeNormalizedCoords,
    glyph_id: u16,
) -> f32 {
    font_ref
        .with_font(|f| {
            GlyphMetrics::new(f, Size::new(size), coords.normalized_coords.coords())
                .advance_width(GlyphId::new(glyph_id))
        })
        .unwrap_or_default()
}

fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| Some(f.head().ok()?.units_per_em()))
        .unwrap_or_default()
}

fn convert_metrics(skrifa_metrics: &Metrics) -> ffi::Metrics {
    ffi::Metrics {
        top: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.y_max),
        bottom: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.y_min),
        x_min: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.x_min),
        x_max: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.x_max),
        ascent: skrifa_metrics.ascent,
        descent: skrifa_metrics.descent,
        leading: skrifa_metrics.leading,
        avg_char_width: skrifa_metrics.average_width.unwrap_or(0.0),
        max_char_width: skrifa_metrics.max_width.unwrap_or(0.0),
        x_height: skrifa_metrics.x_height.unwrap_or(0.0),
        cap_height: skrifa_metrics.cap_height.unwrap_or(0.0),
    }
}

fn get_skia_metrics(
    font_ref: &BridgeFontRef,
    size: f32,
    coords: &BridgeNormalizedCoords,
) -> ffi::Metrics {
    font_ref
        .with_font(|f| {
            let fontations_metrics =
                Metrics::new(f, Size::new(size), coords.normalized_coords.coords());
            Some(convert_metrics(&fontations_metrics))
        })
        .unwrap_or_default()
}

fn get_localized_strings<'a>(font_ref: &'a BridgeFontRef<'a>) -> Box<BridgeLocalizedStrings<'a>> {
    Box::new(BridgeLocalizedStrings {
        localized_strings: font_ref
            .with_font(|f| Some(f.localized_strings(StringId::FAMILY_NAME)))
            .unwrap_or_default(),
    })
}

use crate::ffi::BridgeLocalizedName;

fn localized_name_next(
    bridge_localized_strings: &mut BridgeLocalizedStrings,
    out_localized_name: &mut BridgeLocalizedName,
) -> bool {
    match bridge_localized_strings.localized_strings.next() {
        Some(localized_string) => {
            out_localized_name.string = localized_string.to_string();
            // TODO(b/307906051): Remove the suffix before shipping.
            out_localized_name.string.push_str(" (Fontations)");
            out_localized_name.language = localized_string
                .language()
                .map(|l| l.to_string())
                .unwrap_or_default();
            true
        }
        _ => false,
    }
}

fn english_or_first_font_name(font_ref: &BridgeFontRef, name_id: StringId) -> Option<String> {
    font_ref.with_font(|f| {
        f.localized_strings(name_id)
            .english_or_first()
            .map(|localized_string| localized_string.to_string())
    })
}

fn family_name(font_ref: &BridgeFontRef) -> String {
    english_or_first_font_name(font_ref, StringId::FAMILY_NAME).unwrap_or_default()
}

fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool {
    let postscript_name = english_or_first_font_name(font_ref, StringId::POSTSCRIPT_NAME);
    match postscript_name {
        Some(name) => {
            *out_string = name;
            true
        }
        _ => false,
    }
}

fn resolve_palette(
    font_ref: &BridgeFontRef,
    base_palette: u16,
    palette_overrides: &[PaletteOverride],
) -> Vec<u32> {
    font_ref
        .with_font(|f| {
            let cpal = f.cpal().ok()?;

            let start_index: usize = cpal
                .color_record_indices()
                .get(usize::from(base_palette))?
                .get()
                .into();
            let num_entries: usize = cpal.num_palette_entries().into();

            let color_records = cpal.color_records_array()?.ok()?;
            let mut palette: Vec<u32> = color_records
                .get(start_index..start_index + num_entries)?
                .iter()
                .map(|record| {
                    u32::from_be_bytes([record.alpha, record.red, record.green, record.blue])
                })
                .collect();

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

fn has_colr_glyph(font_ref: &BridgeFontRef, format: ColorGlyphFormat, glyph_id: u16) -> bool {
    font_ref
        .with_font(|f| {
            let colrv1_paintable = f
                .color_glyphs()
                .get_with_format(GlyphId::new(glyph_id), format);
            Some(colrv1_paintable.is_some())
        })
        .unwrap_or_default()
}

fn has_colrv1_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    has_colr_glyph(font_ref, ColorGlyphFormat::ColrV1, glyph_id)
}

fn has_colrv0_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    has_colr_glyph(font_ref, ColorGlyphFormat::ColrV0, glyph_id)
}

use crate::ffi::ClipBox;

fn get_colrv1_clip_box(
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
                .get_with_format(GlyphId::new(glyph_id), ColorGlyphFormat::ColrV1)?
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

/// Implements the behavior expected for `SkTypeface::getTableData`, compare
/// documentation for this method and the FreeType implementation in Skia.
/// * If the target data array is empty, do not copy any data into it, but
///   return the size of the table.
/// * If the target data buffer is shorted than from offset to the end of the
///   table, truncate the data.
/// * If offset is longer than the table's length, return 0.
fn table_data(font_ref: &BridgeFontRef, tag: u32, offset: usize, data: &mut [u8]) -> usize {
    let table_data = font_ref
        .with_font(|f| f.table_data(Tag::from_be_bytes(tag.to_be_bytes())))
        .unwrap_or_default();
    let table_data = table_data.as_ref();
    // Remaining table data size measured from offset to end, or 0 if offset is
    // too large.
    let mut to_copy_length = table_data.len().saturating_sub(offset);
    match data.len() {
        0 => to_copy_length,
        _ => {
            to_copy_length = to_copy_length.min(data.len());
            let table_offset_data = table_data
                .get(offset..offset + to_copy_length)
                .unwrap_or_default();
            data.get_mut(..table_offset_data.len())
                .map_or(0, |data_slice| {
                    data_slice.copy_from_slice(table_offset_data);
                    data_slice.len()
                })
        }
    }
}

fn table_tags(font_ref: &BridgeFontRef, tags: &mut [u32]) -> u16 {
    return font_ref
        .with_font(|f| {
            let table_directory = &f.table_directory;
            let table_tags_iter = table_directory
                .table_records()
                .iter()
                .map(|table| u32::from_be_bytes(table.tag.get().into_bytes()));
            tags.iter_mut()
                .zip(table_tags_iter)
                .for_each(|(out_tag, table_tag)| *out_tag = table_tag);
            Some(table_directory.num_tables())
        })
        .unwrap_or_default();
}

fn variation_position(
    coords: &BridgeNormalizedCoords,
    coordinates: &mut [SkiaDesignCoordinate],
) -> isize {
    if !coordinates.is_empty() {
        if coords.filtered_user_coords.len() > coordinates.len() {
            return -1;
        }
        let skia_design_coordinates =
            coords
                .filtered_user_coords
                .iter()
                .map(|setting| SkiaDesignCoordinate {
                    axis: u32::from_be_bytes(setting.selector.into_bytes()),
                    value: setting.value,
                });
        for (i, coord) in skia_design_coordinates.enumerate() {
            coordinates[i] = coord;
        }
    }
    coords.filtered_user_coords.len().try_into().unwrap()
}

fn populate_axes(font_ref: &BridgeFontRef, mut axis_wrapper: Pin<&mut AxisWrapper>) -> isize {
    font_ref
        .with_font(|f| {
            let axes = f.axes();
            // Populate incoming allocated SkFontParameters::Variation::Axis[] only when a
            // buffer is passed.
            if axis_wrapper.as_ref().size() > 0 {
                for (i, axis) in axes.iter().enumerate() {
                    if !axis_wrapper.as_mut().populate_axis(
                        i,
                        u32::from_be_bytes(axis.tag().into_bytes()),
                        axis.min_value(),
                        axis.default_value(),
                        axis.max_value(),
                        axis.is_hidden(),
                    ) {
                        return None;
                    }
                }
            }
            isize::try_from(axes.len()).ok()
        })
        .unwrap_or(-1)
}

fn make_font_ref_internal<'a>(font_data: &'a [u8], index: u32) -> Result<FontRef<'a>, ReadError> {
    match FileRef::new(font_data) {
        Ok(file_ref) => match file_ref {
            FileRef::Font(font_ref) => Ok(font_ref),
            FileRef::Collection(collection) => collection.get(index),
        },
        Err(e) => Err(e),
    }
}

fn make_font_ref<'a>(font_data: &'a [u8], index: u32) -> Box<BridgeFontRef<'a>> {
    Box::new(BridgeFontRef(make_font_ref_internal(font_data, index).ok()))
}

fn font_ref_is_valid(bridge_font_ref: &BridgeFontRef) -> bool {
    bridge_font_ref.0.is_some()
}

fn get_outline_collection<'a>(font_ref: &'a BridgeFontRef<'a>) -> Box<BridgeOutlineCollection<'a>> {
    Box::new(
        font_ref
            .with_font(|f| Some(BridgeOutlineCollection(Some(f.outline_glyphs()))))
            .unwrap_or_default(),
    )
}

fn font_or_collection<'a>(font_data: &'a [u8], num_fonts: &mut u32) -> bool {
    match FileRef::new(font_data) {
        Ok(FileRef::Collection(collection)) => {
            *num_fonts = collection.len();
            true
        }
        Ok(FileRef::Font(_)) => {
            *num_fonts = 0u32;
            true
        }
        _ => false,
    }
}

fn resolve_into_normalized_coords(
    font_ref: &BridgeFontRef,
    design_coords: &[SkiaDesignCoordinate],
) -> Box<BridgeNormalizedCoords> {
    let variation_tuples = design_coords
        .iter()
        .map(|coord| (Tag::from_be_bytes(coord.axis.to_be_bytes()), coord.value));
    let bridge_normalized_coords = font_ref
        .with_font(|f| {
            Some(BridgeNormalizedCoords {
                filtered_user_coords: f.axes().filter(variation_tuples.clone()).collect(),
                normalized_coords: f.axes().location(variation_tuples),
            })
        })
        .unwrap_or_default();
    Box::new(bridge_normalized_coords)
}

fn draw_colr_glyph(
    font_ref: &BridgeFontRef,
    coords: &BridgeNormalizedCoords,
    glyph_id: u16,
    color_painter: Pin<&mut ColorPainterWrapper>,
) -> bool {
    let mut color_painter_impl = ColorPainterImpl {
        color_painter_wrapper: color_painter,
    };
    font_ref
        .with_font(|f| {
            let paintable = f.color_glyphs().get(GlyphId::new(glyph_id))?;
            paintable
                .paint(coords.normalized_coords.coords(), &mut color_painter_impl)
                .ok()
        })
        .is_some()
}

fn next_color_stop(color_stops: &mut BridgeColorStops, out_stop: &mut ColorStop) -> bool {
    if let Some(color_stop) = color_stops.stops_iterator.next() {
        out_stop.alpha = color_stop.alpha;
        out_stop.stop = color_stop.offset;
        out_stop.palette_index = color_stop.palette_index;
        return true;
    } else {
        return false;
    }
}

fn num_color_stops(color_stops: &BridgeColorStops) -> usize {
    return color_stops.num_stops;
}

struct BridgeFontRef<'a>(Option<FontRef<'a>>);

impl<'a> BridgeFontRef<'a> {
    fn with_font<T>(&'a self, f: impl FnOnce(&'a FontRef) -> Option<T>) -> Option<T> {
        f(self.0.as_ref()?)
    }
}

#[derive(Default)]
struct BridgeOutlineCollection<'a>(Option<OutlineGlyphCollection<'a>>);

#[derive(Default)]
struct BridgeNormalizedCoords {
    normalized_coords: Location,
    filtered_user_coords: Vec<VariationSetting>,
}

struct BridgeLocalizedStrings<'a> {
    #[allow(dead_code)]
    localized_strings: LocalizedStrings<'a>,
}

pub struct BridgeColorStops<'a> {
    pub stops_iterator: Box<dyn Iterator<Item = &'a skrifa::color::ColorStop> + 'a>,
    pub num_stops: usize,
}

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {
    struct ColorStop {
        stop: f32,
        palette_index: u16,
        alpha: f32,
    }

    #[derive(Default)]
    struct Metrics {
        top: f32,
        ascent: f32,
        descent: f32,
        bottom: f32,
        leading: f32,
        avg_char_width: f32,
        max_char_width: f32,
        x_min: f32,
        x_max: f32,
        x_height: f32,
        cap_height: f32,
    }

    struct BridgeLocalizedName {
        string: String,
        language: String,
    }

    struct SkiaDesignCoordinate {
        axis: u32,
        value: f32,
    }

    struct BridgeScalerMetrics {
        has_overlaps: bool,
    }

    struct PaletteOverride {
        index: u16,
        color_8888: u32,
    }

    struct ClipBox {
        x_min: f32,
        y_min: f32,
        x_max: f32,
        y_max: f32,
    }

    struct Transform {
        xx: f32,
        xy: f32,
        yx: f32,
        yy: f32,
        dx: f32,
        dy: f32,
    }

    struct FillLinearParams {
        x0: f32,
        y0: f32,
        x1: f32,
        y1: f32,
    }

    struct FillRadialParams {
        x0: f32,
        y0: f32,
        r0: f32,
        x1: f32,
        y1: f32,
        r1: f32,
    }

    struct FillSweepParams {
        x0: f32,
        y0: f32,
        start_angle: f32,
        end_angle: f32,
    }

    extern "Rust" {
        type BridgeFontRef<'a>;
        unsafe fn make_font_ref<'a>(font_data: &'a [u8], index: u32) -> Box<BridgeFontRef<'a>>;
        // Returns whether BridgeFontRef is a valid font containing at
        // least a valid sfnt structure from which tables can be
        // accessed. This is what instantiation in make_font_ref checks
        // for. (see FontRef::new in read_fonts's lib.rs). Implemented
        // by returning whether the option is Some() and thus whether a
        // FontRef instantiation succeeded and a table directory was
        // accessible.
        fn font_ref_is_valid(bridge_font_ref: &BridgeFontRef) -> bool;

        type BridgeOutlineCollection<'a>;
        unsafe fn get_outline_collection<'a>(
            font_ref: &'a BridgeFontRef<'a>,
        ) -> Box<BridgeOutlineCollection<'a>>;

        /// Returns true on a font or collection, sets `num_fonts``
        /// to 0 if single font file, and to > 0 for a TrueType collection.
        /// Returns false if the data cannot be interpreted as a font or collection.
        unsafe fn font_or_collection<'a>(font_data: &'a [u8], num_fonts: &mut u32) -> bool;

        fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16;
        fn get_path(
            outlines: &BridgeOutlineCollection,
            glyph_id: u16,
            size: f32,
            coords: &BridgeNormalizedCoords,
            path_wrapper: Pin<&mut PathWrapper>,
            scaler_metrics: &mut BridgeScalerMetrics,
        ) -> bool;
        fn advance_width_or_zero(
            font_ref: &BridgeFontRef,
            size: f32,
            coords: &BridgeNormalizedCoords,
            glyph_id: u16,
        ) -> f32;
        fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16;
        fn get_skia_metrics(
            font_ref: &BridgeFontRef,
            size: f32,
            coords: &BridgeNormalizedCoords,
        ) -> Metrics;
        fn num_glyphs(font_ref: &BridgeFontRef) -> u16;
        fn family_name(font_ref: &BridgeFontRef) -> String;
        fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool;

        /// Receives a slice of palette overrides that will be merged
        /// with the specified base palette of the font. The result is a
        /// palette of RGBA, 8-bit per component, colors, consisting of
        /// palette entries merged with overrides.
        fn resolve_palette(
            font_ref: &BridgeFontRef,
            base_palette: u16,
            palette_overrides: &[PaletteOverride],
        ) -> Vec<u32>;

        fn has_colrv1_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool;
        fn has_colrv0_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool;
        fn get_colrv1_clip_box(
            font_ref: &BridgeFontRef,
            coords: &BridgeNormalizedCoords,
            glyph_id: u16,
            size: f32,
            clip_box: &mut ClipBox,
        ) -> bool;

        fn table_data(font_ref: &BridgeFontRef, tag: u32, offset: usize, data: &mut [u8]) -> usize;
        fn table_tags(font_ref: &BridgeFontRef, tags: &mut [u32]) -> u16;
        fn variation_position(
            coords: &BridgeNormalizedCoords,
            coordinates: &mut [SkiaDesignCoordinate],
        ) -> isize;

        fn populate_axes(font_ref: &BridgeFontRef, axis_wrapper: Pin<&mut AxisWrapper>) -> isize;

        type BridgeLocalizedStrings<'a>;
        unsafe fn get_localized_strings<'a>(
            font_ref: &'a BridgeFontRef<'a>,
        ) -> Box<BridgeLocalizedStrings<'a>>;
        fn localized_name_next(
            bridge_localized_strings: &mut BridgeLocalizedStrings,
            out_localized_name: &mut BridgeLocalizedName,
        ) -> bool;

        type BridgeNormalizedCoords;
        fn resolve_into_normalized_coords(
            font_ref: &BridgeFontRef,
            design_coords: &[SkiaDesignCoordinate],
        ) -> Box<BridgeNormalizedCoords>;

        fn draw_colr_glyph(
            font_ref: &BridgeFontRef,
            coords: &BridgeNormalizedCoords,
            glyph_id: u16,
            color_painter: Pin<&mut ColorPainterWrapper>,
        ) -> bool;

        type BridgeColorStops<'a>;
        fn next_color_stop(color_stops: &mut BridgeColorStops, stop: &mut ColorStop) -> bool;
        fn num_color_stops(color_stops: &BridgeColorStops) -> usize;

    }

    unsafe extern "C++" {

        include!("src/ports/fontations/src/skpath_bridge.h");

        type PathWrapper;

        #[allow(dead_code)]
        fn move_to(self: Pin<&mut PathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn line_to(self: Pin<&mut PathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn quad_to(self: Pin<&mut PathWrapper>, cx0: f32, cy0: f32, x: f32, y: f32);
        #[allow(dead_code)]
        fn curve_to(
            self: Pin<&mut PathWrapper>,
            cx0: f32,
            cy0: f32,
            cx1: f32,
            cy1: f32,
            x: f32,
            y: f32,
        );
        #[allow(dead_code)]
        fn close(self: Pin<&mut PathWrapper>);

        type AxisWrapper;

        fn populate_axis(
            self: Pin<&mut AxisWrapper>,
            i: usize,
            axis: u32,
            min: f32,
            def: f32,
            max: f32,
            hidden: bool,
        ) -> bool;
        fn size(self: Pin<&AxisWrapper>) -> usize;

        type ColorPainterWrapper;

        fn push_transform(self: Pin<&mut ColorPainterWrapper>, transform: &Transform);
        fn pop_transform(self: Pin<&mut ColorPainterWrapper>);
        fn push_clip_glyph(self: Pin<&mut ColorPainterWrapper>, glyph_id: u16);
        fn push_clip_rectangle(
            self: Pin<&mut ColorPainterWrapper>,
            x_min: f32,
            y_min: f32,
            x_max: f32,
            y_max: f32,
        );
        fn pop_clip(self: Pin<&mut ColorPainterWrapper>);

        fn fill_solid(self: Pin<&mut ColorPainterWrapper>, palette_index: u16, alpha: f32);
        fn fill_linear(
            self: Pin<&mut ColorPainterWrapper>,
            fill_linear_params: &FillLinearParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );
        fn fill_radial(
            self: Pin<&mut ColorPainterWrapper>,
            fill_radial_params: &FillRadialParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );
        fn fill_sweep(
            self: Pin<&mut ColorPainterWrapper>,
            fill_sweep_params: &FillSweepParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );

        // Optimized functions.
        fn fill_glyph_solid(
            self: Pin<&mut ColorPainterWrapper>,
            glyph_id: u16,
            palette_index: u16,
            alpha: f32,
        );
        fn fill_glyph_linear(
            self: Pin<&mut ColorPainterWrapper>,
            glyph_id: u16,
            fill_transform: &Transform,
            fill_linear_params: &FillLinearParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );
        fn fill_glyph_radial(
            self: Pin<&mut ColorPainterWrapper>,
            glyph_id: u16,
            fill_transform: &Transform,
            fill_radial_params: &FillRadialParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );
        fn fill_glyph_sweep(
            self: Pin<&mut ColorPainterWrapper>,
            glyph_id: u16,
            fill_transform: &Transform,
            fill_sweep_params: &FillSweepParams,
            color_stops: &mut BridgeColorStops,
            extend_mode: u8,
        );

        fn push_layer(self: Pin<&mut ColorPainterWrapper>, colrv1_composite_mode: u8);
        fn pop_layer(self: Pin<&mut ColorPainterWrapper>);

    }
}

/// Tests to exercise COLR and CPAL parts of the Fontations FFI.
/// Run using `$ bazel test --with_fontations //src/ports/fontations:test_ffi`
#[cfg(test)]
mod test {

    use crate::{
        ffi::PaletteOverride, font_or_collection, font_ref_is_valid, make_font_ref, resolve_palette,
    };
    use std::fs;

    const TEST_FONT_FILENAME: &str = "resources/fonts/test_glyphs-glyf_colr_1_variable.ttf";
    const TEST_COLLECTION_FILENAME: &str = "resources/fonts/test.ttc";

    #[test]
    fn test_palette_override() {
        let file_buffer =
            fs::read(TEST_FONT_FILENAME).expect("COLRv0/v1 test font could not be opened.");
        let font_ref = make_font_ref(&file_buffer, 0);
        assert_eq!(font_ref_is_valid(&font_ref), true);

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

        let no_palette = resolve_palette(&font_ref, 10, &out_of_bounds_overrides);
        assert_eq!(no_palette.len(), 0);
    }

    #[test]
    fn test_num_fonts_in_collection() {
        let collection_buffer = fs::read(TEST_COLLECTION_FILENAME)
            .expect("Unable to open TrueType collection test file.");
        let font_buffer =
            fs::read(TEST_FONT_FILENAME).expect("COLRv0/v1 test font could not be opened.");
        let garbage: [u8; 12] = [
            b'0', b'a', b'b', b'0', b'a', b'b', b'0', b'a', b'b', b'0', b'a', b'b',
        ];

        let mut num_fonts = 0;
        let result_collection = font_or_collection(&collection_buffer, &mut num_fonts);
        assert!(result_collection && num_fonts == 2);

        let result_font_file = font_or_collection(&font_buffer, &mut num_fonts);
        assert!(result_font_file);
        assert!(num_fonts == 0u32);

        let result_garbage = font_or_collection(&garbage, &mut num_fonts);
        assert!(!result_garbage);
    }
}
