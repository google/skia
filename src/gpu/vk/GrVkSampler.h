/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSampler_DEFINED
#define GrVkSampler_DEFINED

#include "GrVkVulkan.h"

#include "GrVkResource.h"
#include "GrVkSamplerYcbcrConversion.h"
#include "SkOpts.h"
#include "vk/GrVkTypes.h"

class GrSamplerState;
class GrVkGpu;

class GrVkSampler : public GrVkResource {
public:
    static GrVkSampler* Create(GrVkGpu* gpu, const GrSamplerState&, const GrVkYcbcrConversionInfo&);

    VkSampler sampler() const { return fSampler; }

    struct Key {
        Key(uint16_t samplerKey, const GrVkSamplerYcbcrConversion::Key& ycbcrKey) {
            // We must memset here since the GrVkSamplerYcbcrConversion has a 64 bit value which may
            // force alignment padding to occur in the middle of the Key struct.
            memset(this, 0, sizeof(Key));
            fSamplerKey = samplerKey;
            fYcbcrKey = ycbcrKey;
        }
        uint16_t                        fSamplerKey;
        GrVkSamplerYcbcrConversion::Key fYcbcrKey;

        bool operator==(const Key& that) const {
            return this->fSamplerKey == that.fSamplerKey &&
                   this->fYcbcrKey == that.fYcbcrKey;
        }
    };

    // Helpers for hashing GrVkSampler
    static Key GenerateKey(const GrSamplerState&, const GrVkYcbcrConversionInfo&);

    static const Key& GetKey(const GrVkSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSampler: %d (%d refs)\n", fSampler, this->getRefCnt());
    }
#endif

private:
    GrVkSampler(VkSampler sampler, GrVkSamplerYcbcrConversion* ycbcrConversion, Key key)
            : INHERITED(), fSampler(sampler), fYcbcrConversion(ycbcrConversion), fKey(key) {}

    void freeGPUData(const GrVkGpu* gpu) const override;
    void abandonGPUData() const override;

    VkSampler                   fSampler;
    GrVkSamplerYcbcrConversion* fYcbcrConversion;
    Key                         fKey;

    typedef GrVkResource INHERITED;
};

#endif
