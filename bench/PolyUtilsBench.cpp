/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkPolyUtils.h"

class PolyUtilsBench : public Benchmark {
    // Evaluate SkTriangulateSimplePolygon's performance (via derived classes) on:
    //   a non-self-intersecting star, a circle of tiny line segments and a self-intersecting star

    SkString           fName;
public:
    PolyUtilsBench() {}

    virtual void appendName(SkString*) = 0;
    virtual void makePoly(SkTDArray<SkPoint>* poly) = 0;
    virtual int complexity() { return 0; }

protected:
    const char* onGetName() override {
        fName = "poly_utils_";
        this->appendName(&fName);
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkTDArray<SkPoint> poly;
        this->makePoly(&poly);
        SkAutoSTMalloc<64, uint16_t> indexMap(poly.count());
        for (int i = 0; i < poly.count(); ++i) {
            indexMap[i] = i;
        }
        SkTDArray<uint16_t> triangleIndices;
        for (int i = 0; i < loops; i++) {
            if (SkIsSimplePolygon(poly.begin(), poly.count())) {
                SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                           &triangleIndices);
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};

class StarPolyUtilsBench : public PolyUtilsBench {
public:
    StarPolyUtilsBench() {}

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
            SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
            *poly->push() = SkPoint::Make(c + cosV * r1, c + sinV * r1);
            rad += drad;
            sinV = SkScalarSinCos(rad, &cosV);
            *poly->push() = SkPoint::Make(c + cosV * r2, c + sinV * r2);
            rad += drad;
        }
    }
private:
    typedef PolyUtilsBench INHERITED;
};

class CirclePolyUtilsBench : public PolyUtilsBench {
public:
    CirclePolyUtilsBench() {}

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
            SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
            *poly->push() = SkPoint::Make(c + cosV * r, c + sinV * r);
            rad += drad;
        }
    }
private:
    typedef PolyUtilsBench INHERITED;
};

class IntersectingPolyUtilsBench : public PolyUtilsBench {
public:
    IntersectingPolyUtilsBench() {}

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
        *poly->push() = SkPoint::Make(c, c - r);
        for (int i = 1; i < n; i++) {
            rad += drad;
            SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
            *poly->push() = SkPoint::Make(c + cosV * r, c + sinV * r);
        }
    }
private:
    typedef PolyUtilsBench INHERITED;
};

DEF_BENCH(return new StarPolyUtilsBench();)
DEF_BENCH(return new CirclePolyUtilsBench();)
DEF_BENCH(return new IntersectingPolyUtilsBench();)
