/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintImageFilter_DEFINED
#define SkPaintImageFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"

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
    static sk_sp<SkImageFilter> Make(const SkPaint& paint, const CropRect* cropRect = nullptr);

    bool affectsTransparentBlack() const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SK_FLATTENABLE_HOOKS(SkPaintImageFilter)

    SkPaintImageFilter(const SkPaint& paint, const CropRect* rect);

    SkPaint fPaint;

    typedef SkImageFilter INHERITED;
};

#endif
