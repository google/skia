/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

class ClipStrategyBench : public Benchmark {
public:
    enum class Mode {
        kClipPath,
        kMask,
    };

    ClipStrategyBench(Mode mode, size_t count)
        : fMode(mode)
        , fCount(count)
        , fName("clip_strategy_"){

        if (fMode == Mode::kClipPath) {
            fName.append("path_");
            this->forEachClipCircle([&](float x, float y, float r) {
                fClipPath.addCircle(x, y, r);
            });
        } else {
            fName.append("mask_");
        }
        fName.appendf("%zu", count);
    }

    ~ClipStrategyBench() override = default;

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint p, srcIn;
        p.setAntiAlias(true);
        srcIn.setBlendMode(SkBlendMode::kSrcIn);

        for (int i = 0; i < loops; ++i) {
            SkAutoCanvasRestore acr(canvas, false);

            if (fMode == Mode::kClipPath) {
                canvas->save();
                canvas->clipPath(fClipPath, true);
            } else {
                canvas->saveLayer(nullptr, nullptr);
                this->forEachClipCircle([&](float x, float y, float r) {
                    canvas->drawCircle(x, y, r, p);
                });
                canvas->saveLayer(nullptr, &srcIn);
            }
            canvas->drawColor(SK_ColorGREEN);
        }
    }

private:
    template <typename Func>
    void forEachClipCircle(Func&& func) {
        auto q = static_cast<float>(this->getSize().width()) / (fCount + 1);
        for (size_t i = 1; i <= fCount; ++i) {
            auto x = q * i;
            func(x, x, q / 2);
        }
    }

    Mode     fMode;
    size_t   fCount;
    SkString fName;
    SkPath   fClipPath;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kClipPath, 1  );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kClipPath, 5  );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kClipPath, 10 );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kClipPath, 100);)

DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kMask, 1  );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kMask, 5  );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kMask, 10 );)
DEF_BENCH( return new ClipStrategyBench(ClipStrategyBench::Mode::kMask, 100);)
