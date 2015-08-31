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
        RESIZE_BOX,
        RESIZE_TRIANGLE,
        RESIZE_LANCZOS3,
        RESIZE_HAMMING,
        RESIZE_MITCHELL,

        RESIZE_FirstMethod = RESIZE_BOX,
        RESIZE_LastMethod = RESIZE_MITCHELL,
    };

    static bool Resize(SkBitmap* result, const SkPixmap& src, ResizeMethod method,
                       int dest_width, int dest_height, SkBitmap::Allocator* = nullptr);

     /** Platforms can also optionally overwrite the convolution functions
        if we have SIMD versions of them.
      */

    static void PlatformConvolutionProcs(SkConvolutionProcs*);
};

#endif
