/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendImageFilter_DEFINED
#define SkBlendImageFilter_DEFINED

#include "include/core/SkBlendMode.h"
#include "src/core/SkImageFilter_Base.h"

class SkBlendImageFilter : public SkImageFilter_Base {
public:
    SkBlendImageFilter(SkBlendMode mode, sk_sp<SkImageFilter> inputs[2], const SkRect* cropRect)
          : INHERITED(inputs, 2, cropRect)
          , fMode(mode) {}

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)

    static void RegisterFlattenables();

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds) const;
#endif

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

    SkBlendMode fMode;

    using INHERITED = SkImageFilter_Base;
};

#endif
