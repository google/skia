/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientBase_DEFINED
#define Sk4fGradientBase_DEFINED

#include "SkColor.h"
#include "SkGradientShaderPriv.h"
#include "SkMatrix.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkShader.h"
#include "SkTArray.h"

class SkGradientShaderBase::
GradientShaderBase4fContext : public SkShader::Context {
public:
    GradientShaderBase4fContext(const SkGradientShaderBase&,
                                const ContextRec&);

    uint32_t getFlags() const override { return fFlags; }

protected:
    struct Interval {
        Interval(SkPMColor c0, SkScalar p0,
                 SkPMColor c1, SkScalar p1,
                 const Sk4f& componentScale);
        Interval(const Sk4f& c0, const Sk4f& dc,
                 SkScalar p0, SkScalar p1);

        bool isZeroRamp() const { return fZeroRamp; }

        // true when fx is in [p0,p1)
        bool contains(SkScalar fx) const;

        SkPM4f   fC0, fDc;
        SkScalar fP0, fP1;
        bool     fZeroRamp;
    };

    const Interval* findInterval(SkScalar fx) const;

    SkSTArray<8, Interval, true> fIntervals;
    SkMatrix                     fDstToPos;
    SkMatrix::MapXYProc          fDstToPosProc;
    uint8_t                      fDstToPosClass;
    uint8_t                      fFlags;
    bool                         fDither;
    bool                         fColorsArePremul;

private:
    using INHERITED = SkShader::Context;

    mutable const Interval*      fCachedInterval;
};

#endif // Sk4fGradientBase_DEFINED
