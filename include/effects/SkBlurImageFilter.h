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
    static sk_sp<SkImageFilter> Make(SkScalar sigmaX, SkScalar sigmaY,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

    SkRect computeFastBounds(const SkRect&) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkScalar sigmaX, SkScalar sigmaY, SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        return Make(sigmaX, sigmaY, sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

private:
    SkBlurImageFilter(SkScalar sigmaX,
                      SkScalar sigmaY,
                      sk_sp<SkImageFilter> input,
                      const CropRect* cropRect);

    SkSize   fSigma;
    typedef SkImageFilter INHERITED;
};

#endif
