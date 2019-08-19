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
#include "include/core/SkPathIter.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

static int rand_pts(SkRandom& rand, SkPoint pts[4]) {
    int n = rand.nextU() & 3;
    n += 1;

    for (int i = 0; i < n; ++i) {
        pts[i].fX = rand.nextSScalar1();
        pts[i].fY = rand.nextSScalar1();
    }
    return n;
}

enum IterType {
    kRaw,
    kConsume,
    kNew
};

class PathIterBench : public Benchmark {
    SkString    fName;
    SkPath      fPath;
    IterType    fRaw;

public:
    PathIterBench(IterType raw)  {
        fName.printf("pathiter_%s", raw == 0 ? "raw" : (raw == 1 ? "consume" : "new"));
        fRaw = raw;

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

    virtual void handlePts(const SkPoint pts[]) {}

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(const int loops, SkCanvas*) override {
        for (int j = 0; j < 1000; ++j) {
        switch (fRaw) {
            case kRaw:
                for (int i = 0; i < loops; ++i) {
                    SkPath::RawIter iter(fPath);
                    SkPath::Verb verb;
                    SkPoint      pts[4];

                    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                        this->handlePts(pts);
                    }
                }
                break;
            case kConsume:
                for (int i = 0; i < loops; ++i) {
                    SkPath::Iter iter(fPath, false);
                    SkPath::Verb verb;
                    SkPoint      pts[4];

                    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                        this->handlePts(pts);
                    }
                }
                break;
            case kNew: {
                for (int i = 0; i < loops; ++i) {
                    SkPathIter iter(fPath);
                    while (iter.next()) {
                        this->handlePts(iter.currPts());
                    }
                }
            } break;
        }
        }
    }

private:
    typedef Benchmark INHERITED;
};

DEF_BENCH( return new PathIterBench(kRaw); )
DEF_BENCH( return new PathIterBench(kConsume); )
DEF_BENCH( return new PathIterBench(kNew); )
