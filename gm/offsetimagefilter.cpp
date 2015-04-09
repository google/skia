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

namespace skiagm {

class OffsetImageFilterGM : public GM {
public:
    OffsetImageFilterGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("offsetimagefilter");
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xD000D000);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(65), paint);
    }

    virtual SkISize onISize() {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkBitmap& bitmap, const SkPaint& paint, SkScalar scale, const SkIRect& cropRect) {
        canvas->save();
        SkRect clipRect = SkRect::MakeWH(
            SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
        canvas->clipRect(clipRect);
        canvas->scale(scale, scale);
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        canvas->restore();
        SkPaint strokePaint;
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setColor(SK_ColorRED);

        // Draw a boundary rect around the intersection of the clip rect
        // and crop rect.
        SkMatrix scaleMatrix;
        scaleMatrix.setScale(scale, scale);
        SkRect cropRectFloat;
        scaleMatrix.mapRect(&cropRectFloat, SkRect::Make(cropRect));
        if (clipRect.intersect(cropRectFloat)) {
            canvas->drawRect(clipRect, strokePaint);
        }
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();

            fCheckerboard.allocN32Pixels(80, 80);
            SkCanvas checkerboardCanvas(fCheckerboard);
            sk_tool_utils::draw_checkerboard(&checkerboardCanvas, 0xFFA0A0A0, 0xFF404040, 8);

            fInitialized = true;
        }
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;

        for (int i = 0; i < 4; i++) {
            SkBitmap* bitmap = (i & 0x01) ? &fCheckerboard : &fBitmap;
            SkIRect cropRect = SkIRect::MakeXYWH(i * 12,
                                                 i * 8,
                                                 bitmap->width() - i * 8,
                                                 bitmap->height() - i * 12);
            SkImageFilter::CropRect rect(SkRect::Make(cropRect));
            SkAutoTUnref<SkImageFilter> tileInput(SkBitmapSource::Create(*bitmap));
            SkScalar dx = SkIntToScalar(i*5);
            SkScalar dy = SkIntToScalar(i*10);
            SkAutoTUnref<SkImageFilter> filter(
                SkOffsetImageFilter::Create(dx, dy, tileInput, &rect));
            paint.setImageFilter(filter);
            drawClippedBitmap(canvas, *bitmap, paint, SK_Scalar1, cropRect);
            canvas->translate(SkIntToScalar(bitmap->width() + MARGIN), 0);
        }

        SkIRect cropRect = SkIRect::MakeXYWH(0, 0, 100, 100);
        SkImageFilter::CropRect rect(SkRect::Make(cropRect));
        SkAutoTUnref<SkImageFilter> filter(
            SkOffsetImageFilter::Create(SkIntToScalar(-5), SkIntToScalar(-10), NULL, &rect));
        paint.setImageFilter(filter);
        drawClippedBitmap(canvas, fBitmap, paint, SkIntToScalar(2), cropRect);
    }
private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new OffsetImageFilterGM; }
static GMRegistry reg(MyFactory);

}
