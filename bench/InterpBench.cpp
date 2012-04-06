#include "SkBenchmark.h"
#include "SkColorPriv.h"
#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkPaint.h"

#define TILE(x, width)  (((x) & 0xFFFF) * width >> 16)

class InterpBench : public SkBenchmark {
    enum {
        kBuffer = 128,
        kLoop   = 20000
    };
    SkString    fName;
    int16_t     fDst[kBuffer];
    float       fFx, fDx;
public:
    InterpBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("interp_%s", name);
        fFx = 3.3f;
        fDx = 0.1257f;
    }

    virtual void performTest(int16_t dst[], float x, float dx, int count) = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        int n = SkBENCHLOOP(kLoop * this->mulLoopCount());
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fFx, fDx, kBuffer);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class Fixed16D16Interp : public InterpBench {
public:
    Fixed16D16Interp(void* param) : INHERITED(param, "16.16") {}
    
protected:
    virtual void performTest(int16_t dst[], float fx, float dx, int count) SK_OVERRIDE {
        SkFixed curr = SkFloatToFixed(fx);
        SkFixed step = SkFloatToFixed(dx);
        for (int i = 0; i < count; i += 4) {
            dst[i + 0] = TILE(curr, count); curr += step;
            dst[i + 1] = TILE(curr, count); curr += step;
            dst[i + 2] = TILE(curr, count); curr += step;
            dst[i + 3] = TILE(curr, count); curr += step;
        }
    }
private:
    typedef InterpBench INHERITED;
};

class Fixed32D32Interp : public InterpBench {
public:
    Fixed32D32Interp(void* param) : INHERITED(param, "32.32") {}
    
protected:
    virtual void performTest(int16_t dst[], float fx, float dx, int count) SK_OVERRIDE {
        int64_t curr = (int64_t)(fx * 65536 * 655536);
        int64_t step = (int64_t)(dx * 65536 * 655536);
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = curr >> 16; dst[i + 0] = TILE(tmp, count); curr += step;
            tmp = curr >> 16; dst[i + 1] = TILE(tmp, count); curr += step;
            tmp = curr >> 16; dst[i + 2] = TILE(tmp, count); curr += step;
            tmp = curr >> 16; dst[i + 3] = TILE(tmp, count); curr += step;
        }
    }
private:
    typedef InterpBench INHERITED;
};

class Fixed16D48Interp : public InterpBench {
public:
    Fixed16D48Interp(void* param) : INHERITED(param, "16.48") {}
    
protected:
    virtual void performTest(int16_t dst[], float fx, float dx, int count) SK_OVERRIDE {
        int64_t curr = (int64_t)(fx * 65536 * 655536 * 65536);
        int64_t step = (int64_t)(dx * 65536 * 655536 * 65536);
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = curr >> 32; dst[i + 0] = TILE(tmp, count); curr += step;
            tmp = curr >> 32; dst[i + 1] = TILE(tmp, count); curr += step;
            tmp = curr >> 32; dst[i + 2] = TILE(tmp, count); curr += step;
            tmp = curr >> 32; dst[i + 3] = TILE(tmp, count); curr += step;
        }
    }
private:
    typedef InterpBench INHERITED;
};

class FloatInterp : public InterpBench {
public:
    FloatInterp(void* param) : INHERITED(param, "float") {}
    
protected:
    virtual void performTest(int16_t dst[], float fx, float dx, int count) SK_OVERRIDE {
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = SkFloatToFixed(fx); dst[i + 0] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 1] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 2] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 3] = TILE(tmp, count); fx += dx;
        }
    }
private:
    typedef InterpBench INHERITED;
};

class DoubleInterp : public InterpBench {
public:
    DoubleInterp(void* param) : INHERITED(param, "double") {}
    
protected:
    virtual void performTest(int16_t dst[], float fx, float dx, int count) SK_OVERRIDE {
        double ffx = fx;
        double ddx = dx;
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = SkDoubleToFixed(ffx); dst[i + 0] = TILE(tmp, count); ffx += ddx;
            tmp = SkDoubleToFixed(ffx); dst[i + 1] = TILE(tmp, count); ffx += ddx;
            tmp = SkDoubleToFixed(ffx); dst[i + 2] = TILE(tmp, count); ffx += ddx;
            tmp = SkDoubleToFixed(ffx); dst[i + 3] = TILE(tmp, count); ffx += ddx;
        }
    }
private:
    typedef InterpBench INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkBenchmark* M0(void* p) { return new Fixed16D16Interp(p); }
static SkBenchmark* M1(void* p) { return new Fixed32D32Interp(p); }
static SkBenchmark* M2(void* p) { return new Fixed16D48Interp(p); }
static SkBenchmark* M3(void* p) { return new FloatInterp(p); }
static SkBenchmark* M4(void* p) { return new DoubleInterp(p); }

static BenchRegistry gReg0(M0);
static BenchRegistry gReg1(M1);
static BenchRegistry gReg2(M2);
static BenchRegistry gReg3(M3);
static BenchRegistry gReg4(M4);

