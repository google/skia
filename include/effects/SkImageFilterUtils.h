/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterUtils_DEFINED
#define SkImageFilterUtils_DEFINED

#if SK_SUPPORT_GPU

#include "SkImageFilter.h"

class SkBitmap;
class GrTexture;
class SkImageFilter;

class SK_API SkImageFilterUtils {
public:
    /**
     * Wrap the given texture in a texture-backed SkBitmap.
     */
    static bool WrapTexture(GrTexture* texture, int width, int height, SkBitmap* result);

    /**
     * Recursively evaluate the given filter on the GPU.  If filter is NULL,
     * this function returns src.  If the filter has no GPU implementation, it
     * will be processed in software and uploaded to the GPU.
     */
    static bool GetInputResultGPU(SkImageFilter* filter, SkImageFilter::Proxy* proxy, const SkBitmap& src, SkBitmap* result);
};

#endif

#endif
