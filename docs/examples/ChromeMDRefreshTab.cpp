// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(ChromeMDRefreshTab, 256, 256, false, 0) {
SkPath GetBorderPath(float scale,
                     bool unscale_at_end,
                     bool extend_to_top,
                     float endcap_width,
                     const SkISize& size) {
    //  const float top = scale - 1;
    const float right = size.fWidth * scale;
    const float bottom = size.fHeight * scale;
    const float scaled_endcap_radius = (endcap_width / 2) * scale;

    SkPath path;
    path.moveTo(0, bottom - 1);
    // bottom left
    path.arcTo(scaled_endcap_radius - 1, scaled_endcap_radius - 1, 90, SkPath::kSmall_ArcSize,
               SkPathDirection::kCCW, scaled_endcap_radius - 1,
               bottom - 1 - scaled_endcap_radius);
    // path.rLineTo(0, -1);
    // path.rCubicTo(0.75 * scale, 0, 1.625 * scale, -0.5 * scale, 2 * scale,
    //               -1.5 * scale);
    path.lineTo(scaled_endcap_radius - 1, scaled_endcap_radius + 1);
    // path.lineTo((endcap_width - 2) * scale, top + 1.5 * scale);
    // top left
    path.arcTo(scaled_endcap_radius + 1, scaled_endcap_radius + 1, 90, SkPath::kSmall_ArcSize,
               SkPathDirection::kCW, scaled_endcap_radius * 2, -1);

    // if (extend_to_top) {
    // Create the vertical extension by extending the side diagonals until
    // they reach the top of the bounds.
    //  const float dy = 2.5 * scale - 1;
    //  const float dx = Tab::GetInverseDiagonalSlope() * dy;
    //  path.rLineTo(dx, -dy);
    //  path.lineTo(right - (endcap_width - 2) * scale - dx, 0);
    //  path.rLineTo(dx, dy);
    //} else {
    //  path.rCubicTo(0.375 * scale, -scale, 1.25 * scale, -1.5 * scale, 2 * scale,
    //                -1.5 * scale);
    //  path.lineTo(right - endcap_width * scale, top);
    //  path.rCubicTo(0.75 * scale, 0, 1.625 * scale, 0.5 * scale, 2 * scale,
    //                1.5 * scale);
    //}
    path.lineTo(right - scaled_endcap_radius * 2, -1);
    // path.lineTo(right - 2 * scale, bottom - 1 - 1.5 * scale);

    // top right
    path.arcTo(scaled_endcap_radius + 1, scaled_endcap_radius + 1, 90, SkPath::kSmall_ArcSize,
               SkPathDirection::kCW, right - scaled_endcap_radius + 1,
               scaled_endcap_radius + 1);
    // path.arcTo(SkRect::MakeLTRB(right - 2 * scale, bottom - 1 - 1.5 * scale,
    //    right - 2 * scale + 4, bottom - 1 - 1.5 * scale + 4), 90, 90, true);
    // path.rCubicTo(0.375 * scale, scale, 1.25 * scale, 1.5 * scale, 2 * scale,
    //              1.5 * scale);

    path.lineTo(right - scaled_endcap_radius + 1, bottom - 1 - scaled_endcap_radius);
    // path.rLineTo(0, 1);

    // bottom right
    path.arcTo(scaled_endcap_radius - 1, scaled_endcap_radius - 1, 90, SkPath::kSmall_ArcSize,
               SkPathDirection::kCCW, right, bottom - 1);
    path.close();

    if (unscale_at_end && (scale != 1)) path.transform(SkMatrix::Scale(1.f / scale,
                                                                       1.f / scale));

    return path;
}

SkPath GetInteriorPath(
        float scale, const SkISize& size, float endcap_width, float horizontal_inset = 0) {
    const float right = size.fWidth * scale;
    // The bottom of the tab needs to be pixel-aligned or else when we call
    // ClipPath with anti-aliasing enabled it can cause artifacts.
    const float bottom = std::ceil(size.fHeight * scale);

    // const float scaled_horizontal_inset = horizontal_inset * scale;

    const float scaled_endcap_radius = (endcap_width / 2) * scale;

    // Construct the interior path by intersecting paths representing the left
    // and right halves of the tab.  Compared to computing the full path at once,
    // this makes it easier to avoid overdraw in the top center near minimum
    // width, and to implement cases where |horizontal_inset| != 0.
    SkPath right_path;

    right_path.moveTo(right, bottom);
    // right_path.moveTo(right - 1 - scaled_horizontal_inset, bottom);

    right_path.arcTo(scaled_endcap_radius, scaled_endcap_radius, 90, SkPath::kSmall_ArcSize,
                     SkPathDirection::kCW, right - scaled_endcap_radius,
                     bottom - scaled_endcap_radius);
    // right_path.rCubicTo(-0.75 * scale, 0, -1.625 * scale, -0.5 * scale,
    // -2 * scale, -1.5 * scale);

    right_path.lineTo(right - scaled_endcap_radius, scaled_endcap_radius);
    // right_path.lineTo(
    //    right - 1 - scaled_horizontal_inset - (endcap_width - 2) * scale,
    //    2.5 * scale);

    right_path.arcTo(scaled_endcap_radius, scaled_endcap_radius, 90, SkPath::kSmall_ArcSize,
                     SkPathDirection::kCCW, right - scaled_endcap_radius * 2, 0);
    // right_path.rCubicTo(-0.375 * scale, -1 * scale, -1.25 * scale, -1.5 * scale,
    //                    -2 * scale, -1.5 * scale);

    right_path.lineTo(0, 0);
    right_path.lineTo(0, bottom);
    right_path.close();

    SkPath left_path;
    // const float scaled_endcap_width = 1 + endcap_width * scale;
    left_path.moveTo(scaled_endcap_radius * 2, 0);
    // left_path.moveTo(scaled_endcap_width + scaled_horizontal_inset, scale);

    left_path.arcTo(scaled_endcap_radius, scaled_endcap_radius, 90, SkPath::kSmall_ArcSize,
                    SkPathDirection::kCCW, scaled_endcap_radius, scaled_endcap_radius);
    // left_path.rCubicTo(-0.75 * scale, 0, -1.625 * scale, 0.5 * scale, -2 * scale,
    //                   1.5 * scale);

    left_path.lineTo(scaled_endcap_radius, bottom - scaled_endcap_radius);
    // left_path.lineTo(1 + scaled_horizontal_inset + 2 * scale,
    //                 bottom - 1.5 * scale);

    left_path.arcTo(scaled_endcap_radius, scaled_endcap_radius, 90, SkPath::kSmall_ArcSize,
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
    path.transform(SkMatrix::Translate(0, 30));
    canvas->drawPath(path, p);

    p.setColor(SK_ColorBLUE);
    SkPath border_path = GetBorderPath(1.f, false, false, 16, SkISize::Make(250, 36));
    border_path.transform(SkMatrix::Translate(0, 30));
    canvas->drawPath(border_path, p);

    //  canvas->drawLine(20, 20, 100, 100, p);
}
}  // END FIDDLE
