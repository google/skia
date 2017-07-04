/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkBlendModePriv.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class XfermodeBench : public Benchmark {
public:
    XfermodeBench(SkBlendMode mode, bool aa) : fBlendMode(mode) {
        fAA = aa;
        fName.printf("blendmode_%s_%s", aa ? "mask" : "rect", SkBlendMode_Name(mode));
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        const char* text = "Hamburgefons";
        size_t len = strlen(text);
        SkISize size = canvas->getBaseLayerSize();
        SkRandom random;
        for (int i = 0; i < loops; ++i) {
            SkPaint paint;
            paint.setBlendMode(fBlendMode);
            paint.setColor(random.nextU());
            if (fAA) {
                // Draw text to exercise AA code paths.
                paint.setAntiAlias(true);
                paint.setTextSize(random.nextRangeScalar(12, 96));
                SkScalar x = random.nextRangeScalar(0, (SkScalar)size.fWidth),
                         y = random.nextRangeScalar(0, (SkScalar)size.fHeight);
                for (int j = 0; j < 1000; ++j) {
                    canvas->drawText(text, len, x, y, paint);
                }
            } else {
                // Draw rects to exercise non-AA code paths.
                SkScalar w = random.nextRangeScalar(50, 100);
                SkScalar h = random.nextRangeScalar(50, 100);
                SkRect rect = SkRect::MakeXYWH(
                    random.nextUScalar1() * (size.fWidth - w),
                    random.nextUScalar1() * (size.fHeight - h),
                    w,
                    h
                );
                for (int j = 0; j < 1000; ++j) {
                    canvas->drawRect(rect, paint);
                }
            }
        }
    }

private:
    SkBlendMode fBlendMode;
    SkString    fName;
    bool        fAA;

    typedef Benchmark INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

#define BENCH(...)                                             \
    DEF_BENCH( return new XfermodeBench(__VA_ARGS__, true); )  \
    DEF_BENCH( return new XfermodeBench(__VA_ARGS__, false); )

BENCH(SkBlendMode::kClear)
BENCH(SkBlendMode::kSrc)
BENCH(SkBlendMode::kDst)
BENCH(SkBlendMode::kSrcOver)
BENCH(SkBlendMode::kDstOver)
BENCH(SkBlendMode::kSrcIn)
BENCH(SkBlendMode::kDstIn)
BENCH(SkBlendMode::kSrcOut)
BENCH(SkBlendMode::kDstOut)
BENCH(SkBlendMode::kSrcATop)
BENCH(SkBlendMode::kDstATop)
BENCH(SkBlendMode::kXor)

BENCH(SkBlendMode::kPlus)
BENCH(SkBlendMode::kModulate)
BENCH(SkBlendMode::kScreen)

BENCH(SkBlendMode::kOverlay)
BENCH(SkBlendMode::kDarken)
BENCH(SkBlendMode::kLighten)
BENCH(SkBlendMode::kColorDodge)
BENCH(SkBlendMode::kColorBurn)
BENCH(SkBlendMode::kHardLight)
BENCH(SkBlendMode::kSoftLight)
BENCH(SkBlendMode::kDifference)
BENCH(SkBlendMode::kExclusion)
BENCH(SkBlendMode::kMultiply)

BENCH(SkBlendMode::kHue)
BENCH(SkBlendMode::kSaturation)
BENCH(SkBlendMode::kColor)
BENCH(SkBlendMode::kLuminosity)
