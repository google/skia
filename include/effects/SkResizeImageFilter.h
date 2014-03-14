/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkResizeImageFilter_DEFINED
#define SkResizeImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkScalar.h"
#include "SkRect.h"
#include "SkPoint.h"
#include "SkPaint.h"

/*! \class SkResizeImageFilter
    Resampling image filter. This filter draws its source image resampled using the given scale
    values.
 */

class SK_API SkResizeImageFilter : public SkImageFilter {
public:
    virtual ~SkResizeImageFilter();

    /** Construct a (scaling-only) resampling image filter.
     *  @param sx           The x scale parameter to apply when resizing.
     *  @param sy           The y scale parameter to apply when resizing.
     *  @param filterLevel  The quality of filtering to apply when scaling.
     *  @param input        The input image filter.  If NULL, the src bitmap
     *                      passed to filterImage() is used instead.
     */
    static SkResizeImageFilter* Create(SkScalar sx, SkScalar sy, SkPaint::FilterLevel filterLevel,
                                       SkImageFilter* input = NULL) {
        return SkNEW_ARGS(SkResizeImageFilter, (sx, sy, filterLevel, input));
    }

    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkResizeImageFilter)

protected:
    SkResizeImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkResizeImageFilter(SkScalar sx, SkScalar sy, SkPaint::FilterLevel filterLevel,
                        SkImageFilter* input = NULL);

private:
    SkScalar              fSx, fSy;
    SkPaint::FilterLevel  fFilterLevel;
    typedef SkImageFilter INHERITED;
};

#endif
