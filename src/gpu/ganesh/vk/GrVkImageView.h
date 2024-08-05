/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrVkImageView_DEFINED
#define GrVkImageView_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include <cinttypes>
#include <cstdint>

class GrVkGpu;
class GrVkSamplerYcbcrConversion;

namespace skgpu {
struct VulkanYcbcrConversionInfo;
}

class GrVkImageView : public GrVkManagedResource {
public:
    enum Type {
        kColor_Type,
        kStencil_Type
    };

    static sk_sp<const GrVkImageView> Make(GrVkGpu* gpu,
                                           VkImage image,
                                           VkFormat format,
                                           Type viewType,
                                           uint32_t miplevels,
                                           const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo);

    VkImageView imageView() const { return fImageView; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkImageView: %" PRIdPTR " (%d refs)\n",
                 (intptr_t)fImageView, this->getRefCnt());
    }
#endif

private:
    GrVkImageView(const GrVkGpu* gpu, VkImageView imageView,
                  GrVkSamplerYcbcrConversion* ycbcrConversion)
            : INHERITED(gpu), fImageView(imageView), fYcbcrConversion(ycbcrConversion) {}

    void freeGPUData() const override;

    VkImageView  fImageView;
    GrVkSamplerYcbcrConversion* fYcbcrConversion;

    using INHERITED = GrVkManagedResource;
};

#endif
