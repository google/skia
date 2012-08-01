/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkMorphologyImageFilter(int radiusX, int radiusY);

protected:
    SkMorphologyImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return true; }

    SkISize    radius() const { return fRadius; }

private:
    SkISize    fRadius;
    typedef SkImageFilter INHERITED;
};

class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    SkDilateImageFilter(int radiusX, int radiusY) : INHERITED(radiusX, radiusY) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
    virtual GrTexture* onFilterImageGPU(GrTexture* src, const SkRect& rect) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

protected:
    SkDilateImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    SkErodeImageFilter(int radiusX, int radiusY) : INHERITED(radiusX, radiusY) {}

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
    virtual GrTexture* onFilterImageGPU(GrTexture* src, const SkRect& rect) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

protected:
    SkErodeImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkMorphologyImageFilter INHERITED;
};

#endif

