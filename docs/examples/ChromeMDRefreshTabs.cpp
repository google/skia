// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(ChromeMDRefreshTabs, 256, 256, false, 0) {
SkPath GetInteriorPath(
        float scale, const SkISize& size, float endcap_width, float horizontal_inset = 0) {
    const float right = size.fWidth * scale;
    // The bottom of the tab needs to be pixel-aligned or else when we call
    // ClipPath with anti-aliasing enabled it can cause artifacts.
    const float bottom = std::ceil(size.fHeight * scale);

    // const float scaled_horizontal_inset = horizontal_inset * scale;

    const float endcap_radius = endcap_width / 2;

    // Construct the interior path by intersecting paths representing the left
    // and right halves of the tab.  Compared to computing the full path at once,
    // this makes it easier to avoid overdraw in the top center near minimum
    // width, and to implement cases where |horizontal_inset| != 0.
    SkPath right_path;

    right_path.moveTo(right, bottom);
    // right_path.moveTo(right - 1 - scaled_horizontal_inset, bottom);

    right_path.arcTo(endcap_radius, endcap_radius, 90, SkPath::kSmall_ArcSize,
                     SkPathDirection::kCW, right - endcap_radius, bottom - endcap_radius);
    // right_path.rCubicTo(-0.75 * scale, 0, -1.625 * scale, -0.5 * scale,
    // -2 * scale, -1.5 * scale);

    right_path.lineTo(right - endcap_radius, endcap_radius);
    // right_path.lineTo(
    //    right - 1 - scaled_horizontal_inset - (endcap_width - 2) * scale,
    //    2.5 * scale);

    right_path.arcTo(endcap_radius, endcap_radius, 90, SkPath::kSmall_ArcSize,
                     SkPathDirection::kCCW, right - endcap_width, 0);
    // right_path.rCubicTo(-0.375 * scale, -1 * scale, -1.25 * scale, -1.5 * scale,
    //                    -2 * scale, -1.5 * scale);

    right_path.lineTo(0, 0);
    right_path.lineTo(0, bottom);
    right_path.close();

    SkPath left_path;
    // const float scaled_endcap_width = 1 + endcap_width * scale;
    left_path.moveTo(endcap_width, 0);
    // left_path.moveTo(scaled_endcap_width + scaled_horizontal_inset, scale);

    left_path.arcTo(endcap_radius, endcap_radius, 90, SkPath::kSmall_ArcSize,
                    SkPathDirection::kCCW, endcap_radius, endcap_radius);
    // left_path.rCubicTo(-0.75 * scale, 0, -1.625 * scale, 0.5 * scale, -2 * scale,
    //                   1.5 * scale);

    left_path.lineTo(endcap_radius, bottom - endcap_radius);
    // left_path.lineTo(1 + scaled_horizontal_inset + 2 * scale,
    //                 bottom - 1.5 * scale);

    left_path.arcTo(endcap_radius, endcap_radius, 90, SkPath::kSmall_ArcSize,
                    SkPathDirection::kCW, 0, bottom);
    // left_path.rCubicTo(-0.375 * scale, scale, -1.25 * scale, 1.5 * scale,
    //                   -2 * scale, 1.5 * scale);

    left_path.lineTo(right, bottom);
    left_path.lineTo(right, 0);
    left_path.close();

    SkPath complete_path;
    Op(left_path, right_path, SkPathOp::kIntersect_SkPathOp, &complete_path);
    return complete_path;
}

void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1);
    SkPath path = GetInteriorPath(1.f, SkISize::Make(250, 36), 16);
    canvas->drawPath(path, p);

    //  canvas->drawLine(20, 20, 100, 100, p);
}
}  // END FIDDLE
