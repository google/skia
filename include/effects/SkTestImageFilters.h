/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkScalar scale, sk_sp<SkImageFilter> input) {
        if (!SkScalarIsFinite(scale)) {
            return nullptr;
        }
        // we don't support scale in this range
        if (scale > SK_Scalar1 || scale <= 0) {
            return nullptr;
        }
        return sk_sp<SkImageFilter>(new SkDownSampleImageFilter(scale, std::move(input)));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkScalar scale, SkImageFilter* input = nullptr) {
        return Make(scale, sk_ref_sp<SkImageFilter>(input)).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkDownSampleImageFilter(SkScalar scale, sk_sp<SkImageFilter> input)
        : INHERITED(&input, 1, nullptr), fScale(scale) {}

    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
