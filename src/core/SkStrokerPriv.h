/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkStrokerPriv_DEFINED
#define SkStrokerPriv_DEFINED

#include "src/core/SkStroke.h"

#define CWX(x, y)   (-y)
#define CWY(x, y)   (x)
#define CCWX(x, y)  (y)
#define CCWY(x, y)  (-x)

#define CUBIC_ARC_FACTOR    ((SK_ScalarSqrt2 - SK_Scalar1) * 4 / 3)

// this enables a global which is not thread-safe; doing so triggers a TSAN error in Chrome tests.
#define QUAD_STROKE_APPROX_EXTENDED_DEBUGGING 0  // set to 1 to enable debugging in StrokerTest.cpp

class SkStrokerPriv {
public:
    typedef void (*CapProc)(SkPath* path,
                            const SkPoint& pivot,
                            const SkVector& normal,
                            const SkPoint& stop,
                            SkPath* otherPath);

    typedef void (*JoinProc)(SkPath* outer, SkPath* inner,
                             const SkVector& beforeUnitNormal,
                             const SkPoint& pivot,
                             const SkVector& afterUnitNormal,
                             SkScalar radius, SkScalar invMiterLimit,
                             bool prevIsLine, bool currIsLine);

    static CapProc  CapFactory(SkPaint::Cap);
    static JoinProc JoinFactory(SkPaint::Join);
};

#endif
