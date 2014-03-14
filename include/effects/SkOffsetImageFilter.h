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
                                       const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkOffsetImageFilter, (dx, dy, input, cropRect));
    }
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) const SK_OVERRIDE;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOffsetImageFilter)

protected:
    SkOffsetImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkOffsetImageFilter(SkScalar dx, SkScalar dy, SkImageFilter* input = NULL,
                        const CropRect* cropRect = NULL);

private:
    SkVector fOffset;
};

#endif
