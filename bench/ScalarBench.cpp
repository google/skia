#include "SkBenchmark.h"
#include "SkFloatBits.h"
#include "SkRandom.h"
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
        int n = N * this->mulLoopCount();
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

static SkBenchmark* S0(void* p) { return new FloatComparisonBench(p); }
static SkBenchmark* S1(void* p) { return new ForcedIntComparisonBench(p); }

static BenchRegistry gReg0(S0);
static BenchRegistry gReg1(S1);
