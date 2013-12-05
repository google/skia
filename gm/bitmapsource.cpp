/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmapSource.h"

class BitmapSourceGM : public skiagm::GM {
public:
    BitmapSourceGM() : fInitialized(false) {
    }

protected:
    virtual SkString onShortName() {
        return SkString("bitmapsource");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        fBitmap.allocPixels();
        SkBitmapDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(20), SkIntToScalar(70), paint);
    }

    virtual SkISize onISize() { return SkISize::Make(500, 150); }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            this->make_bitmap();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect dstRect = SkRect::MakeXYWH(0, 10, 60, 60);
            SkRect clipRect = SkRect::MakeXYWH(0, 0, 100, 100);
            SkRect bounds;
            fBitmap.getBounds(&bounds);
            SkAutoTUnref<SkImageFilter> bitmapSource(new SkBitmapSource(fBitmap));
            SkAutoTUnref<SkImageFilter> bitmapSourceSrcRect(new SkBitmapSource(fBitmap, srcRect, srcRect));
            SkAutoTUnref<SkImageFilter> bitmapSourceSrcRectDstRect(new SkBitmapSource(fBitmap, srcRect, dstRect));
            SkAutoTUnref<SkImageFilter> bitmapSourceDstRectOnly(new SkBitmapSource(fBitmap, bounds, dstRect));

            SkPaint paint;
            paint.setImageFilter(bitmapSource);
            canvas->save();
            canvas->clipRect(clipRect);
            canvas->drawPaint(paint);
            canvas->restore();
            canvas->translate(SkIntToScalar(100), 0);

            paint.setImageFilter(bitmapSourceSrcRect);
            canvas->save();
            canvas->clipRect(clipRect);
            canvas->drawPaint(paint);
            canvas->restore();
            canvas->translate(SkIntToScalar(100), 0);

            paint.setImageFilter(bitmapSourceSrcRectDstRect);
            canvas->save();
            canvas->clipRect(clipRect);
            canvas->drawPaint(paint);
            canvas->restore();
            canvas->translate(SkIntToScalar(100), 0);

            paint.setImageFilter(bitmapSourceDstRectOnly);
            canvas->save();
            canvas->clipRect(clipRect);
            canvas->drawPaint(paint);
            canvas->restore();
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap;
    bool fInitialized;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new BitmapSourceGM; }
static skiagm::GMRegistry reg(MyFactory);
