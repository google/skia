/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkMorphologyImageFilter.h"

#define WIDTH 700
#define HEIGHT 560

namespace skiagm {

class MorphologyGM : public GM {
public:
    MorphologyGM() {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
        return SkString("morphology");
    }

    void onOnceBeforeDraw() override {
        fBitmap.allocN32Pixels(135, 135);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x0);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        const char* str1 = "ABC";
        const char* str2 = "XYZ";
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(64);
        canvas.drawString(str1, 10, 55, paint);
        canvas.drawString(str2, 10, 110, paint);
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void drawClippedBitmap(SkCanvas* canvas, const SkPaint& paint, int x, int y) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(
          SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        struct {
            int fWidth, fHeight;
            int fRadiusX, fRadiusY;
        } samples[] = {
            { 140, 140,   0,   0 },
            { 140, 140,   0,   2 },
            { 140, 140,   2,   0 },
            { 140, 140,   2,   2 },
            {  24,  24,  25,  25 },
        };
        SkPaint paint;
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(25, 20, 100, 80));

        for (unsigned j = 0; j < 4; ++j) {
            for (unsigned i = 0; i < SK_ARRAY_COUNT(samples); ++i) {
                const SkImageFilter::CropRect* cr = j & 0x02 ? &cropRect : nullptr;
                if (j & 0x01) {
                    paint.setImageFilter(SkErodeImageFilter::Make(samples[i].fRadiusX,
                                                                  samples[i].fRadiusY,
                                                                  nullptr,
                                                                  cr));
                } else {
                    paint.setImageFilter(SkDilateImageFilter::Make(samples[i].fRadiusX,
                                                                   samples[i].fRadiusY,
                                                                   nullptr,
                                                                   cr));
                }
                this->drawClippedBitmap(canvas, paint, i * 140, j * 140);
            }
        }
    }

private:
    SkBitmap fBitmap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MorphologyGM;)

}
