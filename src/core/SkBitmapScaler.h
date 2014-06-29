/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapScaler_DEFINED
#define SkBitmapScaler_DEFINED

#include "SkBitmap.h"
#include "SkConvolver.h"

/** \class SkBitmapScaler

    Provides the interface for high quality image resampling.
 */

class SK_API SkBitmapScaler {
public:
    enum ResizeMethod {
        // Quality Methods
        //
        // Those enumeration values express a desired quality/speed tradeoff.
        // They are translated into an algorithm-specific method that depends
        // on the capabilities (CPU, GPU) of the underlying platform.
        // It is possible for all three methods to be mapped to the same
        // algorithm on a given platform.

        // Good quality resizing. Fastest resizing with acceptable visual quality.
        // This is typically intended for use during interactive layouts
        // where slower platforms may want to trade image quality for large
        // increase in resizing performance.
        //
        // For example the resizing implementation may devolve to linear
        // filtering if this enables GPU acceleration to be used.
        //
        // Note that the underlying resizing method may be determined
        // on the fly based on the parameters for a given resize call.
        // For example an implementation using a GPU-based linear filter
        // in the common case may still use a higher-quality software-based
        // filter in cases where using the GPU would actually be slower - due
        // to too much latency - or impossible - due to image format or size
        // constraints.
        RESIZE_GOOD,

        // Medium quality resizing. Close to high quality resizing (better
        // than linear interpolation) with potentially some quality being
        // traded-off for additional speed compared to RESIZE_BEST.
        //
        // This is intended, for example, for generation of large thumbnails
        // (hundreds of pixels in each dimension) from large sources, where
        // a linear filter would produce too many artifacts but where
        // a RESIZE_HIGH might be too costly time-wise.
        RESIZE_BETTER,

        // High quality resizing. The algorithm is picked to favor image quality.
        RESIZE_BEST,

        //
        // Algorithm-specific enumerations
        //

        // Box filter. This is a weighted average of all of the pixels touching
        // the destination pixel. For enlargement, this is nearest neighbor.
        //
        // You probably don't want this, it is here for testing since it is easy to
        // compute. Use RESIZE_LANCZOS3 instead.
        RESIZE_BOX,
        RESIZE_TRIANGLE,
        RESIZE_LANCZOS3,
        RESIZE_HAMMING,
        RESIZE_MITCHELL,

        // enum aliases for first and last methods by algorithm or by quality.
        RESIZE_FIRST_QUALITY_METHOD = RESIZE_GOOD,
        RESIZE_LAST_QUALITY_METHOD = RESIZE_BEST,
        RESIZE_FIRST_ALGORITHM_METHOD = RESIZE_BOX,
        RESIZE_LAST_ALGORITHM_METHOD = RESIZE_MITCHELL,
    };

    static bool Resize(SkBitmap* result,
                       const SkBitmap& source,
                       ResizeMethod method,
                       float dest_width, float dest_height,
                       SkBitmap::Allocator* allocator = NULL);

    static SkBitmap Resize(const SkBitmap& source,
                           ResizeMethod method,
                           float dest_width, float dest_height,
                           SkBitmap::Allocator* allocator = NULL);

     /** Platforms can also optionally overwrite the convolution functions
        if we have SIMD versions of them.
      */

    static void PlatformConvolutionProcs(SkConvolutionProcs*);
};

#endif
