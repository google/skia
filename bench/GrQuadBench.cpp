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
    GrQuadBoundsBench(GrQuad::Type type, bool correct)
            : fType(type)
            , fCorrect(correct) {
        fName.printf("grquadbounds_%d_%s", (int) type, correct ? "correct" : "old");
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        SkRandom r;
        for (int i = 0; i < 1000; ++i) {
            for (int j = 0; j < 4; ++j) {
                fQuads[i].xs()[j] = r.nextRangeF(-100.f, -100.f);
                fQuads[i].ys()[j] = r.nextRangeF(-100.f, -100.f);
                if (fType == GrQuad::Type::kPerspective) {
                    fQuads[i].ws()[j] = r.nextRangeF(-1.f, 10.f);
                } else {
                    fQuads[i].ws()[j] = 1.f;
                }
            }
            fQuads[i].setQuadType(fType);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        SkRect bounds = SkRect::MakeEmpty();
        for (int i = 0; i < loops; ++i) {
            for (int j = 0; j < 1000; ++j) {
                bounds.joinPossiblyEmptyRect(fCorrect ? GrQuadUtils::ProjectedBounds(fQuads[j]) : fQuads[j].bounds());
            }
        }
    }

    SkString     fName;
    GrQuad::Type fType;
    bool         fCorrect;
    GrQuad       fQuads[1000];

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new GrQuadBoundsBench(GrQuad::Type::kAxisAligned, false); )
DEF_BENCH( return new GrQuadBoundsBench(GrQuad::Type::kAxisAligned, true); )
DEF_BENCH( return new GrQuadBoundsBench(GrQuad::Type::kPerspective, false); )
DEF_BENCH( return new GrQuadBoundsBench(GrQuad::Type::kPerspective, true); )
