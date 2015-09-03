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
    SkString onShortName() override {
        return SkString("bitmapsource");
    }

    SkISize onISize() override { return SkISize::Make(500, 150); }

    void onOnceBeforeDraw() override {
        fBitmap = sk_tool_utils::create_string_bitmap(100, 100, 0xFFFFFFFF, 20, 70, 96, "e");
    }

    static void FillRectFiltered(SkCanvas* canvas, const SkRect& clipRect, SkImageFilter* filter) {
        SkPaint paint;
        paint.setImageFilter(filter);
        canvas->save();
        canvas->clipRect(clipRect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect dstRect = SkRect::MakeXYWH(0, 10, 60, 60);
            SkRect clipRect = SkRect::MakeXYWH(0, 0, 100, 100);
            SkRect bounds;
            fBitmap.getBounds(&bounds);
            SkAutoTUnref<SkImageFilter> bitmapSource(SkBitmapSource::Create(fBitmap));
            SkAutoTUnref<SkImageFilter> bitmapSourceSrcRect(SkBitmapSource::Create(fBitmap, srcRect, srcRect));
            SkAutoTUnref<SkImageFilter> bitmapSourceSrcRectDstRect(SkBitmapSource::Create(fBitmap, srcRect, dstRect));
            SkAutoTUnref<SkImageFilter> bitmapSourceDstRectOnly(SkBitmapSource::Create(fBitmap, bounds, dstRect));

            // Draw an unscaled bitmap.
            FillRectFiltered(canvas, clipRect, bitmapSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source bitmap (srcRect -> srcRect).
            FillRectFiltered(canvas, clipRect, bitmapSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw a subset of the bitmap scaled to a destination rect (srcRect -> dstRect).
            FillRectFiltered(canvas, clipRect, bitmapSourceSrcRectDstRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the entire bitmap scaled to a destination rect (bounds -> dstRect).
            FillRectFiltered(canvas, clipRect, bitmapSourceDstRectOnly);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    SkBitmap fBitmap;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapSourceGM; )
