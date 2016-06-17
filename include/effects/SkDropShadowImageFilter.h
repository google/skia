/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDropShadowImageFilter_DEFINED
#define SkDropShadowImageFilter_DEFINED

#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkScalar.h"

class SK_API SkDropShadowImageFilter : public SkImageFilter {
public:
    enum ShadowMode {
        kDrawShadowAndForeground_ShadowMode,
        kDrawShadowOnly_ShadowMode,

        kLast_ShadowMode = kDrawShadowOnly_ShadowMode
    };

    static const int kShadowModeCount = kLast_ShadowMode+1;

    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                     SkColor color, ShadowMode shadowMode,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

    SkRect computeFastBounds(const SkRect&) const override;
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                 SkColor color, ShadowMode shadowMode,
                                 SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        return Make(dx, dy, sigmaX, sigmaY, color, shadowMode,
                    sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

private:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor,
                            ShadowMode shadowMode, sk_sp<SkImageFilter> input,
                            const CropRect* cropRect);

    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    ShadowMode fShadowMode;

    typedef SkImageFilter INHERITED;
};

#endif
