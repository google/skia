/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)
#define SK_WORK_AROUND_BUSTED_EGLGETPROCADDDRESS_FN
#include "../egl/GrGLMakeNativeInterface_egl.cpp"
#endif//defined(SK_BUILD_FOR_ANDROID)
