/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkGradient.h"
#include "include/private/base/SkTemplates.h"

using namespace skia_private;

class HardStopGradientBench_ScaleNumHardStops : public Benchmark {
public:
    HardStopGradientBench_ScaleNumHardStops(int colorCount, int hardStopCount) {
        SkASSERT(hardStopCount <= colorCount/2);

        fName.printf("hardstop_scale_num_hard_stops_%03d_colors_%03d_hard_stops",
                     colorCount, hardStopCount);

        fColorCount    = colorCount;
        fHardStopCount = hardStopCount;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkISize onGetSize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onPreDraw(SkCanvas* canvas) override {
        // Left to right
        SkPoint points[2] = {
            SkPoint::Make(0,        kSize/2),
            SkPoint::Make(kSize-1,  kSize/2),
        };

        constexpr int kNumColorChoices = 4;
        SkColor4f color_choices[kNumColorChoices] = {
            SkColors::kRed,
            SkColors::kGreen,
            SkColors::kBlue,
            SkColors::kYellow,
        };

        const size_t N = fColorCount;

        // Alternate between different choices
        AutoTArray<SkColor4f> colors(N);
        for (size_t i = 0; i < N; i++) {
            colors[i] = color_choices[i % kNumColorChoices];
        }

        // Create requisite number of hard stops, and evenly
        // space positions after that
        AutoTArray<float> positions(N);
        int k = 0;
        for (int i = 0; i < fHardStopCount; i++) {
            float val = k/2.0f;
            positions[k++] = val / N;
            positions[k++] = val / N;
        }
        for (size_t i = k; i < N; i++) {
            positions[i] = i / (N - 1.0f);
        }

        fPaint.setShader(SkShaders::LinearGradient(points,
                            {{{colors.get(), N}, {positions.get(), N}, SkTileMode::kClamp}, {}}));
    }

    /*
     * Draw simple linear gradient from left to right
     */
    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            canvas->drawPaint(fPaint);
        }
    }

private:
    static const int kSize = 500;

    SkString fName;
    int      fColorCount;
    int      fHardStopCount;
    SkPaint  fPaint;

    using INHERITED = Benchmark;
};

DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(10, 1);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(10, 2);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(10, 5);)

DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(20,  1);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(20,  5);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(20, 10);)

DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(50,  1);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(50, 10);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(50, 25);)

DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(100,  1);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(100, 25);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumHardStops(100, 50);)
