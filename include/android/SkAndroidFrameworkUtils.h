/*
 * Copyright 2017 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidFrameworkUtils_DEFINED
#define SkAndroidFrameworkUtils_DEFINED

#ifdef SK_BUILD_FOR_ANDROID

class SkCanvas;

/** \class SkAndroidFrameworkUtils

    SkAndroidFrameworkUtils expose private APIs used only by Android framework.
*/
class SkAndroidFrameworkUtils {
public:
    /*
     * @param data     Refs the data while this object exists, unrefs on destruction
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */

    /**
     *  clipWithStencil draws the current clip into a stencil buffer with reference value and mask
     *  set to 0x1. This function works only on a GPU canvas.
     *  @param canvas  A GPU canvas that has a non-empty clip
     */
    static void clipWithStencil(SkCanvas* canvas);
};

#endif // SK_BUILD_FOR_ANDROID

#endif // SkAndroidFrameworkUtils_DEFINED
