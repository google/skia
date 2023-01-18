/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/utils/SkPolyUtils.h"

using namespace skia_private;

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

class PolyUtilsBench : public Benchmark {
public:
    // Evaluate SkTriangulateSimplePolygon's performance (via derived classes) on:
    //   a non-self-intersecting star, a circle of tiny line segments and a self-intersecting star
    enum class Type { kConvexCheck, kSimpleCheck, kInsetConvex, kOffsetSimple, kTessellateSimple };

    PolyUtilsBench(Type type) : fType(type) {}

    virtual void appendName(SkString*) = 0;
    virtual void makePoly(SkTDArray<SkPoint>* poly) = 0;
    virtual int complexity() { return 0; }

protected:
    const char* onGetName() override {
        fName = "poly_utils_";
        this->appendName(&fName);
        switch (fType) {
        case Type::kConvexCheck:
            fName.append("_c");
            break;
        case Type::kSimpleCheck:
            fName.append("_s");
            break;
        case Type::kInsetConvex:
            fName.append("_i");
            break;
        case Type::kOffsetSimple:
            fName.append("_o");
            break;
        case Type::kTessellateSimple:
            fName.append("_t");
            break;
        }
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkTDArray<SkPoint> poly;
        this->makePoly(&poly);
        switch (fType) {
            case Type::kConvexCheck:
                for (int i = 0; i < loops; i++) {
                    (void)SkIsConvexPolygon(poly.begin(), poly.size());
                }
                break;
            case Type::kSimpleCheck:
                for (int i = 0; i < loops; i++) {
                    (void)SkIsSimplePolygon(poly.begin(), poly.size());
                }
                break;
            case Type::kInsetConvex:
                if (SkIsConvexPolygon(poly.begin(), poly.size())) {
                    SkTDArray<SkPoint> result;
                    for (int i = 0; i < loops; i++) {
                        (void)SkInsetConvexPolygon(poly.begin(), poly.size(), 10, &result);
                        (void)SkInsetConvexPolygon(poly.begin(), poly.size(), 40, &result);
                    }
                }
                break;
            case Type::kOffsetSimple:
                if (SkIsSimplePolygon(poly.begin(), poly.size())) {
                    SkTDArray<SkPoint> result;
                    SkRect bounds;
                    bounds.setBounds(poly.begin(), poly.size());
                    for (int i = 0; i < loops; i++) {
                        (void)SkOffsetSimplePolygon(poly.begin(), poly.size(), bounds, 10,
                                                    &result);
                        (void)SkOffsetSimplePolygon(poly.begin(), poly.size(), bounds, -10,
                                                    &result);
                    }
                }
                break;
            case Type::kTessellateSimple:
                if (SkIsSimplePolygon(poly.begin(), poly.size())) {
                    AutoSTMalloc<64, uint16_t> indexMap(poly.size());
                    for (int i = 0; i < poly.size(); ++i) {
                        indexMap[i] = i;
                    }
                    SkTDArray<uint16_t> triangleIndices;
                    for (int i = 0; i < loops; i++) {
                        SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                   &triangleIndices);
                    }
                }
                break;
        }
    }

private:
    SkString           fName;
    Type               fType;

    using INHERITED = Benchmark;
};

class StarPolyUtilsBench : public PolyUtilsBench {
public:
    StarPolyUtilsBench(PolyUtilsBench::Type type) : INHERITED(type) {}

    void appendName(SkString* name) override {
        name->append("star");
    }
    void makePoly(SkTDArray<SkPoint>* poly) override {
        // create non-intersecting star
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r1 = SkIntToScalar(20);
        const SkScalar r2 = SkIntToScalar(3);
        const int n = 500;
        SkScalar rad = 0;
        const SkScalar drad = SK_ScalarPI / n;
        for (int i = 0; i < n; i++) {
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r1, c + SkScalarSin(rad) * r1);
            rad += drad;
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r2, c + SkScalarSin(rad) * r2);
            rad += drad;
        }
    }
private:
    using INHERITED = PolyUtilsBench;
};

class CirclePolyUtilsBench : public PolyUtilsBench {
public:
    CirclePolyUtilsBench(PolyUtilsBench::Type type) : INHERITED(type) {}

    void appendName(SkString* name) override {
        name->append("circle");
    }
    void makePoly(SkTDArray<SkPoint>* poly) override {
        // create circle with many vertices
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r = SkIntToScalar(20);
        const int n = 1000;
        SkScalar rad = 0;
        const SkScalar drad = 2 * SK_ScalarPI / n;
        for (int i = 0; i < n; i++) {
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r, c + SkScalarSin(rad) * r);
            rad += drad;
        }
    }
private:
    using INHERITED = PolyUtilsBench;
};

class IntersectingPolyUtilsBench : public PolyUtilsBench {
public:
    IntersectingPolyUtilsBench(PolyUtilsBench::Type type) : INHERITED(type) {}

    void appendName(SkString* name) override {
        name->append("intersecting");
    }
    void makePoly(SkTDArray<SkPoint>* poly) override {
        // create self-intersecting star
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r = SkIntToScalar(20);
        const int n = 1000;

        SkScalar rad = -SK_ScalarPI / 2;
        const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;
        *poly->append() = SkPoint::Make(c, c - r);
        for (int i = 1; i < n; i++) {
            rad += drad;
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r, c + SkScalarSin(rad) * r);
        }
    }
private:
    using INHERITED = PolyUtilsBench;
};

// familiar videogame character
class NotchPolyUtilsBench : public PolyUtilsBench {
public:
    NotchPolyUtilsBench(PolyUtilsBench::Type type) : INHERITED(type) {}

    void appendName(SkString* name) override {
        name->append("notch");
    }
    void makePoly(SkTDArray<SkPoint>* poly) override {
        // create 3/4 circle with many vertices
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r = SkIntToScalar(20);
        const int n = 1000;
        SkScalar rad = 0;
        const SkScalar drad = 3 * SK_ScalarPI / (2*n);
        for (int i = 0; i < n; i++) {
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r, c + SkScalarSin(rad) * r);
            rad += drad;
        }
        // and the mouth
        *poly->append() = SkPoint::Make(45, 45);
    }
private:
    using INHERITED = PolyUtilsBench;
};

class IceCreamPolyUtilsBench : public PolyUtilsBench {
public:
    IceCreamPolyUtilsBench(PolyUtilsBench::Type type) : INHERITED(type) {}

    void appendName(SkString* name) override {
        name->append("icecream");
    }
    void makePoly(SkTDArray<SkPoint>* poly) override {
        // create 3/4 circle with many vertices
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r = SkIntToScalar(20);
        const int n = 1000;
        SkScalar rad = 0;
        const SkScalar drad = 3 * SK_ScalarPI / (2*n);
        for (int i = 0; i < n; i++) {
            *poly->append() = SkPoint::Make(c + SkScalarCos(rad) * r, c + SkScalarSin(rad) * r);
            rad += drad;
        }
        // and the tip of the cone
        *poly->append() = SkPoint::Make(90, 0);
    }
private:
    using INHERITED = PolyUtilsBench;
};

DEF_BENCH(return new StarPolyUtilsBench(PolyUtilsBench::Type::kConvexCheck);)
DEF_BENCH(return new StarPolyUtilsBench(PolyUtilsBench::Type::kSimpleCheck);)
DEF_BENCH(return new StarPolyUtilsBench(PolyUtilsBench::Type::kInsetConvex);)
DEF_BENCH(return new StarPolyUtilsBench(PolyUtilsBench::Type::kOffsetSimple);)
DEF_BENCH(return new StarPolyUtilsBench(PolyUtilsBench::Type::kTessellateSimple);)
DEF_BENCH(return new CirclePolyUtilsBench(PolyUtilsBench::Type::kConvexCheck);)
DEF_BENCH(return new CirclePolyUtilsBench(PolyUtilsBench::Type::kSimpleCheck);)
DEF_BENCH(return new CirclePolyUtilsBench(PolyUtilsBench::Type::kInsetConvex);)
DEF_BENCH(return new CirclePolyUtilsBench(PolyUtilsBench::Type::kOffsetSimple);)
DEF_BENCH(return new CirclePolyUtilsBench(PolyUtilsBench::Type::kTessellateSimple);)
DEF_BENCH(return new IntersectingPolyUtilsBench(PolyUtilsBench::Type::kConvexCheck);)
DEF_BENCH(return new IntersectingPolyUtilsBench(PolyUtilsBench::Type::kSimpleCheck);)
DEF_BENCH(return new IntersectingPolyUtilsBench(PolyUtilsBench::Type::kInsetConvex);)
DEF_BENCH(return new IntersectingPolyUtilsBench(PolyUtilsBench::Type::kOffsetSimple);)
DEF_BENCH(return new IntersectingPolyUtilsBench(PolyUtilsBench::Type::kTessellateSimple);)
DEF_BENCH(return new NotchPolyUtilsBench(PolyUtilsBench::Type::kConvexCheck);)
DEF_BENCH(return new NotchPolyUtilsBench(PolyUtilsBench::Type::kSimpleCheck);)
DEF_BENCH(return new NotchPolyUtilsBench(PolyUtilsBench::Type::kInsetConvex);)
DEF_BENCH(return new NotchPolyUtilsBench(PolyUtilsBench::Type::kOffsetSimple);)
DEF_BENCH(return new NotchPolyUtilsBench(PolyUtilsBench::Type::kTessellateSimple);)
DEF_BENCH(return new IceCreamPolyUtilsBench(PolyUtilsBench::Type::kConvexCheck);)
DEF_BENCH(return new IceCreamPolyUtilsBench(PolyUtilsBench::Type::kSimpleCheck);)
DEF_BENCH(return new IceCreamPolyUtilsBench(PolyUtilsBench::Type::kInsetConvex);)
DEF_BENCH(return new IceCreamPolyUtilsBench(PolyUtilsBench::Type::kOffsetSimple);)
DEF_BENCH(return new IceCreamPolyUtilsBench(PolyUtilsBench::Type::kTessellateSimple);)

#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)
