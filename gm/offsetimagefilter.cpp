/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <utility>

#define WIDTH 600
#define HEIGHT 100
#define MARGIN 12

class OffsetImageFilterGM : public skiagm::GM {
public:
    OffsetImageFilterGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString getName() const override { return SkString("offsetimagefilter"); }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onOnceBeforeDraw() override {
        fBitmap = ToolUtils::CreateStringImage(80, 80, 0xD000D000, 15, 65, 96, "e");

        fCheckerboard = ToolUtils::create_checkerboard_image(80, 80, 0xFFA0A0A0, 0xFF404040, 8);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;

        for (int i = 0; i < 4; i++) {
            sk_sp<SkImage> image = (i & 0x01) ? fCheckerboard : fBitmap;
            SkIRect cropRect = SkIRect::MakeXYWH(i * 12,
                                                 i * 8,
                                                 image->width() - i * 8,
                                                 image->height() - i * 12);
            sk_sp<SkImageFilter> tileInput(SkImageFilters::Image(image, SkFilterMode::kNearest));
            SkScalar dx = SkIntToScalar(i*5);
            SkScalar dy = SkIntToScalar(i*10);
            paint.setImageFilter(SkImageFilters::Offset(dx, dy, std::move(tileInput), &cropRect));
            DrawClippedImage(canvas, image.get(), paint, 1, cropRect);
            canvas->translate(SkIntToScalar(image->width() + MARGIN), 0);
        }

        SkIRect cropRect = SkIRect::MakeXYWH(0, 0, 100, 100);
        paint.setImageFilter(SkImageFilters::Offset(-5, -10, nullptr, &cropRect));
        DrawClippedImage(canvas, fBitmap.get(), paint, 2, cropRect);
    }
private:
    static void DrawClippedImage(SkCanvas* canvas, const SkImage* image, const SkPaint& paint,
                          SkScalar scale, const SkIRect& cropRect) {
        SkRect clipRect = SkRect::MakeIWH(image->width(), image->height());

        canvas->save();
        canvas->clipRect(clipRect);
        canvas->scale(scale, scale);
        canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
        canvas->restore();

        // Draw a boundary rect around the intersection of the clip rect and crop rect.
        SkRect cropRectFloat;
        SkMatrix::Scale(scale, scale).mapRect(&cropRectFloat, SkRect::Make(cropRect));
        if (clipRect.intersect(cropRectFloat)) {
            SkPaint strokePaint;
            strokePaint.setStyle(SkPaint::kStroke_Style);
            strokePaint.setStrokeWidth(2);
            strokePaint.setColor(SK_ColorRED);
            canvas->drawRect(clipRect, strokePaint);
        }
    }

    sk_sp<SkImage> fBitmap, fCheckerboard;

    using INHERITED = skiagm::GM;
};
DEF_GM( return new OffsetImageFilterGM; )

//////////////////////////////////////////////////////////////////////////////

class SimpleOffsetImageFilterGM : public skiagm::GM {
public:
    SimpleOffsetImageFilterGM() {}

protected:
    SkString getName() const override { return SkString("simple-offsetimagefilter"); }

    SkISize getISize() override { return SkISize::Make(640, 200); }

    void doDraw(SkCanvas* canvas, const SkRect& r, sk_sp<SkImageFilter> imgf,
                const SkIRect* cropR = nullptr, const SkRect* clipR = nullptr) {
        SkPaint p;

        if (clipR) {
            p.setColor(0xFF00FF00);
            p.setStyle(SkPaint::kStroke_Style);
            canvas->drawRect(clipR->makeInset(SK_ScalarHalf, SK_ScalarHalf), p);
            p.setStyle(SkPaint::kFill_Style);
        }

        // Visualize the crop rect for debugging
        if (imgf && cropR) {
            p.setColor(0x66FF00FF);
            p.setStyle(SkPaint::kStroke_Style);

            SkRect cr = SkRect::Make(*cropR).makeInset(SK_ScalarHalf, SK_ScalarHalf);
            canvas->drawRect(cr, p);
            p.setStyle(SkPaint::kFill_Style);
        }

        p.setColor(0x660000FF);
        canvas->drawRect(r, p);

        if (clipR) {
            canvas->save();
            canvas->clipRect(*clipR);
        }
        if (imgf) {
            p.setImageFilter(std::move(imgf));
        }
        p.setColor(0x66FF0000);
        canvas->drawRect(r, p);

        if (clipR) {
            canvas->restore();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkIRect cr0 = SkIRect::MakeWH(40, 40);
        SkIRect cr1 = SkIRect::MakeWH(20, 20);
        SkIRect cr2 = SkIRect::MakeXYWH(40, 0, 40, 40);
        const SkRect r = SkRect::Make(cr0);
        const SkRect r2 = SkRect::Make(cr2);

        canvas->translate(40, 40);

        canvas->save();
        this->doDraw(canvas, r, nullptr);

        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(20, 20, nullptr));

        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(20, 20, nullptr, &cr0), &cr0);

        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(20, 20, nullptr), /* cropR */ nullptr, &r);

        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(20, 20, nullptr, &cr1), &cr1);

        SkRect clipR = SkRect::MakeXYWH(40, 40, 40, 40);
        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(20, 20, nullptr), /* cropR */ nullptr, &clipR);
        canvas->restore();

        // 2nd row
        canvas->translate(0, 80);

        /*
         *  combos of clip and crop rects that align with src and dst
         */

        // crop==clip==src
        this->doDraw(canvas, r, SkImageFilters::Offset(40, 0, nullptr, &cr0), &cr0, &r);

        // crop==src, clip==dst
        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(40, 0, nullptr, &cr0), &cr0, &r2);

        // crop==dst, clip==src
        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(40, 0, nullptr, &cr2), &cr2, &r);

        // crop==clip==dst
        canvas->translate(100, 0);
        this->doDraw(canvas, r, SkImageFilters::Offset(40, 0, nullptr, &cr2), &cr2, &r2);
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new SimpleOffsetImageFilterGM; )
