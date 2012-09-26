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
        kLoop   = 10000
    };
    SkString    fName;
    float       fSrc[kBuffer], fDst[kBuffer];
public:
    MathBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("math_%s", name);

        SkRandom rand;
        for (int i = 0; i < kBuffer; ++i) {
            fSrc[i] = rand.nextSScalar1();
        }

        fIsRendering = false;
    }

    virtual void performTest(float* SK_RESTRICT dst,
                              const float* SK_RESTRICT src,
                              int count) = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        int n = SkBENCHLOOP(kLoop * this->mulLoopCount());
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fSrc, kBuffer);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class MathBenchU32 : public MathBench {
public:
    MathBenchU32(void* param, const char name[]) : INHERITED(param, name) {}

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
    NoOpMathBench(void* param) : INHERITED(param, "noOp") {}
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

class SlowISqrtMathBench : public MathBench {
public:
    SlowISqrtMathBench(void* param) : INHERITED(param, "slowIsqrt") {}
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
    FastISqrtMathBench(void* param) : INHERITED(param, "fastIsqrt") {}
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
    QMul64Bench(void* param) : INHERITED(param, "qmul64") {}
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
    QMul32Bench(void* param) : INHERITED(param, "qmul32") {}
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
        N = SkBENCHLOOP(1000),
        NN = SkBENCHLOOP(1000),
    };
    float fData[N];
public:

    IsFiniteBench(void* param, int index) : INHERITED(param) {
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
        fIsRendering = false;
    }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        IsFiniteProc proc = fProc;
        const float* data = fData;
        // do this so the compiler won't throw away the function call
        int counter = 0;

        if (proc) {
            for (int j = 0; j < NN; ++j) {
                for (int i = 0; i < N - 4; ++i) {
                    counter += proc(&data[i]);
                }
            }
        } else {
            for (int j = 0; j < NN; ++j) {
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
        ARRAY = SkBENCHLOOP(1000),
        LOOP = SkBENCHLOOP(1000),
    };
    float fData[ARRAY];
    bool fFast;
public:

    FloorBench(void* param, bool fast) : INHERITED(param), fFast(fast) {
        SkRandom rand;

        for (int i = 0; i < ARRAY; ++i) {
            fData[i] = rand.nextSScalar1();
        }

        if (fast) {
            fName = "floor_fast";
        } else {
            fName = "floor_std";
        }
        fIsRendering = false;
    }

    virtual void process(float) {}

protected:
    virtual void onDraw(SkCanvas* canvas) {
        SkRandom rand;
        float accum = 0;
        const float* data = fData;

        if (fFast) {
            for (int j = 0; j < LOOP; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += fast_floor(data[i]);
                }
                this->process(accum);
            }
        } else {
            for (int j = 0; j < LOOP; ++j) {
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

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* M0(void* p) { return new NoOpMathBench(p); }
static SkBenchmark* M1(void* p) { return new SlowISqrtMathBench(p); }
static SkBenchmark* M2(void* p) { return new FastISqrtMathBench(p); }
static SkBenchmark* M3(void* p) { return new QMul64Bench(p); }
static SkBenchmark* M4(void* p) { return new QMul32Bench(p); }

static SkBenchmark* M5neg1(void* p) { return new IsFiniteBench(p, -1); }
static SkBenchmark* M50(void* p) { return new IsFiniteBench(p, 0); }
static SkBenchmark* M51(void* p) { return new IsFiniteBench(p, 1); }
static SkBenchmark* M52(void* p) { return new IsFiniteBench(p, 2); }
static SkBenchmark* M53(void* p) { return new IsFiniteBench(p, 3); }
static SkBenchmark* M54(void* p) { return new IsFiniteBench(p, 4); }
static SkBenchmark* M55(void* p) { return new IsFiniteBench(p, 5); }

static SkBenchmark* F0(void* p) { return new FloorBench(p, false); }
static SkBenchmark* F1(void* p) { return new FloorBench(p, true); }

static BenchRegistry gReg0(M0);
static BenchRegistry gReg1(M1);
static BenchRegistry gReg2(M2);
static BenchRegistry gReg3(M3);
static BenchRegistry gReg4(M4);

static BenchRegistry gReg5neg1(M5neg1);
static BenchRegistry gReg50(M50);
static BenchRegistry gReg51(M51);
static BenchRegistry gReg52(M52);
static BenchRegistry gReg53(M53);
static BenchRegistry gReg54(M54);
static BenchRegistry gReg55(M55);

static BenchRegistry gRF0(F0);
static BenchRegistry gRF1(F1);
