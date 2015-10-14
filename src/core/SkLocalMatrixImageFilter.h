/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixImageFilter_DEFINED
#define SkLocalMatrixImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkReadBuffer.h"
#include "SkString.h"

/**
 *  Wraps another imagefilter + matrix, such that using this filter will give the same result
 *  as using the wrapped filter with the matrix applied to its context.
 */
class SkLocalMatrixImageFilter : public SkImageFilter {
public:
    static SkImageFilter* Create(const SkMatrix& localM, SkImageFilter* input);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLocalMatrixImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;
    bool onFilterBounds(const SkIRect& src, const SkMatrix&, SkIRect* dst) const override;

private:
    SkLocalMatrixImageFilter(const SkMatrix& localM, SkImageFilter* input);

    SkMatrix                    fLocalM;

    typedef SkImageFilter INHERITED;
};

#endif
