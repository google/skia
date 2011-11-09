/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"

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
        return make_isize(500, 500);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setImageFilter(new SkBlurImageFilter(24.0f, 0.0f))->unref();
        canvas->saveLayer(NULL, &paint);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(100);
        paint.setAntiAlias(true);
        const char* str = "The quick brown fox jumped over the lazy dog.";
        srand(1234);
        SkISize size = canvas->getDeviceSize();
        for (int i = 0; i < 25; ++i) {
            int x = rand() % size.fWidth;
            int y = rand() % size.fHeight;
            paint.setColor(rand() % 0x1000000 | 0xFF000000);
            paint.setTextSize(rand() % 300);
            canvas->drawText(str, strlen(str), x, y, paint);
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
