/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkM44.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

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
