/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidFrameworkUtils_DEFINED
#define SkAndroidFrameworkUtils_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_ANDROID

class SkCanvas;

/**
 *  SkAndroidFrameworkUtils expose private APIs used only by Android framework.
 */
class SkAndroidFrameworkUtils {
public:

#if SK_SUPPORT_GPU
    /**
     *  clipWithStencil draws the current clip into a stencil buffer with reference value and mask
     *  set to 0x1. This function works only on a GPU canvas.
     *
     *  @param  canvas A GPU canvas that has a non-empty clip.
     *
     *  @return true on success or false if clip is empty or not a GPU canvas.
     */
    static bool clipWithStencil(SkCanvas* canvas);
#endif //SK_SUPPORT_GPU
};

#endif // SK_BUILD_FOR_ANDROID

#endif // SkAndroidFrameworkUtils_DEFINED
