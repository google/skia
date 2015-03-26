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
                                     const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkBlurImageFilter, (sigmaX, sigmaY, input, cropRect));
    }

    void computeFastBounds(const SkRect&, SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurImageFilter)

protected:
    SkBlurImageFilter(SkScalar sigmaX,
                      SkScalar sigmaY,
                      SkImageFilter* input,
                      const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const override;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const override;

    bool canFilterImageGPU() const override { return true; }
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                SkBitmap* result, SkIPoint* offset) const override;

private:
    SkSize   fSigma;
    typedef SkImageFilter INHERITED;
};

#endif
