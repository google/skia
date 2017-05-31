
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkCamera.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPoint3.h"
#include "SkShadowUtils.h"
#include "SkUtils.h"
#include "SkView.h"
#include "sk_tool_utils.h"

////////////////////////////////////////////////////////////////////////////

class ShadowUtilsView : public SampleView {
    SkTArray<SkPath> fPaths;
    SkScalar         fZDelta;

    bool      fShowAmbient;
    bool      fShowSpot;
    bool      fUseAlt;
    bool      fShowObject;
    bool      fIgnoreShadowAlpha;

public:
    ShadowUtilsView()
        : fZDelta(0)
        , fShowAmbient(true)
        , fShowSpot(true)
        , fUseAlt(false)
        , fShowObject(false)
        , fIgnoreShadowAlpha(false) {}

protected:
    void onOnceBeforeDraw() override {
        fPaths.push_back().addRoundRect(SkRect::MakeWH(50, 50), 10, 10);
        SkRRect oddRRect;
        oddRRect.setNinePatch(SkRect::MakeWH(50, 50), 9, 13, 6, 16);
        fPaths.push_back().addRRect(oddRRect);
        fPaths.push_back().addRect(SkRect::MakeWH(50, 50));
        fPaths.push_back().addCircle(25, 25, 25);
        fPaths.push_back().cubicTo(100, 50, 20, 100, 0, 0);
        fPaths.push_back().addOval(SkRect::MakeWH(20, 60));
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ShadowUtils");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
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
                this->inval(nullptr);
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

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
        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                  lightPos, lightWidth,
                                  ambientAlpha, 0, SK_ColorRED, flags);
        SkShadowUtils::DrawShadow(canvas, path, zPlaneParams,
                                  lightPos, lightWidth,
                                  0, spotAlpha, SK_ColorBLUE, flags);

        if (fShowObject) {
            canvas->drawPath(path, paint);
        } else {
            SkPaint strokePaint;

            strokePaint.setColor(paint.getColor());
            strokePaint.setStyle(SkPaint::kStroke_Style);

            canvas->drawPath(path, strokePaint);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
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
        matrices.push()->reset();
        SkMatrix* m = matrices.push();
        m->setRotate(33.f, 25.f, 25.f);
        m->postScale(1.2f, 0.8f, 25.f, 25.f);
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setAntiAlias(true);
        SkPoint3 zPlaneParams = SkPoint3::Make(0, 0, SkTMax(1.0f, kHeight + fZDelta));
        for (auto& m : matrices) {
            for (auto flags : { kNone_ShadowFlag, kTransparentOccluder_ShadowFlag }) {
                for (const auto& path : fPaths) {
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
                    drawShadowedPath(canvas, path, zPlaneParams, paint, kAmbientAlpha, lightPos,
                                     kLightR, kSpotAlpha, flags);
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

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShadowUtilsView; }
static SkViewRegister reg(MyFactory);
