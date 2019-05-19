/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkShadowUtils.h"
#include "src/core/SkDrawShadowInfo.h"

class ShadowBench : public Benchmark {
// Draws a set of shadowed rrects filling the canvas, in various modes:
// * opaque or transparent
// * use analytic fast path or geometric tessellation
public:
    ShadowBench(bool transparent, bool forceGeometric)
        : fTransparent(transparent)
        , fForceGeometric(forceGeometric) {
        computeName("shadows");
    }

protected:
    enum {
        kWidth = 640,
        kHeight = 480,
        kRRSize = 50,
        kRRRadius = 6,
        kRRSpace = 8,
        kRRStep = kRRSize + kRRSpace,
        kElevation = 16,
        kNumRRects = ((kWidth - kRRSpace) / kRRStep)*((kHeight - kRRSpace) / kRRStep)
    };

    void computeName(const char root[]) {
        static const char kTransChars[2] = {
            'o', 't'
        };
        static const char kGeomChars[2] = {
            'a', 'g'
        };

        fBaseName.printf("%s_%c_%c", root, kTransChars[fTransparent], kGeomChars[fForceGeometric]);
    }

    void genRRects() {
        int i = 0;
        for (int x = kRRSpace; x < kWidth - kRRStep; x += kRRStep) {
            for (int y = kRRSpace; y < kHeight - kRRStep; y += kRRStep) {
                SkRect rect = SkRect::MakeXYWH(x, y, kRRSize, kRRSize);
                fRRects[i].addRRect(SkRRect::MakeRectXY(rect, kRRRadius, kRRRadius));
                ++i;
            }
        }
        SkASSERT(i == kNumRRects);
    }

    const char* onGetName() override { return fBaseName.c_str(); }

    void onDelayedSetup() override {
        fRec.fZPlaneParams = SkPoint3::Make(0, 0, kElevation);
        fRec.fLightPos = SkPoint3::Make(270, 0, 600);
        fRec.fLightRadius = 800;
        fRec.fAmbientColor = 0x19000000;
        fRec.fSpotColor = 0x40000000;
        fRec.fFlags = 0;
        if (fTransparent) {
            fRec.fFlags |= SkShadowFlags::kTransparentOccluder_ShadowFlag;
        }
        if (fForceGeometric) {
            fRec.fFlags |= SkShadowFlags::kGeometricOnly_ShadowFlag;
        }

        this->genRRects();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        this->setupPaint(&paint);

        for (int i = 0; i < loops; ++i) {
            // use the private canvas call so we don't include the time to stuff data in the Rec
            canvas->private_draw_shadow_rec(fRRects[i % kNumRRects], fRec);
        }
    }

private:
    SkString fBaseName;

    SkPath  fRRects[kNumRRects];
    SkDrawShadowRec fRec;
    int    fTransparent;
    int    fForceGeometric;

    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ShadowBench(false, false);)
DEF_BENCH(return new ShadowBench(false, true);)
DEF_BENCH(return new ShadowBench(true, false);)
DEF_BENCH(return new ShadowBench(true, true);)

