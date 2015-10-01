/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkConfig8888.h"
#include "SkString.h"
#include "sk_tool_utils.h"

class PremulAndUnpremulAlphaOpsBench : public Benchmark {
    enum {
        W = 256,
        H = 256,
    };
    SkBitmap fBmp1, fBmp2;

public:
    PremulAndUnpremulAlphaOpsBench(SkColorType ct) {
        fColorType = ct;
        fName.printf("premul_and_unpremul_alpha_%s", sk_tool_utils::colortype_name(ct));
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkImageInfo info = SkImageInfo::Make(W, H, fColorType, kUnpremul_SkAlphaType);
        fBmp1.allocPixels(info);   // used in writePixels

        for (int h = 0; h < H; ++h) {
            for (int w = 0; w < W; ++w) {
                // SkColor places A in the right slot for either RGBA or BGRA
                *fBmp1.getAddr32(w, h) = SkColorSetARGB(h & 0xFF, w & 0xFF, w & 0xFF, w & 0xFF);
            }
        }

        fBmp2.allocPixels(info);    // used in readPixels()
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        for (int loop = 0; loop < loops; ++loop) {
            // Unpremul -> Premul
            canvas->writePixels(fBmp1.info(), fBmp1.getPixels(), fBmp1.rowBytes(), 0, 0);
            // Premul -> Unpremul
            canvas->readPixels(fBmp2.info(), fBmp2.getPixels(), fBmp2.rowBytes(), 0, 0);
        }
    }

private:
    SkColorType fColorType;
    SkString fName;

    typedef Benchmark INHERITED;
};


DEF_BENCH(return new PremulAndUnpremulAlphaOpsBench(kRGBA_8888_SkColorType));
DEF_BENCH(return new PremulAndUnpremulAlphaOpsBench(kBGRA_8888_SkColorType));
