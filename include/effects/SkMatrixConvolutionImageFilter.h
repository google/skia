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

/*! \class SkMatrixConvolutionImageFilter
    Matrix convolution image filter.  This filter applies an NxM image
    processing kernel to a given input image.  This can be used to produce
    effects such as sharpening, blurring, edge detection, etc.
 */

class SK_API SkMatrixConvolutionImageFilter : public SkImageFilter {
public:
    /*! \enum TileMode */
    enum TileMode {
      kClamp_TileMode,         /*!< Clamp to the image's edge pixels. */
      kRepeat_TileMode,        /*!< Wrap around to the image's opposite edge. */
      kClampToBlack_TileMode,  /*!< Fill with transparent black. */
    };

    virtual ~SkMatrixConvolutionImageFilter();

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
    static SkMatrixConvolutionImageFilter* Create(const SkISize& kernelSize,
                                                  const SkScalar* kernel,
                                                  SkScalar gain,
                                                  SkScalar bias,
                                                  const SkIPoint& kernelOffset,
                                                  TileMode tileMode,
                                                  bool convolveAlpha,
                                                  SkImageFilter* input = NULL,
                                                  const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkMatrixConvolutionImageFilter, (kernelSize, kernel, gain, bias,
                                                           kernelOffset, tileMode, convolveAlpha,
                                                           input, cropRect));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMatrixConvolutionImageFilter)

protected:
    SkMatrixConvolutionImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect,
                             GrTexture*,
                             const SkMatrix& ctm,
                             const SkIRect& bounds) const SK_OVERRIDE;
#endif

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkMatrixConvolutionImageFilter(const SkISize& kernelSize,
                                   const SkScalar* kernel,
                                   SkScalar gain,
                                   SkScalar bias,
                                   const SkIPoint& kernelOffset,
                                   TileMode tileMode,
                                   bool convolveAlpha,
                                   SkImageFilter* input = NULL,
                                   const CropRect* cropRect = NULL);

private:
    SkISize   fKernelSize;
    SkScalar* fKernel;
    SkScalar  fGain;
    SkScalar  fBias;
    SkIPoint  fKernelOffset;
    TileMode  fTileMode;
    bool      fConvolveAlpha;
    typedef SkImageFilter INHERITED;

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
};

#endif
