/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurImageFilter_DEFINED
#define SkBlurImageFilter_DEFINED

#include "SkSingleInputImageFilter.h"
#include "SkSize.h"

class SK_API SkBlurImageFilter : public SkSingleInputImageFilter {
public:
    SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY, SkImageFilter* input = NULL);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurImageFilter)

protected:
    explicit SkBlurImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;

    bool canFilterImageGPU() const SK_OVERRIDE { return true; }
    virtual GrTexture* onFilterImageGPU(GrTexture* src, const SkRect& rect) SK_OVERRIDE;

private:
    SkSize   fSigma;
    typedef SkSingleInputImageFilter INHERITED;
};

#endif

