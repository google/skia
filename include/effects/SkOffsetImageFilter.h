/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetImageFilter_DEFINED
#define SkOffsetImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkPoint.h"

class SK_API SkOffsetImageFilter : public SkImageFilter {
public:
    static SkImageFilter* Create(SkScalar dx, SkScalar dy, SkImageFilter* input = NULL,
                                 const CropRect* cropRect = NULL) {
        if (!SkScalarIsFinite(dx) || !SkScalarIsFinite(dy)) {
            return NULL;
        }
        return new SkOffsetImageFilter(dx, dy, input, cropRect);
    }

    void computeFastBounds(const SkRect& src, SkRect* dst) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOffsetImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&, SkBitmap* result,
                       SkIPoint* loc) const override;
    bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const override;

private:
    SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input, const CropRect*);

    SkVector fOffset;

    typedef SkImageFilter INHERITED;
};

#endif
