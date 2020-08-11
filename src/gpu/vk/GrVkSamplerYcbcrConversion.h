/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSamplerYcbcrConverison_DEFINED
#define GrVkSamplerYcbcrConverison_DEFINED

#include "src/gpu/vk/GrVkManagedResource.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkOpts.h"

class GrVkGpu;

class GrVkSamplerYcbcrConversion : public GrVkManagedResource {
public:
    static GrVkSamplerYcbcrConversion* Create(GrVkGpu* gpu, const GrVkYcbcrConversionInfo&);

    VkSamplerYcbcrConversion ycbcrConversion() const { return fYcbcrConversion; }

    struct Key {
        Key() : fVkFormat(VK_FORMAT_UNDEFINED), fExternalFormat(0), fConversionKey(0) {}
        Key(VkFormat vkFormat, uint64_t externalFormat, uint8_t conversionKey) {
            memset(this, 0, sizeof(Key));
            fVkFormat = vkFormat;
            fExternalFormat = externalFormat;
            fConversionKey = conversionKey;
        }

        VkFormat fVkFormat;
        uint64_t fExternalFormat;
        uint8_t  fConversionKey;

        bool operator==(const Key& that) const {
            return this->fVkFormat == that.fVkFormat &&
                   this->fExternalFormat == that.fExternalFormat &&
                   this->fConversionKey == that.fConversionKey;
        }
    };

    // Helpers for hashing GrVkSamplerYcbcrConversion
    static Key GenerateKey(const GrVkYcbcrConversionInfo& ycbcrInfo);

    static const Key& GetKey(const GrVkSamplerYcbcrConversion& ycbcrConversion) {
        return ycbcrConversion.fKey;
    }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSamplerYcbcrConversion: %d (%d refs)\n", fYcbcrConversion, this->getRefCnt());
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

    typedef GrVkManagedResource INHERITED;
};

#endif

