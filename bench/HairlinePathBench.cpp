/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"

enum Flags {
    kBig_Flag = 1 << 0,
    kAA_Flag = 1 << 1
};

#define FLAGS00 Flags(0)
#define FLAGS01 Flags(kBig_Flag)
#define FLAGS10 Flags(kAA_Flag)
#define FLAGS11 Flags(kBig_Flag | kAA_Flag)

static const int points[] = {
    10, 10, 15, 5, 20, 20,
    30, 5, 25, 20, 15, 12,
    21, 21, 30, 30, 12, 4,
    32, 28, 20, 18, 12, 10
};

static const int kMaxPathSize = 10;

class HairlinePathBench : public SkBenchmark {
public:
    HairlinePathBench(Flags flags) : fFlags(flags) {
        fPaint.setStyle(SkPaint::kStroke_Style);
        fPaint.setStrokeWidth(SkIntToScalar(0));
    }

    virtual void appendName(SkString*) = 0;
    virtual void makePath(SkPath*) = 0;

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        fName.printf("path_hairline_%s_%s_",
                     fFlags & kBig_Flag ? "big" : "small",
                     fFlags & kAA_Flag ? "AA" : "noAA");
        this->appendName(&fName);
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        paint.setAntiAlias(fFlags & kAA_Flag ? true : false);

        SkPath path;
        this->makePath(&path);
        if (fFlags & kBig_Flag) {
            SkMatrix m;
            m.setScale(SkIntToScalar(3), SkIntToScalar(3));
            path.transform(m);
        }

        for (int i = 0; i < loops; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    SkPaint     fPaint;
    SkString    fName;
    Flags       fFlags;
    typedef SkBenchmark INHERITED;
};

class LinePathBench : public HairlinePathBench {
public:
    LinePathBench(Flags flags) : INHERITED(flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("line");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand;
        int size = SK_ARRAY_COUNT(points);
        int hSize = size / 2;
        for (int i = 0; i < kMaxPathSize; ++i) {
            int xTrans = 10 + 40 * (i%(kMaxPathSize/2));
            int yTrans = 0;
            if (i > kMaxPathSize/2 - 1) {
                yTrans = 40;
            }
            int base1 = 2 * rand.nextULessThan(hSize);
            int base2 = 2 * rand.nextULessThan(hSize);
            int base3 = 2 * rand.nextULessThan(hSize);
            path->moveTo(SkIntToScalar(points[base1] + xTrans),
                         SkIntToScalar(points[base1+1] + yTrans));
            path->lineTo(SkIntToScalar(points[base2] + xTrans),
                         SkIntToScalar(points[base2+1] + yTrans));
            path->lineTo(SkIntToScalar(points[base3] + xTrans),
                         SkIntToScalar(points[base3+1] + yTrans));
        }
    }
private:
    typedef HairlinePathBench INHERITED;
};

class QuadPathBench : public HairlinePathBench {
public:
    QuadPathBench(Flags flags) : INHERITED(flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("quad");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand;
        int size = SK_ARRAY_COUNT(points);
        int hSize = size / 2;
        for (int i = 0; i < kMaxPathSize; ++i) {
            int xTrans = 10 + 40 * (i%(kMaxPathSize/2));
            int yTrans = 0;
            if (i > kMaxPathSize/2 - 1) {
                yTrans = 40;
            }
            int base1 = 2 * rand.nextULessThan(hSize);
            int base2 = 2 * rand.nextULessThan(hSize);
            int base3 = 2 * rand.nextULessThan(hSize);
            path->moveTo(SkIntToScalar(points[base1] + xTrans),
                         SkIntToScalar(points[base1+1] + yTrans));
            path->quadTo(SkIntToScalar(points[base2] + xTrans),
                         SkIntToScalar(points[base2+1] + yTrans),
                         SkIntToScalar(points[base3] + xTrans),
                         SkIntToScalar(points[base3+1] + yTrans));
        }
    }
private:
    typedef HairlinePathBench INHERITED;
};

class ConicPathBench : public HairlinePathBench {
public:
    ConicPathBench(Flags flags) : INHERITED(flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("conic");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand;
        SkRandom randWeight;
        int size = SK_ARRAY_COUNT(points);
        int hSize = size / 2;
        for (int i = 0; i < kMaxPathSize; ++i) {
            int xTrans = 10 + 40 * (i%(kMaxPathSize/2));
            int yTrans = 0;
            if (i > kMaxPathSize/2 - 1) {
                yTrans = 40;
            }
            int base1 = 2 * rand.nextULessThan(hSize);
            int base2 = 2 * rand.nextULessThan(hSize);
            int base3 = 2 * rand.nextULessThan(hSize);
            float weight = randWeight.nextRangeF(0.0f, 2.0f);
            path->moveTo(SkIntToScalar(points[base1] + xTrans),
                         SkIntToScalar(points[base1+1] + yTrans));
            path->conicTo(SkIntToScalar(points[base2] + xTrans),
                          SkIntToScalar(points[base2+1] + yTrans),
                         SkIntToScalar(points[base3] + xTrans),
                         SkIntToScalar(points[base3+1] + yTrans),
                         weight);
        }
    }
private:
    typedef HairlinePathBench INHERITED;
};

class CubicPathBench : public HairlinePathBench {
public:
    CubicPathBench(Flags flags) : INHERITED(flags) {}

    virtual void appendName(SkString* name) SK_OVERRIDE {
        name->append("cubic");
    }
    virtual void makePath(SkPath* path) SK_OVERRIDE {
        SkRandom rand;
        int size = SK_ARRAY_COUNT(points);
        int hSize = size / 2;
        for (int i = 0; i < kMaxPathSize; ++i) {
            int xTrans = 10 + 40 * (i%(kMaxPathSize/2));
            int yTrans = 0;
            if (i > kMaxPathSize/2 - 1) {
                yTrans = 40;
            }
            int base1 = 2 * rand.nextULessThan(hSize);
            int base2 = 2 * rand.nextULessThan(hSize);
            int base3 = 2 * rand.nextULessThan(hSize);
            int base4 = 2 * rand.nextULessThan(hSize);
            path->moveTo(SkIntToScalar(points[base1] + xTrans),
                         SkIntToScalar(points[base1+1] + yTrans));
            path->cubicTo(SkIntToScalar(points[base2] + xTrans),
                         SkIntToScalar(points[base2+1] + yTrans),
                         SkIntToScalar(points[base3] + xTrans),
                         SkIntToScalar(points[base3+1] + yTrans),
                         SkIntToScalar(points[base4] + xTrans),
                         SkIntToScalar(points[base4+1] + yTrans));
        }
    }
private:
    typedef HairlinePathBench INHERITED;
};

// FLAG00 - no AA, small
// FLAG01 - no AA, small
// FLAG10 - AA, big
// FLAG11 - AA, big

DEF_BENCH( return new LinePathBench(FLAGS00); )
DEF_BENCH( return new LinePathBench(FLAGS01); )
DEF_BENCH( return new LinePathBench(FLAGS10); )
DEF_BENCH( return new LinePathBench(FLAGS11); )

DEF_BENCH( return new QuadPathBench(FLAGS00); )
DEF_BENCH( return new QuadPathBench(FLAGS01); )
DEF_BENCH( return new QuadPathBench(FLAGS10); )
DEF_BENCH( return new QuadPathBench(FLAGS11); )

// Don't have default path renderer for conics yet on GPU, so must use AA
// DEF_BENCH( return new ConicPathBench(FLAGS00); )
// DEF_BENCH( return new ConicPathBench(FLAGS01); )
DEF_BENCH( return new ConicPathBench(FLAGS10); )
DEF_BENCH( return new ConicPathBench(FLAGS11); )

DEF_BENCH( return new CubicPathBench(FLAGS00); )
DEF_BENCH( return new CubicPathBench(FLAGS01); )
DEF_BENCH( return new CubicPathBench(FLAGS10); )
DEF_BENCH( return new CubicPathBench(FLAGS11); )
