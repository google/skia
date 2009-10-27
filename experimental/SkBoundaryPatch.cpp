#include "SkBoundaryPatch.h"

SkBoundaryPatch::SkBoundaryPatch() {
    sk_bzero(fCurve, sizeof(fCurve));
};

SkBoundaryPatch::~SkBoundaryPatch() {
    for (int i = 0; i < 4; i++) {
        SkSafeUnref(fCurve[i]);
    }
}

SkBoundaryCurve* SkBoundaryPatch::setCurve(Edge e, SkBoundaryCurve* curve) {
    SkASSERT((unsigned)e < 4);

    SkRefCnt_SafeAssign(fCurve[e], curve);
    return curve;
}

static SkPoint SkMakePoint(SkScalar x, SkScalar y) {
    SkPoint pt;
    pt.set(x, y);
    return pt;
}

static SkPoint SkPointInterp(const SkPoint& a, const SkPoint& b, SkScalar t) {
    return SkMakePoint(SkScalarInterp(a.fX, b.fX, t),
                       SkScalarInterp(a.fY, b.fY, t));
}

SkPoint SkBoundaryPatch::evaluate(SkScalar unitU, SkScalar unitV) {
    SkPoint u = SkPointInterp(fCurve[kLeft]->evaluate(unitV),
                              fCurve[kRight]->evaluate(unitV), unitU);
    SkPoint v = SkPointInterp(fCurve[kTop]->evaluate(unitU),
                              fCurve[kBottom]->evaluate(unitU), unitV);
    return SkMakePoint(SkScalarAve(u.fX, v.fX),
                       SkScalarAve(u.fY, v.fY));
}

////////////////////////////////////////////////////////////////////////

#include "SkGeometry.h"

SkPoint SkLineBoundaryCurve::evaluate(SkScalar t) {
    return SkPointInterp(fPts[0], fPts[1], t);
}

SkPoint SkCubicBoundaryCurve::evaluate(SkScalar t) {
    SkPoint loc;
    SkEvalCubicAt(fPts, t, &loc, NULL, NULL);
    return loc;
}


