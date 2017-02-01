/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkShadowUtils.h"

void draw_shadow(SkCanvas* canvas, const SkPath& path, int height, SkColor color, SkPoint3 lightPos,
                 SkScalar lightR, bool isAmbient, uint32_t flags) {
    SkScalar ambientAlpha = isAmbient ? .5f : 0.f;
    SkScalar spotAlpha = isAmbient ? 0.f : .5f;
    SkShadowUtils::DrawShadow(canvas, path, height, lightPos, lightR, ambientAlpha, spotAlpha,
                              color, flags);
}

static constexpr int kW = 500;
static constexpr int kH = 500;

DEF_SIMPLE_GM(shadow_utils, canvas, kW, kH) {
    SkTArray<SkPath> paths;
    paths.push_back().addRoundRect(SkRect::MakeWH(50, 50), 10, 10);
    SkRRect oddRRect;
    oddRRect.setNinePatch(SkRect::MakeWH(50, 50), 9, 13, 6, 16);
    paths.push_back().addRRect(oddRRect);
    paths.push_back().addRect(SkRect::MakeWH(50, 50));
    paths.push_back().addCircle(25, 25, 25);
    paths.push_back().cubicTo(100, 50, 20, 100, 0, 0);

    static constexpr SkScalar kPad = 15.f;
    static constexpr SkPoint3 kLightPos = {kW / 2, kH / 2, 40};
    static constexpr SkScalar kLightR = 100.f;
    static constexpr SkScalar kHeight = 25.f;
    canvas->translate(3 * kPad, 3 * kPad);
    canvas->save();
    SkScalar x = 0;
    SkScalar dy = 0;
    for (auto flags : {kNone_ShadowFlag, kTransparentOccluder_ShadowFlag}) {
        for (const auto& path : paths) {
            SkScalar w = path.getBounds().width() + kHeight;
            SkScalar dx = w + kPad;
            if (x + dx > kW - 3 * kPad) {
                canvas->restore();
                canvas->translate(0, dy);
                canvas->save();
                x = 0;
                dy = 0;
            }
            draw_shadow(canvas, path, kHeight, SK_ColorRED, kLightPos, kLightR, true, flags);
            draw_shadow(canvas, path, kHeight, SK_ColorBLUE, kLightPos, kLightR, false, flags);

            SkPaint paint;
            paint.setColor(SK_ColorGREEN);
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(0);

            canvas->drawPath(path, paint);
            canvas->translate(dx, 0);
            x += dx;
            dy = SkTMax(dy, path.getBounds().height() + kPad + kHeight);
        }
    }
    // Show where the light is (specified in device space).
    SkMatrix m = canvas->getTotalMatrix();
    if (m.invert(&m)) {
        canvas->save();
        canvas->concat(m);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        canvas->drawCircle(kLightPos.fX, kLightPos.fY, kLightR / 10.f, paint);
        canvas->restore();
    }
}
