/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSamplerYcbcrConverison_DEFINED
#define GrVkSamplerYcbcrConverison_DEFINED

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMacros.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/core/SkChecksum.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include <cinttypes>
#include <cstdint>
#include <optional>

class GrVkGpu;
namespace skgpu {
struct VulkanYcbcrConversionInfo;
}

class GrVkSamplerYcbcrConversion : public GrVkManagedResource {
public:
    static GrVkSamplerYcbcrConversion* Create(GrVkGpu* gpu,
                                              const skgpu::VulkanYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    // If the format does not support
    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
    // sampler's minFilter and magFilter must match the conversion's chromaFilter, which can be
    // found in fRequiredFilter. If not set, minFilter and magFilter can be independently set.
    std::optional<VkFilter> requiredFilter() const { return fRequiredFilter; }

    SK_BEGIN_REQUIRE_DENSE
    struct Key {
        Key() = default;
        Key(VkFormat vkFormat, uint64_t externalFormat, uint32_t conversionKey) {
            fVkFormat = vkFormat;
            fExternalFormat = externalFormat;
            fConversionKey = conversionKey;
        }

        VkFormat fVkFormat = VK_FORMAT_UNDEFINED;
        uint32_t fConversionKey = 0;
        uint64_t fExternalFormat = 0;

        bool operator==(const Key& that) const {
            return this->fVkFormat == that.fVkFormat &&
                   this->fExternalFormat == that.fExternalFormat &&
                   this->fConversionKey == that.fConversionKey;
        }
    };
    SK_END_REQUIRE_DENSE

    // Helpers for hashing GrVkSamplerYcbcrConversion
    static Key GenerateKey(const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo);

    static const Key& GetKey(const GrVkSamplerYcbcrConversion& ycbcrConversion) {
        return ycbcrConversion.fKey;
    }
    static uint32_t Hash(const Key& key) {
        return SkChecksum::Hash32(&key, sizeof(Key));
    }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSamplerYcbcrConversion: %" PRIdPTR " (%d refs)\n", (intptr_t)fYcbcrConversion,
                 this->getRefCnt());
    }
#endif

private:
    GrVkSamplerYcbcrConversion(const GrVkGpu* gpu,
                               VkSamplerYcbcrConversion ycbcrConversion,
                               std::optional<VkFilter> requiredFilter,
                               Key key)
            : INHERITED(gpu)
            , fYcbcrConversion(ycbcrConversion)
            , fRequiredFilter(requiredFilter)
            , fKey(key) {}

    void freeGPUData() const override;

    VkSamplerYcbcrConversion fYcbcrConversion;
    std::optional<VkFilter> fRequiredFilter;
    Key                      fKey;

    using INHERITED = GrVkManagedResource;
};

#endif

