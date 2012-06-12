
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkFloatBits.h"
#include "SkRandom.h"
#include "SkRect.h"
#include "SkString.h"

class ScalarBench : public SkBenchmark {
    SkString    fName;
    enum { N = 100000 };
public:
    ScalarBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("scalar_%s", name);
    }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        int n = SkBENCHLOOP(N * this->mulLoopCount());
        for (int i = 0; i < n; i++) {
            this->performTest();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

// we want to stop the compiler from eliminating code that it thinks is a no-op
// so we have a non-static global we increment, hoping that will convince the
// compiler to execute everything
int gScalarBench_NonStaticGlobal;

#define always_do(pred)                     \
    do {                                    \
        if (pred) {                         \
            ++gScalarBench_NonStaticGlobal; \
        }                                   \
    } while (0)

// having unknown values in our arrays can throw off the timing a lot, perhaps
// handling NaN values is a lot slower. Anyway, this guy is just meant to put
// reasonable values in our arrays.
template <typename T> void init9(T array[9]) {
    SkRandom rand;
    for (int i = 0; i < 9; i++) {
        array[i] = rand.nextSScalar1();
    }
}

class FloatComparisonBench : public ScalarBench {
public:
    FloatComparisonBench(void* param) : INHERITED(param, "compare_float") {
        init9(fArray);
    }
protected:
    virtual int mulLoopCount() const { return 4; }
    virtual void performTest() {
        always_do(fArray[6] != 0.0f || fArray[7] != 0.0f || fArray[8] != 1.0f);
        always_do(fArray[2] != 0.0f || fArray[5] != 0.0f);
    }
private:
    float fArray[9];
    typedef ScalarBench INHERITED;
};

class ForcedIntComparisonBench : public ScalarBench {
public:
    ForcedIntComparisonBench(void* param)
        : INHERITED(param, "compare_forced_int") {
        init9(fArray);
    }
protected:
    virtual int mulLoopCount() const { return 4; }
    virtual void performTest() {
        always_do(SkScalarAs2sCompliment(fArray[6]) |
                  SkScalarAs2sCompliment(fArray[7]) |
                  (SkScalarAs2sCompliment(fArray[8]) - kPersp1Int));
        always_do(SkScalarAs2sCompliment(fArray[2]) |
                  SkScalarAs2sCompliment(fArray[5]));
    }
private:
    static const int32_t kPersp1Int = 0x3f800000;
    SkScalar fArray[9];
    typedef ScalarBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class RectBoundsBench : public SkBenchmark {
    enum {
        PTS = 100,
        N = SkBENCHLOOP(10000)
    };
    SkPoint fPts[PTS];

public:
    RectBoundsBench(void* param) : INHERITED(param) {
        SkRandom rand;
        for (int i = 0; i < PTS; ++i) {
            fPts[i].fX = rand.nextSScalar1();
            fPts[i].fY = rand.nextSScalar1();
        }
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return "rect_bounds";
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkRect r;
        for (int i = 0; i < N; ++i) {
            r.set(fPts, PTS);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* S0(void* p) { return new FloatComparisonBench(p); }
static SkBenchmark* S1(void* p) { return new ForcedIntComparisonBench(p); }
static SkBenchmark* S2(void* p) { return new RectBoundsBench(p); }

static BenchRegistry gReg0(S0);
static BenchRegistry gReg1(S1);
static BenchRegistry gReg2(S2);
