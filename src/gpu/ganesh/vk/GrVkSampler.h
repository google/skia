/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkSampler_DEFINED
#define GrVkSampler_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkChecksum.h"
#include "src/gpu/ganesh/vk/GrVkManagedResource.h"
#include "src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.h"

#include <atomic>
#include <cinttypes>

class GrSamplerState;
class GrVkGpu;

class GrVkSampler : public GrVkManagedResource {
public:
    static GrVkSampler* Create(GrVkGpu* gpu, GrSamplerState, const GrVkYcbcrConversionInfo&);

    VkSampler sampler() const { return fSampler; }
    const VkSampler* samplerPtr() const { return &fSampler; }

    SK_BEGIN_REQUIRE_DENSE
    struct Key {
        Key(uint32_t samplerKey, const GrVkSamplerYcbcrConversion::Key& ycbcrKey) {
            fSamplerKey = samplerKey;
            fYcbcrKey = ycbcrKey;
        }
        GrVkSamplerYcbcrConversion::Key fYcbcrKey;
        uint32_t                        fSamplerKey;
        uint32_t                        fPadding = 0;

        bool operator==(const Key& that) const {
            return this->fSamplerKey == that.fSamplerKey &&
                   this->fYcbcrKey == that.fYcbcrKey;
        }
    };
    SK_END_REQUIRE_DENSE

    // Helpers for hashing GrVkSampler
    static Key GenerateKey(GrSamplerState, const GrVkYcbcrConversionInfo&);

    static const Key& GetKey(const GrVkSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkChecksum::Hash32(&key, sizeof(Key));
    }

    uint32_t uniqueID() const { return fUniqueID; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSampler: %" PRIdPTR " (%d refs)\n", (intptr_t)fSampler, this->getRefCnt());
    }
#endif

private:
    GrVkSampler(const GrVkGpu* gpu, VkSampler sampler,
                GrVkSamplerYcbcrConversion* ycbcrConversion, Key key)
            : INHERITED(gpu)
            , fSampler(sampler)
            , fYcbcrConversion(ycbcrConversion)
            , fKey(key)
            , fUniqueID(GenID()) {}

    void freeGPUData() const override;

    static uint32_t GenID() {
        static std::atomic<uint32_t> nextID{1};
        uint32_t id;
        do {
            id = nextID++;
        } while (id == SK_InvalidUniqueID);
        return id;
    }

    VkSampler                   fSampler;
    GrVkSamplerYcbcrConversion* fYcbcrConversion;
    Key                         fKey;
    uint32_t                    fUniqueID;

    using INHERITED = GrVkManagedResource;
};

#endif
