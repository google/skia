/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImage_opts_SSE4_DEFINED
#define SkBlurImage_opts_SSE4_DEFINED

#include "SkBlurImage_opts.h"

bool SkBoxBlurGetPlatformProcs_SSE4(SkBoxBlurProc* boxBlurX,
                                    SkBoxBlurProc* boxBlurY,
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX);

#endif
