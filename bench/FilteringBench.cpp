/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

class FilteringBench : public Benchmark {
public:
    FilteringBench(SkFilterMode fm, SkMipmapMode mm) : fSampling(fm, mm) {
        fName.printf("samplingoptions_filter_%d_mipmap_%d", (int)fm, (int)mm);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        auto img = ToolUtils::GetResourceAsImage("images/ship.png");
        // need to force raster since lazy doesn't support filteroptions yet
        img = img->makeRasterImage();

        fRect = SkRect::MakeIWH(img->width(), img->height());
        fShader = img->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, fSampling);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        // scale so we will trigger lerping between levels if we mipmapping
        canvas->scale(0.75f, 0.75f);

        SkPaint paint;
        paint.setShader(fShader);
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < 10; ++j) {
                canvas->drawRect(fRect, paint);
            }
        }
    }

private:
    SkString        fName;
    SkRect          fRect;
    sk_sp<SkShader> fShader;
    SkSamplingOptions fSampling;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new FilteringBench(SkFilterMode::kLinear,  SkMipmapMode::kLinear); )
DEF_BENCH( return new FilteringBench(SkFilterMode::kLinear,  SkMipmapMode::kNearest); )
DEF_BENCH( return new FilteringBench(SkFilterMode::kLinear,  SkMipmapMode::kNone); )

DEF_BENCH( return new FilteringBench(SkFilterMode::kNearest, SkMipmapMode::kLinear); )
DEF_BENCH( return new FilteringBench(SkFilterMode::kNearest, SkMipmapMode::kNearest); )
DEF_BENCH( return new FilteringBench(SkFilterMode::kNearest, SkMipmapMode::kNone); )
