/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixImageFilter_DEFINED
#define SkMatrixImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkScalar.h"
#include "SkSize.h"
#include "SkPoint.h"
#include "SkPaint.h"

/*! \class SkMatrixImageFilter
    Matrix transformation image filter.  This filter draws its source
    input transformed by the given matrix.
 */

class SK_API SkMatrixImageFilter : public SkImageFilter {
public:
    /** Construct a 2D transformation image filter.
     *  @param transform    The matrix to apply when drawing the src bitmap
     *  @param filterLevel  The quality of filtering to apply when scaling.
     *  @param input        The input image filter.  If NULL, the src bitmap
     *                      passed to filterImage() is used instead.
     */

    static SkMatrixImageFilter* Create(const SkMatrix& transform,
                                       SkPaint::FilterLevel,
                                       SkImageFilter* input = NULL,
                                       uint32_t uniqueID = 0);
    virtual ~SkMatrixImageFilter();

    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMatrixImageFilter)

protected:
    SkMatrixImageFilter(const SkMatrix& transform,
                        SkPaint::FilterLevel,
                        SkImageFilter* input,
                        uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkMatrixImageFilter(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

private:
    SkMatrix              fTransform;
    SkPaint::FilterLevel  fFilterLevel;
    typedef SkImageFilter INHERITED;
};

#endif
