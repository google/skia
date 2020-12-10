/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetContext_DEFINED
#define GrRenderTargetContext_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "src/gpu/GrSurfaceDrawContext.h"

// Android Framework is relying on this header existing and the old name
// of GrSurfaceDrawContext.

using GrRenderTargetContext = GrSurfaceDrawContext;

#endif

#endif
