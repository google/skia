/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/* Tests text vertical text rendering with different fonts and centering.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkTypeface.h"

namespace skiagm {

class VertText2GM : public GM {
public:
    VertText2GM() {}

protected:
    void onOnceBeforeDraw() override {
        const int pointSize = 24;
        textHeight = SkIntToScalar(pointSize);
        fProp = SkTypeface::MakeFromName(sk_tool_utils::platform_font_name("sans-serif"),
                SkFontStyle());
        fMono = SkTypeface::MakeFromName(sk_tool_utils::platform_font_name("monospace"),
                SkFontStyle());
    }

    SkString onShortName() override {
        SkString name("verttext2");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        for (int i = 0; i < 3; ++i) {
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setAntiAlias(true);
            y = textHeight;
            canvas->drawLine(0, SkIntToScalar(10),
                    SkIntToScalar(110), SkIntToScalar(10), paint);
            canvas->drawLine(0, SkIntToScalar(240),
                    SkIntToScalar(110), SkIntToScalar(240), paint);
            canvas->drawLine(0, SkIntToScalar(470),
                    SkIntToScalar(110), SkIntToScalar(470), paint);
            drawText(canvas, SkString("Proportional / Top Aligned"),
                     fProp,  SkPaint::kLeft_Align);
            drawText(canvas, SkString("<   Proportional / Centered   >"),
                     fProp,  SkPaint::kCenter_Align);
            drawText(canvas, SkString("Monospaced / Top Aligned"),
                     fMono, SkPaint::kLeft_Align);
            drawText(canvas, SkString("<    Monospaced / Centered    >"),
                     fMono, SkPaint::kCenter_Align);
            canvas->rotate(SkIntToScalar(-15));
            canvas->translate(textHeight * 4, SkIntToScalar(50));
            if (i > 0) {
                canvas->translate(0, SkIntToScalar(50));
            }
        }
    }

    void drawText(SkCanvas* canvas, const SkString& string,
                  sk_sp<SkTypeface> family, SkPaint::Align alignment) {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        paint.setVerticalText(true);
        paint.setTextAlign(alignment);
        paint.setTypeface(std::move(family));
        paint.setTextSize(textHeight);

        canvas->drawText(string.c_str(), string.size(), y,
                SkIntToScalar(alignment == SkPaint::kLeft_Align ? 10 : 240),
                paint);
        y += textHeight;
    }

private:
    typedef GM INHERITED;
    SkScalar y, textHeight;
    sk_sp<SkTypeface> fProp;
    sk_sp<SkTypeface> fMono;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new VertText2GM; }
static GMRegistry reg(MyFactory);

}
