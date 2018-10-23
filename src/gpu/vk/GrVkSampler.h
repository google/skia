/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkSampler_DEFINED
#define GrVkSampler_DEFINED

#include "GrVkResource.h"

#include "SkOpts.h"
#include "vk/GrVkDefines.h"
#include "vk/GrVkTypes.h"

class GrSamplerState;
class GrVkGpu;


class GrVkSampler : public GrVkResource {
public:
    static GrVkSampler* Create(const GrVkGpu* gpu, const GrSamplerState&,
                               uint32_t maxMipLevel, const GrVkYcbcrConversionInfo& ycbcrInfo);

    VkSampler sampler() const { return fSampler; }

    struct Key {
        Key(uint16_t samplerKey, const GrVkYcbcrConversionInfo& ycbcrInfo) {
            // We must memset here since the GrVkYcbcrConversion has a 64 bit value which may force
            // alignment padded to occur in the middle of the Key struct.
            memset(this, 0, sizeof(Key));
            fSamplerKey = samplerKey;
            fYcbcrInfo = ycbcrInfo;
        }
        uint16_t                fSamplerKey;
        GrVkYcbcrConversionInfo fYcbcrInfo;

        bool operator==(const Key& that) const {
            return this->fSamplerKey == that.fSamplerKey &&
                   this->fYcbcrInfo == that.fYcbcrInfo;
        }
    };

    // Helpers for hashing GrVkSampler
    static Key GenerateKey(const GrSamplerState&, uint32_t maxMipLevel,
                           const GrVkYcbcrConversionInfo& ycbcrInfo);

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
    GrVkSampler(VkSampler sampler, Key key) : INHERITED(), fSampler(sampler), fKey(key) {}

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkSampler  fSampler;

    Key fKey;

    typedef GrVkResource INHERITED;
};

#endif
