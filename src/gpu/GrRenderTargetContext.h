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

#include "src/gpu/v1/SurfaceDrawContext_v1.h"

// Android Framework is relying on this header existing and the old name
// of skgpu::v1::SurfaceDrawContext.

using GrRenderTargetContext = skgpu::v1::SurfaceDrawContext;

#endif

#endif
