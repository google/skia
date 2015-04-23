/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsCurve_DEFINE
#define SkPathOpsCurve_DEFINE

#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"

#ifndef SK_RELEASE
#include "SkPath.h"
#endif

struct SkOpCurve {
    SkPoint fPts[4];
    SkScalar fWeight;
    SkDEBUGCODE(SkPath::Verb fVerb);

    const SkPoint& operator[](int n) const {
        SkASSERT(n >= 0 && n <= SkPathOpsVerbToPoints(fVerb));
        return fPts[n];
    }

    void dump() const;

    void set(const SkDQuad& quad) {
        for (int index = 0; index < SkDQuad::kPointCount; ++index) {
            fPts[index] = quad[index].asSkPoint();
        }
        SkDEBUGCODE(fWeight = 1);
        SkDEBUGCODE(fVerb = SkPath::kQuad_Verb);
    }

    void set(const SkDCubic& cubic) {
        for (int index = 0; index < SkDCubic::kPointCount; ++index) {
            fPts[index] = cubic[index].asSkPoint();
        }
        SkDEBUGCODE(fWeight = 1);
        SkDEBUGCODE(fVerb = SkPath::kCubic_Verb);
    }
};

struct SkDCurve {
    union {
        SkDLine fLine;
        SkDQuad fQuad;
        SkDConic fConic;
        SkDCubic fCubic;
    };
    SkDEBUGCODE(SkPath::Verb fVerb);

    const SkDPoint& operator[](int n) const {
        SkASSERT(n >= 0 && n <= SkPathOpsVerbToPoints(fVerb));
        return fCubic[n];
    }

    SkDPoint& operator[](int n) {
        SkASSERT(n >= 0 && n <= SkPathOpsVerbToPoints(fVerb));
        return fCubic[n];
    }

    void dumpID(int ) const;
};

static SkDPoint dline_xy_at_t(const SkPoint a[2], SkScalar , double t) {
    SkDLine line;
    line.set(a);
    return line.ptAtT(t);
}

static SkDPoint dquad_xy_at_t(const SkPoint a[3], SkScalar , double t) {
    SkDQuad quad;
    quad.set(a);
    return quad.ptAtT(t);
}

static SkDPoint dconic_xy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    SkDConic conic;
    conic.set(a, weight);
    return conic.ptAtT(t);
}

static SkDPoint dcubic_xy_at_t(const SkPoint a[4], SkScalar , double t) {
    SkDCubic cubic;
    cubic.set(a);
    return cubic.ptAtT(t);
}

static SkDPoint (* const CurveDPointAtT[])(const SkPoint[], SkScalar , double ) = {
    NULL,
    dline_xy_at_t,
    dquad_xy_at_t,
    dconic_xy_at_t,
    dcubic_xy_at_t
};

static SkPoint fline_xy_at_t(const SkPoint a[2], SkScalar weight, double t) {
    return dline_xy_at_t(a, weight, t).asSkPoint();
}

static SkPoint fquad_xy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    return dquad_xy_at_t(a, weight, t).asSkPoint();
}

static SkPoint fconic_xy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    return dconic_xy_at_t(a, weight, t).asSkPoint();
}

static SkPoint fcubic_xy_at_t(const SkPoint a[4], SkScalar weight, double t) {
    return dcubic_xy_at_t(a, weight, t).asSkPoint();
}

static SkPoint (* const CurvePointAtT[])(const SkPoint[], SkScalar , double ) = {
    NULL,
    fline_xy_at_t,
    fquad_xy_at_t,
    fconic_xy_at_t,
    fcubic_xy_at_t
};

static SkDVector dline_dxdy_at_t(const SkPoint a[2], SkScalar , double ) {
    SkDLine line;
    line.set(a);
    return line[1] - line[0];
}

static SkDVector dquad_dxdy_at_t(const SkPoint a[3], SkScalar , double t) {
    SkDQuad quad;
    quad.set(a);
    return quad.dxdyAtT(t);
}

static SkDVector dconic_dxdy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    SkDConic conic;
    conic.set(a, weight);
    return conic.dxdyAtT(t);
}

static SkDVector dcubic_dxdy_at_t(const SkPoint a[4], SkScalar , double t) {
    SkDCubic cubic;
    cubic.set(a);
    return cubic.dxdyAtT(t);
}

static SkDVector (* const CurveDSlopeAtT[])(const SkPoint[], SkScalar , double ) = {
    NULL,
    dline_dxdy_at_t,
    dquad_dxdy_at_t,
    dconic_dxdy_at_t,
    dcubic_dxdy_at_t
};

static SkVector fline_dxdy_at_t(const SkPoint a[2], SkScalar , double ) {
    return a[1] - a[0];
}

static SkVector fquad_dxdy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    return dquad_dxdy_at_t(a, weight, t).asSkVector();
}

static SkVector fconic_dxdy_at_t(const SkPoint a[3], SkScalar weight, double t) {
    return dconic_dxdy_at_t(a, weight, t).asSkVector();
}

static SkVector fcubic_dxdy_at_t(const SkPoint a[4], SkScalar weight, double t) {
    return dcubic_dxdy_at_t(a, weight, t).asSkVector();
}

static SkVector (* const CurveSlopeAtT[])(const SkPoint[], SkScalar , double ) = {
    NULL,
    fline_dxdy_at_t,
    fquad_dxdy_at_t,
    fconic_dxdy_at_t,
    fcubic_dxdy_at_t
};

static SkPoint quad_top(const SkPoint a[3], SkScalar , double startT, double endT, double* topT) {
    SkDQuad quad;
    quad.set(a);
    SkDPoint topPt = quad.top(startT, endT, topT);
    return topPt.asSkPoint();
}

static SkPoint conic_top(const SkPoint a[3], SkScalar weight, double startT, double endT,
        double* topT) {
    SkDConic conic;
    conic.set(a, weight);
    SkDPoint topPt = conic.top(startT, endT, topT);
    return topPt.asSkPoint();
}

static SkPoint cubic_top(const SkPoint a[4], SkScalar , double startT, double endT, double* topT) {
    SkDCubic cubic;
    cubic.set(a);
    SkDPoint topPt = cubic.top(startT, endT, topT);
    return topPt.asSkPoint();
}

static SkPoint (* const CurveTop[])(const SkPoint[], SkScalar , double , double , double* ) = {
    NULL,
    NULL,
    quad_top,
    conic_top,
    cubic_top
};

static bool line_is_vertical(const SkPoint a[2], SkScalar , double startT, double endT) {
    SkDLine line;
    line.set(a);
    SkDPoint dst[2] = { line.ptAtT(startT), line.ptAtT(endT) };
    return AlmostEqualUlps(dst[0].fX, dst[1].fX);
}

static bool quad_is_vertical(const SkPoint a[3], SkScalar , double startT, double endT) {
    SkDQuad quad;
    quad.set(a);
    SkDQuad dst = quad.subDivide(startT, endT);
    return AlmostEqualUlps(dst[0].fX, dst[1].fX) && AlmostEqualUlps(dst[1].fX, dst[2].fX);
}

static bool conic_is_vertical(const SkPoint a[3], SkScalar weight, double startT, double endT) {
    SkDConic conic;
    conic.set(a, weight);
    SkDConic dst = conic.subDivide(startT, endT);
    return AlmostEqualUlps(dst[0].fX, dst[1].fX) && AlmostEqualUlps(dst[1].fX, dst[2].fX);
}

static bool cubic_is_vertical(const SkPoint a[4], SkScalar , double startT, double endT) {
    SkDCubic cubic;
    cubic.set(a);
    SkDCubic dst = cubic.subDivide(startT, endT);
    return AlmostEqualUlps(dst[0].fX, dst[1].fX) && AlmostEqualUlps(dst[1].fX, dst[2].fX)
            && AlmostEqualUlps(dst[2].fX, dst[3].fX);
}

static bool (* const CurveIsVertical[])(const SkPoint[], SkScalar , double , double) = {
    NULL,
    line_is_vertical,
    quad_is_vertical,
    conic_is_vertical,
    cubic_is_vertical
};

static void line_intersect_ray(const SkPoint a[2], SkScalar , const SkDLine& ray,
        SkIntersections* i) {
    SkDLine line;
    line.set(a);
    i->intersectRay(line, ray);
}

static void quad_intersect_ray(const SkPoint a[3], SkScalar , const SkDLine& ray,
        SkIntersections* i) {
    SkDQuad quad;
    quad.set(a);
    i->intersectRay(quad, ray);
}

static void conic_intersect_ray(const SkPoint a[3], SkScalar weight, const SkDLine& ray,
        SkIntersections* i) {
    SkDConic conic;
    conic.set(a, weight);
    i->intersectRay(conic, ray);
}

static void cubic_intersect_ray(const SkPoint a[4], SkScalar , const SkDLine& ray,
        SkIntersections* i) {
    SkDCubic cubic;
    cubic.set(a);
    i->intersectRay(cubic, ray);
}

static void (* const CurveIntersectRay[])(const SkPoint[] , SkScalar , const SkDLine& ,
        SkIntersections* ) = {
    NULL,
    line_intersect_ray,
    quad_intersect_ray,
    conic_intersect_ray,
    cubic_intersect_ray
};

#endif
