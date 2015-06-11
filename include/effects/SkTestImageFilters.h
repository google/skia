/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    static SkDownSampleImageFilter* Create(SkScalar scale, SkImageFilter* input = NULL) {
        if (!SkScalarIsFinite(scale)) {
            return NULL;
        }
        // we don't support scale in this range
        if (scale > SK_Scalar1 || scale <= 0) {
            return NULL;
        }
        return SkNEW_ARGS(SkDownSampleImageFilter, (scale, input));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

protected:
    SkDownSampleImageFilter(SkScalar scale, SkImageFilter* input)
      : INHERITED(1, &input), fScale(scale) {}
    void flatten(SkWriteBuffer&) const override;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const override;

private:
    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
