/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkContextThreadSafeProxy_DEFINED
#define GrVkContextThreadSafeProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContextThreadSafeProxy.h"

class GrCaps;
struct GrContextOptions;

namespace skgpu {
enum class Mipmapped : bool;
enum class Protected : bool;
}  // namespace skgpu

class GrVkContextThreadSafeProxy : public GrContextThreadSafeProxy {
public:
    GrVkContextThreadSafeProxy(const GrContextOptions&);

    bool isValidCharacterizationForVulkan(sk_sp<const GrCaps>,
                                          bool isTextureable,
                                          skgpu::Mipmapped isMipmapped,
                                          skgpu::Protected isProtected,
                                          bool vkRTSupportsInputAttachment,
                                          bool forVulkanSecondaryCommandBuffer) override;
};

#endif
