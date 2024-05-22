
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRRect.h"
#include "include/utils/SkCamera.h"
#include "include/utils/SkShadowUtils.h"
#include "src/base/SkUTF.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"
#include "tools/viewer/Slide.h"

using namespace skia_private;

////////////////////////////////////////////////////////////////////////////

class ShadowUtilsSlide : public Slide {
    TArray<SkPath> fConvexPaths;
    TArray<SkPath> fConcavePaths;
    SkScalar         fZDelta;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;
    bool      fIgnoreShadowAlpha;

public:
    ShadowUtilsSlide()
        : fZDelta(0)
        , fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(false)
        , fIgnoreShadowAlpha(false) {
        fName = "ShadowUtils";
    }

    void load(SkScalar w, SkScalar h) override {
        fConvexPaths.push_back().addRoundRect(SkRect::MakeWH(50, 50), 10, 10);
        SkRRect oddRRect;
        oddRRect.setNinePatch(SkRect::MakeWH(50, 50), 9, 13, 6, 16);
        fConvexPaths.push_back().addRRect(oddRRect);
        fConvexPaths.push_back().addRect(SkRect::MakeWH(50, 50));
        fConvexPaths.push_back().addCircle(25, 25, 25);
        fConvexPaths.push_back().cubicTo(100, 50, 20, 100, 0, 0);
        fConvexPaths.push_back().addOval(SkRect::MakeWH(20, 60));

        // star
        fConcavePaths.push_back().moveTo(0.0f, -33.3333f);
        fConcavePaths.back().lineTo(9.62f, -16.6667f);
        fConcavePaths.back().lineTo(28.867f, -16.6667f);
        fConcavePaths.back().lineTo(19.24f, 0.0f);
        fConcavePaths.back().lineTo(28.867f, 16.6667f);
        fConcavePaths.back().lineTo(9.62f, 16.6667f);
        fConcavePaths.back().lineTo(0.0f, 33.3333f);
        fConcavePaths.back().lineTo(-9.62f, 16.6667f);
        fConcavePaths.back().lineTo(-28.867f, 16.6667f);
        fConcavePaths.back().lineTo(-19.24f, 0.0f);
        fConcavePaths.back().lineTo(-28.867f, -16.6667f);
        fConcavePaths.back().lineTo(-9.62f, -16.6667f);
        fConcavePaths.back().close();

        // dumbbell
        fConcavePaths.push_back().moveTo(50, 0);
        fConcavePaths.back().cubicTo(100, 25, 60, 50, 50, 0);
        fConcavePaths.back().cubicTo(0, -25, 40, -50, 50, 0);
    }

    bool onChar(SkUnichar uni) override {
            bool handled = false;
            switch (uni) {
                case 'W':
                    fShowAmbient = !fShowAmbient;
                    handled = true;
                    break;
                case 'S':
                    fShowSpot = !fShowSpot;
                    handled = true;
                    break;
                case 'T':
                    fUseAlt = !fUseAlt;
                    handled = true;
                    break;
                case 'O':
                    fShowObject = !fShowObject;
                    handled = true;
                    break;
                case '>':
                    fZDelta += 0.5f;
                    handled = true;
                    break;
                case '<':
                    fZDelta -= 0.5f;
                    handled = true;
                    break;
                case '?':
                    fIgnoreShadowAlpha = !fIgnoreShadowAlpha;
                    handled = true;
                    break;
                default:
                    break;
            }
            if (handled) {
                return true;
            }
            return false;
    }

    void draw(SkCanvas* canvas) override {
        this->drawBG(canvas);

        static constexpr int kW = 800;
        static constexpr SkScalar kPad = 15.f;
        static constexpr SkScalar kLightR = 100.f;
        static constexpr SkScalar kHeight = 50.f;
        static constexpr SkScalar kAmbientAlpha = 0.5f;
        static constexpr SkScalar kSpotAlpha = 0.5f;
        static constexpr SkPoint3 lightPos = { 250, 400, 500 };

        canvas->translate(3 * kPad, 3 * kPad);
        canvas->save();
        SkScalar x = 0;
        SkScalar dy = 0;
        SkTDArray<SkMatrix> matrices;
        matrices.append()->reset();
        matrices.append()->setRotate(33.f, 25.f, 25.f).postScale(1.2f, 0.8f, 25.f, 25.f);
        SkPaint greenPaint;
        greenPaint.setColor(SK_ColorGREEN);
        greenPaint.setAntiAlias(true);
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, std::max(1.0f, kHeight + fZDelta));

        // convex paths
        for (auto& m : matrices) {
            for (auto flags : { kNone_ShadowFlag, kTransparentOccluder_ShadowFlag }) {
                for (const auto& path : fConvexPaths) {
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
                    this->drawShadowedPath(canvas, path, zPlaneParams, greenPaint, kAmbientAlpha,
                                           lightPos, kLightR, kSpotAlpha, flags);
                    canvas->restore();

                    canvas->translate(dx, 0);
                    x += dx;
                    dy = std::max(dy, postMBounds.height() + kPad + kHeight);
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
            for (const auto& path : fConcavePaths) {
                SkRect postMBounds = path.getBounds();
                m.mapRect(&postMBounds);
                SkScalar w = postMBounds.width();
                SkScalar dx = w + kPad;

                canvas->save();
                canvas->concat(m);
                this->drawShadowedPath(canvas, path, zPlaneParams, greenPaint, kAmbientAlpha,
                                       lightPos, kLightR, kSpotAlpha, kNone_ShadowFlag);
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
            SkPaint blackPaint;
            blackPaint.setColor(SK_ColorBLACK);
            blackPaint.setAntiAlias(true);
            canvas->drawCircle(lightPos.fX, lightPos.fY, kLightR / 10.f, blackPaint);
            canvas->restore();
        }
    }

private:
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFFFFFFF);
    }

    void drawShadowedPath(SkCanvas* canvas, const SkPath& path,
                          const SkPoint3& zPlaneParams,
                          const SkPaint& paint, SkScalar ambientAlpha,
                          const SkPoint3& lightPos, SkScalar lightWidth, SkScalar spotAlpha,
                          uint32_t flags) {
        if (fIgnoreShadowAlpha) {
            ambientAlpha = 255;
            spotAlpha = 255;
        }
        if (!fShowAmbient) {
            ambientAlpha = 0;
        }
        if (!fShowSpot) {
            spotAlpha = 0;
        }
        if (fUseAlt) {
            flags |= SkShadowFlags::kGeometricOnly_ShadowFlag;
        }

        SkColor ambientColor = SkColorSetARGB(ambientAlpha * 255, 255, 0, 0);
        SkColor spotColor = SkColorSetARGB(spotAlpha * 255, 0, 0, 255);
        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                  lightPos, lightWidth,
                                  ambientColor, spotColor, flags);

        if (fShowObject) {
            canvas->drawPath(path, paint);
        } else {
            SkPaint strokePaint;

            strokePaint.setColor(paint.getColor());
            strokePaint.setStyle(SkPaint::kStroke_Style);

            canvas->drawPath(path, strokePaint);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new ShadowUtilsSlide(); )
