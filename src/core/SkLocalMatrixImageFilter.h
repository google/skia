/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixImageFilter_DEFINED
#define SkLocalMatrixImageFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/core/SkImageFilter_Base.h"

/**
 *  Wraps another imagefilter + matrix, such that using this filter will give the same result
 *  as using the wrapped filter with the matrix applied to its context.
 */
class SkLocalMatrixImageFilter : public SkImageFilter_Base {
public:
    static sk_sp<SkImageFilter> Make(const SkMatrix& localM, sk_sp<SkImageFilter> input);

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

    bool onCanHandleComplexCTM() const override { return true; }

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixImageFilter)

    SkLocalMatrixImageFilter(const SkMatrix& localM, sk_sp<SkImageFilter> input);

    SkMatrix fLocalM;

    typedef SkImageFilter_Base INHERITED;
};

#endif
