/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImage_opts_DEFINED
#define SkBlurImage_opts_DEFINED

#include "SkColorPriv.h"

typedef void (*SkBoxBlurProc)(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
                              int leftOffset, int rightOffset, int width, int height);

bool SkBoxBlurGetPlatformProcs(SkBoxBlurProc* boxBlurX,
                               SkBoxBlurProc* boxBlurY,
                               SkBoxBlurProc* boxBlurXY,
                               SkBoxBlurProc* boxBlurYX);
#endif
