// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use skrifa::{
    outline::{DrawSettings, OutlinePen},
    prelude::Size,
    GlyphId,
};

use crate::{
    ffi::{BridgeScalerMetrics, FfiPoint},
    hinting::BridgeHintingInstance,
    BridgeNormalizedCoords, BridgeOutlineCollection,
};

const PATH_EXTRACTION_RESERVE: usize = 150;

pub struct VerbsPointsPen<'a> {
    verbs: &'a mut Vec<u8>,
    points: &'a mut Vec<FfiPoint>,
    started: bool,
    current: FfiPoint,
}

impl FfiPoint {
    fn new(x: f32, y: f32) -> Self {
        Self { x, y }
    }
}

// Values need to match SkPathVerb.
#[repr(u8)]
enum PathVerb {
    MoveTo = 0,
    LineTo = 1,
    QuadTo = 2,
    CubicTo = 4,
    Close = 5,
}

impl<'a> VerbsPointsPen<'a> {
    fn new(verbs: &'a mut Vec<u8>, points: &'a mut Vec<FfiPoint>) -> Self {
        verbs.clear();
        points.clear();
        verbs.reserve(PATH_EXTRACTION_RESERVE);
        points.reserve(PATH_EXTRACTION_RESERVE);
        Self {
            verbs,
            points,
            started: false,
            current: FfiPoint::default(),
        }
    }

    fn going_to(&mut self, point: &FfiPoint) {
        if !self.started {
            self.started = true;
            self.verbs.push(PathVerb::MoveTo as u8);
            self.points.push(self.current);
        }
        self.current = *point;
    }

    fn current_is_not(&self, point: &FfiPoint) -> bool {
        self.current != *point
    }
}

impl<'a> OutlinePen for VerbsPointsPen<'a> {
    fn move_to(&mut self, x: f32, y: f32) {
        let pt0 = FfiPoint::new(x, -y);
        if self.started {
            self.close();
            self.started = false;
        }
        self.current = pt0;
    }

    fn line_to(&mut self, x: f32, y: f32) {
        let pt0 = FfiPoint::new(x, -y);
        if self.current_is_not(&pt0) {
            self.going_to(&pt0);
            self.verbs.push(PathVerb::LineTo as u8);
            self.points.push(pt0);
        }
    }

    fn quad_to(&mut self, cx0: f32, cy0: f32, x: f32, y: f32) {
        let pt0 = FfiPoint::new(cx0, -cy0);
        let pt1 = FfiPoint::new(x, -y);
        if self.current_is_not(&pt0) || self.current_is_not(&pt1) {
            self.going_to(&pt1);
            self.verbs.push(PathVerb::QuadTo as u8);
            self.points.push(pt0);
            self.points.push(pt1);
        }
    }

    fn curve_to(&mut self, cx0: f32, cy0: f32, cx1: f32, cy1: f32, x: f32, y: f32) {
        let pt0 = FfiPoint::new(cx0, -cy0);
        let pt1 = FfiPoint::new(cx1, -cy1);
        let pt2 = FfiPoint::new(x, -y);
        if self.current_is_not(&pt0) || self.current_is_not(&pt1) || self.current_is_not(&pt2) {
            self.going_to(&pt2);
            self.verbs.push(PathVerb::CubicTo as u8);
            self.points.push(pt0);
            self.points.push(pt1);
            self.points.push(pt2);
        }
    }

    fn close(&mut self) {
        if let Some(verb) = self.verbs.last().cloned() {
            if verb == PathVerb::QuadTo as u8
                || verb == PathVerb::CubicTo as u8
                || verb == PathVerb::LineTo as u8
                || verb == PathVerb::MoveTo as u8
            {
                self.verbs.push(PathVerb::Close as u8);
            }
        }
    }
}

pub fn get_path_verbs_points(
    outlines: &BridgeOutlineCollection,
    glyph_id: u16,
    size: f32,
    coords: &BridgeNormalizedCoords,
    hinting_instance: &BridgeHintingInstance,
    verbs: &mut Vec<u8>,
    points: &mut Vec<FfiPoint>,
    scaler_metrics: &mut BridgeScalerMetrics,
) -> bool {
    outlines
        .0
        .as_ref()
        .and_then(|outlines| {
            let glyph = outlines.get(GlyphId::from(glyph_id))?;

            let draw_settings = match &hinting_instance.0 {
                Some(instance) => DrawSettings::hinted(instance, false),
                _ => DrawSettings::unhinted(Size::new(size), &coords.normalized_coords),
            };

            let mut verbs_points_pen = VerbsPointsPen::new(verbs, points);
            match glyph.draw(draw_settings, &mut verbs_points_pen) {
                Err(_) => None,
                Ok(metrics) => {
                    scaler_metrics.has_overlaps = metrics.has_overlaps;
                    (
                        scaler_metrics.has_adjusted_advance,
                        scaler_metrics.adjusted_advance,
                    ) = metrics.advance_width.map_or((false, 0.0), |a| (true, a));
                    Some(())
                }
            }
        })
        .is_some()
}

pub fn shrink_verbs_points_if_needed(verbs: &mut Vec<u8>, points: &mut Vec<FfiPoint>) {
    verbs.shrink_to(PATH_EXTRACTION_RESERVE);
    points.shrink_to(PATH_EXTRACTION_RESERVE);
}
