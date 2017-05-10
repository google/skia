/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkResourceCache.h"
#include "SkShadowUtils.h"

void draw_shadow(SkCanvas* canvas, const SkPath& path, int height, SkColor color, SkPoint3 lightPos,
                 SkScalar lightR, bool isAmbient, uint32_t flags) {
    SkScalar ambientAlpha = isAmbient ? .5f : 0.f;
    SkScalar spotAlpha = isAmbient ? 0.f : .5f;
    SkShadowUtils::DrawShadow(canvas, path, height, lightPos, lightR, ambientAlpha, spotAlpha,
                              color, flags);
}

static constexpr int kW = 800;
static constexpr int kH = 800;

DEF_SIMPLE_GM(shadow_utils, canvas, kW, kH) {
    SkTArray<SkPath> paths;
    paths.push_back().addRoundRect(SkRect::MakeWH(50, 50), 10, 10);
    SkRRect oddRRect;
    oddRRect.setNinePatch(SkRect::MakeWH(50, 50), 9, 13, 6, 16);
    paths.push_back().addRRect(oddRRect);
    paths.push_back().addRect(SkRect::MakeWH(50, 50));
    paths.push_back().addCircle(25, 25, 25);
    paths.push_back().cubicTo(100, 50, 20, 100, 0, 0);
    paths.push_back().addOval(SkRect::MakeWH(20, 60));

    static constexpr SkScalar kPad = 15.f;
    static constexpr SkPoint3 kLightPos = {250, 400, 500};
    static constexpr SkScalar kLightR = 100.f;
    static constexpr SkScalar kHeight = 50.f;
    canvas->translate(3 * kPad, 3 * kPad);
    canvas->save();
    SkScalar x = 0;
    SkScalar dy = 0;
    SkTDArray<SkMatrix> matrices;
    matrices.push()->reset();
    SkMatrix* m = matrices.push();
    m->setRotate(33.f, 25.f, 25.f);
    m->postScale(1.2f, 0.8f, 25.f, 25.f);
    for (auto& m : matrices) {
        for (auto flags : {kNone_ShadowFlag, kTransparentOccluder_ShadowFlag}) {
            for (const auto& path : paths) {
                SkRect postMBounds = path.getBounds();
                m.mapRect(&postMBounds);
                SkScalar w = postMBounds.width() + kHeight;
                SkScalar dx = w + kPad;
                if (x + dx > kW - 3 * kPad) {
                    canvas->restore();
                    canvas->translate(0, dy);
                    canvas->save();
                    x = 0;
                    dy = 0;
                }

                canvas->save();
                canvas->concat(m);
                draw_shadow(canvas, path, kHeight, SK_ColorRED, kLightPos, kLightR, true, flags);
                draw_shadow(canvas, path, kHeight, SK_ColorBLUE, kLightPos, kLightR, false, flags);

                // Draw the path outline in green on top of the ambient and spot shadows.
                SkPaint paint;
                paint.setColor(SK_ColorGREEN);
                paint.setAntiAlias(true);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(0);
                canvas->drawPath(path, paint);
                canvas->restore();

                canvas->translate(dx, 0);
                x += dx;
                dy = SkTMax(dy, postMBounds.height() + kPad + kHeight);
            }
        }
    }
    // Show where the light is in x,y as a circle (specified in device space).
    SkMatrix invCanvasM = canvas->getTotalMatrix();
    if (invCanvasM.invert(&invCanvasM)) {
        canvas->save();
        canvas->concat(invCanvasM);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        canvas->drawCircle(kLightPos.fX, kLightPos.fY, kLightR / 10.f, paint);
        canvas->restore();
    }
}
