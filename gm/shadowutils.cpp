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

void draw_paths(SkCanvas* canvas, bool hideOccluders) {
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
    static constexpr SkScalar kLightR = 100.f;
    static constexpr SkScalar kHeight = 50.f;

    // transform light position relative to canvas to handle tiling
    SkPoint lightXY = canvas->getTotalMatrix().mapXY(250, 400);
    SkPoint3 lightPos = { lightXY.fX, lightXY.fY, 500 };

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
        for (auto flags : { kNone_ShadowFlag, kTransparentOccluder_ShadowFlag }) {
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
                draw_shadow(canvas, path, kHeight, SK_ColorRED, lightPos, kLightR, true, flags);
                draw_shadow(canvas, path, kHeight, SK_ColorBLUE, lightPos, kLightR, false, flags);

                // Draw the path outline in green on top of the ambient and spot shadows.
                SkPaint paint;
                paint.setAntiAlias(true);
                if (hideOccluders) {
                    if (SkToBool(flags & kTransparentOccluder_ShadowFlag)) {
                        paint.setColor(SK_ColorCYAN);
                    } else {
                        paint.setColor(SK_ColorGREEN);
                    }
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setStrokeWidth(0);
                } else {
                    paint.setColor(SK_ColorLTGRAY);
                    if (SkToBool(flags & kTransparentOccluder_ShadowFlag)) {
                        paint.setAlpha(128);
                    }
                    paint.setStyle(SkPaint::kFill_Style);
                }
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
        canvas->drawCircle(lightPos.fX, lightPos.fY, kLightR / 10.f, paint);
        canvas->restore();
    }
}

DEF_SIMPLE_GM(shadow_utils, canvas, kW, kH) {
    draw_paths(canvas, true);
}

DEF_SIMPLE_GM(shadow_utils_occl, canvas, kW, kH) {
    draw_paths(canvas, false);
}
