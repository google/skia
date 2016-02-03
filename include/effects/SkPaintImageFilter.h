/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintImageFilter_DEFINED
#define SkPaintImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkPaint.h"

class SK_API SkPaintImageFilter : public SkImageFilter {
public:
    /** Create a new image filter which fills the given rectangle using the
     *  given paint. If no rectangle is specified, an output is produced with
     *  the same bounds as the input primitive (even though the input
     *  primitive's pixels are not used for processing).
     *  @param paint  Paint to use when filling the rect.
     *  @param rect   Rectangle of output pixels. If NULL or a given crop edge is
     *                not specified, the source primitive's bounds are used
     *                instead.
     */
    static SkImageFilter* Create(const SkPaint& paint, const CropRect* rect = NULL);

    bool canComputeFastBounds() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPaintImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&, SkBitmap* result,
                       SkIPoint* loc) const override;

private:
    SkPaintImageFilter(const SkPaint& paint, const CropRect* rect);

    SkPaint fPaint;

    typedef SkImageFilter INHERITED;
};

#endif
