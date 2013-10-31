/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColor.h"
#include "SkBicubicImageFilter.h"

namespace skiagm {

class BicubicGM : public GM {
public:
    BicubicGM() : fInitialized(false) {
        this->setBGColor(0x00000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("bicubicfilter");
    }

    void make_checkerboard(int width, int height) {
        SkASSERT(width % 2 == 0);
        SkASSERT(height % 2 == 0);
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        fCheckerboard.allocPixels();
        SkAutoLockPixels lock(fCheckerboard);
        for (int y = 0; y < height; y += 2) {
            SkPMColor* s = fCheckerboard.getAddr32(0, y);
            for (int x = 0; x < width; x += 2) {
                *s++ = 0xFFFFFFFF;
                *s++ = 0xFF000000;
            }
            s = fCheckerboard.getAddr32(0, y + 1);
            for (int x = 0; x < width; x += 2) {
                *s++ = 0xFF000000;
                *s++ = 0xFFFFFFFF;
            }
        }
    }

    virtual SkISize onISize() {
        return make_isize(400, 300);
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_checkerboard(4, 4);
            fInitialized = true;
        }
        SkScalar sk32 = SkIntToScalar(32);
        canvas->clear(0x00000000);
        SkPaint bilinearPaint, bicubicPaint;
        SkSize scale = SkSize::Make(sk32, sk32);
        canvas->save();
        canvas->scale(sk32, sk32);
        bilinearPaint.setFilterLevel(SkPaint::kLow_FilterLevel);
        canvas->drawBitmap(fCheckerboard, 0, 0, &bilinearPaint);
        canvas->restore();
        SkAutoTUnref<SkImageFilter> bicubic(SkBicubicImageFilter::CreateMitchell(scale));
        bicubicPaint.setImageFilter(bicubic);
        SkRect srcBounds;
        fCheckerboard.getBounds(&srcBounds);
        canvas->translate(SkIntToScalar(140), 0);
        canvas->saveLayer(&srcBounds, &bicubicPaint);
        canvas->drawBitmap(fCheckerboard, 0, 0);
        canvas->restore();
    }

private:
    typedef GM INHERITED;
    SkBitmap fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BicubicGM; }
static GMRegistry reg(MyFactory);

}
