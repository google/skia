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
    #define RENDERENGINE_ABORTF(...) SK_ABORT(__VA_ARGS__)
#else
    #define RENDERENGINE_ABORTF(...)
#endif

#endif// SkRenderEngineAbortf_DEFINED
