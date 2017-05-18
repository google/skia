/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkSampler_DEFINED
#define GrVkSampler_DEFINED

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

class GrSamplerParams;
class GrVkGpu;


class GrVkSampler : public GrVkResource {
public:
    static GrVkSampler* Create(const GrVkGpu* gpu, const GrSamplerParams&, uint32_t mipLevels);

    VkSampler sampler() const { return fSampler; }

    // Helpers for hashing GrVkSampler
    static uint16_t GenerateKey(const GrSamplerParams&, uint32_t mipLevels);

    static const uint16_t& GetKey(const GrVkSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const uint16_t& key) { return key; }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSampler: %d (%d refs)\n", fSampler, this->getRefCnt());
    }
#endif

private:
    GrVkSampler(VkSampler sampler, uint16_t key) : INHERITED(), fSampler(sampler), fKey(key) {}

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkSampler  fSampler;
    uint16_t   fKey;

    typedef GrVkResource INHERITED;
};

#endif
