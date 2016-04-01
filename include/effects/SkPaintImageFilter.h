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
    static sk_sp<SkImageFilter> Make(const SkPaint& paint, const CropRect* cropRect = nullptr) {
        return sk_sp<SkImageFilter>(new SkPaintImageFilter(paint, cropRect));
    }

    bool affectsTransparentBlack() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPaintImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(const SkPaint& paint, const CropRect* rect = nullptr) {
        return Make(paint, rect).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkPaintImageFilter(const SkPaint& paint, const CropRect* rect);

    SkPaint fPaint;

    typedef SkImageFilter INHERITED;
};

#endif
