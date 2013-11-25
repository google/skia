/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPerlinNoiseShader.h"

namespace skiagm {

class PerlinNoiseGM : public GM {
public:
    PerlinNoiseGM() {
        this->setBGColor(0xFF000000);
        fSize = SkISize::Make(80, 80);
    }

protected:
    virtual SkString onShortName() {
        return SkString("perlinnoise");
    }

    virtual SkISize onISize() {
        return make_isize(200, 500);
    }

    void drawClippedRect(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                         SkIntToScalar(fSize.width()), SkIntToScalar(fSize.height())));
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                    SkIntToScalar(fSize.width()),
                                    SkIntToScalar(fSize.height()));
        canvas->drawRect(r, paint);
        canvas->restore();
    }

    void test(SkCanvas* canvas, int x, int y, SkPerlinNoiseShader::Type type,
              float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed,
              bool stitchTiles) {
        SkShader* shader = (type == SkPerlinNoiseShader::kFractalNoise_Type) ?
            SkPerlinNoiseShader::CreateFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves,
                                                   seed, stitchTiles ? &fSize : NULL) :
            SkPerlinNoiseShader::CreateTubulence(baseFrequencyX, baseFrequencyY, numOctaves,
                                                seed, stitchTiles ? &fSize : NULL);
        SkPaint paint;
        paint.setShader(shader)->unref();
        drawClippedRect(canvas, x, y, paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->clear(0x00000000);
        test(canvas,   0,   0, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 0, 0, false);
        test(canvas, 100,   0, SkPerlinNoiseShader::kTurbulence_Type,
             0.1f, 0.1f, 0, 0, false);

        test(canvas,   0, 100, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 2, 0, false);
        test(canvas, 100, 100, SkPerlinNoiseShader::kFractalNoise_Type,
             0.2f, 0.4f, 5, 0, true);

        test(canvas,   0, 200, SkPerlinNoiseShader::kTurbulence_Type,
             0.1f, 0.1f, 2, 0, true);
        test(canvas, 100, 200, SkPerlinNoiseShader::kTurbulence_Type,
             0.2f, 0.4f, 5, 0, false);

        test(canvas,   0, 300, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 3, 1, false);
        test(canvas, 100, 300, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 3, 4, false);

        canvas->scale(0.75f, 1.0f);

        test(canvas,   0, 400, SkPerlinNoiseShader::kFractalNoise_Type,
             0.1f, 0.1f, 2, 0, false);
        test(canvas, 100, 400, SkPerlinNoiseShader::kFractalNoise_Type,
             0.2f, 0.4f, 5, 0, true);
    }

private:
    typedef GM INHERITED;
    SkISize fSize;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new PerlinNoiseGM; }
static GMRegistry reg(MyFactory);

}
