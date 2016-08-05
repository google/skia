/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "SkPathEffect.h"
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

static sk_sp<SkPicture> make_test_picture(int width, int height) {
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

    // TODO: Maybe add the ID here along with the depth

    paint.setColor(0xFFEE8888);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(200,150,350,300), paint);

    paint.setColor(0xFF88EE88);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(150,200,300,350), paint);

    paint.setColor(0xFF8888EE);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(100,100,250,250), paint);
    // TODO: Add an assert that Z order matches painter's order
    // TODO: think about if the Z-order always matching painting order is too strict

    return recorder.finishRecordingAsPicture();
}

namespace skiagm {

class ShadowMapsGM : public GM {
public:
    ShadowMapsGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

    void onOnceBeforeDraw() override {
        // Create a light set consisting of
        //   - bluish directional light pointing more right than down
        //   - reddish directional light pointing more down than right
        //   - soft white ambient light

        SkLights::Builder builder;
        builder.add(SkLights::Light(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                    SkVector3::Make(0.2f, 0.1f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                    SkVector3::Make(0.1f, 0.2f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
        fLights = builder.finish();
    }

protected:
    static const int kWidth = 400;
    static const int kHeight = 400;

    SkString onShortName() override {
        return SkString("shadowmaps");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        // This picture stores the picture of the scene.
        // It's used to generate the depth maps.
        sk_sp<SkPicture> pic(make_test_picture(kWidth, kHeight));

        for (int i = 0; i < fLights->numLights(); ++i) {
            // skip over ambient lights; they don't cast shadows
            if (SkLights::Light::kAmbient_LightType == fLights->light(i).type()) {
                continue;
            }
            // TODO: compute the correct size of the depth map from the light properties
            // TODO: maybe add a kDepth_8_SkColorType

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
            depthMapCanvas->drawPicture(pic);

            fLights->light(i).setShadowMap(surf->makeImageSnapshot());
        }

        sk_sp<SkImage> povDepthMap;
        sk_sp<SkImage> diffuseMap;

        // TODO: pass the depth to the shader in vertices, or uniforms
        //       so we don't have to render depth and color separately

        // povDepthMap
        {
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

            depthMapCanvas->drawPicture(pic);

            povDepthMap = surf->makeImageSnapshot();
        }

        // diffuseMap
        {
            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                                 kBGRA_8888_SkColorType,
                                                 kOpaque_SkAlphaType);

            sk_sp<SkSurface> surf(canvas->makeSurface(info));
            surf->getCanvas()->drawPicture(pic);

            diffuseMap = surf->makeImageSnapshot();
        }

        SkPaint paint;
        paint.setShader(make_shadow_shader(std::move(povDepthMap), std::move(diffuseMap), fLights));

        canvas->drawRect(SkRect::MakeIWH(kWidth, kHeight), paint);
    }

private:
    sk_sp<SkLights> fLights;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ShadowMapsGM;)
}

#endif
