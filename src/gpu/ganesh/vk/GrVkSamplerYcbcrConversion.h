/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSamplerYcbcrConverison_DEFINED
#define GrVkSamplerYcbcrConverison_DEFINED

#include "src/gpu/ganesh/vk/GrVkManagedResource.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkChecksum.h"

#include <cinttypes>

class GrVkGpu;

class GrVkSamplerYcbcrConversion : public GrVkManagedResource {
public:
    static GrVkSamplerYcbcrConversion* Create(GrVkGpu* gpu, const GrVkYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

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
    static Key GenerateKey(const GrVkYcbcrConversionInfo& ycbcrInfo);

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
    GrVkSamplerYcbcrConversion(const GrVkGpu* gpu, VkSamplerYcbcrConversion ycbcrConversion,
                               Key key)
            : INHERITED(gpu)
            , fYcbcrConversion(ycbcrConversion)
            , fKey(key) {}

    void freeGPUData() const override;

    VkSamplerYcbcrConversion fYcbcrConversion;
    Key                      fKey;

    using INHERITED = GrVkManagedResource;
};

#endif

