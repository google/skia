/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixImageFilter_DEFINED
#define SkMatrixImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkMatrix.h"

/*! \class SkMatrixImageFilter
    Matrix transformation image filter.  This filter draws its source
    input transformed by the given matrix.
 */

class SK_API SkMatrixImageFilter : public SkImageFilter {
public:
    /** Construct a 2D transformation image filter.
     *  @param transform     The matrix to apply when drawing the src bitmap
     *  @param filterQuality The quality of filtering to apply when scaling.
     *  @param input         The input image filter.  If nullptr, the src bitmap
     *                       passed to filterImage() is used instead.
     */

    static sk_sp<SkImageFilter> Make(const SkMatrix& transform,
                                     SkFilterQuality filterQuality,
                                     sk_sp<SkImageFilter> input);

    SkRect computeFastBounds(const SkRect&) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMatrixImageFilter)

protected:
    SkMatrixImageFilter(const SkMatrix& transform,
                        SkFilterQuality,
                        sk_sp<SkImageFilter> input);
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

private:
    SkMatrix              fTransform;
    SkFilterQuality       fFilterQuality;
    typedef SkImageFilter INHERITED;
};

#endif
