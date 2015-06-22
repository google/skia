/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkBitmapSource.h"
#include "SkOffsetImageFilter.h"
#include "gm.h"

#define WIDTH 600
#define HEIGHT 100
#define MARGIN 12

class OffsetImageFilterGM : public skiagm::GM {
public:
    OffsetImageFilterGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("offsetimagefilter");
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xD000D000);
        paint.setTextSize(96);
        canvas.drawText("e", 1, 15, 65, paint);
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint,
                           SkScalar scale, const SkIRect& cropRect) {
        SkRect clipRect = SkRect::MakeIWH(bitmap.width(), bitmap.height());

        canvas->save();
        canvas->clipRect(clipRect);
        canvas->scale(scale, scale);
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();

        // Draw a boundary rect around the intersection of the clip rect and crop rect.
        SkRect cropRectFloat;
        SkMatrix::MakeScale(scale, scale).mapRect(&cropRectFloat, SkRect::Make(cropRect));
        if (clipRect.intersect(cropRectFloat)) {
            SkPaint strokePaint;
            strokePaint.setStyle(SkPaint::kStroke_Style);
            strokePaint.setColor(SK_ColorRED);
            canvas->drawRect(clipRect, strokePaint);
        }
    }

    void onOnceBeforeDraw() override {
        make_bitmap();
        
        fCheckerboard.allocN32Pixels(80, 80);
        SkCanvas checkerboardCanvas(fCheckerboard);
        sk_tool_utils::draw_checkerboard(&checkerboardCanvas, 0xFFA0A0A0, 0xFF404040, 8);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;

        for (int i = 0; i < 4; i++) {
            const SkBitmap* bitmap = (i & 0x01) ? &fCheckerboard : &fBitmap;
            SkIRect cropRect = SkIRect::MakeXYWH(i * 12,
                                                 i * 8,
                                                 bitmap->width() - i * 8,
                                                 bitmap->height() - i * 12);
            SkImageFilter::CropRect rect(SkRect::Make(cropRect));
            SkAutoTUnref<SkImageFilter> tileInput(SkBitmapSource::Create(*bitmap));
            SkScalar dx = SkIntToScalar(i*5);
            SkScalar dy = SkIntToScalar(i*10);
            SkAutoTUnref<SkImageFilter> filter(SkOffsetImageFilter::Create(dx, dy, tileInput,
                                                                           &rect));
            paint.setImageFilter(filter);
            drawClippedBitmap(canvas, *bitmap, paint, 1, cropRect);
            canvas->translate(SkIntToScalar(bitmap->width() + MARGIN), 0);
        }

        SkIRect cropRect = SkIRect::MakeXYWH(0, 0, 100, 100);
        SkImageFilter::CropRect rect(SkRect::Make(cropRect));
        SkAutoTUnref<SkImageFilter> filter(SkOffsetImageFilter::Create(-5, -10, NULL, &rect));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, 2, cropRect);
    }
private:
    typedef skiagm::GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
};
DEF_GM( return new OffsetImageFilterGM; )

//////////////////////////////////////////////////////////////////////////////

