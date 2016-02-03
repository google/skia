/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkTestImageFilters.h"

///////////////////////////////////////////////////////////////////////////////

static void draw_paint(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorBLACK);
    canvas->save();
    canvas->clipRect(r);
    canvas->drawPaint(paint);
    canvas->restore();
}

static void draw_path(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    paint.setImageFilter(imf);
    paint.setAntiAlias(true);
    canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
}

static void draw_text(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;
    paint.setImageFilter(imf);
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setTextSize(r.height()/2);
    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas->drawText("Text", 4, r.centerX(), r.centerY(), paint);
}

static void draw_bitmap(SkCanvas* canvas, const SkRect& r, SkImageFilter* imf) {
    SkPaint paint;

    SkIRect bounds;
    r.roundOut(&bounds);

    SkBitmap bm;
    bm.allocN32Pixels(bounds.width(), bounds.height());
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas c(bm);
    draw_path(&c, r, nullptr);

    paint.setImageFilter(imf);
    canvas->drawBitmap(bm, 0, 0, &paint);
}

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersCroppedGM : public skiagm::GM {
public:
    ImageFiltersCroppedGM () {}

protected:
    SkString onShortName() override {
        return SkString("imagefilterscropped");
    }

    SkISize onISize() override { return SkISize::Make(400, 960); }

    void make_checkerboard() {
        fCheckerboard.allocN32Pixels(80, 80);
        SkCanvas canvas(fCheckerboard);
        canvas.clear(SK_ColorTRANSPARENT);
        SkPaint darkPaint;
        darkPaint.setColor(sk_tool_utils::color_to_565(0xFF404040));
        SkPaint lightPaint;
        lightPaint.setColor(sk_tool_utils::color_to_565(0xFFA0A0A0));
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

    void draw_frame(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawRect(r, paint);
    }

    virtual void onOnceBeforeDraw() override{
        make_checkerboard();
    }

    virtual void onDraw(SkCanvas* canvas) override {
        void (*drawProc[])(SkCanvas*, const SkRect&, SkImageFilter*) = {
            draw_bitmap, draw_path, draw_paint, draw_text
        };

        SkAutoTUnref<SkColorFilter> cf(
            SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
        SkImageFilter::CropRect cropRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)), SkImageFilter::CropRect::kHasAll_CropEdge);
        SkImageFilter::CropRect bogusRect(SkRect::Make(SkIRect::MakeXYWH(-100, -100, 10, 10)), SkImageFilter::CropRect::kHasAll_CropEdge);

        SkAutoTUnref<SkImageFilter> offset(SkOffsetImageFilter::Create(
            SkIntToScalar(-10), SkIntToScalar(-10)));

        SkAutoTUnref<SkImageFilter> cfOffset(SkColorFilterImageFilter::Create(cf.get(), offset.get()));

        SkAutoTUnref<SkImageFilter> erodeX(SkErodeImageFilter::Create(8, 0, nullptr, &cropRect));
        SkAutoTUnref<SkImageFilter> erodeY(SkErodeImageFilter::Create(0, 8, nullptr, &cropRect));

        SkImageFilter* filters[] = {
            nullptr,
            SkColorFilterImageFilter::Create(cf.get(), nullptr, &cropRect),
            SkBlurImageFilter::Create(0.0f, 0.0f, nullptr, &cropRect),
            SkBlurImageFilter::Create(1.0f, 1.0f, nullptr, &cropRect),
            SkBlurImageFilter::Create(8.0f, 0.0f, nullptr, &cropRect),
            SkBlurImageFilter::Create(0.0f, 8.0f, nullptr, &cropRect),
            SkBlurImageFilter::Create(8.0f, 8.0f, nullptr, &cropRect),
            SkErodeImageFilter::Create(1, 1, nullptr, &cropRect),
            SkErodeImageFilter::Create(8, 0, erodeY, &cropRect),
            SkErodeImageFilter::Create(0, 8, erodeX, &cropRect),
            SkErodeImageFilter::Create(8, 8, nullptr, &cropRect),
            SkMergeImageFilter::Create(nullptr, cfOffset.get(), SkXfermode::kSrcOver_Mode, &cropRect),
            SkBlurImageFilter::Create(8.0f, 8.0f, nullptr, &bogusRect),
            SkColorFilterImageFilter::Create(cf.get(), nullptr, &bogusRect),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(16);
        SkScalar DX = r.width() + MARGIN;
        SkScalar DY = r.height() + MARGIN;

        canvas->translate(MARGIN, MARGIN);
        for (size_t j = 0; j < SK_ARRAY_COUNT(drawProc); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkPaint paint;
                canvas->drawBitmap(fCheckerboard, 0, 0);
                drawProc[j](canvas, r, filters[i]);
                canvas->translate(0, DY);
            }
            canvas->restore();
            canvas->translate(DX, 0);
        }

        for(size_t j = 0; j < SK_ARRAY_COUNT(filters); ++j) {
            SkSafeUnref(filters[j]);
        }
    }

private:
    SkBitmap fCheckerboard;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageFiltersCroppedGM; )
