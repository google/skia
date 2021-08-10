/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSampler_DEFINED
#define GrMtlSampler_DEFINED

#import <Metal/Metal.h>

#include "src/core/SkOpts.h"
#include "src/gpu/GrManagedResource.h"
#include <atomic>

class GrSamplerState;
class GrMtlGpu;

// A wrapper for a MTLSamplerState object with caching support.
class GrMtlSampler : public GrManagedResource {
public:
    static GrMtlSampler* Create(const GrMtlGpu* gpu, GrSamplerState);
    ~GrMtlSampler() override { fMtlSamplerState = nil; }

    id<MTLSamplerState> mtlSampler() const { return fMtlSamplerState; }

    typedef uint32_t Key;

    // Helpers for hashing GrMtlSampler
    static Key GenerateKey(GrSamplerState);

    static const Key& GetKey(const GrMtlSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** output a human-readable dump of this resource's information
     */
    void dumpInfo() const override {
        SkDebugf("GrMtlSampler: %p (%ld refs)\n", fMtlSamplerState,
                 CFGetRetainCount((CFTypeRef)fMtlSamplerState));
    }
#endif

    void freeGPUData() const override {
        fMtlSamplerState = nil;
    }

private:
    GrMtlSampler(id<MTLSamplerState> mtlSamplerState, Key key)
        : fMtlSamplerState(mtlSamplerState)
        , fKey(key) {}

    mutable id<MTLSamplerState> fMtlSamplerState;
    Key                 fKey;
};

#endif
