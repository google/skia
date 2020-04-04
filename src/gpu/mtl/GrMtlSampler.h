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
#include <atomic>

class GrSamplerState;
class GrMtlGpu;

// A wrapper for a MTLSamplerState object with caching support.
class GrMtlSampler : public SkRefCnt {
public:
    static GrMtlSampler* Create(const GrMtlGpu* gpu, const GrSamplerState&);
    ~GrMtlSampler() { fMtlSamplerState = nil; }

    id<MTLSamplerState> mtlSampler() const { return fMtlSamplerState; }

    typedef uint32_t Key;

    // Helpers for hashing GrMtlSampler
    static Key GenerateKey(const GrSamplerState&);

    static const Key& GetKey(const GrMtlSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

private:
    GrMtlSampler(id<MTLSamplerState> mtlSamplerState, Key key)
        : fMtlSamplerState(mtlSamplerState)
        , fKey(key) {}

    id<MTLSamplerState> fMtlSamplerState;
    Key                 fKey;
};

#endif
