/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkString.h"
#include "include/core/SkM44.h"
#include "include/utils/SkRandom.h"

class Matrix44Bench : public Benchmark {
    SkString    fName;
public:
    Matrix44Bench(const char name[]) {
        fName.printf("matrix44_%s", name);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest() = 0;

protected:
    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    typedef Benchmark INHERITED;
};

class EqualsMatrix44Bench : public Matrix44Bench {
public:
    EqualsMatrix44Bench()
        : INHERITED("equals")
    {
        fM1.set(0, 0, 0);
        fM2.set(3, 3, 0);
    }
protected:
    void performTest() override {
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

class SetIdentityMatrix44Bench : public Matrix44Bench {
public:
    SetIdentityMatrix44Bench()
        : INHERITED("setidentity")
    {
        double rowMajor[16] =
                { 1, 2, 3, 4,
                  5, 6, 7, 8,
                  9, 10, 11, 12,
                  13, 14, 15, 16};
        mat.setRowMajord(rowMajor);
    }
protected:
    void performTest() override {
        for (int i = 0; i < 10; ++i) {
            mat.setIdentity();
        }
    }
private:
    SkMatrix44 mat;
    typedef Matrix44Bench INHERITED;
};

class PreScaleMatrix44Bench : public Matrix44Bench {
public:
    PreScaleMatrix44Bench()
        : INHERITED("prescale")
    {
        fX = fY = fZ = 1.5f;
    }
protected:
    void performTest() override {
        fM0.reset();
        for (int i = 0; i < 10; ++i) {
            fM0.preScale(fX, fY, fZ);
        }
    }
private:
    SkMatrix44 fM0;
    SkScalar   fX, fY, fZ;
    typedef Matrix44Bench INHERITED;
};

class InvertMatrix44Bench : public Matrix44Bench {
public:
    InvertMatrix44Bench()
        : INHERITED("invert")
    {
        fM0.setDouble(0, 0, -1.1);
        fM0.setDouble(0, 1, 2.1);
        fM0.setDouble(0, 2, -3.1);
        fM0.setDouble(0, 3, 4.1);
        fM0.setDouble(1, 0, 5.1);
        fM0.setDouble(1, 1, -6.1);
        fM0.setDouble(1, 2, 7.1);
        fM0.setDouble(1, 3, 8.1);
        fM0.setDouble(2, 0, -9.1);
        fM0.setDouble(2, 1, 10.1);
        fM0.setDouble(2, 2, 11.1);
        fM0.setDouble(2, 3, -12.1);
        fM0.setDouble(3, 0, -13.1);
        fM0.setDouble(3, 1, 14.1);
        fM0.setDouble(3, 2, -15.1);
        fM0.setDouble(3, 3, 16.1);
    }
protected:
    void performTest() override {
        for (int i = 0; i < 10; ++i) {
            fM0.invert(&fM1);
        }
    }
private:
    SkMatrix44 fM0, fM1;
    typedef Matrix44Bench INHERITED;
};

class InvertAffineMatrix44Bench : public Matrix44Bench {
public:
    InvertAffineMatrix44Bench()
        : INHERITED("invertaffine")
    {
        fM0.setDouble(0, 0, -1.1);
        fM0.setDouble(0, 1, 2.1);
        fM0.setDouble(0, 2, -3.1);
        fM0.setDouble(0, 3, 4.1);
        fM0.setDouble(1, 0, 5.1);
        fM0.setDouble(1, 1, -6.1);
        fM0.setDouble(1, 2, 7.1);
        fM0.setDouble(1, 3, 8.1);
        fM0.setDouble(2, 0, -9.1);
        fM0.setDouble(2, 1, 10.1);
        fM0.setDouble(2, 2, 11.1);
        fM0.setDouble(2, 3, -12.1);
        // bottom row (perspective component) remains (0, 0, 0, 1).
    }
protected:
    void performTest() override {
        for (int i = 0; i < 10; ++i) {
            fM0.invert(&fM1);
        }
    }
private:
    SkMatrix44 fM0, fM1;
    typedef Matrix44Bench INHERITED;
};

class InvertScaleTranslateMatrix44Bench : public Matrix44Bench {
public:
    InvertScaleTranslateMatrix44Bench()
        : INHERITED("invertscaletranslate")
    {
        fM0.setDouble(0, 0, -1.1);
        fM0.setDouble(0, 3, 4.1);

        fM0.setDouble(1, 1, -6.1);
        fM0.setDouble(1, 3, 8.1);

        fM0.setDouble(2, 2, 11.1);
        fM0.setDouble(2, 3, -12.1);
    }
protected:
    void performTest() override {
        for (int i = 0; i < 10; ++i) {
            fM0.invert(&fM1);
        }
    }
private:
    SkMatrix44 fM0, fM1;
    typedef Matrix44Bench INHERITED;
};

class InvertTranslateMatrix44Bench : public Matrix44Bench {
public:
    InvertTranslateMatrix44Bench()
        : INHERITED("inverttranslate")
    {
        fM0.setDouble(0, 3, 4.1);
        fM0.setDouble(1, 3, 8.1);
        fM0.setDouble(2, 3, -12.1);
    }
protected:
    void performTest() override {
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
    PostScaleMatrix44Bench()
        : INHERITED("postscale")
    {
        fX = fY = fZ = 1.5f;
    }
protected:
    void performTest() override {
        fM0.reset();
        for (int i = 0; i < 10; ++i) {
            fM0.postScale(fX, fY, fZ);
        }
    }
private:
    SkMatrix44 fM0;
    SkScalar   fX, fY, fZ;
    typedef Matrix44Bench INHERITED;
};

class SetConcatMatrix44Bench : public Matrix44Bench {
public:
    // SkMatrix44::setConcat() has a fast path for matrices that are at most scale+translate.
    SetConcatMatrix44Bench(bool fastPath)
        : INHERITED(fastPath ? "setconcat_fast" : "setconcat_general")
{
        if (fastPath) {
            const SkScalar v = 1.5f;
            fM1.setScale(v,v,v);
            fM2.setTranslate(v,v,v);
        } else {
            SkRandom rand;
            for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                fM1.setFloat(x,y, rand.nextF());
                fM2.setFloat(x,y, rand.nextF());
            }}
        }
    }
protected:
    void performTest() override {
        fM0.reset();    // just to normalize this test with prescale/postscale
        for (int i = 0; i < 10000; ++i) {
            fM0.setConcat(fM1, fM2);
        }
    }
private:
    SkMatrix44 fM0, fM1, fM2;
    typedef Matrix44Bench INHERITED;
};

class GetTypeMatrix44Bench : public Matrix44Bench {
public:
    GetTypeMatrix44Bench()
        : INHERITED("gettype")
    {}
protected:
    // Putting random generation of the matrix inside performTest()
    // would help us avoid anomalous runs, but takes up 25% or
    // more of the function time.
    void performTest() override {
        for (int i = 0; i < 20; ++i) {
            fMatrix.set(1, 2, 1);   // to invalidate the type-cache
            fMatrix.getType();
        }
    }
private:
    SkMatrix44 fMatrix;
    typedef Matrix44Bench INHERITED;
};

DEF_BENCH( return new SetIdentityMatrix44Bench(); )
DEF_BENCH( return new EqualsMatrix44Bench(); )
DEF_BENCH( return new PreScaleMatrix44Bench(); )
DEF_BENCH( return new PostScaleMatrix44Bench(); )
DEF_BENCH( return new InvertMatrix44Bench(); )
DEF_BENCH( return new InvertAffineMatrix44Bench(); )
DEF_BENCH( return new InvertScaleTranslateMatrix44Bench(); )
DEF_BENCH( return new InvertTranslateMatrix44Bench(); )
DEF_BENCH( return new SetConcatMatrix44Bench(true); )
DEF_BENCH( return new SetConcatMatrix44Bench(false); )
DEF_BENCH( return new GetTypeMatrix44Bench(); )

/////////////////////////////////////////////////////////////////////////////////////////////////

class M4Bench : public Benchmark {
    SkString    fName;
public:
    M4Bench(const char name[]) {
        fName.printf("m4_%s", name);

        SkRandom rand;
        float value[32];
        for (auto& v : value) {
            v = rand.nextF();
        }
        fM1.setColMajor(value + 0);
        fM2.setColMajor(value + 16);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    virtual void performTest() = 0;

protected:
    SkM44 fM0, fM1, fM2;

    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    typedef Benchmark INHERITED;
};

class M4NEQ : public M4Bench {
public:
    M4NEQ() : INHERITED("neq") {}
protected:
    void performTest() override {
        for (int i = 0; i < 10000; ++i) {
            fEQ = (fM2 == fM1); // should always be false
        }
    }
private:
    bool fEQ;
    typedef M4Bench INHERITED;
};

class M4EQ : public M4Bench {
public:
    M4EQ() : INHERITED("eq") {}
protected:
    void performTest() override {
        fM2 = fM1;
        for (int i = 0; i < 10000; ++i) {
            fEQ = (fM2 == fM1); // should always be true
        }
    }
private:
    bool fEQ;
    typedef M4Bench INHERITED;
};

class M4Concat : public M4Bench {
public:
    M4Concat() : INHERITED("op_concat") {}
protected:
    void performTest() override {
        for (int i = 0; i < 10000; ++i) {
            fM0 = SkM44(fM1, fM2);
        }
    }
private:
    typedef M4Bench INHERITED;
};

class M4SetConcat : public M4Bench {
public:
    M4SetConcat() : INHERITED("set_concat") {}
protected:
    void performTest() override {
        for (int i = 0; i < 10000; ++i) {
            fM0.setConcat(fM1, fM2);
        }
    }
private:
    typedef M4Bench INHERITED;
};

DEF_BENCH( return new M4EQ(); )
DEF_BENCH( return new M4NEQ(); )
DEF_BENCH( return new M4Concat(); )
DEF_BENCH( return new M4SetConcat(); )

class M4_map4 : public M4Bench {
public:
    M4_map4() : INHERITED("map4") {}
protected:
    void performTest() override {
        SkV4 v = {1, 2, 3, 4};
        for (int i = 0; i < 100000; ++i) {
            fV = fM0 * v;
        }
    }
private:
    SkV4 fV;
    typedef M4Bench INHERITED;
};
DEF_BENCH( return new M4_map4(); )

class M4_map2 : public M4Bench {
public:
    M4_map2() : INHERITED("map2") {}
protected:
    void performTest() override {
        SkMatrix m;
        m.setRotate(1);
        for (int i = 0; i < 100000; ++i) {
            fV = m.mapXY(5, 6);
        }
    }
private:
    SkPoint fV;
    typedef M4Bench INHERITED;
};
DEF_BENCH( return new M4_map2(); )
