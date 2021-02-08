/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAlphaThresholdImageFilter_DEFINED
#define SkAlphaThresholdImageFilter_DEFINED

#include "include/core/SkRegion.h"
#include "src/core/SkImageFilter_Base.h"

class SkAlphaThresholdImageFilter final : public SkImageFilter_Base {
public:
    SkAlphaThresholdImageFilter(const SkRegion& region, SkScalar innerThreshold,
                                SkScalar outerThreshold, sk_sp<SkImageFilter> input,
                                const SkRect* cropRect = nullptr)
            : INHERITED(&input, 1, cropRect)
            , fRegion(region)
            , fInnerThreshold(innerThreshold)
            , fOuterThreshold(outerThreshold) {}

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    SK_FLATTENABLE_HOOKS(SkAlphaThresholdImageFilter)

    static void RegisterFlattenables();

#if SK_SUPPORT_GPU
    GrSurfaceProxyView createMaskTexture(GrRecordingContext*,
                                        const SkMatrix&,
                                        const SkIRect& bounds) const;
#endif

    SkRegion fRegion;
    SkScalar fInnerThreshold;
    SkScalar fOuterThreshold;

    using INHERITED = SkImageFilter_Base;
};

#endif
