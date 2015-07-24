/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"
#include "SkRandom.h"

#define WIDTH 500
#define HEIGHT 500

namespace skiagm {

class ImageBlurGM : public GM {
public:
    ImageBlurGM(SkScalar sigmaX, SkScalar sigmaY, const char* suffix)
        : fSigmaX(sigmaX), fSigmaY(sigmaY) {
        this->setBGColor(0xFF000000);
        fName.printf("imageblur%s", suffix);
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setImageFilter(SkBlurImageFilter::Create(fSigmaX, fSigmaY))->unref();
        canvas->saveLayer(NULL, &paint);
        const char* str = "The quick brown fox jumped over the lazy dog.";

        SkRandom rand;
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textPaint);
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            textPaint.setColor(sk_tool_utils::color_to_565(rand.nextBits(24) | 0xFF000000));
            textPaint.setTextSize(rand.nextRangeScalar(0, 300));
            canvas->drawText(str, strlen(str), SkIntToScalar(x),
                             SkIntToScalar(y), textPaint);
        }
        canvas->restore();
    }

private:
    SkScalar fSigmaX;
    SkScalar fSigmaY;
    SkString fName;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory1(void*) { return new ImageBlurGM(24.0f, 0.0f, ""); }
static GMRegistry reg1(MyFactory1);

static GM* MyFactory2(void*) { return new ImageBlurGM(80.0f, 80.0f, "_large"); }
static GMRegistry reg2(MyFactory2);

}
