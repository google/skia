/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkFixed.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRandom.h"

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
        return backend == Backend::kNonRendering;
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
    using INHERITED = Benchmark;
};

class MathBenchU32 : public MathBench {
public:
    MathBenchU32(const char name[]) : INHERITED(name) {}

protected:
    virtual void performITest(uint32_t* SK_RESTRICT dst,
                              const uint32_t* SK_RESTRICT src,
                              int count) = 0;

    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        uint32_t* d = reinterpret_cast<uint32_t*>(dst);
        const uint32_t* s = reinterpret_cast<const uint32_t*>(src);
        this->performITest(d, s, count);
    }
private:
    using INHERITED = MathBench;
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
    using INHERITED = MathBench;
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
    using INHERITED = MathBench;
};


class SlowISqrtMathBench : public MathBench {
public:
    SlowISqrtMathBench() : INHERITED("slowIsqrt") {}
protected:
    void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src, int count) override {
        for (int i = 0; i < count; ++i) {
            dst[i] = 1.0f / std::sqrt(src[i]);
        }
    }
private:
    using INHERITED = MathBench;
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
    using INHERITED = MathBench;
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
    using INHERITED = MathBenchU32;
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
    using INHERITED = MathBenchU32;
};

///////////////////////////////////////////////////////////////////////////////

static bool isFinite_int(float x) {
    uint32_t bits = SkFloat2Bits(x);    // need unsigned for our shifts
    int exponent = bits << 1 >> 24;
    return exponent != 0xFF;
}

static bool isFinite_mulzero(float x) {
    float y = x * 0;
    return y == y;
}

static bool isfinite_and_int(const float data[4]) {
    return  isFinite_int(data[0]) && isFinite_int(data[1]) && isFinite_int(data[2]) && isFinite_int(data[3]);
}

static bool isfinite_and_mulzero(const float data[4]) {
    return  isFinite_mulzero(data[0]) && isFinite_mulzero(data[1]) && isFinite_mulzero(data[2]) && isFinite_mulzero(data[3]);
}

#define mulzeroadd(data)    (data[0]*0 + data[1]*0 + data[2]*0 + data[3]*0)

static bool isfinite_plus_int(const float data[4]) {
    return  isFinite_int(mulzeroadd(data));
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
    MAKEREC(isfinite_and_mulzero),
    MAKEREC(isfinite_plus_int),
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
        return backend == Backend::kNonRendering;
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
                    if ((false)) { // avoid bit rot, suppress warning
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

    using INHERITED = Benchmark;
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
        return backend == Backend::kNonRendering;
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
                    accum += std::floor(data[i]);
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

    using INHERITED = Benchmark;
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
        return backend == Backend::kNonRendering;
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

    using INHERITED = Benchmark;
};

class CTZBench : public Benchmark {
    enum {
        ARRAY = 1000,
    };
    uint32_t fData[ARRAY];
    bool fUsePortable;

public:
    CTZBench(bool usePortable) : fUsePortable(usePortable) {

        SkRandom rand;
        for (int i = 0; i < ARRAY; ++i) {
            fData[i] = rand.nextU();
        }

        if (fUsePortable) {
            fName = "ctz_portable";
        } else {
            fName = "ctz_intrinsic";
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    // just so the compiler doesn't remove our loops
    virtual void process(int) {}

protected:
    void onDraw(int loops, SkCanvas*) override {
        int accum = 0;

        if (fUsePortable) {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += SkCTZ_portable(fData[i]);
                }
                this->process(accum);
            }
        } else {
            for (int j = 0; j < loops; ++j) {
                for (int i = 0; i < ARRAY; ++i) {
                    accum += SkCTZ(fData[i]);
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

    using INHERITED = Benchmark;
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
        return backend == Backend::kNonRendering;
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

    using INHERITED = Benchmark;
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
        return backend == Backend::kNonRendering;
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
    using INHERITED = Benchmark;
};

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

DEF_BENCH( return new FloorBench(false); )
DEF_BENCH( return new FloorBench(true); )

DEF_BENCH( return new CLZBench(false); )
DEF_BENCH( return new CLZBench(true); )
DEF_BENCH( return new CTZBench(false); )
DEF_BENCH( return new CTZBench(true); )

DEF_BENCH( return new NormalizeBench(); )

DEF_BENCH( return new FixedMathBench(); )

//////////////////////////////////////////////////////////////

#include "src/base/SkFloatBits.h"
class Floor2IntBench : public Benchmark {
    enum {
        ARRAY = 1000,
    };
    float fData[ARRAY];
    const bool fSat;
public:

    Floor2IntBench(bool sat) : fSat(sat) {
        SkRandom rand;

        for (int i = 0; i < ARRAY; ++i) {
            fData[i] = SkBits2Float(rand.nextU());
        }

        if (sat) {
            fName = "floor2int_sat";
        } else {
            fName = "floor2int_undef";
        }
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    // These exist to try to stop the compiler from detecting what we doing, and throwing
    // parts away (or knowing exactly how big the loop counts are).
    virtual void process(unsigned) {}
    virtual int count() { return ARRAY; }

protected:
    void onDraw(int loops, SkCanvas*) override {
        // used unsigned to avoid undefined behavior if/when the += might overflow
        unsigned accum = 0;

        for (int j = 0; j < loops; ++j) {
            int n = this->count();
            if (fSat) {
                for (int i = 0; i < n; ++i) {
                    accum += sk_float_floor2int(fData[i]);
                }
            } else {
                for (int i = 0; i < n; ++i) {
                    accum += sk_float_floor2int_no_saturate(fData[i]);
                }
            }
            this->process(accum);
        }
    }

    const char* onGetName() override { return fName; }

private:
    const char* fName;

    using INHERITED = Benchmark;
};
DEF_BENCH( return new Floor2IntBench(false); )
DEF_BENCH( return new Floor2IntBench(true); )

