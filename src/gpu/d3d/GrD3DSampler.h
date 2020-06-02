/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DSampler_DEFINED
#define GrD3DSampler_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/core/SkOpts.h"

class GrD3DGpu;
class GrSamplerState;

// A wrapper for a sampler represented as a D3D12_CPU_DESCRIPTOR_HANDLE with caching support.
class GrD3DSampler : public SkRefCnt {
public:
    static GrD3DSampler* Create(GrD3DGpu* gpu, const GrSamplerState&);

    D3D12_CPU_DESCRIPTOR_HANDLE sampler() const { return fSampler; }
    const D3D12_CPU_DESCRIPTOR_HANDLE* samplerPtr() const { return &fSampler; }

    typedef uint32_t Key;

    // Helpers for hashing GrVkSampler
    static Key GenerateKey(const GrSamplerState&);

    static const Key& GetKey(const GrD3DSampler& sampler) { return sampler.fKey; }
    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

private:
    GrD3DSampler(D3D12_CPU_DESCRIPTOR_HANDLE samplerDescriptor, Key key)
        : fSampler(samplerDescriptor)
        , fKey(key) {}

    D3D12_CPU_DESCRIPTOR_HANDLE fSampler;
    Key                         fKey;
};

#endif
