/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkColorPriv.h"
#include "SkFixed.h"
#include "SkMathPriv.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

static float sk_fsel(float pred, float result_ge, float result_lt) {
    return pred >= 0 ? result_ge : result_lt;
}

static float fast_floor(float x) {
//    float big = sk_fsel(x, 0x1.0p+23, -0x1.0p+23);
    float big = sk_fsel(x, (float)(1 << 23), -(float)(1 << 23));
    return (x + big) - big;
}

class MathBench : public Benchmark {
    enum {
        kBuffer = 100,
    };
    SkString    fName;
    float       fSrc[kBuffer], fDst[kBuffer];
public:
    MathBench(const char name[])  {
        fName.printf("math_%s", name);

        SkRandom rand;
        for (int i = 0; i < kBuffer; ++i) {
            fSrc[i] = rand.nextSScalar1();
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        int n = loops * this->mulLoopCount();
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fSrc, kBuffer);
        }
    }

private:
    typedef Benchmark INHERITED;
};

class MathBenchU32 : public MathBench {
public:
    MathBenchU32(const char name[]) : INHERITED(name) {}

protected:
    virtual void performITest(uint32_t* SK_RESTRICT dst,
                              const uint32_t* SK_RESTRICT src,
                              int count) = 0;

    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        uint32_t* d = SkTCast<uint32_t*>(dst);
        const uint32_t* s = SkTCast<const uint32_t*>(src);
        this->performITest(d, s, count);
    }
private:
    typedef MathBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class NoOpMathBench : public MathBench {
public:
    NoOpMathBench() : INHERITED("noOp") {}
protected:
    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = src[i] + 1;
        }
    }
private:
    typedef MathBench INHERITED;
};

class SkRSqrtMathBench : public MathBench {
public:
    SkRSqrtMathBench() : INHERITED("sk_float_rsqrt") {}
protected:
    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = sk_float_rsqrt(src[i]);
        }
    }
private:
    typedef MathBench INHERITED;
};


class SlowISqrtMathBench : public MathBench {
public:
    SlowISqrtMathBench() : INHERITED("slowIsqrt") {}
protected:
    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = 1.0f / sk_float_sqrt(src[i]);
        }
    }
private:
    typedef MathBench INHERITED;
};

class FastISqrtMathBench : public MathBench {
public:
    FastISqrtMathBench() : INHERITED("fastIsqrt") {}
protected:
    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = sk_float_rsqrt(src[i]);
        }
    }
private:
    typedef MathBench INHERITED;
};

static inline uint32_t QMul64(uint32_t value, U8CPU alpha) {
    SkASSERT((uint8_t)alpha == alpha);
    const uint32_t mask = 0xFF00FF;

    uint64_t tmp = value;
    tmp = (tmp & mask) | ((tmp & ~mask) << 24);
    tmp *= alpha;
    return (uint32_t) (((tmp >> 8) & mask) | ((tmp >> 32) & ~mask));
}

class QMul64Bench : public MathBenchU32 {
public:
    QMul64Bench() : INHERITED("qmul64") {}
protected:
    void performITest(uint32_t* SK_RESTRICT dst,
                      const uint32_t* SK_RESTRICT src,
                      int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = QMul64(src[i], (uint8_t)i);
        }
    }
private:
    typedef MathBenchU32 INHERITED;
};

class QMul32Bench : public MathBenchU32 {
public:
    QMul32Bench() : INHERITED("qmul32") {}
protected:
    void performITest(uint32_t* SK_RESTRICT dst,
                      const uint32_t* SK_RESTRICT src,
                      int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = SkAlphaMulQ(src[i], (uint8_t)i);
        }
    }
private:
    typedef MathBenchU32 INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static bool isFinite_int(float x) {
    uint32_t bits = SkFloat2Bits(x);    // need unsigned for our shifts
    int exponent = bits << 1 >> 24;
    return exponent != 0xFF;
}

static bool isFinite_float(float x) {
    return SkToBool(sk_float_isfinite(x));
}

static bool isFinite_mulzero(float x) {
    float y = x * 0;
    return y == y;
}

static bool isfinite_and_int(const float data[4]) {
    return  isFinite_int(data[0]) && isFinite_int(data[1]) && isFinite_int(data[2]) && isFinite_int(data[3]);
}

static bool isfinite_and_float(const float data[4]) {
    return  isFinite_float(data[0]) && isFinite_float(data[1]) && isFinite_float(data[2]) && isFinite_float(data[3]);
}

static bool isfinite_and_mulzero(const float data[4]) {
    return  isFinite_mulzero(data[0]) && isFinite_mulzero(data[1]) && isFinite_mulzero(data[2]) && isFinite_mulzero(data[3]);
}

#define mulzeroadd(data)    (data[0]*0 + data[1]*0 + data[2]*0 + data[3]*0)

static bool isfinite_plus_int(const float data[4]) {
    return  isFinite_int(mulzeroadd(data));
}

static bool isfinite_plus_float(const float data[4]) {
    return  !sk_float_isnan(mulzeroadd(data));
}

static bool isfinite_plus_mulzero(const float data[4]) {
    float x = mulzeroadd(data);
    return x == x;
}

typedef bool (*IsFiniteProc)(const float[]);

#define MAKEREC(name)   { name, #name }

static const struct {
    IsFiniteProc    fProc;
    const char*     fName;
} gRec[] = {
    MAKEREC(isfinite_and_int),
    MAKEREC(isfinite_and_float),
    MAKEREC(isfinite_and_mulzero),
    MAKEREC(isfinite_plus_int),
    MAKEREC(isfinite_plus_float),
    MAKEREC(isfinite_plus_mulzero),
};

#undef MAKEREC

static bool isFinite(const SkRect& r) {
    // x * 0 will be NaN iff x is infinity or NaN.
    // a + b will be NaN iff either a or b is NaN.
    float value = r.fLeft * 0 + r.fTop * 0 + r.fRight * 0 + r.fBottom * 0;

    // value is either NaN or it is finite (zero).
    // value==value will be true iff value is not NaN
    return value == value;
}

class IsFiniteBench : public Benchmark {
    enum {
        N = 1000,
    };
    float fData[N];
public:

    IsFiniteBench(int index)  {
        SkRandom rand;

        for (int i = 0; i < N; ++i) {
            fData[i] = rand.nextSScalar1();
        }

        if (index < 0) {
            fProc = nullptr;
            fName = "isfinite_rect";
        } else {
            fProc = gRec[index].fProc;
            fName = gRec[index].fName;
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    void onDraw(int loops, SkCanvas*) override {
        IsFiniteProc proc = fProc;
        const float* data = fData;
        // do this so the compiler won't throw away the function call
        int counter = 0;

        if (proc) {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < N - 4; ++i) {
                    counter += proc(&data[i]);
                }
            }
        } else {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < N - 4; ++i) {
                    const SkRect* r = reinterpret_cast<const SkRect*>(&data[i]);
                    if (false) { // avoid bit rot, suppress warning
                        isFinite(*r);
                    }
                    counter += r->isFinite();
                }
            }
        }

        SkPaint paint;
        if (paint.getAlpha() == 0) {
            SkDebugf("%d\n", counter);
        }
    }

    const char* onGetName() override {
        return fName;
    }

private:
    IsFiniteProc    fProc;
    const char*     fName;

    typedef Benchmark INHERITED;
};

class FloorBench : public Benchmark {
    enum {
        ARRAY = 1000,
    };
    float fData[ARRAY];
    bool fFast;
public:

    FloorBench(bool fast) : fFast(fast) {
        SkRandom rand;

        for (int i = 0; i < ARRAY; ++i) {
            fData[i] = rand.nextSScalar1();
        }

        if (fast) {
            fName = "floor_fast";
        } else {
            fName = "floor_std";
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual void process(float) {}

protected:
    void onDraw(int loops, SkCanvas*) override {
        SkRandom rand;
        float accum = 0;
        const float* data = fData;

        if (fFast) {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += fast_floor(data[i]);
                }
                this->process(accum);
            }
        } else {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += sk_float_floor(data[i]);
                }
                this->process(accum);
            }
        }
    }

    const char* onGetName() override {
        return fName;
    }

private:
    const char*     fName;

    typedef Benchmark INHERITED;
};

class CLZBench : public Benchmark {
    enum {
        ARRAY = 1000,
    };
    uint32_t fData[ARRAY];
    bool fUsePortable;

public:
    CLZBench(bool usePortable) : fUsePortable(usePortable) {

        SkRandom rand;
        for (int i = 0; i < ARRAY; ++i) {
            fData[i] = rand.nextU();
        }

        if (fUsePortable) {
            fName = "clz_portable";
        } else {
            fName = "clz_intrinsic";
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    // just so the compiler doesn't remove our loops
    virtual void process(int) {}

protected:
    void onDraw(int loops, SkCanvas*) override {
        int accum = 0;

        if (fUsePortable) {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += SkCLZ_portable(fData[i]);
                }
                this->process(accum);
            }
        } else {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += SkCLZ(fData[i]);
                }
                this->process(accum);
            }
        }
    }

    const char* onGetName() override {
        return fName;
    }

private:
    const char* fName;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class NormalizeBench : public Benchmark {
    enum {
        ARRAY =1000,
    };
    SkVector fVec[ARRAY];

public:
    NormalizeBench() {
        SkRandom rand;
        for (int i = 0; i < ARRAY; ++i) {
            fVec[i].set(rand.nextSScalar1(), rand.nextSScalar1());
        }

        fName = "point_normalize";
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    // just so the compiler doesn't remove our loops
    virtual void process(int) {}

protected:
    void onDraw(int loops, SkCanvas*) override {
        int accum = 0;

        for (int j = 0; j < loops; ++j) {
            for (int i = 0; i < ARRAY; ++i) {
                accum += fVec[i].normalize();
            }
            this->process(accum);
        }
    }

    const char* onGetName() override {
        return fName;
    }

private:
    const char* fName;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class FixedMathBench : public Benchmark {
    enum {
        N = 1000,
    };
    float fData[N];
    SkFixed fResult[N];
public:

    FixedMathBench()  {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fData[i] = rand.nextSScalar1();
        }

    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    void onDraw(int loops, SkCanvas*) override {
        for (int j = 0; j < loops; ++j) {
            for (int i = 0; i < N - 4; ++i) {
                fResult[i] = SkFloatToFixed(fData[i]);
            }
        }

        SkPaint paint;
        if (paint.getAlpha() == 0) {
            SkDebugf("%d\n", fResult[0]);
        }
    }

    const char* onGetName() override {
        return "float_to_fixed";
    }

private:
    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class DivModBench : public Benchmark {
    SkString fName;
public:
    explicit DivModBench(const char* name) {
        fName.printf("divmod_%s", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        volatile T a = 0, b = 0;
        T div = 0, mod = 0;
        for (int i = 0; i < loops; i++) {
            if ((T)i == 0) continue;  // Small T will wrap around.
            SkTDivMod((T)(i+1), (T)i, &div, &mod);
            a ^= div;
            b ^= mod;
        }
    }
};
DEF_BENCH(return new DivModBench<uint8_t>("uint8_t"))
DEF_BENCH(return new DivModBench<uint16_t>("uint16_t"))
DEF_BENCH(return new DivModBench<uint32_t>("uint32_t"))
DEF_BENCH(return new DivModBench<uint64_t>("uint64_t"))

DEF_BENCH(return new DivModBench<int8_t>("int8_t"))
DEF_BENCH(return new DivModBench<int16_t>("int16_t"))
DEF_BENCH(return new DivModBench<int32_t>("int32_t"))
DEF_BENCH(return new DivModBench<int64_t>("int64_t"))

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new NoOpMathBench(); )
DEF_BENCH( return new SkRSqrtMathBench(); )
DEF_BENCH( return new SlowISqrtMathBench(); )
DEF_BENCH( return new FastISqrtMathBench(); )
DEF_BENCH( return new QMul64Bench(); )
DEF_BENCH( return new QMul32Bench(); )

DEF_BENCH( return new IsFiniteBench(-1); )
DEF_BENCH( return new IsFiniteBench(0); )
DEF_BENCH( return new IsFiniteBench(1); )
DEF_BENCH( return new IsFiniteBench(2); )
DEF_BENCH( return new IsFiniteBench(3); )
DEF_BENCH( return new IsFiniteBench(4); )
DEF_BENCH( return new IsFiniteBench(5); )

DEF_BENCH( return new FloorBench(false); )
DEF_BENCH( return new FloorBench(true); )

DEF_BENCH( return new CLZBench(false); )
DEF_BENCH( return new CLZBench(true); )

DEF_BENCH( return new NormalizeBench(); )

DEF_BENCH( return new FixedMathBench(); )


struct FloatToIntBench : public Benchmark {
    enum { N = 1000000 };
    float fFloats[N];
    int   fInts  [N];

    const char* onGetName() override { return "float_to_int"; }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDelayedSetup() override  {
        const auto f32 = 4294967296.0f;
        for (int i = 0; i < N; ++i) {
            fFloats[i] = -f32 + i*(2*f32/N);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            for (int i = 0; i < N; i++) {
                fInts[i] = SkFloatToIntFloor(fFloats[i]);
            }
        }
    }
};
DEF_BENCH( return new FloatToIntBench; )
