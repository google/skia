/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSampler_DEFINED
#define GrMtlSampler_DEFINED

#import <metal/metal.h>

#include "SkOpts.h"
#include <atomic>

class GrSamplerState;
class GrMtlGpu;

// A wrapper for a MTLSamplerState object with caching support.
class GrMtlSampler {
public:
    static GrMtlSampler* Create(const GrMtlGpu* gpu, const GrSamplerState&, uint32_t maxMipLevel);

    id<MTLSamplerState> mtlSampler() const { return fMtlSamplerState; }

    typedef uint32_t Key;

    // Helpers for hashing GrMtlSampler
    static Key GenerateKey(const GrSamplerState&, uint32_t maxMipLevel);

    static const Key& GetKey(const GrMtlSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

    uint32_t uniqueID() const { return fUniqueID; }

private:
    GrMtlSampler(id<MTLSamplerState> mtlSamplerState, Key key)
        : fMtlSamplerState(mtlSamplerState)
        , fKey(key)
        , fUniqueID(GenID()) {}

    static uint32_t GenID() {
        static std::atomic<uint32_t> nextID{1};
        uint32_t id;
        do {
            id = nextID++;
        } while (id == SK_InvalidUniqueID);
        return id;
    }

    id<MTLSamplerState> fMtlSamplerState;
    Key                 fKey;
    uint32_t            fUniqueID;
};

#endif
