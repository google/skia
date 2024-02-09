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
#include "src/base/SkRandom.h"

#define TILE(x, width)  (((x) & 0xFFFF) * width >> 16)

class InterpBench : public Benchmark {
    enum {
        kBuffer = 128,
        kLoop   = 20000
    };
    SkString    fName;
    int16_t     fDst[kBuffer];
    float       fFx, fDx;
public:
    InterpBench(const char name[])  {
        fName.printf("interp_%s", name);
        fFx = 3.3f;
        fDx = 0.1257f;
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

    virtual void performTest(int16_t dst[], float x, float dx, int count) = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        int n = loops * this->mulLoopCount();
        for (int i = 0; i < n; i++) {
            this->performTest(fDst, fFx, fDx, kBuffer);
        }
    }

private:
    using INHERITED = Benchmark;
};

class Fixed16D16Interp : public InterpBench {
public:
    Fixed16D16Interp() : INHERITED("16.16") {}

protected:
    void performTest(int16_t dst[], float fx, float dx, int count) override {
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
    using INHERITED = InterpBench;
};

class Fixed32D32Interp : public InterpBench {
public:
    Fixed32D32Interp() : INHERITED("32.32") {}

protected:
    void performTest(int16_t dst[], float fx, float dx, int count) override {
        int64_t curr = (int64_t)(fx * 65536 * 655536);
        int64_t step = (int64_t)(dx * 65536 * 655536);
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = (SkFixed)(curr >> 16);
            dst[i + 0] = TILE(tmp, count);
            curr += step;

            tmp = (SkFixed)(curr >> 16);
            dst[i + 1] = TILE(tmp, count);
            curr += step;

            tmp = (SkFixed)(curr >> 16);
            dst[i + 2] = TILE(tmp, count);
            curr += step;

            tmp = (SkFixed)(curr >> 16);
            dst[i + 3] = TILE(tmp, count);
            curr += step;
        }
    }
private:
    using INHERITED = InterpBench;
};

class Fixed16D48Interp : public InterpBench {
public:
    Fixed16D48Interp() : INHERITED("16.48") {}

protected:
    void performTest(int16_t dst[], float fx, float dx, int count) override {
        int64_t curr = (int64_t)(fx * 65536 * 655536 * 65536);
        int64_t step = (int64_t)(dx * 65536 * 655536 * 65536);
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = (SkFixed) (curr >> 32); dst[i + 0] = TILE(tmp, count); curr += step;
            tmp = (SkFixed) (curr >> 32); dst[i + 1] = TILE(tmp, count); curr += step;
            tmp = (SkFixed) (curr >> 32); dst[i + 2] = TILE(tmp, count); curr += step;
            tmp = (SkFixed) (curr >> 32); dst[i + 3] = TILE(tmp, count); curr += step;
        }
    }
private:
    using INHERITED = InterpBench;
};

class FloatInterp : public InterpBench {
public:
    FloatInterp() : INHERITED("float") {}

protected:
    void performTest(int16_t dst[], float fx, float dx, int count) override {
        SkFixed tmp;
        for (int i = 0; i < count; i += 4) {
            tmp = SkFloatToFixed(fx); dst[i + 0] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 1] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 2] = TILE(tmp, count); fx += dx;
            tmp = SkFloatToFixed(fx); dst[i + 3] = TILE(tmp, count); fx += dx;
        }
    }
private:
    using INHERITED = InterpBench;
};

class DoubleInterp : public InterpBench {
public:
    DoubleInterp() : INHERITED("double") {}

protected:
    void performTest(int16_t dst[], float fx, float dx, int count) override {
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
    using INHERITED = InterpBench;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new Fixed16D16Interp(); )
DEF_BENCH( return new Fixed32D32Interp(); )
DEF_BENCH( return new Fixed16D48Interp(); )
DEF_BENCH( return new FloatInterp(); )
DEF_BENCH( return new DoubleInterp(); )
