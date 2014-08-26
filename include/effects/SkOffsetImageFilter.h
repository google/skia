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
    typedef SkImageFilter INHERITED;

public:
    static SkOffsetImageFilter* Create(SkScalar dx, SkScalar dy, SkImageFilter* input = NULL,
                                       const CropRect* cropRect = NULL,
                                       uint32_t uniqueID = 0) {
        if (!SkScalarIsFinite(dx) || !SkScalarIsFinite(dy)) {
            return NULL;
        }
        return SkNEW_ARGS(SkOffsetImageFilter, (dx, dy, input, cropRect, uniqueID));
    }
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) const SK_OVERRIDE;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOffsetImageFilter)

protected:
    SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input, const CropRect* cropRect, uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkOffsetImageFilter(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const SK_OVERRIDE;

private:
    SkVector fOffset;
};

#endif
