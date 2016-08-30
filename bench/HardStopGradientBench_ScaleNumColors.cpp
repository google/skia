/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkCanvas.h"
#include "SkShader.h"
#include "SkGradientShader.h"
#include "SkString.h"
#include "SkColor.h"
#include "SkPaint.h"

static const char* get_tilemode_name(SkShader::TileMode tilemode) {
    switch (tilemode) {
        case SkShader::kClamp_TileMode:
            return "clamp";
        case SkShader::kRepeat_TileMode:
            return "repeat";
        case SkShader::kMirror_TileMode:
            return "mirror";
        default:
            SkDEBUGFAIL("Unknown tilemode");
            return "error";
    }
}

class HardStopGradientBench_ScaleNumColors : public Benchmark {
public:
    HardStopGradientBench_ScaleNumColors(SkShader::TileMode tilemode, int count) {
        fName.printf("hardstop_scale_num_colors_%s_%03d_colors", get_tilemode_name(tilemode), count);

        fTileMode   = tilemode;
        fColorCount = count;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(kSize, kSize);
    }

    /*
     * Set up a linear gradient from left to right with
     * fColorCount colors alternating between four
     * different colors. The positions are evenly spaced,
     * with the exception of the first two; these create a
     * hard stop in order to trigger the hard stop code.
     */
    void onPreDraw(SkCanvas* canvas) override {
        // Left to right
        SkPoint points[2] = {
            SkPoint::Make(0,        kSize/2), 
            SkPoint::Make(kSize-1,  kSize/2),
        };

        constexpr int kNumColorChoices = 4;
        SkColor color_choices[kNumColorChoices] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
        };

        // Alternate between different choices
        SkColor  colors[100];
        for (int i = 0; i < fColorCount; i++) {
            colors[i] = color_choices[i % kNumColorChoices];
        }

        // Create a hard stop
        SkScalar positions[100];
        positions[0] = 0.0f;
        positions[1] = 0.0f;
        for (int i = 2; i < fColorCount; i++) {
            // Evenly spaced afterwards
            positions[i] = i / (fColorCount - 1.0f);
        }

        fPaint.setShader(SkGradientShader::MakeLinear(points,
                                                      colors,
                                                      positions,
                                                      fColorCount,
                                                      fTileMode,
                                                      0,
                                                      nullptr));
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

    SkShader::TileMode  fTileMode;
    SkString            fName;
    int                 fColorCount;
    SkPaint             fPaint;

    typedef Benchmark INHERITED;
};

// Clamp
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,   3);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,   4);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,   5);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,  10);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,  25);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode,  50);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kClamp_TileMode, 100);)

// Repeat
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,   3);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,   4);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,   5);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,  10);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,  25);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode,  50);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kRepeat_TileMode, 100);)

// Mirror
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,   3);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,   4);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,   5);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,  10);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,  25);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode,  50);)
DEF_BENCH(return new HardStopGradientBench_ScaleNumColors(SkShader::kMirror_TileMode, 100);)
