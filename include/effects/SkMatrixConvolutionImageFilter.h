/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixConvolutionImageFilter_DEFINED
#define SkMatrixConvolutionImageFilter_DEFINED

#include "include/core/SkImageFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"

class SkBitmap;
enum class SkTileMode;

/*! \class SkMatrixConvolutionImageFilter
    Matrix convolution image filter.  This filter applies an NxM image
    processing kernel to a given input image.  This can be used to produce
    effects such as sharpening, blurring, edge detection, etc.
 */

class SK_API SkMatrixConvolutionImageFilter {
public:
    /*! \enum TileMode
     * DEPRECATED: Use SkTileMode instead. */
    enum TileMode {
      kClamp_TileMode = 0,         /*!< Clamp to the image's edge pixels. */
      kRepeat_TileMode,        /*!< Wrap around to the image's opposite edge. */
      kClampToBlack_TileMode,  /*!< Fill with transparent black. */
      kLast_TileMode = kClampToBlack_TileMode,

      // TODO: remove kMax - it is non-standard but used by Chromium!
      kMax_TileMode = kClampToBlack_TileMode
    };

    static sk_sp<SkImageFilter> Make(const SkISize& kernelSize,
                                     const SkScalar* kernel,
                                     SkScalar gain,
                                     SkScalar bias,
                                     const SkIPoint& kernelOffset,
                                     TileMode tileMode,
                                     bool convolveAlpha,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

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
                              TileMode). EXPERIMENTAL: kMirror not supported yet.
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
                                     SkTileMode tileMode,
                                     bool convolveAlpha,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkMatrixConvolutionImageFilter() = delete;
};

#endif
