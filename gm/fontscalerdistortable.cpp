/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "Resources.h"
#include "SkFixed.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkTypeface.h"

namespace skiagm {

class FontScalerDistortableGM : public GM {
public:
    FontScalerDistortableGM() {
        this->setBGColor(0xFFFFFFFF);
    }

    virtual ~FontScalerDistortableGM() { }

protected:

    SkString onShortName() override {
        return SkString("fontscalerdistortable");
    }

    SkISize onISize() override {
        return SkISize::Make(550, 700);
    }

    static void rotate_about(SkCanvas* canvas, SkScalar degrees, SkScalar px, SkScalar py) {
        canvas->translate(px, py);
        canvas->rotate(degrees);
        canvas->translate(-px, -py);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);

        SkAutoTDelete<SkStreamAsset> distortable(GetResourceAsStream("/fonts/Distortable.ttf"));
        if (!distortable) {
            return;
        }
        const char* text = "abc";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 5; ++i) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkFixed axis = SkDoubleToFixed(0.5 + (5*j + i) * ((2.0 - 0.5) / (2 * 5)));
                SkAutoTUnref<SkTypeface> typeface(SkTypeface::CreateFromFontData(
                    new SkFontData(distortable->duplicate(), 0, &axis, 1)));
                paint.setTypeface(typeface);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(30 + i * 100), SkIntToScalar(20));
                rotate_about(canvas, SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.set(x - SkIntToScalar(3), SkIntToScalar(15),
                          x - SkIntToScalar(1), SkIntToScalar(280));
                    canvas->drawRect(r, p);
                }

                for (int ps = 6; ps <= 22; ps++) {
                    paint.setTextSize(SkIntToScalar(ps));
                    canvas->drawText(text, textLen, x, y, paint);
                    y += paint.getFontMetrics(nullptr);
                }
            }
            canvas->translate(0, SkIntToScalar(360));
            paint.setSubpixelText(true);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FontScalerDistortableGM; }
static GMRegistry reg(MyFactory);

}
