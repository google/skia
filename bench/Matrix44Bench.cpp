/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkMatrix44.h"
#include "SkRandom.h"
#include "SkString.h"

class Matrix44Bench : public SkBenchmark {
    SkString    fName;
    enum { N = 10000 };
public:
    Matrix44Bench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("matrix44_%s", name);
        fIsRendering = false;
    }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas*) {
        int n = SkBENCHLOOP(N * this->mulLoopCount());
        for (int i = 0; i < n; i++) {
            this->performTest();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class EqualsMatrix44Bench : public Matrix44Bench {
public:
    EqualsMatrix44Bench(void* param) : INHERITED(param, "equals") {
        fM1.set(0, 0, 0);
        fM2.set(3, 3, 0);
    }
protected:
    virtual void performTest() {
        for (int i = 0; i < 10; ++i) {
            (void) (fM0 == fM1);
            (void) (fM1 == fM2);
            (void) (fM2 == fM0);
        }
    }
private:
    SkMatrix44 fM0, fM1, fM2;
    typedef Matrix44Bench INHERITED;
};

class PreScaleMatrix44Bench : public Matrix44Bench {
public:
    PreScaleMatrix44Bench(void* param) : INHERITED(param, "prescale") {
        fX = fY = fZ = SkDoubleToMScalar(1.5);
    }
protected:
    virtual void performTest() {
        fM0.reset();
        for (int i = 0; i < 10; ++i) {
            fM0.preScale(fX, fY, fZ);
        }
    }
private:
    SkMatrix44 fM0;
    SkMScalar  fX, fY, fZ;
    typedef Matrix44Bench INHERITED;
};

class InvertMatrix44Bench : public Matrix44Bench {
public:
    InvertMatrix44Bench(void* param) : INHERITED(param, "invert") {
    fM0.set(0, 0, -1.1);
    fM0.set(0, 1, 2.1);
    fM0.set(0, 2, -3.1);
    fM0.set(0, 3, 4.1);
    fM0.set(1, 0, 5.1);
    fM0.set(1, 1, -6.1);
    fM0.set(1, 2, 7.1);
    fM0.set(1, 3, 8.1);
    fM0.set(2, 0, -9.1);
    fM0.set(2, 1, 10.1);
    fM0.set(2, 2, 11.1);
    fM0.set(2, 3, -12.1);
    fM0.set(3, 0, -13.1);
    fM0.set(3, 1, 14.1);
    fM0.set(3, 2, -15.1);
    fM0.set(3, 3, 16.1);
    }
protected:
    virtual void performTest() {
        for (int i = 0; i < 10; ++i) {
            fM0.invert(&fM1);
        }
    }
private:
    SkMatrix44 fM0, fM1;
    typedef Matrix44Bench INHERITED;
};

class PostScaleMatrix44Bench : public Matrix44Bench {
public:
    PostScaleMatrix44Bench(void* param) : INHERITED(param, "postscale") {
        fX = fY = fZ = SkDoubleToMScalar(1.5);
    }
protected:
    virtual void performTest() {
        fM0.reset();
        for (int i = 0; i < 10; ++i) {
            fM0.postScale(fX, fY, fZ);
        }
    }
private:
    SkMatrix44 fM0;
    SkMScalar  fX, fY, fZ;
    typedef Matrix44Bench INHERITED;
};

class SetConcatMatrix44Bench : public Matrix44Bench {
public:
    SetConcatMatrix44Bench(void* param) : INHERITED(param, "setconcat") {
        fX = fY = fZ = SkDoubleToMScalar(1.5);
        fM1.setScale(fX, fY, fZ);
        fM2.setTranslate(fX, fY, fZ);
    }
protected:
    virtual void performTest() {
        fM0.reset();    // just to normalize this test with prescale/postscale
        for (int i = 0; i < 10; ++i) {
            fM0.setConcat(fM1, fM2);
        }
    }
private:
    SkMatrix44 fM0, fM1, fM2;
    SkMScalar  fX, fY, fZ;
    typedef Matrix44Bench INHERITED;
};

class GetTypeMatrix44Bench : public Matrix44Bench {
public:
    GetTypeMatrix44Bench(void* param) : INHERITED(param, "gettype") {}
protected:
    // Putting random generation of the matrix inside performTest()
    // would help us avoid anomalous runs, but takes up 25% or
    // more of the function time.
    virtual void performTest() {
        for (int i = 0; i < 20; ++i) {
            fMatrix.set(1, 2, 1);   // to invalidate the type-cache
            fMatrix.getType();
        }
    }
private:
    SkMatrix44 fMatrix;
    typedef Matrix44Bench INHERITED;
};

DEF_BENCH( return new EqualsMatrix44Bench(p); )
DEF_BENCH( return new PreScaleMatrix44Bench(p); )
DEF_BENCH( return new PostScaleMatrix44Bench(p); )
DEF_BENCH( return new InvertMatrix44Bench(p); )
DEF_BENCH( return new SetConcatMatrix44Bench(p); )
DEF_BENCH( return new GetTypeMatrix44Bench(p); )
