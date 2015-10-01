
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkMatrix.h"
#include "SkMatrixUtils.h"
#include "SkRandom.h"
#include "SkString.h"

class MatrixBench : public Benchmark {
    SkString    fName;
public:
    MatrixBench(const char name[])  {
        fName.printf("matrix_%s", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
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
    typedef Benchmark INHERITED;
};


class EqualsMatrixBench : public MatrixBench {
public:
    EqualsMatrixBench() : INHERITED("equals") {}
protected:
    void performTest() override {
        SkMatrix m0, m1, m2;

        m0.reset();
        m1.reset();
        m2.reset();

        // xor into a volatile prevents these comparisons from being optimized away.
        volatile bool junk = false;
        junk ^= (m0 == m1);
        junk ^= (m1 == m2);
        junk ^= (m2 == m0);
    }
private:
    typedef MatrixBench INHERITED;
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
    typedef MatrixBench INHERITED;
};

// having unknown values in our arrays can throw off the timing a lot, perhaps
// handling NaN values is a lot slower. Anyway, this guy is just meant to put
// reasonable values in our arrays.
template <typename T> void init9(T array[9]) {
    SkRandom rand;
    for (int i = 0; i < 9; i++) {
        array[i] = rand.nextSScalar1();
    }
}

class GetTypeMatrixBench : public MatrixBench {
public:
    GetTypeMatrixBench()
        : INHERITED("gettype") {
        fArray[0] = (float) fRnd.nextS();
        fArray[1] = (float) fRnd.nextS();
        fArray[2] = (float) fRnd.nextS();
        fArray[3] = (float) fRnd.nextS();
        fArray[4] = (float) fRnd.nextS();
        fArray[5] = (float) fRnd.nextS();
        fArray[6] = (float) fRnd.nextS();
        fArray[7] = (float) fRnd.nextS();
        fArray[8] = (float) fRnd.nextS();
    }
protected:
    // Putting random generation of the matrix inside performTest()
    // would help us avoid anomalous runs, but takes up 25% or
    // more of the function time.
    void performTest() override {
        fMatrix.setAll(fArray[0], fArray[1], fArray[2],
                       fArray[3], fArray[4], fArray[5],
                       fArray[6], fArray[7], fArray[8]);
        // xoring into a volatile prevents the compiler from optimizing these away
        volatile int junk = 0;
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        junk ^= (fMatrix.getType());
    }
private:
    SkMatrix fMatrix;
    float fArray[9];
    SkRandom fRnd;
    typedef MatrixBench INHERITED;
};

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
    typedef MatrixBench INHERITED;
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
    typedef MatrixBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new EqualsMatrixBench(); )
DEF_BENCH( return new ScaleMatrixBench(); )
DEF_BENCH( return new GetTypeMatrixBench(); )
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

static SkMatrix make_trans() { return SkMatrix::MakeTrans(2, 3); }
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
            fM.mapPoints(fDst, fSrc, N);
        }
    }
};
DEF_BENCH( return new MapPointsMatrixBench("mappoints_identity", SkMatrix::I()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_trans", make_trans()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_scale", make_scale()); )
DEF_BENCH( return new MapPointsMatrixBench("mappoints_affine", make_afine()); )

