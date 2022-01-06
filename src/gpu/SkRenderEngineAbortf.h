/*
* Copyright 2021 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkRenderEngineAbortf_DEFINED
#define SkRenderEngineAbortf_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_IN_RENDERENGINE
    #include "log/log_main.h"

    #ifdef LOG_TAG
        #undef LOG_TAG
    #endif
    #define LOG_TAG "Skia_in_RenderEngine"

    #define RENDERENGINE_ABORTF(...) LOG_ALWAYS_FATAL(__VA_ARGS__)
#else
    #define RENDERENGINE_ABORTF(...)
#endif

#endif// SkRenderEngineAbortf_DEFINED
