/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlendImageFilter.h"
#include "SkBitmapSource.h"

namespace skiagm {

class ImageBlendGM : public GM {
public:
    ImageBlendGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("blend");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fCheckerboard.allocPixels();
        SkDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF404040);
        SkPaint lightPaint;
        lightPaint.setColor(0xFFA0A0A0);
        for (int y = 0; y < 80; y += 16) {
          for (int x = 0; x < 80; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
          }
        }
    }

    virtual SkISize onISize() {
        return make_isize(500, 100);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), 0,
            SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, SkIntToScalar(x), 0, &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> background(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter, (SkBlendImageFilter::kNormal_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 0);
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter, (SkBlendImageFilter::kMultiply_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 100);
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter, (SkBlendImageFilter::kScreen_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 200);
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter, (SkBlendImageFilter::kDarken_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 300);
        paint.setImageFilter(SkNEW_ARGS(SkBlendImageFilter, (SkBlendImageFilter::kLighten_Mode, background)))->unref();
        drawClippedBitmap(canvas, paint, 400);
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageBlendGM; }
static GMRegistry reg(MyFactory);

}
