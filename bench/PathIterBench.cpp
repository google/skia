/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "src/core/SkPathPriv.h"

enum class PathIterType {
    kIter,
    kRaw,
    kEdge,
};
const char* gPathIterNames[] = {
    "iter", "raw", "edge"
};

static int rand_pts(SkRandom& rand, SkPoint pts[4]) {
    int n = rand.nextU() & 3;
    n += 1;

    for (int i = 0; i < n; ++i) {
        pts[i].fX = rand.nextSScalar1();
        pts[i].fY = rand.nextSScalar1();
    }
    return n;
}

class PathIterBench : public Benchmark {
    SkString        fName;
    SkPath          fPath;
    PathIterType    fType;

    int fVerbInc = 0;
    SkScalar fXInc = 0, fYInc = 0;

public:
    PathIterBench(PathIterType t) : fType(t) {
        fName.printf("pathiter_%s", gPathIterNames[static_cast<unsigned>(t)]);

        SkRandom rand;
        for (int i = 0; i < 1000; ++i) {
            SkPoint pts[4];
            int n = rand_pts(rand, pts);
            switch (n) {
                case 1:
                    fPath.moveTo(pts[0]);
                    break;
                case 2:
                    fPath.lineTo(pts[1]);
                    break;
                case 3:
                    fPath.quadTo(pts[1], pts[2]);
                    break;
                case 4:
                    fPath.cubicTo(pts[1], pts[2], pts[3]);
                    break;
            }
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        // Need to do *something* with the results, so the compile doesn't elide
        // away the code we want to time.
        auto handle = [this](int verb, const SkPoint pts[]) {
            fVerbInc += verb;
            fXInc += pts[0].fX;
            fYInc += pts[0].fY;
        };

        switch (fType) {
            case PathIterType::kIter:
                for (int i = 0; i < loops; ++i) {
                    SkPath::Iter iter(fPath, true);
                    SkPath::Verb verb;
                    SkPoint      pts[4];
                    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                        handle(verb, pts);
                    }
                }
                break;
            case PathIterType::kRaw:
                for (int i = 0; i < loops; ++i) {
                    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
                        handle((SkPath::Verb)verb, pts);
                    }
                }
                break;
            case PathIterType::kEdge:
                for (int i = 0; i < loops; ++i) {
                    SkPathEdgeIter iter(fPath);
                    while (auto r = iter.next()) {
                        handle((int)r.fEdge, r.fPts);
                    }
                }
                break;
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new PathIterBench(PathIterType::kIter); )
DEF_BENCH( return new PathIterBench(PathIterType::kRaw); )
DEF_BENCH( return new PathIterBench(PathIterType::kEdge); )
