// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
use cxx;
use font_types::{GlyphId, Pen};
use read_fonts::{FileRef, FontRef, ReadError, TableProvider};
use skrifa::{
    instance::{LocationRef, Size},
    metrics::{GlyphMetrics, Metrics},
    scale::Context,
    string::{LocalizedStrings, StringId},
    MetadataProvider,
};
use std::pin::Pin;

use crate::ffi::SkPathWrapper;

fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16 {
    font_ref
        .0
        .as_ref()
        .and_then(|f| f.charmap().map(codepoint))
        .map_or(0, |id| id.to_u16())
}

fn num_glyphs(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .0
        .as_ref()
        .and_then(|f| f.maxp().ok())
        .map_or(0, |maxp| maxp.num_glyphs())
}

struct PathWrapperPen<'a> {
    path_wrapper: Pin<&'a mut ffi::SkPathWrapper>,
}

// We need to wrap ffi::SkPathWrapper in PathWrapperPen and forward the path
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

fn get_path(
    font_ref: &BridgeFontRef,
    glyph_id: u16,
    size: f32,
    path_wrapper: Pin<&mut SkPathWrapper>,
) -> bool {
    font_ref.0.as_ref().map_or(false, |f| {
        let mut cx = Context::new();
        let mut scaler = cx.new_scaler().size(Size::new(size)).build(f);
        let mut pen_dump = PathWrapperPen {
            path_wrapper: path_wrapper,
        };
        match scaler.outline(GlyphId::new(glyph_id), &mut pen_dump) {
            Ok(_) => true,
            _ => false,
        }
    })
}

fn advance_width_or_zero(font_ref: &BridgeFontRef, size: f32, glyph_id: u16) -> f32 {
    font_ref.0.as_ref().map_or(0.0, |f| {
        GlyphMetrics::new(f, Size::new(size), LocationRef::default())
            .advance_width(GlyphId::new(glyph_id))
            .unwrap_or(0.0)
    })
}

fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .0
        .as_ref()
        .and_then(|f| f.head().ok())
        .map_or(0, |head| head.units_per_em())
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
        avg_char_width: skrifa_metrics.average_width.unwrap_or_else(|| 0.0),
        max_char_width: skrifa_metrics.max_width.unwrap_or_else(|| 0.0),
        x_height: skrifa_metrics.x_height.unwrap_or_else(|| 0.0),
        cap_height: skrifa_metrics.cap_height.unwrap_or_else(|| 0.0),
    }
}

fn get_skia_metrics(font_ref: &BridgeFontRef, size: f32) -> ffi::Metrics {
    font_ref.0.as_ref().map_or(ffi::Metrics::default(), |f| {
        let fontations_metrics = Metrics::new(f, Size::new(size), LocationRef::default());
        convert_metrics(&fontations_metrics)
    })
}

fn get_localized_strings<'a>(font_ref: &'a BridgeFontRef<'a>) -> Box<BridgeLocalizedStrings<'a>> {
    Box::new(BridgeLocalizedStrings {
        localized_strings: match font_ref.0.as_ref() {
            Some(font_ref) => font_ref.localized_strings(StringId::FAMILY_NAME),
            _ => LocalizedStrings::default(),
        },
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
    font_ref.0.as_ref().and_then(|font_ref| {
        font_ref
            .localized_strings(name_id)
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

struct BridgeFontRef<'a>(Option<FontRef<'a>>);

struct BridgeLocalizedStrings<'a> {
    #[allow(dead_code)]
    localized_strings: LocalizedStrings<'a>,
}

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {

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

        fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16;
        fn get_path(
            font_ref: &BridgeFontRef,
            glyph_id: u16,
            size: f32,
            path_wrapper: Pin<&mut SkPathWrapper>,
        ) -> bool;
        fn advance_width_or_zero(font_ref: &BridgeFontRef, size: f32, glyph_id: u16) -> f32;
        fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16;
        fn get_skia_metrics(font_ref: &BridgeFontRef, size: f32) -> Metrics;
        fn num_glyphs(font_ref: &BridgeFontRef) -> u16;
        fn family_name(font_ref: &BridgeFontRef) -> String;
        fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool;

        type BridgeLocalizedStrings<'a>;
        unsafe fn get_localized_strings<'a>(
            font_ref: &'a BridgeFontRef<'a>,
        ) -> Box<BridgeLocalizedStrings<'a>>;
        fn localized_name_next(
            bridge_localized_strings: &mut BridgeLocalizedStrings,
            out_localized_name: &mut BridgeLocalizedName,
        ) -> bool;

    }

    unsafe extern "C++" {

        include!("src/ports/fontations/src/skpath_bridge.h");
        type SkPathWrapper;

        #[allow(dead_code)]
        fn move_to(self: Pin<&mut SkPathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn line_to(self: Pin<&mut SkPathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn quad_to(self: Pin<&mut SkPathWrapper>, cx0: f32, cy0: f32, x: f32, y: f32);
        #[allow(dead_code)]
        fn curve_to(
            self: Pin<&mut SkPathWrapper>,
            cx0: f32,
            cy0: f32,
            cx1: f32,
            cy1: f32,
            x: f32,
            y: f32,
        );
        #[allow(dead_code)]
        fn close(self: Pin<&mut SkPathWrapper>);
        #[allow(dead_code)]
        fn dump(self: Pin<&mut SkPathWrapper>);
    }
}
