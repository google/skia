/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkStrokerPriv_DEFINED
#define SkStrokerPriv_DEFINED

#include "src/core/SkGeometry.h"
#include "src/core/SkStroke.h"

// this enables a global which is not thread-safe; doing so triggers a TSAN error in Chrome tests.
#define QUAD_STROKE_APPROX_EXTENDED_DEBUGGING 0  // set to 1 to enable debugging in StrokerTest.cpp

class SkStrokerPriv {
public:
    SkStrokerPriv(uint8_t cap,
                  uint8_t join,
                  SkScalar radius,
                  SkScalar joinLimit);

    void setCap(uint8_t cap) { fCap = cap; }
    void setJoin(uint8_t join) { fJoin = join; }
    void tempJoin(SkPaint::Join join) {
        fTempJoin = fJoin;
        fJoin = (uint8_t) join;
    }
    void revertJoin() { fJoin = fTempJoin; }
    void setJoinLimit(SkScalar jl) { fJoinLimit = jl > 0.0f ? jl : 0.0f; }
    void setRadius(SkScalar radius) { fRadius = radius; }

    void setPivot(const SkPoint& pivot) { fPivot = pivot; }
    void setStop(const SkVector& stop) { fStop = stop; }

    bool calcUnitNormal(SkScalar x, SkScalar y, SkVector* un, SkVector* r);
    bool calcFromNormal(SkScalar x, SkScalar y, bool isLine, SkVector* r) {
        fFromIsLine = isLine;
        return this->calcUnitNormal(x, y, &fFromUnitNormal, r);
    }
    void setFromNormal(SkVector fmUN, bool isLine) {
        fFromIsLine = isLine;
        fFromUnitNormal = fmUN;
    }
    bool calcToNormal(SkScalar x, SkScalar y, bool isLine, SkVector* r) {
        fToIsLine = isLine;
        return this->calcUnitNormal(x, y, &fToUnitNormal, r);
    }
    void setToNormal(SkVector toUN, bool isLine) {
        fToIsLine = isLine;
        fToUnitNormal = toUN;
    }
    void getToNormal(SkVector *toUN) { *toUN = fToUnitNormal; }

    bool needsCurvature(SkPaint::Join join) { return join == SkPaint::kArcs_Join; }
    bool needsCurvature() { return needsCurvature((SkPaint::Join)fJoin); }
    void setFromCurvature(SkScalar fmC) {
         fFromCurvature = fmC;
    }
    void setToCurvature(SkScalar toC) {
         fToCurvature = toC;
    }

    void addCap(SkPath *path) const;
    void addJoin(SkPath *outer, SkPath *inner) const;

    bool capIs(SkPaint::Cap cap) const { return cap == (SkPaint::Cap) fCap; }
    bool joinIs(SkPaint::Join join) const { return join == (SkPaint::Join) fJoin; }

    static void closerIntersection(const SkPoint& l1,
        const SkPoint& l2, const SkPoint& r, const SkPoint& i1,
        const SkPoint& i2, SkPoint* i);

    static int lineCircleTest(const SkPoint &l, const SkVector& uv,
        const SkPoint& c, const SkScalar r);

    static int circlesTest(const SkPoint& c1, SkScalar r1, const SkPoint& c2,
        SkScalar r2);

    static bool intersectLines(const SkPoint& l1, const SkVector& uv1,
        const SkPoint& l2, const SkVector& uv2, SkPoint* i);

    static void intersectLineCircle(const SkPoint& l, const SkVector& uv,
        const SkPoint& c, SkScalar r, SkPoint* i1, SkPoint* i2);

    static void intersectCircles(const SkPoint& c1, SkScalar r1,
        const SkPoint& c2, SkScalar r2, SkPoint* i1, SkPoint* i2);

    static SkScalar adjustLineCircle(const SkPoint& l, const SkVector& uv,
        const SkPoint& p, const SkVector& cuv);

    static bool adjustCircles(const SkPoint& p1, const SkVector& uv1,
        SkScalar *r1, const SkPoint& p2, const SkVector& uv2, SkScalar *r2,
        bool inside);

    static int PVPCircle(const SkPoint& p1, const SkVector& v1,
        const SkPoint& p2, SkPoint* c, SkScalar* r);

    static SkPaint::Join calcArcsJoin(const SkVector& fromUnitNormal,
        SkScalar fromCurvature, const SkPoint& Pivot,
        const SkVector& toUnitNormal, const SkScalar toCurvature,
        const SkScalar radius, const SkScalar joinLimit, bool swapped,
        SkConic firstConics[SkConic::kMaxConicsForArc],
        SkConic secondConics[SkConic::kMaxConicsForArc],
        int* firstCount, int* secondCount);
private:
    SkScalar    fRadius, fJoinLimit, fFromCurvature, fToCurvature;
    uint8_t     fCap, fJoin, fTempJoin;
    bool        fFromIsLine, fToIsLine;
    SkPoint     fPivot, fStop;
    SkVector    fFromUnitNormal, fToUnitNormal;

    SkStrokerPriv() {}

    void addButtCap(SkPath* path) const;
    void addRoundCap(SkPath* path) const;
    void addSquareCap(SkPath* path) const;

    bool joinSwitch(SkPaint::Join join, SkPath* outer, const SkVector& after,
                    bool swapped) const;

    void addBevelJoin(SkPath* outer, const SkVector& after) const;
    bool addRoundJoin(SkPath* outer, bool swapped) const;
    bool addMiterJoin(SkPath* outer, const SkVector& after, bool swapped) const;
    bool addArcsJoin(SkPath* outer, const SkVector& after, bool swapped) const;
};

#endif
