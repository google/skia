#ifndef SkScalar_DEFINED
#define SkScalar_DEFINED

#include "SkFloatingPoint.h"

typedef float SkScalar;

#define SK_Scalar1                  1.0f
#define SK_ScalarMax                3.402823466e+38f

#define SK_ScalarInfinity           SK_FloatInfinity
#define SK_ScalarNaN                SK_FloatNaN

#define SkDoubleToScalar(x)     sk_double_to_float(x)
#define SK_ScalarMin            (-SK_ScalarMax)

#define SkScalarAbs(x)              sk_float_abs(x)

static inline bool SkScalarIsFinite(SkScalar x) { return sk_float_isfinite(x); }

static inline bool SkScalarsAreFinite(const SkScalar array[], int count) {
    SkScalar prod = 0;
    for (int i = 0; i < count; ++i) {
        prod *= array[i];
    }
    // At this point, prod will either be NaN or 0
    return prod == 0;   // if prod is NaN, this check will return false
}

static inline SkScalar SkMaxScalar(SkScalar a, SkScalar b) { return a > b ? a : b; }
static inline SkScalar SkMinScalar(SkScalar a, SkScalar b) { return a < b ? a : b; }

#endif
