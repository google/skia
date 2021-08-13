/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/SkFloatBits.h"
#include "include/utils/SkRandom.h"

class ScalarBench : public Benchmark {
    SkString    fName;
public:
    ScalarBench(const char name[])  {
        fName.printf("scalar_%s", name);
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

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    using INHERITED = Benchmark;
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

class FloatComparisonBench : public ScalarBench {
public:
    FloatComparisonBench() : INHERITED("compare_float") {
        init9(fArray);
    }
protected:
    int mulLoopCount() const override { return 4; }
    void performTest() override {
        // xoring into a volatile prevents the compiler from optimizing these checks away.
        SK_MAYBE_UNUSED volatile bool junk = false;
        junk ^= (fArray[6] != 0.0f || fArray[7] != 0.0f || fArray[8] != 1.0f);
        junk ^= (fArray[2] != 0.0f || fArray[5] != 0.0f);
    }
private:
    float fArray[9];
    using INHERITED = ScalarBench;
};

class ForcedIntComparisonBench : public ScalarBench {
public:
    ForcedIntComparisonBench()
    : INHERITED("compare_forced_int") {
        init9(fArray);
    }
protected:
    int mulLoopCount() const override { return 4; }
    void performTest() override {
        // xoring into a volatile prevents the compiler from optimizing these checks away.
        SK_MAYBE_UNUSED volatile int32_t junk = 0;
        junk ^= (SkScalarAs2sCompliment(fArray[6]) |
                 SkScalarAs2sCompliment(fArray[7]) |
                (SkScalarAs2sCompliment(fArray[8]) - kPersp1Int));
        junk ^= (SkScalarAs2sCompliment(fArray[2]) |
                 SkScalarAs2sCompliment(fArray[5]));
    }
private:
    static const int32_t kPersp1Int = 0x3f800000;
    SkScalar fArray[9];
    using INHERITED = ScalarBench;
};

class IsFiniteScalarBench : public ScalarBench {
public:
    IsFiniteScalarBench() : INHERITED("isfinite") {
        SkRandom rand;
        for (size_t i = 0; i < ARRAY_N; ++i) {
            fArray[i] = rand.nextSScalar1();
        }
    }
protected:
    int mulLoopCount() const override { return 1; }
    void performTest() override {
        int sum = 0;
        for (size_t i = 0; i < ARRAY_N; ++i) {
            // We pass -fArray[i], so the compiler can't cheat and treat the
            // value as an int (even though we tell it that it is a float)
            sum += SkScalarIsFinite(-fArray[i]);
        }
        // we do this so the compiler won't optimize our loop away...
        this->doSomething(fArray, sum);
    }

    virtual void doSomething(SkScalar array[], int sum) {}
private:
    enum {
        ARRAY_N = 64
    };
    SkScalar fArray[ARRAY_N];

    using INHERITED = ScalarBench;
};

///////////////////////////////////////////////////////////////////////////////

class RectBoundsBench : public Benchmark {
    enum {
        PTS = 100,
    };
    SkPoint fPts[PTS];

public:
    RectBoundsBench() {
        SkRandom rand;
        for (int i = 0; i < PTS; ++i) {
            fPts[i].fX = rand.nextSScalar1();
            fPts[i].fY = rand.nextSScalar1();
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return "rect_bounds";
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r;
        for (int loop = 0; loop < loops; ++loop) {
            for (int i = 0; i < 1000; ++i) {
                r.setBounds(fPts, PTS);
            }
        }
    }

private:
    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new FloatComparisonBench(); )
DEF_BENCH( return new ForcedIntComparisonBench(); )
DEF_BENCH( return new RectBoundsBench(); )
DEF_BENCH( return new IsFiniteScalarBench(); )
