/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkPathPriv.h"

enum class PathIterType {
    kOldIter,
    kNewIter,
    kPriv,
    kEdge,
    kPathIter,
};
const char* gPathIterNames[] = {
    "olditer", "newiter", "priv", "edge", "pathiter",
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

        SkPathBuilder builder;
        builder.moveTo({0, 0});

        SkRandom rand;
        for (int i = 0; i < 1000; ++i) {
            SkPoint pts[4];
            int n = rand_pts(rand, pts);
            switch (n) {
                case 2:
                    builder.lineTo(pts[1]);
                    break;
                case 3:
                    builder.quadTo(pts[1], pts[2]);
                    break;
                case 4:
                    builder.cubicTo(pts[1], pts[2], pts[3]);
                    break;
            }
        }
        fPath = builder.detach();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
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
            case PathIterType::kNewIter:
                for (int i = 0; i < loops; ++i) {
                    SkPath::Iter iter(fPath, false);
                    while (auto rec = iter.next()) {
                        handle((int)rec->fVerb, rec->fPoints.data());
                    }
                }
                break;
            case PathIterType::kOldIter:
                for (int i = 0; i < loops; ++i) {
                    SkPath::Iter iter(fPath, false);
                    for (;;) {
                        SkPoint pts[4];
                        auto verb = iter.next(pts);
                        if (verb == SkPath::kDone_Verb) {
                            break;
                        }
                        handle((int)verb, pts);
                    }
                }
                break;
            case PathIterType::kPriv:
                for (int i = 0; i < loops; ++i)  {
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
            case PathIterType::kPathIter:
                for (int i = 0; i < loops; ++i) {
                    auto iter = fPath.iter();
                    while (auto r = iter.next()) {
                        handle((int)r->fVerb, r->fPoints.data());
                    }
                }
                break;
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new PathIterBench(PathIterType::kNewIter); )
DEF_BENCH( return new PathIterBench(PathIterType::kOldIter); )
DEF_BENCH( return new PathIterBench(PathIterType::kPriv); )
DEF_BENCH( return new PathIterBench(PathIterType::kPathIter); )
DEF_BENCH( return new PathIterBench(PathIterType::kEdge); )
