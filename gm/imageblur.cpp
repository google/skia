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
    ImageBlurGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("imageblur");
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setImageFilter(new SkBlurImageFilter(24.0f, 0.0f))->unref();
        canvas->saveLayer(NULL, &paint);
        const char* str = "The quick brown fox jumped over the lazy dog.";

        SkRandom rand;
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(WIDTH);
            int y = rand.nextULessThan(HEIGHT);
            textPaint.setColor(rand.nextBits(24) | 0xFF000000);
            textPaint.setTextSize(rand.nextRangeScalar(0, 300));
            canvas->drawText(str, strlen(str), SkIntToScalar(x),
                             SkIntToScalar(y), textPaint);
        }
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageBlurGM; }
static GMRegistry reg(MyFactory);

}
