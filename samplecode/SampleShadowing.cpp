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

static sk_sp<SkShader> make_shadow_shader(sk_sp<SkImage> povDepth,
                                          sk_sp<SkImage> diffuse,
                                          sk_sp<SkLights> lights) {

    sk_sp<SkShader> povDepthShader = povDepth->makeShader(SkShader::kClamp_TileMode,
                                                          SkShader::kClamp_TileMode);

    sk_sp<SkShader> diffuseShader = diffuse->makeShader(SkShader::kClamp_TileMode,
                                                        SkShader::kClamp_TileMode);

    return SkShadowShader::Make(std::move(povDepthShader),
                                std::move(diffuseShader),
                                std::move(lights),
                                diffuse->width(), diffuse->height());
}

class ShadowingView : public SampleView {
public:
    ShadowingView() {

        this->setBGColor(0xFFCCCCCC);
        SkLights::Builder builder;
        builder.add(SkLights::Light(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                    SkVector3::Make(0.27f, 0.07f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                    SkVector3::Make(0.03f, 0.27f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
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

    void updateDepthMaps(SkCanvas *canvas) {
        for (int i = 0; i < fLights->numLights(); ++i) {
            // skip over ambient lights; they don't cast shadows
            if (SkLights::Light::kAmbient_LightType == fLights->light(i).type()) {
                continue;
            }

            // TODO: maybe add a kDepth_8_SkColorType when vertices have depth
            // assume max depth is 255.
            // TODO: find actual max depth of picture
            SkISize shMapSize = SkShadowPaintFilterCanvas::
                                ComputeDepthMapSize(fLights->light(i), 255, kWidth, kHeight);

            SkImageInfo info = SkImageInfo::Make(shMapSize.fWidth, shMapSize.fHeight,
                                                 kBGRA_8888_SkColorType,
                                                 kOpaque_SkAlphaType);

            // Create a new surface (that matches the backend of canvas)
            // for each shadow map
            sk_sp<SkSurface> surf(canvas->makeSurface(info));

            // Wrap another SPFCanvas around the surface
            sk_sp<SkShadowPaintFilterCanvas> depthMapCanvas =
                    sk_make_sp<SkShadowPaintFilterCanvas>(surf->getCanvas());

            // set the depth map canvas to have the light we're drawing.
            SkLights::Builder builder;
            builder.add(fLights->light(i));
            sk_sp<SkLights> curLight = builder.finish();

            depthMapCanvas->setLights(std::move(curLight));
            depthMapCanvas->drawPicture(fPicture);

            fLights->light(i).setShadowMap(surf->makeImageSnapshot());
        }
    }

    void updatePovDepthMap(SkCanvas* canvas) {
        // TODO: pass the depth to the shader in vertices, or uniforms
        //       so we don't have to render depth and color separately

        SkLights::Builder builder;
        builder.add(SkLights::Light(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                    SkVector3::Make(0.0f, 0.0f, 1.0f)));
        sk_sp<SkLights> povLight = builder.finish();

        SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                             kBGRA_8888_SkColorType,
                                             kOpaque_SkAlphaType);

        // Create a new surface (that matches the backend of canvas)
        // to create the povDepthMap
        sk_sp<SkSurface> surf(canvas->makeSurface(info));

        // Wrap another SPFCanvas around the surface
        sk_sp<SkShadowPaintFilterCanvas> depthMapCanvas =
                sk_make_sp<SkShadowPaintFilterCanvas>(surf->getCanvas());

        // set the depth map canvas to have the light as the user's POV
        depthMapCanvas->setLights(std::move(povLight));

        depthMapCanvas->drawPicture(fPicture);

        fPovDepthMap = surf->makeImageSnapshot();
    }

    void updateDiffuseMap(SkCanvas* canvas) {
        SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                             kBGRA_8888_SkColorType,
                                             kOpaque_SkAlphaType);

        sk_sp<SkSurface> surf(canvas->makeSurface(info));
        surf->getCanvas()->drawPicture(fPicture);

        fDiffuseMap = surf->makeImageSnapshot();
    }

    void onDrawContent(SkCanvas *canvas) override {
        if (fSceneChanged || !fPovDepthMap) {
            fPicture = this->makeTestPicture(kWidth, kHeight);
            this->updateDepthMaps(canvas);
            this->updatePovDepthMap(canvas);
            this->updateDiffuseMap(canvas);
        }

        if (fLightsChanged) {
            this->updateDepthMaps(canvas);
        }

        if (fSceneChanged || fLightsChanged || !fShadowShader) {
            fShadowShader = make_shadow_shader(fPovDepthMap, fDiffuseMap, fLights);
        }

        SkPaint paint;
        paint.setShader(fShadowShader);

        canvas->drawRect(SkRect::MakeIWH(kWidth, kHeight), paint);
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
                builder.add(SkLights::Light(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                            SkVector3::Make(0.12f + (200.0f - x) * recipX,
                                                            -0.08f + (200.0f - y) * recipY,
                                                            1.0f)));
                builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                            SkVector3::Make(-0.12f + (200.0f - x) * recipX,
                                                            0.12f + (200.0f - y) * recipY,
                                                            1.0f)));
                builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
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

    struct {
        SkRect  fGeometry;
        int     fDepth;
        SkColor fColor;
    } fTestRects[kNumTestRects];

    int fSelectedRect;
    bool fMoveLight;

    sk_sp<SkImage>   fPovDepthMap;
    sk_sp<SkImage>   fDiffuseMap;
    sk_sp<SkPicture> fPicture;
    sk_sp<SkShader>  fShadowShader;

    bool fSceneChanged;
    bool fLightsChanged;

    sk_sp<SkLights> fLights;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
static SkView* MyFactory() { return new ShadowingView; }
static SkViewRegister reg(MyFactory);

#endif
