/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixImageFilter_DEFINED
#define SkLocalMatrixImageFilter_DEFINED

#include "SkImageFilter.h"

/**
 *  Wraps another imagefilter + matrix, such that using this filter will give the same result
 *  as using the wrapped filter with the matrix applied to its context.
 */
class SkLocalMatrixImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(const SkMatrix& localM, sk_sp<SkImageFilter> input);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLocalMatrixImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(const SkMatrix& localM, SkImageFilter* input) {
        return Make(localM, sk_sp<SkImageFilter>(SkSafeRef(input))).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

private:
    SkLocalMatrixImageFilter(const SkMatrix& localM, sk_sp<SkImageFilter> input);

    SkMatrix fLocalM;

    typedef SkImageFilter INHERITED;
};

#endif
