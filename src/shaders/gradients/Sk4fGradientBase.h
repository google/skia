/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientBase_DEFINED
#define Sk4fGradientBase_DEFINED

#include "include/core/SkColor.h"
#include "include/private/SkNx.h"
#include "include/private/SkTArray.h"
#include "src/core/SkMatrixPriv.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/Sk4fGradientPriv.h"
#include "src/shaders/gradients/SkGradientShaderPriv.h"

struct Sk4fGradientInterval {
    Sk4fGradientInterval(const Sk4f& c0, SkScalar t0,
                         const Sk4f& c1, SkScalar t1);

    bool contains(SkScalar t) const {
        // True if t is in [p0,p1].  Note: this helper assumes a
        // natural/increasing interval - so it's not usable in Sk4fLinearGradient.
        SkASSERT(fT0 < fT1);
        return t >= fT0 && t <= fT1;
    }

    // Color bias and color gradient, such that for a t in this interval
    //
    //   C = fCb + t * fCg;
    SkPMColor4f fCb, fCg;
    SkScalar    fT0, fT1;
};

class Sk4fGradientIntervalBuffer {
public:
    void init(const SkGradientShaderBase&, SkColorSpace* dstCS, SkTileMode tileMode,
              bool premulColors, SkScalar alpha, bool reverse);

    const Sk4fGradientInterval* find(SkScalar t) const;
    const Sk4fGradientInterval* findNext(SkScalar t, const Sk4fGradientInterval* prev,
                                         bool increasing) const;

    using BufferType = SkSTArray<8, Sk4fGradientInterval, true>;

    const BufferType* operator->() const { return &fIntervals; }

private:
    BufferType fIntervals;
};

class SkGradientShaderBase::
GradientShaderBase4fContext : public Context {
public:
    GradientShaderBase4fContext(const SkGradientShaderBase&,
                                const ContextRec&);

    uint32_t getFlags() const override { return fFlags; }

    bool isValid() const;

protected:
    Sk4fGradientIntervalBuffer fIntervals;
    SkMatrix                   fDstToPos;
    SkMatrixPriv::MapXYProc    fDstToPosProc;
    uint8_t                    fFlags;
    bool                       fColorsArePremul;
    bool                       fDither;

private:
    using INHERITED = Context;

    void addMirrorIntervals(const SkGradientShaderBase&,
                            const Sk4f& componentScale, bool reverse);
};

#endif // Sk4fGradientBase_DEFINED
