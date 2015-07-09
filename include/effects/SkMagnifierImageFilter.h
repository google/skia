/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMagnifierImageFilter_DEFINED
#define SkMagnifierImageFilter_DEFINED

#include "SkRect.h"
#include "SkImageFilter.h"

class SK_API SkMagnifierImageFilter : public SkImageFilter {
public:
    static SkImageFilter* Create(const SkRect& src, SkScalar inset, SkImageFilter* input = NULL);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMagnifierImageFilter)

protected:
    SkMagnifierImageFilter(const SkRect& srcRect, SkScalar inset, SkImageFilter* input);
    void flatten(SkWriteBuffer&) const override;

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    bool asFragmentProcessor(GrFragmentProcessor**, GrProcessorDataManager*, GrTexture*,
                             const SkMatrix&, const SkIRect& bounds) const override;
#endif

private:
    SkRect fSrcRect;
    SkScalar fInset;
    typedef SkImageFilter INHERITED;
};

#endif
