/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkCanvas.h"
#include "SkMixer.h"
#include "SkPaint.h"
#include "SkShader.h"

class MixerLerpBench : public Benchmark {
public:
    MixerLerpBench() {}

protected:
    const char* onGetName() override { return "mixer-lerp"; }

    void onDelayedSetup() override {
        auto s0 = SkShaders::Color(SK_ColorRED);
        auto s1 = SkShaders::Color(SK_ColorBLUE);
        fShader = SkShaders::Lerp(SkShaders::Color(0xFF880000), s0, s1);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect r = {0, 0, 256, 256};
        SkPaint paint;
        paint.setShader(fShader);
        for (int j = 0; j < 100; ++j) {
            for (int i = 0; i < loops; i++) {
                canvas->drawRect(r, paint);
            }
        }
    }

private:
    sk_sp<SkShader> fShader;
    typedef Benchmark INHERITED;
};
DEF_BENCH( return new MixerLerpBench; )
