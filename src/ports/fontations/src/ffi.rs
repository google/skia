// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

mod base;
mod bitmap;
mod colr;
mod hinting;
mod names;
mod verbs_points_pen;

use crate::{
    base::{
        coordinates_for_shifted_named_instance_index, fill_glyph_to_unicode_map,
        font_or_collection, font_ref_is_valid, get_font_style, get_outline_collection,
        get_skia_metrics, get_unscaled_metrics, has_any_color_table, is_embeddable, is_fixed_pitch,
        is_script_style, is_serif_style, is_subsettable, italic_angle, lookup_glyph_or_zero,
        make_font_ref, make_mapping_index, normalized_coords_equal, num_axes, num_glyphs,
        num_named_instances, outline_format, populate_axes, resolve_into_normalized_coords,
        table_data, table_tags, unhinted_advance_width_or_zero, units_per_em_or_zero,
        variation_position, BridgeFontRef, BridgeMappingIndex, BridgeNormalizedCoords,
        BridgeOutlineCollection,
    },
    bitmap::{bitmap_glyph, bitmap_metrics, has_bitmap_glyph, png_data, BridgeBitmapGlyph},
    colr::{
        draw_colr_glyph, get_colrv1_clip_box, has_colrv0_glyph, has_colrv1_glyph, next_color_stop,
        num_color_stops, resolve_palette, BridgeColorStops,
    },
    hinting::{
        get_bridge_glyph_styles, hinting_reliant, make_hinting_instance,
        make_mono_hinting_instance, no_hinting_instance, BridgeGlyphStyles, BridgeHintingInstance,
    },
    names::{
        family_name, get_localized_strings, localized_name_next, postscript_name,
        BridgeLocalizedStrings,
    },
    verbs_points_pen::{get_path_verbs_points, shrink_verbs_points_if_needed},
};

#[cxx::bridge(namespace = "fontations_ffi")]
pub mod ffi {
    pub struct ColorStop {
        stop: f32,
        palette_index: u16,
        alpha: f32,
    }

    #[derive(Default)]
    pub struct Metrics {
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
        underline_position: f32,
        underline_thickness: f32,
        strikeout_position: f32,
        strikeout_thickness: f32,
    }

    #[derive(Clone, Copy, Default, PartialEq)]
    struct FfiPoint {
        x: f32,
        y: f32,
    }

    pub struct BridgeLocalizedName {
        string: String,
        language: String,
    }

    #[derive(PartialEq, Debug, Default)]
    struct SkiaDesignCoordinate {
        axis: u32,
        value: f32,
    }

    struct BridgeScalerMetrics {
        has_overlaps: bool,
        has_adjusted_advance: bool,
        adjusted_advance: f32,
    }

    pub struct PaletteOverride {
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

    // This type is used to mirror SkFontStyle values for Weight, Slant and Width
    #[derive(Default)]
    pub struct BridgeFontStyle {
        pub weight: i32,
        pub slant: i32,
        pub width: i32,
    }

    #[derive(Default)]
    struct BitmapMetrics {
        // Outer glyph bearings that affect the computed bounds. We distinguish
        // those here from `inner_bearing_*` to account for CoreText behavior in
        // SBIX placement. Where the sbix originOffsetX/Y are applied only
        // within the bounds. Specified in font units.
        // 0 for CBDT, CBLC.
        bearing_x: f32,
        bearing_y: f32,
        // Scale factors to scale image to 1em.
        ppem_x: f32,
        ppem_y: f32,
        // Account for the fact that Sbix and CBDT/CBLC have a different origin
        // definition.
        placement_origin_bottom_left: bool,
        // Specified as a pixel value, to be scaled by `ppem_*` as an
        // offset applied to placing the image within the bounds rectangle.
        inner_bearing_x: f32,
        inner_bearing_y: f32,
        // Some, but not all, bitmap glyphs have a special bitmap advance
        advance: f32,
    }

    pub enum AutoHintingControl {
        ForceForGlyf,
        ForceForGlyfAndCff,
        ForceOff,
        Fallback,
    }

    pub enum OutlineFormat {
        NoOutlines,
        Glyf,
        Cff,
        Cff2,
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

        // Optimization to quickly rule out that the font has any color tables.
        fn has_any_color_table(bridge_font_ref: &BridgeFontRef) -> bool;

        type BridgeOutlineCollection<'a>;
        unsafe fn get_outline_collection<'a>(
            font_ref: &'a BridgeFontRef<'a>,
        ) -> Box<BridgeOutlineCollection<'a>>;

        type BridgeGlyphStyles;
        unsafe fn get_bridge_glyph_styles<'a>() -> Box<BridgeGlyphStyles>;

        /// Returns true on a font or collection, sets `num_fonts``
        /// to 0 if single font file, and to > 0 for a TrueType collection.
        /// Returns false if the data cannot be interpreted as a font or collection.
        unsafe fn font_or_collection<'a>(font_data: &'a [u8], num_fonts: &mut u32) -> bool;

        unsafe fn num_named_instances(font_ref: &BridgeFontRef) -> usize;

        type BridgeMappingIndex;
        unsafe fn make_mapping_index<'a>(font_ref: &'a BridgeFontRef) -> Box<BridgeMappingIndex>;

        unsafe fn hinting_reliant<'a>(font_ref: &'a BridgeOutlineCollection) -> bool;

        type BridgeHintingInstance;
        unsafe fn make_hinting_instance<'a>(
            outlines: &BridgeOutlineCollection,
            bridge_glyph_styles: &BridgeGlyphStyles,
            size: f32,
            coords: &BridgeNormalizedCoords,
            do_light_hinting: bool,
            do_lcd_antialiasing: bool,
            lcd_orientation_vertical: bool,
            autohinting_control: AutoHintingControl,
        ) -> Box<BridgeHintingInstance>;
        unsafe fn make_mono_hinting_instance<'a>(
            outlines: &BridgeOutlineCollection,
            size: f32,
            coords: &BridgeNormalizedCoords,
        ) -> Box<BridgeHintingInstance>;
        unsafe fn no_hinting_instance<'a>() -> Box<BridgeHintingInstance>;

        fn lookup_glyph_or_zero(
            font_ref: &BridgeFontRef,
            map: &BridgeMappingIndex,
            codepoint: &[u32],
            glyphs: &mut [u16],
        );

        fn outline_format(outlines: &BridgeOutlineCollection) -> OutlineFormat;

        fn get_path_verbs_points(
            outlines: &BridgeOutlineCollection,
            glyph_id: u16,
            size: f32,
            coords: &BridgeNormalizedCoords,
            hinting_instance: &BridgeHintingInstance,
            verbs: &mut Vec<u8>,
            points: &mut Vec<FfiPoint>,
            scaler_metrics: &mut BridgeScalerMetrics,
        ) -> bool;

        fn shrink_verbs_points_if_needed(verbs: &mut Vec<u8>, points: &mut Vec<FfiPoint>);

        fn unhinted_advance_width_or_zero(
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
        fn get_unscaled_metrics(
            font_ref: &BridgeFontRef,
            coords: &BridgeNormalizedCoords,
        ) -> Metrics;
        fn num_glyphs(font_ref: &BridgeFontRef) -> u16;
        fn fill_glyph_to_unicode_map(font_ref: &BridgeFontRef, map: &mut [u32]);
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

        type BridgeBitmapGlyph<'a>;
        fn has_bitmap_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool;
        unsafe fn bitmap_glyph<'a>(
            font_ref: &'a BridgeFontRef,
            glyph_id: u16,
            font_size: f32,
        ) -> Box<BridgeBitmapGlyph<'a>>;
        unsafe fn png_data<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a [u8];
        unsafe fn bitmap_metrics<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a BitmapMetrics;

        fn table_data(font_ref: &BridgeFontRef, tag: u32, offset: usize, data: &mut [u8]) -> usize;
        fn table_tags(font_ref: &BridgeFontRef, tags: &mut [u32]) -> u16;
        fn variation_position(
            coords: &BridgeNormalizedCoords,
            coordinates: &mut [SkiaDesignCoordinate],
        ) -> isize;
        // Fills the passed-in slice with the axis coordinates for a given
        // shifted named instance index. A shifted named instance index is a
        // 32bit value that contains the index to a named instance left-shifted
        // by 16bits and offset by 1. This mirrors FreeType behavior to smuggle
        // named instance identifiers through a TrueType collection index.
        // Returns the number of coordinates copied. If the slice length is 0,
        // performs no copy but only returns the number of axis coordinates for
        // the given shifted index. Returns -1 on error.
        fn coordinates_for_shifted_named_instance_index(
            font_ref: &BridgeFontRef,
            shifted_index: u32,
            coords: &mut [SkiaDesignCoordinate],
        ) -> isize;

        fn num_axes(font_ref: &BridgeFontRef) -> usize;

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

        fn normalized_coords_equal(a: &BridgeNormalizedCoords, b: &BridgeNormalizedCoords) -> bool;

        fn draw_colr_glyph(
            font_ref: &BridgeFontRef,
            coords: &BridgeNormalizedCoords,
            glyph_id: u16,
            color_painter: Pin<&mut ColorPainterWrapper>,
        ) -> bool;

        type BridgeColorStops<'a>;
        fn next_color_stop(color_stops: &mut BridgeColorStops, stop: &mut ColorStop) -> bool;
        fn num_color_stops(color_stops: &BridgeColorStops) -> usize;

        fn get_font_style(
            font_ref: &BridgeFontRef,
            coords: &BridgeNormalizedCoords,
            font_style: &mut BridgeFontStyle,
        ) -> bool;

        // Additional low-level access functions needed for generateAdvancedMetrics().
        fn is_embeddable(font_ref: &BridgeFontRef) -> bool;
        fn is_subsettable(font_ref: &BridgeFontRef) -> bool;
        fn is_fixed_pitch(font_ref: &BridgeFontRef) -> bool;
        fn is_serif_style(font_ref: &BridgeFontRef) -> bool;
        fn is_script_style(font_ref: &BridgeFontRef) -> bool;
        fn italic_angle(font_ref: &BridgeFontRef) -> i32;
    }

    unsafe extern "C++" {

        include!("src/ports/fontations/src/skpath_bridge.h");

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

        fn is_bounds_mode(self: Pin<&mut ColorPainterWrapper>) -> bool;
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
