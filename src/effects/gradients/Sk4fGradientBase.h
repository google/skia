/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientBase_DEFINED
#define Sk4fGradientBase_DEFINED

#include "Sk4fGradientPriv.h"
#include "SkColor.h"
#include "SkGradientShaderPriv.h"
#include "SkMatrix.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkShader.h"
#include "SkTArray.h"

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
    SkPM4f   fCb, fCg;
    SkScalar fT0, fT1;
    bool     fZeroRamp;
};

class Sk4fGradientIntervalBuffer {
public:
    void init(const SkColor colors[], const SkScalar pos[], int count,
              SkShader::TileMode tileMode, bool premulColors, SkScalar alpha, bool reverse);

    const Sk4fGradientInterval* find(SkScalar t) const;
    const Sk4fGradientInterval* findNext(SkScalar t, const Sk4fGradientInterval* prev,
                                         bool increasing) const;

    using BufferType = SkSTArray<8, Sk4fGradientInterval, true>;

    const BufferType* operator->() const { return &fIntervals; }

private:
    BufferType fIntervals;
};

class SkGradientShaderBase::
GradientShaderBase4fContext : public SkShader::Context {
public:
    GradientShaderBase4fContext(const SkGradientShaderBase&,
                                const ContextRec&);

    uint32_t getFlags() const override { return fFlags; }

    void shadeSpan(int x, int y, SkPMColor dst[], int count) override;
    void shadeSpan4f(int x, int y, SkPM4f dst[], int count) override;

    bool isValid() const;

protected:
    virtual void mapTs(int x, int y, SkScalar ts[], int count) const = 0;

    Sk4fGradientIntervalBuffer fIntervals;
    SkMatrix                   fDstToPos;
    SkMatrix::MapXYProc        fDstToPosProc;
    uint8_t                    fDstToPosClass;
    uint8_t                    fFlags;
    bool                       fDither;
    bool                       fColorsArePremul;

private:
    using INHERITED = SkShader::Context;

    void addMirrorIntervals(const SkGradientShaderBase&,
                            const Sk4f& componentScale, bool reverse);

    template<DstType, ApplyPremul, SkShader::TileMode tileMode>
    class TSampler;

    template <DstType dstType, ApplyPremul premul>
    void shadePremulSpan(int x, int y, typename DstTraits<dstType, premul>::Type[],
                         int count) const;

    template <DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
    void shadeSpanInternal(int x, int y, typename DstTraits<dstType, premul>::Type[],
                           int count) const;
};

#endif // Sk4fGradientBase_DEFINED
