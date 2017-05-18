/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"

namespace skiagm {

static sk_sp<SkShader> MakeLinear(SkScalar width, SkScalar height, bool alternate,
                            const SkMatrix& localMatrix) {
    SkPoint pts[2] = { {0, 0}, {width, height}};
    SkColor colors[2] = {SK_ColorRED, SK_ColorGREEN};
    if (alternate) {
        pts[1].fY = 0;
        colors[0] = SK_ColorBLUE;
        colors[1] = SK_ColorYELLOW;
    }
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode,
                                        0, &localMatrix);
}

///////////////////////////////////////////////////////////////////////////////

class ShaderBoundsGM : public GM {
public:
    typedef sk_sp<SkShader> (*ShaderGenFunc)(SkScalar width, SkScalar height,
                                             bool alternate, const SkMatrix& localMatrix);
    ShaderBoundsGM(ShaderGenFunc maker, const SkString& name)
        : fShaderMaker(maker),
          fName(name) {
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override { return SkISize::Make(320, 240); }

    SkMatrix onGetInitialTransform() const override {
        SkMatrix result;
        SkScalar scale = 0.8f;
        result.setScale(scale, scale);
        result.postTranslate(SkIntToScalar(7), SkIntToScalar(23));
        return result;
    }

    void onDraw(SkCanvas* canvas) override {
        // The PDF device has already clipped to the content area, but we
        // do it again here so that the raster and pdf results are consistent.
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(320),
                                        SkIntToScalar(240)));

        SkMatrix canvasScale;
        SkScalar scale = 0.7f;
        canvasScale.setScale(scale, scale);
        canvas->concat(canvasScale);

        // Background shader.
        SkPaint paint;
        paint.setShader(MakeShader(559, 387, false));
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(-12), SkIntToScalar(-41),
                                    SkIntToScalar(571), SkIntToScalar(428));
        canvas->drawRect(r, paint);

        // Constrained shader.
        paint.setShader(MakeShader(101, 151, true));
        r = SkRect::MakeXYWH(SkIntToScalar(43), SkIntToScalar(71),
                             SkIntToScalar(101), SkIntToScalar(151));
        canvas->clipRect(r);
        canvas->drawRect(r, paint);
    }

    sk_sp<SkShader> MakeShader(int width, int height, bool background) {
        SkScalar scale = 0.5f;
        if (background) {
            scale = 0.6f;
        }
        SkScalar shaderWidth = width / scale;
        SkScalar shaderHeight = height / scale;
        SkMatrix shaderScale = SkMatrix::MakeScale(scale);
        return fShaderMaker(shaderWidth, shaderHeight, background, shaderScale);
    }

private:
    typedef GM INHERITED;

    ShaderGenFunc fShaderMaker;
    SkString fName;

    sk_sp<SkShader> MakeShader(bool background);
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) {
    return new ShaderBoundsGM(MakeLinear, SkString("shaderbounds_linear"));
}
static GMRegistry reg(MyFactory);

}
