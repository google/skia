/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "src/core/SkMatrixUtils.h"

class MatrixBench : public Benchmark {
    SkString    fName;
public:
    MatrixBench(const char name[])  {
        fName.printf("matrix_%s", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    using INHERITED = Benchmark;
};

class ScaleMatrixBench : public MatrixBench {
public:
    ScaleMatrixBench() : INHERITED("scale") {
        fSX = fSY = 1.5f;
        fM0.reset();
        fM1.setScale(fSX, fSY);
        fM2.setTranslate(fSX, fSY);
    }
protected:
    void performTest() override {
        SkMatrix m;
        m = fM0; m.preScale(fSX, fSY);
        m = fM1; m.preScale(fSX, fSY);
        m = fM2; m.preScale(fSX, fSY);
    }
private:
    SkMatrix fM0, fM1, fM2;
    SkScalar fSX, fSY;
    using INHERITED = MatrixBench;
};

// having unknown values in our arrays can throw off the timing a lot, perhaps
// handling NaN values is a lot slower. Anyway, this is just meant to put
// reasonable values in our arrays.
template <typename T> void init9(T array[9]) {
    SkRandom rand;
    for (int i = 0; i < 9; i++) {
        array[i] = rand.nextSScalar1();
    }
}

class DecomposeMatrixBench : public MatrixBench {
public:
    DecomposeMatrixBench() : INHERITED("decompose") {}

protected:
    void onDelayedSetup() override {
        for (int i = 0; i < 10; ++i) {
            SkScalar rot0 = (fRandom.nextBool()) ? fRandom.nextRangeF(-180, 180) : 0.0f;
            SkScalar sx = fRandom.nextRangeF(-3000.f, 3000.f);
            SkScalar sy = (fRandom.nextBool()) ? fRandom.nextRangeF(-3000.f, 3000.f) : sx;
            SkScalar rot1 = fRandom.nextRangeF(-180, 180);
            fMatrix[i].setRotate(rot0);
            fMatrix[i].postScale(sx, sy);
            fMatrix[i].postRotate(rot1);
        }
    }
    void performTest() override {
        SkPoint rotation1, scale, rotation2;
        for (int i = 0; i < 10; ++i) {
            (void) SkDecomposeUpper2x2(fMatrix[i], &rotation1, &scale, &rotation2);
        }
    }
private:
    SkMatrix fMatrix[10];
    SkRandom fRandom;
    using INHERITED = MatrixBench;
};

class InvertMapRectMatrixBench : public MatrixBench {
public:
    InvertMapRectMatrixBench(const char* name, int flags)
        : INHERITED(name)
        , fFlags(flags) {
        fMatrix.reset();
        fIteration = 0;
        if (flags & kScale_Flag) {
            fMatrix.postScale(1.5f, 2.5f);
        }
        if (flags & kTranslate_Flag) {
            fMatrix.postTranslate(1.5f, 2.5f);
        }
        if (flags & kRotate_Flag) {
            fMatrix.postRotate(45.0f);
        }
        if (flags & kPerspective_Flag) {
            fMatrix.setPerspX(1.5f);
            fMatrix.setPerspY(2.5f);
        }
        if (0 == (flags & kUncachedTypeMask_Flag)) {
            fMatrix.getType();
        }
    }
    enum Flag {
        kScale_Flag             = 0x01,
        kTranslate_Flag         = 0x02,
        kRotate_Flag            = 0x04,
        kPerspective_Flag       = 0x08,
        kUncachedTypeMask_Flag  = 0x10,
    };
protected:
    void performTest() override {
        if (fFlags & kUncachedTypeMask_Flag) {
            // This will invalidate the typemask without
            // changing the matrix.
            fMatrix.setPerspX(fMatrix.getPerspX());
        }
        SkMatrix inv;
        bool invertible = fMatrix.invert(&inv);
        SkASSERT(invertible);
        SkRect transformedRect;
        // an arbitrary, small, non-zero rect to transform
        SkRect srcRect = SkRect::MakeWH(SkIntToScalar(10), SkIntToScalar(10));
        if (invertible) {
            inv.mapRect(&transformedRect, srcRect);
        }
    }
private:
    SkMatrix fMatrix;
    int fFlags;
    unsigned fIteration;
    using INHERITED = MatrixBench;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ScaleMatrixBench(); )
DEF_BENCH( return new DecomposeMatrixBench(); )

DEF_BENCH( return new InvertMapRectMatrixBench("invert_maprect_identity", 0); )

DEF_BENCH(return new InvertMapRectMatrixBench(
                                  "invert_maprect_rectstaysrect",
                                  InvertMapRectMatrixBench::kScale_Flag |
                                  InvertMapRectMatrixBench::kTranslate_Flag); )

DEF_BENCH(return new InvertMapRectMatrixBench(
                                  "invert_maprect_translate",
                                  InvertMapRectMatrixBench::kTranslate_Flag); )

DEF_BENCH(return new InvertMapRectMatrixBench(
                                  "invert_maprect_nonpersp",
                                  InvertMapRectMatrixBench::kScale_Flag |
                                  InvertMapRectMatrixBench::kRotate_Flag |
                                  InvertMapRectMatrixBench::kTranslate_Flag); )

DEF_BENCH( return new InvertMapRectMatrixBench(
                               "invert_maprect_persp",
                               InvertMapRectMatrixBench::kPerspective_Flag); )

DEF_BENCH( return new InvertMapRectMatrixBench(
                           "invert_maprect_typemask_rectstaysrect",
                           InvertMapRectMatrixBench::kUncachedTypeMask_Flag |
                           InvertMapRectMatrixBench::kScale_Flag |
                           InvertMapRectMatrixBench::kTranslate_Flag); )

DEF_BENCH( return new InvertMapRectMatrixBench(
                           "invert_maprect_typemask_nonpersp",
                           InvertMapRectMatrixBench::kUncachedTypeMask_Flag |
                           InvertMapRectMatrixBench::kScale_Flag |
                           InvertMapRectMatrixBench::kRotate_Flag |
                           InvertMapRectMatrixBench::kTranslate_Flag); )

///////////////////////////////////////////////////////////////////////////////

static SkMatrix make_trans() { return SkMatrix::Translate(2, 3); }
static SkMatrix make_scale() { SkMatrix m(make_trans()); m.postScale(1.5f, 0.5f); return m; }
static SkMatrix make_afine() { SkMatrix m(make_trans()); m.postRotate(15); return m; }

class MapPointsMatrixBench : public MatrixBench {
protected:
    SkMatrix fM;
    enum {
        N = 32
    };
    SkPoint fSrc[N], fDst[N];
public:
    MapPointsMatrixBench(const char name[], const SkMatrix& m)
        : MatrixBench(name), fM(m)
    {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fSrc[i].set(rand.nextSScalar1(), rand.nextSScalar1());
        }
    }

    void performTest() override {
        for (int i = 0; i < 1000000; ++i) {
            fM.mapPoints(fDst, fSrc);
        }
    }
};
DEF_BENCH( return new MapPointsMatrixBench("mappoints_identity", SkMatrix::I()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_trans", make_trans()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_scale", make_scale()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_affine", make_afine()); )

class MapSinglePointMatrixBench : public Benchmark {
public:
    enum class Use {
        kPoints,
        kSingle,
        kAffine,
    };

    const SkMatrix fM;
    const Use      fUse;

    enum { N = 32 };
    SkPoint fDst[N];

    SkString fName;

    static const char* usename(Use u) {
        switch (u) {
            case Use::kPoints: return "p";
            case Use::kSingle: return "s";
            case Use::kAffine: return "a";
        }
        return "oops";
    }

    MapSinglePointMatrixBench(const SkMatrix& m, Use use)
        : fM(m), fUse(use)
    {
        const auto t = m.getType();

        fName = "mappt_";
        fName.append(usename(use));
        if (use != Use::kAffine) {
            if (t == SkMatrix::kIdentity_Mask) {
                fName.append("_identity");
            } else {
                if (t & SkMatrix::kAffine_Mask) {
                    fName.append("_affine");
                }
                if (t & SkMatrix::kScale_Mask) {
                    fName.append("_scale");
                }
                if (t & SkMatrix::kTranslate_Mask) {
                    fName.append("_trans");
                }
            }
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

    void performTest() {
        constexpr int K = 1000;
        SkRandom rand;

        switch (fUse) {
            case Use::kPoints:
                for (int i = 0; i < K; ++i) {
                    auto src = SkPoint{rand.nextSScalar1(), rand.nextSScalar1()};
                    for (int j = 0; j < N; ++j) {
                        fM.mapPoints({&fDst[j], 1}, {&src, 1});
                        src.fX += 1;
                    }
                    this->handle(fDst);
                }
                break;
            case Use::kSingle:
                for (int i = 0; i < K; ++i) {
                    auto src = SkPoint{rand.nextSScalar1(), rand.nextSScalar1()};
                    for (int j = 0; j < N; ++j) {
                        fDst[j] = fM.mapPoint(src);
                        src.fX += 1;
                    }
                    this->handle(fDst);
                }
                break;
            case Use::kAffine:
                for (int i = 0; i < K; ++i) {
                    auto src = SkPoint{rand.nextSScalar1(), rand.nextSScalar1()};
                    for (int j = 0; j < N; ++j) {
                        fDst[j] = fM.mapPointAffine(src);
                        src.fX += 1;
                    }
                    this->handle(fDst);
                }
                break;
        }
    }

    virtual void handle(SkPoint[]) {}
};

const SkMatrix m0 = SkMatrix::I();
const SkMatrix m1 = SkMatrix::MakeAll(1, 0, 1,
                                      0, 1, 2,
                                      0, 0, 1);
const SkMatrix m2 = SkMatrix::MakeAll(2, 0, 1,
                                      0, 3, 2,
                                      0, 0, 1);
const SkMatrix m3 = SkMatrix::MakeAll(2, 1, 1,
                                      1, 3, 2,
                                      0, 0, 1);

DEF_BENCH( return new MapSinglePointMatrixBench(m0, MapSinglePointMatrixBench::Use::kAffine); )

DEF_BENCH( return new MapSinglePointMatrixBench(m0, MapSinglePointMatrixBench::Use::kSingle); )
DEF_BENCH( return new MapSinglePointMatrixBench(m1, MapSinglePointMatrixBench::Use::kSingle); )
DEF_BENCH( return new MapSinglePointMatrixBench(m2, MapSinglePointMatrixBench::Use::kSingle); )
DEF_BENCH( return new MapSinglePointMatrixBench(m3, MapSinglePointMatrixBench::Use::kSingle); )

DEF_BENCH( return new MapSinglePointMatrixBench(m0, MapSinglePointMatrixBench::Use::kPoints); )
DEF_BENCH( return new MapSinglePointMatrixBench(m1, MapSinglePointMatrixBench::Use::kPoints); )
DEF_BENCH( return new MapSinglePointMatrixBench(m2, MapSinglePointMatrixBench::Use::kPoints); )
DEF_BENCH( return new MapSinglePointMatrixBench(m3, MapSinglePointMatrixBench::Use::kPoints); )

///////////////////////////////////////////////////////////////////////////////

class MapRectMatrixBench : public MatrixBench {
    SkMatrix fM;
    SkRect   fR;
    bool     fScaleTrans;

    enum { MEGA_LOOP = 1000 * 1000 };
public:
    MapRectMatrixBench(const char name[], bool scale_trans)
        : MatrixBench(name), fScaleTrans(scale_trans)
    {
        fM.setScale(2, 3);
        fM.postTranslate(1, 2);

        fR.setLTRB(10, 10, 100, 200);
    }

    void performTest() override {
        SkRect dst;
        if (fScaleTrans) {
            for (int i = 0; i < MEGA_LOOP; ++i) {
                fM.mapRectScaleTranslate(&dst, fR);
            }
        } else {
            for (int i = 0; i < MEGA_LOOP; ++i) {
                fM.mapRect(&dst, fR);
            }
        }
    }
};
DEF_BENCH( return new MapRectMatrixBench("maprect", false); )
DEF_BENCH( return new MapRectMatrixBench("maprectscaletrans", true); )
