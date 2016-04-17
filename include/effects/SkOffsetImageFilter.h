/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetImageFilter_DEFINED
#define SkOffsetImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkPoint.h"

class SK_API SkOffsetImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

    SkRect computeFastBounds(const SkRect& src) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOffsetImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkScalar dx, SkScalar dy, SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        return Make(dx, dy, sk_ref_sp(input), cropRect).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix&, MapDirection) const override;

private:
    SkOffsetImageFilter(SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input, const CropRect*);

    SkVector fOffset;

    typedef SkImageFilter INHERITED;
};

#endif
