/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkSize.h"

class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    void computeFastBounds(const SkRect& src, SkRect* dst) const override;
    bool onFilterBounds(const SkIRect& src, const SkMatrix& ctm, SkIRect* dst) const override;

    /**
     * All morphology procs have the same signature: src is the source buffer, dst the
     * destination buffer, radius is the morphology radius, width and height are the bounds
     * of the destination buffer (in pixels), and srcStride and dstStride are the
     * number of pixels per row in each buffer. All buffers are 8888.
     */

    typedef void (*Proc)(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride);

protected:
    SkMorphologyImageFilter(int radiusX, int radiusY, SkImageFilter* input,
                            const CropRect* cropRect);
    bool filterImageGeneric(Proc procX, Proc procY,
                            Proxy*, const SkBitmap& src, const Context&,
                            SkBitmap* result, SkIPoint* offset) const;
    void flatten(SkWriteBuffer&) const override;
#if SK_SUPPORT_GPU
    bool canFilterImageGPU() const override { return true; }
    bool filterImageGPUGeneric(bool dilate, Proxy* proxy, const SkBitmap& src,
                               const Context& ctm, SkBitmap* result,
                               SkIPoint* offset) const;
#endif

    SkISize    radius() const { return fRadius; }

private:
    SkISize    fRadius;
    typedef SkImageFilter INHERITED;
};

class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    static SkImageFilter* Create(int radiusX, int radiusY, SkImageFilter* input = NULL,
                                 const CropRect* cropRect = NULL) {
        if (radiusX < 0 || radiusY < 0) {
            return NULL;
        }
        return new SkDilateImageFilter(radiusX, radiusY, input, cropRect);
    }

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context&,
                        SkBitmap* result, SkIPoint* offset) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

private:
    SkDilateImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    static SkImageFilter* Create(int radiusX, int radiusY,
                                      SkImageFilter* input = NULL,
                                      const CropRect* cropRect = NULL) {
        if (radiusX < 0 || radiusY < 0) {
            return NULL;
        }
        return new SkErodeImageFilter(radiusX, radiusY, input, cropRect);
    }

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;

#if SK_SUPPORT_GPU
    bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context&,
                        SkBitmap* result, SkIPoint* offset) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

private:
    SkErodeImageFilter(int radiusX, int radiusY, SkImageFilter* input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

#endif
