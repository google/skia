/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/utils/SkShadowUtils.h"

#include <initializer_list>

using namespace skia_private;

void draw_shadow(SkCanvas* canvas, const SkPath& path, SkScalar height, SkColor color,
                 SkPoint3 lightPos, SkScalar lightR, bool isAmbient, uint32_t flags) {
    SkScalar ambientAlpha = isAmbient ? .5f : 0.f;
    SkScalar spotAlpha = isAmbient ? 0.f : .5f;
    SkColor ambientColor = SkColorSetARGB(ambientAlpha*SkColorGetA(color), SkColorGetR(color),
                                          SkColorGetG(color), SkColorGetB(color));
    SkColor spotColor = SkColorSetARGB(spotAlpha*SkColorGetA(color), SkColorGetR(color),
                                       SkColorGetG(color), SkColorGetB(color));
    SkShadowUtils::DrawShadow(canvas, path, SkPoint3{ 0, 0, height}, lightPos, lightR,
                              ambientColor, spotColor, flags);
}

static constexpr int kW = 800;
static constexpr int kH = 960;

enum ShadowMode {
    kDebugColorNoOccluders,
    kDebugColorOccluders,
    kGrayscale
};

void draw_paths(SkCanvas* canvas, ShadowMode mode) {
    TArray<SkPath> paths;
    paths.push_back(SkPath::RRect(SkRect::MakeWH(50, 50), 10, 10.00002f));
    SkRRect oddRRect;
    oddRRect.setNinePatch(SkRect::MakeWH(50, 50), 9, 13, 6, 16);
    paths.push_back(SkPath::RRect(oddRRect));
    paths.push_back(SkPath::Rect(SkRect::MakeWH(50, 50)));
    paths.push_back(SkPath::Circle(25, 25, 25));
    paths.push_back(SkPathBuilder().cubicTo(100, 50, 20, 100, 0, 0).detach());
    paths.push_back(SkPath::Oval(SkRect::MakeWH(20, 60)));

    // star
    TArray<SkPath> concavePaths;
    concavePaths.push_back().moveTo(0.0f, -33.3333f);
    concavePaths.back().lineTo(9.62f, -16.6667f);
    concavePaths.back().lineTo(28.867f, -16.6667f);
    concavePaths.back().lineTo(19.24f, 0.0f);
    concavePaths.back().lineTo(28.867f, 16.6667f);
    concavePaths.back().lineTo(9.62f, 16.6667f);
    concavePaths.back().lineTo(0.0f, 33.3333f);
    concavePaths.back().lineTo(-9.62f, 16.6667f);
    concavePaths.back().lineTo(-28.867f, 16.6667f);
    concavePaths.back().lineTo(-19.24f, 0.0f);
    concavePaths.back().lineTo(-28.867f, -16.6667f);
    concavePaths.back().lineTo(-9.62f, -16.6667f);
    concavePaths.back().close();

    // dumbbell
    concavePaths.push_back().moveTo(50, 0);
    concavePaths.back().cubicTo(100, 25, 60, 50, 50, 0);
    concavePaths.back().cubicTo(0, -25, 40, -50, 50, 0);

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
    matrices.append()->reset();
    matrices.append()->setRotate(33.f, 25.f, 25.f).postScale(1.2f, 0.8f, 25.f, 25.f);
    for (auto& m : matrices) {
        for (int flags : { kNone_ShadowFlag, kTransparentOccluder_ShadowFlag }) {
            int pathCounter = 0;
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

                // flip a couple of paths to test 180° rotation
                if (kTransparentOccluder_ShadowFlag == flags && 0 == pathCounter % 3) {
                    canvas->save();
                    canvas->rotate(180, 25, 25);
                }
                if (kDebugColorNoOccluders == mode || kDebugColorOccluders == mode) {
                    draw_shadow(canvas, path, kHeight, SK_ColorRED, lightPos, kLightR,
                                true, flags);
                    draw_shadow(canvas, path, kHeight, SK_ColorBLUE, lightPos, kLightR,
                                false, flags);
                } else if (kGrayscale == mode) {
                    SkColor ambientColor = SkColorSetARGB(0.1f * 255, 0, 0, 0);
                    SkColor spotColor = SkColorSetARGB(0.25f * 255, 0, 0, 0);
                    SkShadowUtils::DrawShadow(canvas, path, SkPoint3{0, 0, kHeight}, lightPos,
                                              kLightR, ambientColor, spotColor, flags);
                }

                SkPaint paint;
                paint.setAntiAlias(true);
                if (kDebugColorNoOccluders == mode) {
                    // Draw the path outline in green on top of the ambient and spot shadows.
                    if (SkToBool(flags & kTransparentOccluder_ShadowFlag)) {
                        paint.setColor(SK_ColorCYAN);
                    } else {
                        paint.setColor(SK_ColorGREEN);
                    }
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setStrokeWidth(0);
                } else {
                    paint.setColor(kDebugColorOccluders == mode ? SK_ColorLTGRAY : SK_ColorWHITE);
                    if (SkToBool(flags & kTransparentOccluder_ShadowFlag)) {
                        paint.setAlphaf(0.5f);
                    }
                    paint.setStyle(SkPaint::kFill_Style);
                }
                canvas->drawPath(path, paint);
                if (kTransparentOccluder_ShadowFlag == flags && 0 == pathCounter % 3) {
                    canvas->restore();
                }
                canvas->restore();

                canvas->translate(dx, 0);
                x += dx;
                dy = std::max(dy, postMBounds.height() + kPad + kHeight);
                ++pathCounter;
            }
        }
    }

    // concave paths
    canvas->restore();
    canvas->translate(kPad, dy);
    canvas->save();
    x = kPad;
    dy = 0;
    for (auto& m : matrices) {
        // for the concave paths we are not clipping, so transparent and opaque are the same
        for (const auto& path : concavePaths) {
            SkRect postMBounds = path.getBounds();
            m.mapRect(&postMBounds);
            SkScalar w = postMBounds.width() + kHeight;
            SkScalar dx = w + kPad;

            canvas->save();
            canvas->concat(m);

            if (kDebugColorNoOccluders == mode || kDebugColorOccluders == mode) {
                draw_shadow(canvas, path, kHeight, SK_ColorRED, lightPos, kLightR,
                            true, kNone_ShadowFlag);
                draw_shadow(canvas, path, kHeight, SK_ColorBLUE, lightPos, kLightR,
                            false, kNone_ShadowFlag);
            } else if (kGrayscale == mode) {
                SkColor ambientColor = SkColorSetARGB(0.1f * 255, 0, 0, 0);
                SkColor spotColor = SkColorSetARGB(0.25f * 255, 0, 0, 0);
                SkShadowUtils::DrawShadow(canvas, path, SkPoint3{ 0, 0, kHeight }, lightPos,
                                          kLightR, ambientColor, spotColor, kNone_ShadowFlag);
            }

            SkPaint paint;
            paint.setAntiAlias(true);
            if (kDebugColorNoOccluders == mode) {
                // Draw the path outline in green on top of the ambient and spot shadows.
                paint.setColor(SK_ColorGREEN);
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeWidth(0);
            } else {
                paint.setColor(kDebugColorOccluders == mode ? SK_ColorLTGRAY : SK_ColorWHITE);
                paint.setStyle(SkPaint::kFill_Style);
            }
            canvas->drawPath(path, paint);
            canvas->restore();

            canvas->translate(dx, 0);
            x += dx;
            dy = std::max(dy, postMBounds.height() + kPad + kHeight);
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
    draw_paths(canvas, kDebugColorNoOccluders);
}

DEF_SIMPLE_GM(shadow_utils_occl, canvas, kW, kH) {
    draw_paths(canvas, kDebugColorOccluders);
}

DEF_SIMPLE_GM(shadow_utils_gray, canvas, kW, kH) {
    draw_paths(canvas, kGrayscale);
}

#include "include/effects/SkGradientShader.h"
#include "src/core/SkColorFilterPriv.h"

DEF_SIMPLE_GM(shadow_utils_gaussian_colorfilter, canvas, 512, 256) {
    const SkRect r = SkRect::MakeWH(256, 256);

    const SkColor colors[] = { 0, 0xFF000000 };
    auto sh = SkGradientShader::MakeRadial({r.centerX(), r.centerY()}, r.width(),
                                           colors, nullptr, std::size(colors),
                                           SkTileMode::kClamp);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);

    SkPaint paint;
    paint.setShader(sh);
    canvas->drawRect(r, redPaint);
    canvas->drawRect(r, paint);

    canvas->translate(256, 0);
    paint.setColorFilter(SkColorFilterPriv::MakeGaussian());
    canvas->drawRect(r, redPaint);
    canvas->drawRect(r, paint);
}

DEF_SIMPLE_GM(shadow_utils_directional, canvas, 256, 384) {
    static constexpr SkScalar kLightR = 1.f;
    static constexpr SkScalar kHeight = 12.f;

    SkPath rrect(SkPath::RRect(SkRect::MakeLTRB(-25, -25, 25, 25), 10, 10));
    SkPoint3 lightPos = { -45, -45, 77.9422863406f };

    SkColor ambientColor = SkColorSetARGB(0.02f * 255, 0, 0, 0);
    SkColor spotColor = SkColorSetARGB(0.35f * 255, 0, 0, 0);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setStyle(SkPaint::kFill_Style);

    // translation
    canvas->save();
    canvas->translate(35, 35);
    for (int i = 0; i < 3; ++i) {
        SkShadowUtils::DrawShadow(canvas, rrect, SkPoint3{ 0, 0, kHeight }, lightPos,
                                  kLightR, ambientColor, spotColor,
                                  kDirectionalLight_ShadowFlag);
        canvas->drawPath(rrect, paint);
        canvas->translate(80, 0);
    }
    canvas->restore();

    // rotation
    for (int i = 0; i < 3; ++i) {
        canvas->save();
        canvas->translate(35 + 80*i, 105);
        canvas->rotate(20.f*(i + 1));
        SkShadowUtils::DrawShadow(canvas, rrect, SkPoint3{ 0, 0, kHeight }, lightPos,
                                  kLightR, ambientColor, spotColor,
                                  kDirectionalLight_ShadowFlag);

        canvas->drawPath(rrect, paint);
        canvas->restore();
    }

    // scale
    for (int i = 0; i < 3; ++i) {
        canvas->save();
        SkScalar scaleFactor = std::pow(2.0, -i);
        canvas->translate(35 + 80*i, 185);
        canvas->scale(scaleFactor, scaleFactor);
        SkShadowUtils::DrawShadow(canvas, rrect, SkPoint3{ 0, 0, kHeight }, lightPos,
                                  kLightR, ambientColor, spotColor,
                                  kDirectionalLight_ShadowFlag);

        canvas->drawPath(rrect, paint);
        canvas->restore();
    }

    // perspective
    for (int i = 0; i < 3; ++i) {
        canvas->save();
        SkMatrix mat;
        mat.reset();
        mat[SkMatrix::kMPersp1] = 0.005f;
        mat[SkMatrix::kMPersp2] = 1.005f;
        canvas->translate(35 + 80*i, 265);
        canvas->concat(mat);
        SkShadowUtils::DrawShadow(canvas, rrect, SkPoint3{ 0, 0, kHeight }, lightPos,
                                  kLightR, ambientColor, spotColor,
                                  kDirectionalLight_ShadowFlag);

        canvas->drawPath(rrect, paint);
        canvas->restore();
    }

}
