// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use std::sync::OnceLock;

use skrifa::{
    outline::{
        Engine, GlyphStyles, HintingInstance, HintingOptions, OutlineGlyphFormat, SmoothMode,
        Target,
    },
    prelude::Size,
};

use crate::{ffi::AutoHintingControl, BridgeNormalizedCoords, BridgeOutlineCollection};

// Rust side lazily created GlyphStyles that are computed when first needed.
// This helps optimize make_hinting_instance in the Fontations ScalerContext.
#[derive(Default)]
pub struct BridgeGlyphStyles {
    glyph_styles: OnceLock<GlyphStyles>,
}

pub struct BridgeHintingInstance(pub Option<HintingInstance>);

pub fn get_bridge_glyph_styles<'a>() -> Box<BridgeGlyphStyles> {
    Box::new(BridgeGlyphStyles::default())
}

pub unsafe fn hinting_reliant<'a>(font_ref: &'a BridgeOutlineCollection) -> bool {
    if let Some(outlines) = &font_ref.0 {
        outlines.require_interpreter()
    } else {
        false
    }
}

pub unsafe fn no_hinting_instance<'a>() -> Box<BridgeHintingInstance> {
    Box::new(BridgeHintingInstance(None))
}

pub unsafe fn make_hinting_instance<'a>(
    outlines: &BridgeOutlineCollection,
    bridge_glyph_styles: &BridgeGlyphStyles,
    size: f32,
    coords: &BridgeNormalizedCoords,
    do_light_hinting: bool,
    do_lcd_antialiasing: bool,
    lcd_orientation_vertical: bool,
    autohinting_control: AutoHintingControl,
) -> Box<BridgeHintingInstance> {
    let hinting_instance = match &outlines.0 {
        Some(outlines) => {
            let smooth_mode = match (
                do_light_hinting,
                do_lcd_antialiasing,
                lcd_orientation_vertical,
            ) {
                (true, _, _) => SmoothMode::Light,
                (false, true, false) => SmoothMode::Lcd,
                (false, true, true) => SmoothMode::VerticalLcd,
                _ => SmoothMode::Normal,
            };

            let hinting_target = Target::Smooth {
                mode: smooth_mode,
                // See https://docs.rs/skrifa/latest/skrifa/outline/enum.Target.html#variant.Smooth.field.mode
                // Configure additional params to match FreeType.
                symmetric_rendering: true,
                preserve_linear_metrics: false,
            };

            // Do not force-autohint for CFF to match FreeType, compare
            // https://gitlab.freedesktop.org/freetype/freetype/-/blob/57617782464411201ce7bbc93b086c1b4d7d84a5/src/base/ftobjs.c#L1001
            // Engine::AutoFallback (see Skrifa docs) means:
            // "Specifically, PostScript (CFF/CFF2) fonts will always use the hinting engine in the
            // PostScript interpreter and TrueType fonts will use the interpreter for TrueType
            // instructions if one of the fpgm or prep tables is non-empty, falling back to the
            // automatic hinter otherwise."
            // So Engine::AutoFallback does not engage autohinting for CFF.
            let engine_type = match (autohinting_control, outlines.format()) {
                (
                    AutoHintingControl::ForceForGlyf,
                    Some(OutlineGlyphFormat::Glyf),
                )
                | (AutoHintingControl::ForceForGlyfAndCff, _) => {
                    let glyph_styles = Some(
                        bridge_glyph_styles
                            .glyph_styles
                            .get_or_init(|| GlyphStyles::new(outlines)),
                    );
                    Engine::Auto(glyph_styles.cloned())
                }
                (AutoHintingControl::ForceOff, _) => Engine::Interpreter,
                _ => Engine::AutoFallback,
            };

            HintingInstance::new(
                outlines,
                Size::new(size),
                &coords.normalized_coords,
                HintingOptions {
                    engine: engine_type,
                    target: hinting_target,
                },
            )
            .ok()
        }
        _ => None,
    };
    Box::new(BridgeHintingInstance(hinting_instance))
}

pub unsafe fn make_mono_hinting_instance<'a>(
    outlines: &BridgeOutlineCollection,
    size: f32,
    coords: &BridgeNormalizedCoords,
) -> Box<BridgeHintingInstance> {
    let hinting_instance = outlines.0.as_ref().and_then(|outlines| {
        HintingInstance::new(
            outlines,
            Size::new(size),
            &coords.normalized_coords,
            skrifa::outline::HintingMode::Strong,
        )
        .ok()
    });
    Box::new(BridgeHintingInstance(hinting_instance))
}
