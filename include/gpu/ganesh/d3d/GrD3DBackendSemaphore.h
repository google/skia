/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#ifndef GrD3DBackendSemaphore_DEFINED
#define GrD3DBackendSemaphore_DEFINED

#include "include/gpu/ganesh/GrBackendSemaphore.h"
#include "include/private/base/SkAPI.h"
#include "include/private/gpu/ganesh/GrD3DTypesMinimal.h"

namespace GrBackendSemaphores {
SK_API GrBackendSemaphore MakeD3D(const GrD3DFenceInfo& info);
SK_API GrD3DFenceInfo GetD3DFenceInfo(const GrBackendSemaphore&);
}  // namespace GrBackendSemaphores

#endif
