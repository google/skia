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

static SkShader* make_shader(int w, int h) {
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    const SkPoint center = { SkScalarHalf(w), SkScalarHalf(h) };
    const SkScalar radius = w / 2;

    return SkGradientShader::CreateRadial(center, radius, colors, NULL,
                                          SK_ARRAY_COUNT(colors),
                                          SkShader::kMirror_TileMode);
}

class VerticesGM : public skiagm::GM {
    SkPoint     fPts[9];
    SkPoint     fTexs[9];
    SkColor     fColors[9];
    SkShader*   fShader;

public:
    VerticesGM() : fShader(NULL) {
    }

    virtual ~VerticesGM() {
        SkSafeUnref(fShader);
    }

protected:
    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        fPts[0].set(0, 0);      fPts[1].set(100, 10);   fPts[2].set(200, 0);
        fPts[3].set(10, 100);   fPts[4].set(100, 100);  fPts[5].set(190, 100);
        fPts[6].set(0, 200);    fPts[7].set(100, 190);  fPts[8].set(200, 200);

        int w = 200;
        int h = 200;

        fTexs[0].set(0, 0);     fTexs[1].set(w/2, 0);   fTexs[2].set(w, 0);
        fTexs[3].set(0, h/2);   fTexs[4].set(w/2, h/2); fTexs[5].set(w, h/2);
        fTexs[6].set(0, h);     fTexs[7].set(w/2, h);   fTexs[8].set(w, h);

        fShader = make_shader(w, h);

        SkRandom rand;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fColors); ++i) {
            fColors[i] = rand.nextU() | 0xFF202020;
        }
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("vertices");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(800, 800);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
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

        canvas->translate(20, 20);
        for (size_t j = 0; j < SK_ARRAY_COUNT(modes); ++j) {
            SkXfermode* xfer = SkXfermode::Create(modes[j]);
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
                canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode,
                                     SK_ARRAY_COUNT(fPts), fPts,
                                     rec[i].fTexs, rec[i].fColors,
                                     xfer, fan, SK_ARRAY_COUNT(fan), paint);
                canvas->translate(250, 0);
            }
            canvas->restore();
            canvas->translate(0, 250);
            xfer->unref();
        }
    }

    virtual uint32_t onGetFlags() const {
        return kSkipPipe_Flag | kSkipPicture_Flag;
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(VerticesGM); )
