#include "SkBenchmark.h"
#include "SkColorPriv.h"
#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkPaint.h"

static float sk_fsel(float pred, float result_ge, float result_lt) {
    return pred >= 0 ? result_ge : result_lt;
}

static float fast_floor(float x) {
//    float big = sk_fsel(x, 0x1.0p+23, -0x1.0p+23);
    float big = sk_fsel(x, (float)(1 << 23), -(float)(1 << 23));
    return (x + big) - big;
}

class MathBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) {
        int n = loops * this->mulLoopCount();
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fSrc, kBuffer);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class MathBenchU32 : public MathBench {
public:
    MathBenchU32(const char name[]) : INHERITED(name) {}

protected:
    virtual void performITest(uint32_t* SK_RESTRICT dst,
                              const uint32_t* SK_RESTRICT src,
                              int count) = 0;

    virtual void performTest(float* SK_RESTRICT dst,
                             const float* SK_RESTRICT src,
                             int count) SK_OVERRIDE {
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
    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) {
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
    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) {
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
    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) {
        for (int i = 0; i < count; ++i) {
            dst[i] = 1.0f / sk_float_sqrt(src[i]);
        }
    }
private:
    typedef MathBench INHERITED;
};

static inline float SkFastInvSqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *SkTCast<int*>(&x);
    i = 0x5f3759df - (i>>1);
    x = *SkTCast<float*>(&i);
    x = x*(1.5f-xhalf*x*x);
//    x = x*(1.5f-xhalf*x*x); // this line takes err from 10^-3 to 10^-6
    return x;
}

class FastISqrtMathBench : public MathBench {
public:
    FastISqrtMathBench() : INHERITED("fastIsqrt") {}
protected:
    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) {
        for (int i = 0; i < count; ++i) {
            dst[i] = SkFastInvSqrt(src[i]);
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
    virtual void performITest(uint32_t* SK_RESTRICT dst,
                              const uint32_t* SK_RESTRICT src,
                              int count) SK_OVERRIDE {
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
    virtual void performITest(uint32_t* SK_RESTRICT dst,
                              const uint32_t* SK_RESTRICT src,
                              int count) SK_OVERRIDE {
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

class IsFiniteBench : public SkBenchmark {
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
            fProc = NULL;
            fName = "isfinite_rect";
        } else {
            fProc = gRec[index].fProc;
            fName = gRec[index].fName;
        }
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual void onDraw(const int loops, SkCanvas*) {
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

    virtual const char* onGetName() {
        return fName;
    }

private:
    IsFiniteProc    fProc;
    const char*     fName;

    typedef SkBenchmark INHERITED;
};

class FloorBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    virtual void process(float) {}

protected:
    virtual void onDraw(const int loops, SkCanvas*) {
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

    virtual const char* onGetName() {
        return fName;
    }

private:
    const char*     fName;

    typedef SkBenchmark INHERITED;
};

class CLZBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    // just so the compiler doesn't remove our loops
    virtual void process(int) {}

protected:
    virtual void onDraw(const int loops, SkCanvas*) {
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

    virtual const char* onGetName() {
        return fName;
    }

private:
    const char* fName;

    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class NormalizeBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

    // just so the compiler doesn't remove our loops
    virtual void process(int) {}

protected:
    virtual void onDraw(const int loops, SkCanvas*) {
        int accum = 0;

        for (int j = 0; j < loops; ++j) {
            for (int i = 0; i < ARRAY; ++i) {
                accum += fVec[i].normalize();
            }
            this->process(accum);
        }
    }

    virtual const char* onGetName() {
        return fName;
    }

private:
    const char* fName;

    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class FixedMathBench : public SkBenchmark {
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

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual void onDraw(const int loops, SkCanvas*) {
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

    virtual const char* onGetName() {
        return "float_to_fixed";
    }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class DivModBench : public SkBenchmark {
    SkString fName;
public:
    explicit DivModBench(const char* name) {
        fName.printf("divmod_%s", name);
    }

    virtual bool isSuitableFor(Backend backend) SK_OVERRIDE {
        return backend == kNonRendering_Backend;
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) {
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
