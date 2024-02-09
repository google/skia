/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkM44.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "src/core/SkMatrixPriv.h"

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
        fM1 = SkM44::ColMajor(value + 0);
        fM2 = SkM44::ColMajor(value + 16);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
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
    using INHERITED = Benchmark;
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
    using INHERITED = M4Bench;
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
    using INHERITED = M4Bench;
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
    using INHERITED = M4Bench;
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
    using INHERITED = M4Bench;
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
    using INHERITED = M4Bench;
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
    using INHERITED = M4Bench;
};
DEF_BENCH( return new M4_map2(); )


enum class MapMatrixType {
    kTranslateOnly,
    kScaleTranslate,
    kRotate,
    kPerspective,
    kPerspectiveClipped
};
class MapRectBench : public Benchmark {
    SkString fName;

public:
    MapRectBench(MapMatrixType type, const char name[]) {
        SkRandom rand;
        const char* typeName;
        switch(type) {
            case MapMatrixType::kTranslateOnly:
                typeName = "t";
                fM = SkM44::Translate(rand.nextF(), rand.nextF());
                break;
            case MapMatrixType::kScaleTranslate:
                typeName = "s+t";
                fM = SkM44::Scale(rand.nextF(), rand.nextF());
                fM.postTranslate(rand.nextF(), rand.nextF());
                break;
            case MapMatrixType::kRotate:
                typeName = "r";
                fM = SkM44::Rotate({0.f, 0.f, 1.f}, SkDegreesToRadians(45.f));
                break;
            case MapMatrixType::kPerspective:
                typeName = "p";
                // Hand chosen to have all corners with w > 0 and w != 1
                fM = SkM44::Perspective(0.01f, 10.f, SK_ScalarPI / 3.f);
                fM.preTranslate(0.f, 5.f, -0.1f);
                fM.preConcat(SkM44::Rotate({0.f, 1.f, 0.f}, 0.008f /* radians */));
                break;
            case MapMatrixType::kPerspectiveClipped:
                typeName = "pc";
                // Hand chosen to have some corners with w > 0 and some with w < 0
                fM = SkM44();
                fM.setRow(3, {-.2f, -.6f, 0.f, 8.f});
                break;
        }
        fS = SkRect::MakeXYWH(10.f * rand.nextF(), 10.f * rand.nextF(),
                              150.f * rand.nextF(), 150.f * rand.nextF());

        fName.printf("mapRect_%s_%s", name, typeName);
    }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    virtual void performTest() = 0;

protected:
    SkM44 fM;
    SkRect fS, fD;

    virtual int mulLoopCount() const { return 1; }

    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            this->performTest();
        }
    }

private:
    using INHERITED = Benchmark;
};

class M4_mapRectBench : public MapRectBench {
public:
    M4_mapRectBench(MapMatrixType type) : INHERITED(type, "m4") {}

protected:
    void performTest() override {
        for (int i = 0; i < 100000; ++i) {
            fD = SkMatrixPriv::MapRect(fM, fS);
        }
    }

private:
    using INHERITED = MapRectBench;
};
DEF_BENCH(return new M4_mapRectBench(MapMatrixType::kTranslateOnly);)
DEF_BENCH(return new M4_mapRectBench(MapMatrixType::kScaleTranslate);)
DEF_BENCH(return new M4_mapRectBench(MapMatrixType::kRotate);)
DEF_BENCH(return new M4_mapRectBench(MapMatrixType::kPerspective);)
DEF_BENCH(return new M4_mapRectBench(MapMatrixType::kPerspectiveClipped);)

class M33_mapRectBench : public MapRectBench {
public:
    M33_mapRectBench(MapMatrixType type) : INHERITED(type, "m33") {
        fM33 = fM.asM33();
    }

protected:
    void performTest() override {
        for (int i = 0; i < 100000; ++i) {
            fD = fM33.mapRect(fS);
        }
    }
private:
    SkMatrix fM33;
    using INHERITED = MapRectBench;
};

DEF_BENCH(return new M33_mapRectBench(MapMatrixType::kTranslateOnly);)
DEF_BENCH(return new M33_mapRectBench(MapMatrixType::kScaleTranslate);)
DEF_BENCH(return new M33_mapRectBench(MapMatrixType::kRotate);)
DEF_BENCH(return new M33_mapRectBench(MapMatrixType::kPerspective);)
DEF_BENCH(return new M33_mapRectBench(MapMatrixType::kPerspectiveClipped);)
