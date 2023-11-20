/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#ifndef GrVkBackendSemaphore_DEFINED
#define GrVkBackendSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/base/SkAPI.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

namespace GrBackendSemaphores {
SK_API GrBackendSemaphore MakeVk(VkSemaphore semaphore);
SK_API VkSemaphore GetVkSemaphore(const GrBackendSemaphore&);
}  // namespace GrBackendSemaphores

#endif
