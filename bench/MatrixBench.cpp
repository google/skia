
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkMatrix.h"
#include "SkMatrixUtils.h"
#include "SkRandom.h"
#include "SkString.h"

class MatrixBench : public SkBenchmark {
    SkString    fName;
public:
    MatrixBench(const char name[])  {
        fName.printf("matrix_%s", name);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

// we want to stop the compiler from eliminating code that it thinks is a no-op
// so we have a non-static global we increment, hoping that will convince the
// compiler to execute everything
int gMatrixBench_NonStaticGlobal;

#define always_do(pred)                     \
    do {                                    \
        if (pred) {                         \
            ++gMatrixBench_NonStaticGlobal; \
        }                                   \
    } while (0)

class EqualsMatrixBench : public MatrixBench {
public:
    EqualsMatrixBench() : INHERITED("equals") {}
protected:
    virtual void performTest() {
        SkMatrix m0, m1, m2;

        m0.reset();
        m1.reset();
        m2.reset();
        always_do(m0 == m1);
        always_do(m1 == m2);
        always_do(m2 == m0);
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
    virtual void performTest() {
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

// Test the performance of setConcat() non-perspective case:
// using floating point precision only.
class FloatConcatMatrixBench : public MatrixBench {
public:
    FloatConcatMatrixBench() : INHERITED("concat_floatfloat") {
        init9(mya);
        init9(myb);
        init9(myr);
    }
protected:
    virtual int mulLoopCount() const { return 4; }

    static inline void muladdmul(float a, float b, float c, float d,
                                   float* result) {
      *result = a * b + c * d;
    }
    virtual void performTest() {
        const float* a = mya;
        const float* b = myb;
        float* r = myr;
        muladdmul(a[0], b[0], a[1], b[3], &r[0]);
        muladdmul(a[0], b[1], a[1], b[4], &r[1]);
        muladdmul(a[0], b[2], a[1], b[5], &r[2]);
        r[2] += a[2];
        muladdmul(a[3], b[0], a[4], b[3], &r[3]);
        muladdmul(a[3], b[1], a[4], b[4], &r[4]);
        muladdmul(a[3], b[2], a[4], b[5], &r[5]);
        r[5] += a[5];
        r[6] = r[7] = 0.0f;
        r[8] = 1.0f;
    }
private:
    float mya [9];
    float myb [9];
    float myr [9];
    typedef MatrixBench INHERITED;
};

static inline float SkDoubleToFloat(double x) {
    return static_cast<float>(x);
}

// Test the performance of setConcat() non-perspective case:
// using floating point precision but casting up to float for
// intermediate results during computations.
class FloatDoubleConcatMatrixBench : public MatrixBench {
public:
    FloatDoubleConcatMatrixBench() : INHERITED("concat_floatdouble") {
        init9(mya);
        init9(myb);
        init9(myr);
    }
protected:
    virtual int mulLoopCount() const { return 4; }

    static inline void muladdmul(float a, float b, float c, float d,
                                   float* result) {
      *result = SkDoubleToFloat((double)a * b + (double)c * d);
    }
    virtual void performTest() {
        const float* a = mya;
        const float* b = myb;
        float* r = myr;
        muladdmul(a[0], b[0], a[1], b[3], &r[0]);
        muladdmul(a[0], b[1], a[1], b[4], &r[1]);
        muladdmul(a[0], b[2], a[1], b[5], &r[2]);
        r[2] += a[2];
        muladdmul(a[3], b[0], a[4], b[3], &r[3]);
        muladdmul(a[3], b[1], a[4], b[4], &r[4]);
        muladdmul(a[3], b[2], a[4], b[5], &r[5]);
        r[5] += a[5];
        r[6] = r[7] = 0.0f;
        r[8] = 1.0f;
    }
private:
    float mya [9];
    float myb [9];
    float myr [9];
    typedef MatrixBench INHERITED;
};

// Test the performance of setConcat() non-perspective case:
// using double precision only.
class DoubleConcatMatrixBench : public MatrixBench {
public:
    DoubleConcatMatrixBench() : INHERITED("concat_double") {
        init9(mya);
        init9(myb);
        init9(myr);
    }
protected:
    virtual int mulLoopCount() const { return 4; }

    static inline void muladdmul(double a, double b, double c, double d,
                                   double* result) {
      *result = a * b + c * d;
    }
    virtual void performTest() {
        const double* a = mya;
        const double* b = myb;
        double* r = myr;
        muladdmul(a[0], b[0], a[1], b[3], &r[0]);
        muladdmul(a[0], b[1], a[1], b[4], &r[1]);
        muladdmul(a[0], b[2], a[1], b[5], &r[2]);
        r[2] += a[2];
        muladdmul(a[3], b[0], a[4], b[3], &r[3]);
        muladdmul(a[3], b[1], a[4], b[4], &r[4]);
        muladdmul(a[3], b[2], a[4], b[5], &r[5]);
        r[5] += a[5];
        r[6] = r[7] = 0.0;
        r[8] = 1.0;
    }
private:
    double mya [9];
    double myb [9];
    double myr [9];
    typedef MatrixBench INHERITED;
};

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
    virtual void performTest() {
        fMatrix.setAll(fArray[0], fArray[1], fArray[2],
                       fArray[3], fArray[4], fArray[5],
                       fArray[6], fArray[7], fArray[8]);
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
        fMatrix.dirtyMatrixTypeCache();
        always_do(fMatrix.getType());
    }
private:
    SkMatrix fMatrix;
    float fArray[9];
    SkRandom fRnd;
    typedef MatrixBench INHERITED;
};

class ScaleTransMixedMatrixBench : public MatrixBench {
 public:
    ScaleTransMixedMatrixBench() : INHERITED("scaletrans_mixed") {
        fMatrix.setAll(fRandom.nextSScalar1(), fRandom.nextSScalar1(), fRandom.nextSScalar1(),
                       fRandom.nextSScalar1(), fRandom.nextSScalar1(), fRandom.nextSScalar1(),
                       fRandom.nextSScalar1(), fRandom.nextSScalar1(), fRandom.nextSScalar1());
        int i;
        for (i = 0; i < kCount; i++) {
            fSrc[i].fX = fRandom.nextSScalar1();
            fSrc[i].fY = fRandom.nextSScalar1();
            fDst[i].fX = fRandom.nextSScalar1();
            fDst[i].fY = fRandom.nextSScalar1();
        }
    }
 protected:
    virtual void performTest() {
        SkPoint* dst = fDst;
        const SkPoint* src = fSrc;
        int count = kCount;
        float mx = fMatrix[SkMatrix::kMScaleX];
        float my = fMatrix[SkMatrix::kMScaleY];
        float tx = fMatrix[SkMatrix::kMTransX];
        float ty = fMatrix[SkMatrix::kMTransY];
        do {
            dst->fY = SkScalarMulAdd(src->fY, my, ty);
            dst->fX = SkScalarMulAdd(src->fX, mx, tx);
            src += 1;
            dst += 1;
        } while (--count);
    }
 private:
    enum {
        kCount = 16
    };
    SkMatrix fMatrix;
    SkPoint fSrc [kCount];
    SkPoint fDst [kCount];
    SkRandom fRandom;
    typedef MatrixBench INHERITED;
};

class ScaleTransDoubleMatrixBench : public MatrixBench {
 public:
    ScaleTransDoubleMatrixBench() : INHERITED("scaletrans_double") {
        init9(fMatrix);
        int i;
        for (i = 0; i < kCount; i++) {
            fSrc[i].fX = fRandom.nextSScalar1();
            fSrc[i].fY = fRandom.nextSScalar1();
            fDst[i].fX = fRandom.nextSScalar1();
            fDst[i].fY = fRandom.nextSScalar1();
        }
    }
 protected:
    virtual void performTest() {
        SkPoint* dst = fDst;
        const SkPoint* src = fSrc;
        int count = kCount;
        // As doubles, on Z600 Linux systems this is 2.5x as expensive as mixed mode
        float mx = (float) fMatrix[SkMatrix::kMScaleX];
        float my = (float) fMatrix[SkMatrix::kMScaleY];
        float tx = (float) fMatrix[SkMatrix::kMTransX];
        float ty = (float) fMatrix[SkMatrix::kMTransY];
        do {
            dst->fY = src->fY * my + ty;
            dst->fX = src->fX * mx + tx;
            src += 1;
            dst += 1;
        } while (--count);
    }
 private:
    enum {
        kCount = 16
    };
    double fMatrix [9];
    SkPoint fSrc [kCount];
    SkPoint fDst [kCount];
    SkRandom fRandom;
    typedef MatrixBench INHERITED;
};

class DecomposeMatrixBench : public MatrixBench {
public:
    DecomposeMatrixBench() : INHERITED("decompose") {}

protected:
    virtual void onPreDraw() {
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
    virtual void performTest() {
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
    virtual void performTest() {
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
DEF_BENCH( return new FloatConcatMatrixBench(); )
DEF_BENCH( return new FloatDoubleConcatMatrixBench(); )
DEF_BENCH( return new DoubleConcatMatrixBench(); )
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

DEF_BENCH( return new ScaleTransMixedMatrixBench(); )
DEF_BENCH( return new ScaleTransDoubleMatrixBench(); )
