/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImageFilter_DEFINED
#define SkBlurImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkSize.h"

class SK_API SkBlurImageFilter : public SkImageFilter {
public:
    static SkBlurImageFilter* Create(SkScalar sigmaX,
                                     SkScalar sigmaY,
                                     SkImageFilter* input = NULL,
                                     const CropRect* cropRect = NULL, uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkBlurImageFilter, (sigmaX, sigmaY, input, cropRect, uniqueID));
    }

    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurImageFilter)

protected:
    SkBlurImageFilter(SkScalar sigmaX,
                      SkScalar sigmaY,
                      SkImageFilter* input,
                      const CropRect* cropRect,
                      uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkBlurImageFilter(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

    bool canFilterImageGPU() const SK_OVERRIDE { return true; }
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;

private:
    SkSize   fSigma;
    typedef SkImageFilter INHERITED;
};

#endif
