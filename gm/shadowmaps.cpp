/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "sk_tool_utils.h"
#include "SkPathEffect.h"
#include "SkPictureRecorder.h"
#include "SkShadowPaintFilterCanvas.h"
#include "SkShadowShader.h"
#include "SkSurface.h"

#ifdef SK_EXPERIMENTAL_SHADOWING


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
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                                     SkVector3::Make(0.2f, 0.1f, 1.0f)));
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                                     SkVector3::Make(0.1f, 0.2f, 1.0f)));
        builder.setAmbientLightColor(SkColor3f::Make(0.4f, 0.4f, 0.4f));
        fLights = builder.finish();

        fShadowParams.fShadowRadius = 4.0f;
        fShadowParams.fBiasingConstant = 0.3f;
        fShadowParams.fMinVariance = 1024;
        fShadowParams.fType = SkShadowParams::kVariance_ShadowType;
    }

protected:
    static constexpr int kWidth = 400;
    static constexpr int kHeight = 400;

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
        canvas->setLights(fLights);
        canvas->drawShadowedPicture(pic, nullptr, nullptr, fShadowParams);
    }

private:
    sk_sp<SkLights> fLights;
    SkShadowParams fShadowParams;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ShadowMapsGM;)
}

#endif
