/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMorphologyImageFilter_DEFINED
#define SkMorphologyImageFilter_DEFINED

#include "SkColor.h"
#include "SkFlattenable.h"
#include "SkImageFilter.h"
#include "SkSize.h"

///////////////////////////////////////////////////////////////////////////////
class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkRect computeFastBounds(const SkRect& src) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

    /**
     * All morphology procs have the same signature: src is the source buffer, dst the
     * destination buffer, radius is the morphology radius, width and height are the bounds
     * of the destination buffer (in pixels), and srcStride and dstStride are the
     * number of pixels per row in each buffer. All buffers are 8888.
     */

    typedef void (*Proc)(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride);

protected:
    enum Op {
        kErode_Op,
        kDilate_Op,
    };

    virtual Op op() const = 0;

    SkMorphologyImageFilter(int radiusX, int radiusY,
                            sk_sp<SkImageFilter> input,
                            const CropRect* cropRect);
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source,
                                        const Context&,
                                        SkIPoint* offset) const override;
    void flatten(SkWriteBuffer&) const override;

    SkISize radius() const { return fRadius; }

private:
    SkISize  fRadius;

    typedef SkImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
class SK_API SkDilateImageFilter : public SkMorphologyImageFilter {
public:
    static sk_sp<SkImageFilter> Make(int radiusX, int radiusY,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

protected:
    Op op() const override { return kDilate_Op; }

private:
    SK_FLATTENABLE_HOOKS(SkDilateImageFilter)

    SkDilateImageFilter(int radiusX, int radiusY,
                        sk_sp<SkImageFilter> input,
                        const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
class SK_API SkErodeImageFilter : public SkMorphologyImageFilter {
public:
    static sk_sp<SkImageFilter> Make(int radiusX, int radiusY,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

protected:
    Op op() const override { return kErode_Op; }

private:
    SK_FLATTENABLE_HOOKS(SkErodeImageFilter)

    SkErodeImageFilter(int radiusX, int radiusY,
                       sk_sp<SkImageFilter> input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

#endif
