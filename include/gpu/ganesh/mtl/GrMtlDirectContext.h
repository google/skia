/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlDirectContext_DEFINED
#define GrMtlDirectContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class GrDirectContext;
struct GrContextOptions;
struct GrMtlBackendContext;

namespace GrDirectContexts {
/**
 * The Vulkan context (VkQueue, VkDevice, VkInstance) must be kept alive until the returned
 * GrDirectContext is destroyed. This also means that any objects created with this
 * GrDirectContext (e.g. SkSurfaces, SkImages, etc.) must also be released as they may hold
 * refs on the GrDirectContext. Once all these objects and the GrDirectContext are released,
 * then it is safe to delete the vulkan objects.
 */
SK_API sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext&, const GrContextOptions&);
SK_API sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext&);
}  // namespace GrDirectContexts

#endif
