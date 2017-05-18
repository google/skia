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

enum class Kind {
    k001,
    k011,
    kCentered,
};

const char* kindstr(Kind k) {
    switch (k) {
        case Kind::k001:
            return "001";
        case Kind::k011:
            return "011";
        case Kind::kCentered:
            return "centered";
        default:
            return "Invalid kind";
    }
}

class HardStopGradientBench_SpecialHardStops : public Benchmark {
public:
    HardStopGradientBench_SpecialHardStops(int w, int h, Kind kind) {
        fW    = w;
        fH    = h;
        fKind = kind;

        fName.printf("hardstop_special_%03dx%03d_%s", fW, fH, kindstr(fKind));
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(fW, fH);
    }

    void onPreDraw(SkCanvas* canvas) override {
        SkPoint points[2] = {
            SkPoint::Make(   0.0f, fH/2.0f),
            SkPoint::Make(fW+2.0f, fH/2.0f),
        };

        SkColor colors[4] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
        };

        SkScalar pos_001[3] = {
            0.0f,
            0.0f,
            1.0f,
        };

        SkScalar pos_011[3] = {
            0.0f,
            1.0f,
            1.0f,
        };

        SkScalar pos_centered[4] = {
            0.0f,
            0.5f,
            0.5f,
            1.0f,
        };

        SkScalar* positions = fKind == Kind::k001 ? pos_001 :
                              fKind == Kind::k011 ? pos_011 : 
                                                    pos_centered;

        int count = fKind == Kind::kCentered ? 4 : 3;

        fPaint.setShader(SkGradientShader::MakeLinear(points,
                                                      colors,
                                                      positions,
                                                      count,
                                                      SkShader::kClamp_TileMode,
                                                      0,
                                                      nullptr));
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            canvas->drawPaint(fPaint);
        }
    }

private:
    SkString fName;
    int      fW;
    int      fH;
    Kind     fKind;
    SkPaint  fPaint; 
};

DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(100, 100, Kind::k001););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(200, 200, Kind::k001););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(300, 300, Kind::k001););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(400, 400, Kind::k001););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(500, 500, Kind::k001););

DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(100, 100, Kind::k011););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(200, 200, Kind::k011););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(300, 300, Kind::k011););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(400, 400, Kind::k011););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(500, 500, Kind::k011););

DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(100, 100, Kind::kCentered););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(200, 200, Kind::kCentered););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(300, 300, Kind::kCentered););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(400, 400, Kind::kCentered););
DEF_BENCH(return new HardStopGradientBench_SpecialHardStops(500, 500, Kind::kCentered););
