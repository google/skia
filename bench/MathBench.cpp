#include "SkBenchmark.h"
#include "SkColorPriv.h"
#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkString.h"

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
    }

    virtual void performTest(float dst[], const float src[], int count) = 0;

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
    virtual void performITest(uint32_t* dst, const uint32_t* src, int count) = 0;
    
    virtual void performTest(float* SK_RESTRICT dst, const float* SK_RESTRICT src,
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
    virtual void performTest(float dst[], const float src[], int count) {
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
    virtual void performTest(float dst[], const float src[], int count) {
        for (int i = 0; i < count; ++i) {
            dst[i] = 1.0f / sk_float_sqrt(src[i]);
        }
    }
private:
    typedef MathBench INHERITED;
};

static inline float SkFastInvSqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i>>1);
    x = *(float*)&i;
    x = x*(1.5f-xhalf*x*x);
//    x = x*(1.5f-xhalf*x*x); // this line takes err from 10^-3 to 10^-6
    return x;
}

class FastISqrtMathBench : public MathBench {
public:
    FastISqrtMathBench(void* param) : INHERITED(param, "fastIsqrt") {}
protected:
    virtual void performTest(float dst[], const float src[], int count) {
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
    return ((tmp >> 8) & mask) | ((tmp >> 32) & ~mask);
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

static SkBenchmark* M0(void* p) { return new NoOpMathBench(p); }
static SkBenchmark* M1(void* p) { return new SlowISqrtMathBench(p); }
static SkBenchmark* M2(void* p) { return new FastISqrtMathBench(p); }
static SkBenchmark* M3(void* p) { return new QMul64Bench(p); }
static SkBenchmark* M4(void* p) { return new QMul32Bench(p); }

static BenchRegistry gReg0(M0);
static BenchRegistry gReg1(M1);
static BenchRegistry gReg2(M2);
static BenchRegistry gReg3(M3);
static BenchRegistry gReg4(M4);
