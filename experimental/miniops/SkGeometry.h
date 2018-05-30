#ifndef SkGeometry_DEFINED
#define SkGeometry_DEFINED

#include "SkMatrix.h"

enum class SkCubicType {
    kSerpentine,
    kLoop,
    kLocalCusp,       // Cusp at a non-infinite parameter value with an inflection at t=infinity.
    kCuspAtInfinity,  // Cusp with a cusp at t=infinity and a local inflection.
    kQuadratic,
    kLineOrPoint
};

struct SkConic {
    SkConic() {}
    SkConic(const SkPoint pts[3], SkScalar w);
    bool SK_WARN_UNUSED_RESULT chopAt(SkScalar t, SkConic dst[2]) const;

    SkPoint  fPts[3];
    SkScalar fW;
};

SkCubicType SkClassifyCubic(const SkPoint p[4], double t[2] = nullptr, double s[2] = nullptr,
                            double d[4] = nullptr);
int SkChopQuadAtMaxCurvature(const SkPoint src[3], SkPoint dst[5]);
SkScalar SkFindQuadMaxCurvature(const SkPoint src[3]);

#endif
