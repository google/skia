/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkSize.h"

class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect);

protected:
    SkMorphologyImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return true; }
#endif

    SkISize    radius() const { return fRadius; }

private:
    SkISize    fRadius;
    typedef SkImageFilter INHERITED;
};

class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    SkDilateImageFilter(int radiusX, int radiusY,
                        SkImageFilter* input = NULL,
                        const CropRect* cropRect = NULL)
    : INHERITED(radiusX, radiusY, input, cropRect) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

protected:
    SkDilateImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    SkErodeImageFilter(int radiusX, int radiusY,
                       SkImageFilter* input = NULL,
                       const CropRect* cropRect = NULL)
    : INHERITED(radiusX, radiusY, input, cropRect) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

protected:
    SkErodeImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

#endif
