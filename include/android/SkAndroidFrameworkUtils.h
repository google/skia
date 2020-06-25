/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidFrameworkUtils_DEFINED
#define SkAndroidFrameworkUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

class SkCanvas;
struct SkIRect;
struct SkRect;
class SkSurface;

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

    static void SafetyNetLog(const char*);

    static sk_sp<SkSurface> getSurfaceFromCanvas(SkCanvas* canvas);

    static int SaveBehind(SkCanvas* canvas, const SkRect* subset);

    // Operating within the canvas' clip stack, this resets the geometry of the clip to be an
    // intersection with the device-space 'rect'. If 'rect' is null, this will use the rect that
    // was last set using androidFramework_setDeviceClipRestriction on the canvas. If that was never
    // set, it will restrict the clip to the canvas' dimensions.
    //
    // TODO: Eventually, make 'rect' non-optional and no longer store the restriction per canvas.
    static void ReplaceClip(SkCanvas* canvas, const SkIRect* rect = nullptr);

    /**
     * Unrolls a chain of nested SkPaintFilterCanvas to return the base wrapped canvas.
     *
     *  @param  canvas A SkPaintFilterCanvas or any other SkCanvas subclass.
     *
     *  @return SkCanvas that was found in the innermost SkPaintFilterCanvas.
     */
    static SkCanvas* getBaseWrappedCanvas(SkCanvas* canvas);
};

#endif // SK_BUILD_FOR_ANDROID_ANDROID

#endif // SkAndroidFrameworkUtils_DEFINED
