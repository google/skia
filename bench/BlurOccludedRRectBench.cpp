/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "sk_tool_utils.h"

class BlurOccludedRRectBench : public Benchmark {
public:
    BlurOccludedRRectBench() {}

    const char* onGetName() override {
        return "bluroccludedrrect";
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(1024, 2048);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int l = 0; l < loops; ++l) {
            canvas->clear(0xFFFAFAFA);

            SkPaint opaque;
            opaque.setAntiAlias(true);
            opaque.setColor(SK_ColorWHITE);

            const SkRect r = SkRect::MakeWH(480, 230);
            const SkRRect rr = SkRRect::MakeRectXY(r, 8, 8);
            SkRect occRect = sk_tool_utils::compute_central_occluder(rr);

            for (int i = 0; i < 2; ++i) {
                canvas->save();

                canvas->translate(i*502.0f+20, 10.0f);

                for (int j = 0; j < 8; ++j) {
                    canvas->save();

                        canvas->translate(0.0f, j*256.0f);

                        SkPaint firstBlur;
                        firstBlur.setAntiAlias(true);
                        firstBlur.setColor(0x09000000);
                        firstBlur.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                                       2.5f,
                                                                       occRect));

                        canvas->drawRRect(rr, firstBlur);

                        canvas->save();
                            canvas->translate(1.5f, 1.5f);

                            SkPaint secondBlur;
                            secondBlur.setAntiAlias(true);
                            secondBlur.setColor(0x30000000);
                            secondBlur.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                                                            6.0f,
                                                                            occRect));

                            canvas->drawRRect(rr, secondBlur);

                        canvas->restore();

                        canvas->drawRRect(rr, opaque);

                    canvas->restore();
                }

                canvas->restore();
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new BlurOccludedRRectBench();)
