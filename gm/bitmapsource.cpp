/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBitmapSource.h"

// This GM exercises the SkBitmapSource ImageFilter class.

class BitmapSourceGM : public skiagm::GM {
public:
    BitmapSourceGM() {
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("bitmapsource");
    }

    void makeBitmap() {
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

    virtual SkISize onISize() SK_OVERRIDE { return SkISize::Make(500, 150); }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        this->makeBitmap();
    }

    static void fillRectFiltered(SkCanvas* canvas, const SkRect& clipRect, SkImageFilter* filter) {
        SkPaint paint;
        paint.setImageFilter(filter);
        canvas->save();
        canvas->clipRect(clipRect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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

            // Draw an unscaled bitmap.
            fillRectFiltered(canvas, clipRect, bitmapSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source bitmap (srcRect -> srcRect).
            fillRectFiltered(canvas, clipRect, bitmapSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw a subset of the bitmap scaled to a destination rect (srcRect -> dstRect).
            fillRectFiltered(canvas, clipRect, bitmapSourceSrcRectDstRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the entire bitmap scaled to a destination rect (bounds -> dstRect).
            fillRectFiltered(canvas, clipRect, bitmapSourceDstRectOnly);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    SkBitmap fBitmap;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapSourceGM; )
