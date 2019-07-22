/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "include/utils/SkRandom.h"

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER

// This bench replicates a problematic use case of a draw looper used
// to create an inner blurred rect
class RectoriBench : public Benchmark {
public:
    RectoriBench() {}

protected:

    const char* onGetName() override {
        return "rectori";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
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
            paint.setLooper(this->createLooper(-translate, blurSigma));
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

    sk_sp<SkDrawLooper> createLooper(SkScalar xOff, SkScalar sigma) {
        SkLayerDrawLooper::Builder looperBuilder;

        //-----------------------------------------------
        SkLayerDrawLooper::LayerInfo info;

        // TODO: add a color filter to better match what is seen in the wild
        info.fPaintBits = /* SkLayerDrawLooper::kColorFilter_Bit |*/
                          SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkBlendMode::kDst;
        info.fOffset.set(xOff, 0);
        info.fPostTranslate = false;

        SkPaint* paint = looperBuilder.addLayer(info);

        paint->setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));

        //-----------------------------------------------
        info.fPaintBits = 0;
        info.fOffset.set(0, 0);

        paint = looperBuilder.addLayer(info);
        return looperBuilder.detach();
    }

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new RectoriBench();)

#endif
