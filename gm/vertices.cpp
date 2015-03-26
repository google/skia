/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkRandom.h"

static SkShader* make_shader(SkScalar w, SkScalar h) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
        SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW,
    };
    const SkPoint pts[] = { { w/4, 0 }, { 3*w/4, h } };

    return SkGradientShader::CreateLinear(pts, colors, NULL,
                                          SK_ARRAY_COUNT(colors),
                                          SkShader::kMirror_TileMode);
}

class VerticesGM : public skiagm::GM {
    SkPoint     fPts[9];
    SkPoint     fTexs[9];
    SkColor     fColors[9];
    SkShader*   fShader;
    unsigned    fAlpha;

public:
    VerticesGM(unsigned alpha) : fShader(NULL), fAlpha(alpha) {
    }

    virtual ~VerticesGM() {
        SkSafeUnref(fShader);
    }

protected:

    void onOnceBeforeDraw() override {
        const SkScalar X = 150;
        const SkScalar Y = 150;

        fPts[0].set(0, 0);      fPts[1].set(X/2, 10);   fPts[2].set(X, 0);
        fPts[3].set(10, Y/2);   fPts[4].set(X/2, Y/2);  fPts[5].set(X-10, Y/2);
        fPts[6].set(0, Y);    fPts[7].set(X/2, Y-10);  fPts[8].set(X, Y);

        const SkScalar w = 200;
        const SkScalar h = 200;

        fTexs[0].set(0, 0);     fTexs[1].set(w/2, 0);   fTexs[2].set(w, 0);
        fTexs[3].set(0, h/2);   fTexs[4].set(w/2, h/2); fTexs[5].set(w, h/2);
        fTexs[6].set(0, h);     fTexs[7].set(w/2, h);   fTexs[8].set(w, h);

        fShader = make_shader(w, h);

        SkRandom rand;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fColors); ++i) {
            fColors[i] = rand.nextU() | 0xFF000000;
        }
    }

    SkString onShortName() override {
        SkString name("vertices");
        if (0xFF != fAlpha) {
            name.appendf("_%02X", fAlpha);
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(600, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        // start with the center of a 3x3 grid
        static const uint16_t fan[] = {
            4,
            0, 1, 2, 5, 8, 7, 6, 3, 0
        };

        const struct {
            const SkColor*  fColors;
            const SkPoint*  fTexs;
        } rec[] = {
            { fColors,  NULL },
            { NULL,     fTexs },
            { fColors,  fTexs },
        };

        const SkXfermode::Mode modes[] = {
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kModulate_Mode,
        };

        SkPaint paint;
        paint.setShader(fShader);
        paint.setAlpha(fAlpha);

        canvas->translate(20, 20);
        for (size_t j = 0; j < SK_ARRAY_COUNT(modes); ++j) {
            SkXfermode* xfer = SkXfermode::Create(modes[j]);
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
                canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode,
                                     SK_ARRAY_COUNT(fPts), fPts,
                                     rec[i].fTexs, rec[i].fColors,
                                     xfer, fan, SK_ARRAY_COUNT(fan), paint);
                canvas->translate(200, 0);
            }
            canvas->restore();
            canvas->translate(0, 200);
            xfer->unref();
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW_ARGS(VerticesGM, (0xFF)); )
DEF_GM( return SkNEW_ARGS(VerticesGM, (0x80)); )
