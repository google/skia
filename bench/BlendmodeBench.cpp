/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkBlendModePriv.h"
#include "tools/Resources.h"

namespace {
enum Type {
    kText,
    kRect,
    kSprite,
};
}

const char* gTypeNames[] = {
    "mask", "rect", "sprite",
};

// Benchmark that draws non-AA rects or AA text with an SkXfermode::Mode.
class XfermodeBench : public Benchmark {
public:
    XfermodeBench(SkBlendMode mode, Type t) : fBlendMode(mode) {
        fType = t;
        fName.printf("blendmicro_%s_%s", gTypeNames[t], SkBlendMode_Name(mode));
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        if (fType == kSprite) {
            fImage = GetResourceAsImage("images/color_wheel.png");
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const char* text = "Hamburgefons";
        size_t len = strlen(text);
        SkISize size = canvas->getBaseLayerSize();
        SkRandom random;
        while (loops > 0) {
            SkPaint paint;
            paint.setBlendMode(fBlendMode);
            paint.setColor(random.nextU());
            switch (fType) {
                case kText: {
                    // Draw text to exercise AA code paths.
                    SkFont font;
                    font.setSize(random.nextRangeScalar(12, 96));
                    SkScalar x = random.nextRangeScalar(0, (SkScalar)size.fWidth),
                             y = random.nextRangeScalar(0, (SkScalar)size.fHeight);
                    auto blob = SkTextBlob::MakeFromText(text, len, font, SkTextEncoding::kUTF8);
                    int iterations = std::min(1000, loops);
                    for (int j = 0; j < iterations; ++j) {
                        canvas->drawTextBlob(blob, x, y, paint);
                    }
                    loops -= iterations;
                } break;
                case kRect: {
                    // Draw rects to exercise non-AA code paths.
                    SkScalar w = random.nextRangeScalar(50, 100);
                    SkScalar h = random.nextRangeScalar(50, 100);
                    SkRect rect = SkRect::MakeXYWH(
                        random.nextUScalar1() * (size.fWidth - w),
                        random.nextUScalar1() * (size.fHeight - h),
                        w,
                        h
                    );
                    int iterations = std::min(1000, loops);
                    for (int j = 0; j < iterations; ++j) {
                        canvas->drawRect(rect, paint);
                    }
                    loops -= iterations;
                } break;
                case kSprite:
                    paint.setAlphaf(1.0f);
                    for (int i = 0; i < 10; ++i) {
                        canvas->drawImage(fImage, 0, 0, SkSamplingOptions(), &paint);
                    }
                    loops -= 1;
                    break;
            }
        }
    }

private:
    SkBlendMode fBlendMode;
    SkString    fName;
    sk_sp<SkImage> fImage;
    Type        fType;

    using INHERITED = Benchmark;
};

//////////////////////////////////////////////////////////////////////////////

#define BENCH(mode)                                      \
    DEF_BENCH( return new XfermodeBench(mode, kText); )  \
    DEF_BENCH( return new XfermodeBench(mode, kRect); )  \
    DEF_BENCH( return new XfermodeBench(mode, kSprite); )

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
