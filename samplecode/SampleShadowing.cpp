/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkPictureRecorder.h"
#include "SkShadowPaintFilterCanvas.h"
#include "SkShadowShader.h"
#include "SkSurface.h"

#ifdef SK_EXPERIMENTAL_SHADOWING

class ShadowingView : public SampleView {
public:
    ShadowingView() {

        this->setBGColor(0xFFCCCCCC);
        SkLights::Builder builder;
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                                     SkVector3::Make(0.2f, 0.05f, 1.0f)));
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                                     SkVector3::Make(0.05f, 0.2f, 1.0f)));
        builder.add(SkLights::Light::MakeAmbient(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
        fLights = builder.finish();

        fTestRects[0].fColor = 0xFFEE8888;
        fTestRects[0].fDepth = 80;
        fTestRects[0].fGeometry = SkRect::MakeLTRB(200,150,350,300);

        fTestRects[1].fColor = 0xFF88EE88;
        fTestRects[1].fDepth = 160;
        fTestRects[1].fGeometry = SkRect::MakeLTRB(150,200,300,350);

        fTestRects[2].fColor = 0xFF8888EE;
        fTestRects[2].fDepth = 240;
        fTestRects[2].fGeometry = SkRect::MakeLTRB(100,100,250,250);

        fSceneChanged = true;
        fLightsChanged = true;

        fSelectedRect = -1;
        fMoveLight = false;

        fClearShadowMaps = false;
    }

protected:
    bool onQuery(SkEvent *evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "shadowing");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'L':
                    fMoveLight = !fMoveLight;
                    break;
                case 'd':
                    // Raster generated shadow maps have their origin in the UL corner
                    // GPU shadow maps can have an arbitrary origin.
                    // We override the 'd' keypress so that when the device is cycled,
                    // the shadow maps will be re-generated according to the new backend.
                    fClearShadowMaps = true;
                    break;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    sk_sp<SkPicture> makeTestPicture(int width, int height) {
        SkPictureRecorder recorder;

        // LONG RANGE TODO: eventually add SkBBHFactory (bounding box factory)
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeIWH(width, height));

        SkASSERT(canvas->getTotalMatrix().isIdentity());
        SkPaint paint;
        paint.setColor(SK_ColorGRAY);

        // LONG RANGE TODO: tag occluders
        // LONG RANGE TODO: track number of IDs we need (hopefully less than 256)
        //                  and determinate the mapping from z to id

        // universal receiver, "ground"
        canvas->drawRect(SkRect::MakeIWH(width, height), paint);

        for (int i = 0; i < kNumTestRects; i++) {
            paint.setColor(fTestRects[i].fColor);
            if (i == 0) {
                canvas->translateZ(fTestRects[0].fDepth);
            } else {
                canvas->translateZ(fTestRects[i].fDepth - fTestRects[i-1].fDepth);
            }
            canvas->drawRect(fTestRects[i].fGeometry, paint);
        }

        return recorder.finishRecordingAsPicture();
    }

    void onDrawContent(SkCanvas *canvas) override {
        if (fSceneChanged) {
            fPicture = this->makeTestPicture(kWidth, kHeight);
        }

        if (fSceneChanged || fLightsChanged || fClearShadowMaps) {
            for (int i = 0; i < fLights->numLights(); i++) {
                fLights->light(i).setShadowMap(nullptr);
            }
            fSceneChanged = false;
            fLightsChanged = false;
            fClearShadowMaps = false;

            canvas->setLights(fLights);
            canvas->drawShadowedPicture(fPicture, nullptr, nullptr);
        }
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        return new SkView::Click(this);
    }

    bool onClick(Click *click) override {
        SkScalar x = click->fCurr.fX;
        SkScalar y = click->fCurr.fY;

        SkScalar dx = x - click->fPrev.fX;
        SkScalar dy = y - click->fPrev.fY;

        if (fMoveLight) {
            if (dx != 0 || dy != 0) {
                float recipX = 1.0f / kWidth;
                float recipY = 1.0f / kHeight;

                SkLights::Builder builder;
                builder.add(SkLights::Light::MakeDirectional(
                        SkColor3f::Make(0.2f, 0.3f, 0.4f),
                        SkVector3::Make(0.2f + (200.0f - x) * recipX,
                                        0.05f + (200.0f - y) * recipY,
                                        1.0f)));
                builder.add(SkLights::Light::MakeDirectional(
                        SkColor3f::Make(0.4f, 0.3f, 0.2f),
                        SkVector3::Make(0.05f + (200.0f - x) * recipX,
                                        0.2f + (200.0f - y) * recipY,
                                        1.0f)));
                builder.add(SkLights::Light::MakeAmbient(
                        SkColor3f::Make(0.4f, 0.4f, 0.4f)));
                fLights = builder.finish();

                fLightsChanged = true;
                this->inval(nullptr);
            }
            return true;
        }

        if (click->fState == Click::State::kUp_State) {
            fSelectedRect = -1;
            return true;
        }

        if (fSelectedRect > -1) {
            fTestRects[fSelectedRect].fGeometry.offset(dx, dy);

            fSceneChanged = true;
            this->inval(nullptr);
            return true;
        }

        // assume last elements are highest
        for (int i = kNumTestRects - 1; i >= 0; i--) {
            if (fTestRects[i].fGeometry.contains(SkRect::MakeXYWH(x, y, 1, 1))) {
                fSelectedRect = i;
                fTestRects[i].fGeometry.offset(dx, dy);

                fSceneChanged = true;
                this->inval(nullptr);
                break;
            }
        }

        return true;
    }

private:
    static constexpr int kNumTestRects = 3;

    static const int kWidth = 400;
    static const int kHeight = 400;
    bool fClearShadowMaps;

    struct {
        SkRect  fGeometry;
        int     fDepth;
        SkColor fColor;
    } fTestRects[kNumTestRects];

    int fSelectedRect;
    bool fMoveLight;

    sk_sp<SkPicture> fPicture;

    bool fSceneChanged;
    bool fLightsChanged;

    sk_sp<SkLights> fLights;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
static SkView* MyFactory() { return new ShadowingView; }
static SkViewRegister reg(MyFactory);

#endif
