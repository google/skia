#include "SkBenchmark.h"
#include "SkMatrix.h"
#include "SkString.h"

class MatrixBench : public SkBenchmark {
    SkString    fName;
    enum { N = 100000 };
public:
    MatrixBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("matrix_%s", name);
    }

    virtual void performTest() = 0;

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int i = 0; i < N; i++) {
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
    EqualsMatrixBench(void* param) : INHERITED(param, "equals") {}
protected:
    virtual void performTest() {
        SkMatrix m0, m1, m2;
        
        m0.reset();
        m1.reset();
        m2.reset();
        always_do(m0 == m1);
        always_do(m1 == m2);
        always_do(m2 == m0);
        always_do(m0.getType());
        always_do(m1.getType());
        always_do(m2.getType());
    }
private:
    typedef MatrixBench INHERITED;
};

class ScaleMatrixBench : public MatrixBench {
public:
    ScaleMatrixBench(void* param) : INHERITED(param, "scale") {
        
        fM0.reset();
        fM1.setScale(fSX, fSY);
        fM2.setTranslate(fSX, fSY);
        fSX = fSY = SkFloatToScalar(1.5f);
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

static SkBenchmark* M0(void* p) { return new EqualsMatrixBench(p); }
static SkBenchmark* M1(void* p) { return new ScaleMatrixBench(p); }

static BenchRegistry gReg0(M0);
static BenchRegistry gReg1(M1);

