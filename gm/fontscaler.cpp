/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkTypeface.h"

namespace skiagm {

static const struct {
    const char* fName;
    SkTypeface::Style fStyle;
} gFaces[] = {
    { NULL, SkTypeface::kNormal },
    { NULL, SkTypeface::kBold },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal }
};

static const int gFaceCount = SK_ARRAY_COUNT(gFaces);

class FontScalerGM : public GM {
public:
    FontScalerGM() {
        this->setBGColor(0xFFFFFFFF);

        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i].fName,
                                                   gFaces[i].fStyle);
        }
    }

    virtual ~FontScalerGM() {
        for (int i = 0; i < gFaceCount; i++) {
            SkSafeUnref(fFaces[i]);
        }
    }

protected:
    virtual SkString onShortName() {
        return SkString("fontscaler");
    }

    virtual SkISize onISize() {
        return make_isize(1450, 750);
    }

    static void rotate_about(SkCanvas* canvas, SkScalar degrees, SkScalar px, SkScalar py) {
        canvas->translate(px, py);
        canvas->rotate(degrees);
        canvas->translate(-px, -py);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;

        // test handling of obscene cubic values (currently broken)
        if (false) {
            SkPoint pts[4];
            pts[0].set(1.61061274e+09f, 6291456);
            pts[1].set(-7.18397061e+15f, -1.53091184e+13f);
            pts[2].set(-1.30077315e+16f, -2.77196141e+13f);
            pts[3].set(-1.30077315e+16f, -2.77196162e+13f);

            SkPath path;
            path.moveTo(pts[0]);
            path.cubicTo(pts[1], pts[2], pts[3]);
            canvas->drawPath(path, paint);
        }

        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        //With freetype the default (normal hinting) can be really ugly.
        //Most distros now set slight (vertical hinting only) in any event.
        paint.setHinting(SkPaint::kSlight_Hinting);
        SkSafeUnref(paint.setTypeface(SkTypeface::CreateFromName("Times Roman", SkTypeface::kNormal)));

//        const char* text = "abcdefghijklmnopqrstuvwxyz";
        const char* text = "Hamburgefons ooo mmm";
        const size_t textLen = strlen(text);

        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 6; ++i) {
                SkScalar x = SkIntToScalar(10);
                SkScalar y = SkIntToScalar(20);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SkIntToScalar(50 + i * 230),
                                  SkIntToScalar(20));
                rotate_about(canvas, SkIntToScalar(i * 5), x, y * 10);

                {
                    SkPaint p;
                    p.setAntiAlias(true);
                    SkRect r;
                    r.set(x-3, 15, x-1, 280);
                    canvas->drawRect(r, p);
                }

                int index = 0;
                for (int ps = 6; ps <= 22; ps++) {
                    paint.setTextSize(SkIntToScalar(ps));
                    canvas->drawText(text, textLen, x, y, paint);
                    y += paint.getFontMetrics(NULL);
                    index += 1;
                }
            }
            canvas->translate(0, 360);
            paint.setSubpixelText(true);
        }
    }

private:
    SkTypeface* fFaces[gFaceCount];
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new FontScalerGM; }
static GMRegistry reg(MyFactory);

}

