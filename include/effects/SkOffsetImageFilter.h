/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetImageFilter_DEFINED
#define SkOffsetImageFilter_DEFINED

#include "SkFlattenable.h"
#include "SkImageFilter.h"
#include "SkPoint.h"

class SK_API SkOffsetImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOffsetImageFilter)

    SkOffsetImageFilter(SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input, const CropRect*);

    SkVector fOffset;

    typedef SkImageFilter INHERITED;
};

#endif
