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

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRRect.h"

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
        const SkColor color = SK_ColorBLUE;
        const SkRect partialClip = SkRect::MakeLTRB(50, 50, 400, 400);
        const SkRRect complexClip = SkRRect::MakeRectXY(partialClip, 15, 15);

        // TODO (michaelludwig): Any benefit to changing the clip geometry?
        for (int i = 0; i < loops; i++) {
            canvas->save();
            switch(fType) {
                case kPartial_ClearType:
                    canvas->clipRect(partialClip);
                    break;
                case kComplex_ClearType:
                    canvas->clipRRect(complexClip);
                    break;
                case kFull_ClearType:
                    // Don't add any extra clipping, since it defaults to the entire "device"
                    break;
            }

            canvas->clear(color);
            canvas->restore();
        }
    }

private:
    ClearType fType;
};

DEF_BENCH( return new ClearBench(ClearBench::kFull_ClearType); )
DEF_BENCH( return new ClearBench(ClearBench::kPartial_ClearType); )
DEF_BENCH( return new ClearBench(ClearBench::kComplex_ClearType); )
