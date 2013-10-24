/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "SkCanvas.h"
#include "SkTypeface.h"

namespace skiagm {

#define TEXT_SIZE   48
static const char gText[] = "Hello";

//Before shaping
//static const char gText[] = "「テスト。」";
//static const char gText[] = {0xE3,0x80,0x8C, 0xE3,0x83,0x86, 0xE3,0x82,0xB9, 0xE3,0x83,0x88, 0xE3,0x80,0x82, 0xE3,0x80,0x8D, 0x0};

//After shaping
//static const char gText[] = "﹁テスト︒﹂";
//static const char gText[] = {0xEF,0xB9,0x81, 0xE3,0x83,0x86, 0xE3,0x82,0xB9, 0xE3,0x83,0x88, 0xEF,0xB8,0x92, 0xEF,0xB9,0x82, 0x0};

static const size_t gLen = sizeof(gText) - sizeof(gText[0]);

class VertTextGM : public GM {
public:
    VertTextGM()
        // : fFace(SkTypeface::CreateFromName("unifont", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromFile("MotoyaL04Mincho_3.ttf"))

        // Bitmap fonts on OS X.
        // : fFace(SkTypeface::CreateFromName("GB18030 Bitmap", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("Apple Color Emoji", SkTypeface::kNormal))

        // OTF CFF fonts on OS X.
        // : fFace(SkTypeface::CreateFromName("Hiragino Mincho ProN", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("Hiragino Kaku Gothic Pro", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("Hiragino Sans GB", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("STIXGeneral", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("Yuppy SC", SkTypeface::kNormal))
        // : fFace(SkTypeface::CreateFromName("Yuppy TC", SkTypeface::kNormal))

    {
    }

    //SkAutoTUnref<SkTypeface> fFace;

protected:

    SkString onShortName() {
        return SkString("verttext");
    }

    SkISize onISize() { return make_isize(640, 480); }

    static void drawBaseline(SkCanvas* canvas, const SkPaint& paint,
                             SkScalar x, SkScalar y) {
        SkScalar total = paint.measureText(gText, gLen);

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0x80FF0000);
        canvas->drawLine(x, y,
                         paint.isVerticalText() ? x : x + total,
                         paint.isVerticalText() ? y + total : y,
                         p);

        p.setColor(0xFF0000FF);
        SkScalar adv[gLen];
        int numChars = paint.getTextWidths(gText, gLen, adv, NULL);
        for (int i = 0; i < numChars; ++i) {
            canvas->drawCircle(x, y, SK_Scalar1 * 3 / 2, p);
            if (paint.isVerticalText()) {
                y += adv[i];
            } else {
                x += adv[i];
            }
        }
        canvas->drawCircle(x, y, SK_Scalar1 * 3 / 2, p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(50);

        for (int i = 0; i < 4; ++i) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setTextSize(SkIntToScalar(TEXT_SIZE));
            //paint.setTypeface(fFace);
            //paint.setFakeBoldText(true);

            paint.setVerticalText(false);
            drawBaseline(canvas, paint, x, y);
            canvas->drawText(gText, gLen, x, y, paint);

            paint.setVerticalText(true);
            drawBaseline(canvas, paint, x, y);
            canvas->drawText(gText, gLen, x, y, paint);

            x += SkIntToScalar(40);
            y += SkIntToScalar(120);

            canvas->rotate(SkIntToScalar(-15));
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new VertTextGM; }
static GMRegistry reg(MyFactory);

}
