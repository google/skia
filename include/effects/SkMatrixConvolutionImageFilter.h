/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixConvolutionImageFilter_DEFINED
#define SkMatrixConvolutionImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkScalar.h"
#include "SkSize.h"
#include "SkPoint.h"

class SkBitmap;

/*! \class SkMatrixConvolutionImageFilter
    Matrix convolution image filter.  This filter applies an NxM image
    processing kernel to a given input image.  This can be used to produce
    effects such as sharpening, blurring, edge detection, etc.
 */

class SK_API SkMatrixConvolutionImageFilter : public SkImageFilter {
public:
    /*! \enum TileMode */
    enum TileMode {
      kClamp_TileMode = 0,         /*!< Clamp to the image's edge pixels. */
      kRepeat_TileMode,        /*!< Wrap around to the image's opposite edge. */
      kClampToBlack_TileMode,  /*!< Fill with transparent black. */
      kMax_TileMode = kClampToBlack_TileMode
    };

    ~SkMatrixConvolutionImageFilter() override;

    /** Construct a matrix convolution image filter.
        @param kernelSize     The kernel size in pixels, in each dimension (N by M).
        @param kernel         The image processing kernel.  Must contain N * M
                              elements, in row order.
        @param gain           A scale factor applied to each pixel after
                              convolution.  This can be used to normalize the
                              kernel, if it does not sum to 1.
        @param bias           A bias factor added to each pixel after convolution.
        @param kernelOffset   An offset applied to each pixel coordinate before
                              convolution.  This can be used to center the kernel
                              over the image (e.g., a 3x3 kernel should have an
                              offset of {1, 1}).
        @param tileMode       How accesses outside the image are treated.  (@see
                              TileMode).
        @param convolveAlpha  If true, all channels are convolved.  If false,
                              only the RGB channels are convolved, and
                              alpha is copied from the source image.
        @param input          The input image filter.  If NULL, the src bitmap
                              passed to filterImage() is used instead.
        @param cropRect       The rectangle to which the output processing will be limited.
    */
    static sk_sp<SkImageFilter> Make(const SkISize& kernelSize,
                                     const SkScalar* kernel,
                                     SkScalar gain,
                                     SkScalar bias,
                                     const SkIPoint& kernelOffset,
                                     TileMode tileMode,
                                     bool convolveAlpha,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMatrixConvolutionImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(const SkISize& kernelSize,
                                 const SkScalar* kernel,
                                 SkScalar gain,
                                 SkScalar bias,
                                 const SkIPoint& kernelOffset,
                                 TileMode tileMode,
                                 bool convolveAlpha,
                                 SkImageFilter* input = NULL,
                                 const CropRect* cropRect = NULL) {
        return Make(kernelSize, kernel, gain, bias, kernelOffset, tileMode, convolveAlpha,
                    sk_ref_sp<SkImageFilter>(input), cropRect).release();
    }
#endif

protected:
    SkMatrixConvolutionImageFilter(const SkISize& kernelSize,
                                   const SkScalar* kernel,
                                   SkScalar gain,
                                   SkScalar bias,
                                   const SkIPoint& kernelOffset,
                                   TileMode tileMode,
                                   bool convolveAlpha,
                                   sk_sp<SkImageFilter> input,
                                   const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix&, MapDirection) const override;
    bool affectsTransparentBlack() const override;

private:
    SkISize   fKernelSize;
    SkScalar* fKernel;
    SkScalar  fGain;
    SkScalar  fBias;
    SkIPoint  fKernelOffset;
    TileMode  fTileMode;
    bool      fConvolveAlpha;

    template <class PixelFetcher, bool convolveAlpha>
    void filterPixels(const SkBitmap& src,
                      SkBitmap* result,
                      const SkIRect& rect,
                      const SkIRect& bounds) const;
    template <class PixelFetcher>
    void filterPixels(const SkBitmap& src,
                      SkBitmap* result,
                      const SkIRect& rect,
                      const SkIRect& bounds) const;
    void filterInteriorPixels(const SkBitmap& src,
                              SkBitmap* result,
                              const SkIRect& rect,
                              const SkIRect& bounds) const;
    void filterBorderPixels(const SkBitmap& src,
                            SkBitmap* result,
                            const SkIRect& rect,
                            const SkIRect& bounds) const;

    typedef SkImageFilter INHERITED;
};

#endif
