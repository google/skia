/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticImageFilter_DEFINED
#define SkArithmeticImageFilter_DEFINED

#include "src/core/SkImageFilter_Base.h"

class SkArithmeticImageFilter final : public SkImageFilter_Base {
public:
    SkArithmeticImageFilter(float k1, float k2, float k3, float k4, bool enforcePMColor,
                            sk_sp<SkImageFilter> inputs[2], const SkRect* cropRect)
            : INHERITED(inputs, 2, cropRect)
            , fK{k1, k2, k3, k4}
            , fEnforcePMColor(enforcePMColor) {}

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

    void flatten(SkWriteBuffer& buffer) const override;

private:
    SK_FLATTENABLE_HOOKS(SkArithmeticImageFilter)

    static void RegisterFlattenables();

    bool affectsTransparentBlack() const override { return !SkScalarNearlyZero(fK[3]); }

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds) const;
#endif

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

    SkV4 fK;
    bool fEnforcePMColor;

    using INHERITED = SkImageFilter_Base;
};


#endif
