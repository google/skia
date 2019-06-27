/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This benchmark attempts to measure the time to do a fullscreen clear, an axis-aligned partial
// clear, and a clear restricted to an axis-aligned rounded rect. The fullscreen and axis-aligned
// partial clears on the GPU should follow a fast path that maps to backend-specialized clear
// operations, whereas the rounded-rect clear cannot be.

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/effects/SkGradientShader.h"

#include "src/gpu/GrRenderTargetContext.h"

static sk_sp<SkShader> make_shader() {
    static const SkPoint kPts[] = {{0, 0}, {10, 10}};
    static const SkColor kColors[] = {SK_ColorBLUE, SK_ColorWHITE};
    return SkGradientShader::MakeLinear(kPts, kColors, nullptr, 2, SkTileMode::kClamp);
}

class ClearBench : public Benchmark {
public:
    enum ClearType {
        kFull_ClearType,
        kPartial_ClearType,
        kComplex_ClearType
    };

    ClearBench(ClearType type) : fType(type) {}

protected:
    const char* onGetName() override {
        switch(fType) {
        case kFull_ClearType:
            return "Clear-Full";
        case kPartial_ClearType:
            return "Clear-Partial";
        case kComplex_ClearType:
            return "Clear-Complex";
        }
        SkASSERT(false);
        return "Unreachable";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        static const SkRect kPartialClip = SkRect::MakeLTRB(50, 50, 400, 400);
        static const SkRRect kComplexClip = SkRRect::MakeRectXY(kPartialClip, 15, 15);
        // Small to limit fill cost, but intersects the clips to confound batching
        static const SkRect kInterruptRect = SkRect::MakeXYWH(200, 200, 3, 3);

        // For the draw that sits between consecutive clears, use a shader that is simple but
        // requires local coordinates so that Ganesh does not convert it into a solid color rect,
        // which could then turn into a scissored-clear behind the scenes.
        SkPaint interruptPaint;
        interruptPaint.setShader(make_shader());

        GrRenderTargetContext* rtc = canvas->internal_private_accessTopLayerRenderTargetContext();
        if (rtc) {
            // Tell the GrRenderTargetContext to not reset its draw op list on a fullscreen clear.
            // If we don't do this, fullscreen clear ops would be created and constantly discard the
            // previous iteration's op so execution would only invoke one actual clear on the GPU
            // (not what we want to measure).
            rtc->testingOnly_SetPreserveOpsOnFullClear();
        }

        for (int i = 0; i < loops; i++) {
            canvas->save();
            switch(fType) {
                case kPartial_ClearType:
                    canvas->clipRect(kPartialClip);
                    break;
                case kComplex_ClearType:
                    canvas->clipRRect(kComplexClip);
                    break;
                case kFull_ClearType:
                    // Don't add any extra clipping, since it defaults to the entire "device"
                    break;
            }

            // The clear we care about measuring
            canvas->clear(SK_ColorBLUE);
            canvas->restore();

            // Perform as minimal a draw as possible that intersects with the clear region in
            // order to prevent the clear ops from being batched together.
            canvas->drawRect(kInterruptRect, interruptPaint);
        }
    }

private:
    ClearType fType;
};

DEF_BENCH( return new ClearBench(ClearBench::kFull_ClearType); )
DEF_BENCH( return new ClearBench(ClearBench::kPartial_ClearType); )
DEF_BENCH( return new ClearBench(ClearBench::kComplex_ClearType); )
