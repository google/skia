/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"

class GrQuadBoundsBench : public Benchmark {
public:
    GrQuadBoundsBench(bool perspective)
            : fPerspective(perspective) {
        fName.printf("grquad_bounds_%s", perspective ? "3d" : "2d");
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    inline static constexpr int kQuadCount = 1000;

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkRandom r;
        for (int i = 0; i < kQuadCount; ++i) {
            for (int j = 0; j < 4; ++j) {
                fQuads[i].xs()[j] = r.nextRangeF(-100.f, 100.f);
                fQuads[i].ys()[j] = r.nextRangeF(-100.f, 100.f);
                if (fPerspective) {
                    // Biased towards in front of the viewpoint, but do include some that require
                    // the vertices to be clipped against w = 0.
                    fQuads[i].ws()[j] = r.nextRangeF(-1.f, 10.f);
                } else {
                    fQuads[i].ws()[j] = 1.f;
                }
            }
            fQuads[i].setQuadType(fPerspective ? GrQuad::Type::kPerspective
                                               : GrQuad::Type::kGeneral);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        SkScalar area = 0.f;
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < kQuadCount; ++j) {
                SkRect qb = fQuads[j].bounds();
                area += qb.width() + qb.height();
            }
        }
        // Must persist this calculation in order to prevent the compiler from optimizing the
        // loops away.
        fArea = area;
    }

    SkString     fName;
    bool         fPerspective;
    GrQuad       fQuads[kQuadCount];
    SkScalar     fArea;

    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GrQuadBoundsBench(/* persp */ false); )
DEF_BENCH( return new GrQuadBoundsBench(/* persp */ true); )
