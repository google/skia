/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkBlurMaskFilter.h"
#include "SkLayerDrawLooper.h"

// This bench replicates a problematic use case of a draw looper used
// to create an inner blurred rect
class RectoriBench : public SkBenchmark {
public:
    RectoriBench() {}

protected:

    virtual const char* onGetName() {
        return "rectori";
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom Random;

        for (int i = 0; i < loops; i++) {
            SkScalar blurSigma = Random.nextRangeScalar(1.5f, 25.0f);
            SkScalar size = Random.nextRangeScalar(20*blurSigma, 50*blurSigma);

            SkScalar x = Random.nextRangeScalar(0.0f, W - size);
            SkScalar y = Random.nextRangeScalar(0.0f, H - size);

            SkRect inner = { x, y, x + size, y + size };

            SkRect outer(inner);
            // outer is always outset either 2x or 4x the blur radius (we go with 2x)
            outer.outset(2*blurSigma, 2*blurSigma);

            SkPath p;

            p.addRect(outer);
            p.addRect(inner);
            p.setFillType(SkPath::kEvenOdd_FillType);

            // This will be used to translate the normal draw outside the
            // clip rect and translate the blurred version back inside
            SkScalar translate = 2.0f * size;

            SkPaint paint;
            paint.setLooper(this->createLooper(-translate, blurSigma))->unref();
            paint.setColor(0xff000000 | Random.nextU());
            paint.setAntiAlias(true);

            canvas->save();
            // clip always equals inner rect so we get the inside blur
            canvas->clipRect(inner);
            canvas->translate(translate, 0);
            canvas->drawPath(p, paint);
            canvas->restore();
        }
    }

private:
    enum {
        W = 640,
        H = 480,
    };

    SkLayerDrawLooper* createLooper(SkScalar xOff, SkScalar sigma) {
        SkLayerDrawLooper* looper = new SkLayerDrawLooper;

        //-----------------------------------------------
        SkLayerDrawLooper::LayerInfo info;

        info.fFlagsMask = 0;
        // TODO: add a color filter to better match what is seen in the wild
        info.fPaintBits = /* SkLayerDrawLooper::kColorFilter_Bit |*/
                          SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkXfermode::kDst_Mode;
        info.fOffset.set(xOff, 0);
        info.fPostTranslate = false;

        SkPaint* paint = looper->addLayer(info);

        SkMaskFilter* mf = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                                    sigma,
                                                    SkBlurMaskFilter::kHighQuality_BlurFlag);
        paint->setMaskFilter(mf)->unref();

        //-----------------------------------------------
        info.fPaintBits = 0;
        info.fOffset.set(0, 0);

        paint = looper->addLayer(info);
        return looper;
    }

    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return SkNEW_ARGS(RectoriBench, ()); )
