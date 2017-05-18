/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkDrawShadowRec.h"
#include "SkShadowUtils.h"
#include "SkString.h"

class ShadowBench : public Benchmark {
public:
    enum {
        kWidth = 640,
        kHeight = 480,
        kRRSize = 50,
        kRRRadius = 6,
        kRRSpace = 8,
        kRRStep = kRRSize + kRRSpace,
        kElevation = 16,
        N = ((kWidth - kRRSpace) / kRRStep)*((kHeight - kRRSpace) / kRRStep)
    };
    SkPath  fRRects[N];
    SkDrawShadowRec fRec;
    int    fTransparent;
    int    fForceGeometric;

    char kTransChars[2] = {
        'o', 't'
    };
    char kGeomChars[2] = {
        'a', 'g'
    };

    ShadowBench(bool transparent, bool forceGeometric)
        : fTransparent(transparent)
        , fForceGeometric(forceGeometric) {}

    const char* computeName(const char root[]) {
        fBaseName.printf("%s_%c_%c", root, kTransChars[fTransparent], kGeomChars[fForceGeometric]);
        return fBaseName.c_str();
    }

    bool isVisual() override { return true; }

protected:

    void genRRects() {
        int i = 0;
        for (int x = kRRSpace; x < kWidth - kRRStep; x += kRRStep) {
            for (int y = kRRSpace; y < kHeight - kRRStep; y += kRRStep) {
                SkRect rect = SkRect::MakeXYWH(x, y, kRRSize, kRRSize);
                fRRects[i].addRRect(SkRRect::MakeRectXY(rect, kRRRadius, kRRRadius));
                ++i;
            }
        }
        SkASSERT(i == N);
    }

    const char* onGetName() override { return computeName("shadows"); }

    void onDelayedSetup() override {
        fRec.fZPlaneParams = SkPoint3::Make(0, 0, kElevation);
        fRec.fLightPos = SkPoint3::Make(270, 0, 600);
        fRec.fLightRadius = 800;
        fRec.fAmbientAlpha = 0.1f;
        fRec.fSpotAlpha = 0.25f;
        fRec.fColor = SK_ColorBLACK;
        fRec.fFlags = 0;
        if (fTransparent) {
            fRec.fFlags |= SkShadowFlags::kTransparentOccluder_ShadowFlag;
        }
        if (fForceGeometric) {
            fRec.fFlags |= SkShadowFlags::kGeometricOnly_ShadowFlag;
        }

        genRRects();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        this->setupPaint(&paint);

        for (int i = 0; i < loops; ++i) {
            // use the private canvas call so we don't include the time to stuff data in the Rec
            canvas->private_draw_shadow_rec(fRRects[i % N], fRec);
            canvas->drawPath(fRRects[i % N], paint);
        }
    }

private:
    SkString fBaseName;
    typedef Benchmark INHERITED;
};

DEF_BENCH(return new ShadowBench(0, 0);)
DEF_BENCH(return new ShadowBench(0, 1);)
DEF_BENCH(return new ShadowBench(1, 0);)
DEF_BENCH(return new ShadowBench(1, 1);)

