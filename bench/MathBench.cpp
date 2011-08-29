#include "SkBenchmark.h"
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
        int n = kLoop * this->mulLoopCount();
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fSrc, kBuffer);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

int gMathBench_NonStaticGlobal;

#define always_do(pred)                     \
    do {                                    \
        if (pred) {                         \
            ++gMathBench_NonStaticGlobal; \
        }                                   \
    } while (0)

class NoOpMathBench : public MathBench {
public:
    NoOpMathBench(void* param) : INHERITED(param, "slowIsqrt") {}
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

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* M0(void* p) { return new NoOpMathBench(p); }
static SkBenchmark* M1(void* p) { return new SlowISqrtMathBench(p); }
static SkBenchmark* M2(void* p) { return new FastISqrtMathBench(p); }

static BenchRegistry gReg0(M0);
static BenchRegistry gReg1(M1);
static BenchRegistry gReg2(M2);
