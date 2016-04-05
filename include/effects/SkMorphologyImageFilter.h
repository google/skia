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

///////////////////////////////////////////////////////////////////////////////
class SK_API SkMorphologyImageFilter : public SkImageFilter {
public:
    SkRect computeFastBounds(const SkRect& src) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

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
                                     const CropRect* cropRect = nullptr) {
        if (radiusX < 0 || radiusY < 0) {
            return nullptr;
        }
        return sk_sp<SkImageFilter>(new SkDilateImageFilter(radiusX, radiusY,
                                                            std::move(input),
                                                            cropRect));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDilateImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(int radiusX, int radiusY,
                                 SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        return Make(radiusX, radiusY,
                    sk_ref_sp<SkImageFilter>(input),
                    cropRect).release();
    }
#endif

protected:
    Op op() const override { return kDilate_Op; }

private:
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
                                     const CropRect* cropRect = nullptr) {
        if (radiusX < 0 || radiusY < 0) {
            return nullptr;
        }
        return sk_sp<SkImageFilter>(new SkErodeImageFilter(radiusX, radiusY,
                                                           std::move(input),
                                                           cropRect));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkErodeImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(int radiusX, int radiusY,
                                 SkImageFilter* input = nullptr,
                                 const CropRect* cropRect = nullptr) {
        return Make(radiusX, radiusY,
                    sk_ref_sp<SkImageFilter>(input),
                    cropRect).release();
    }
#endif

protected:
    Op op() const override { return kErode_Op; }

private:
    SkErodeImageFilter(int radiusX, int radiusY,
                       sk_sp<SkImageFilter> input, const CropRect* cropRect)
        : INHERITED(radiusX, radiusY, input, cropRect) {}

    typedef SkMorphologyImageFilter INHERITED;
};

#endif
