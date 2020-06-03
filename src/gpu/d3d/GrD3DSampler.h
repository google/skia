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
    static sk_sp<GrD3DSampler> Make(GrD3DGpu* gpu, const GrSamplerState&);

    ~GrD3DSampler();

    D3D12_CPU_DESCRIPTOR_HANDLE sampler() const { return fSampler; }
    const D3D12_CPU_DESCRIPTOR_HANDLE* samplerPtr() const { return &fSampler; }

    typedef uint32_t Key;

    // Helpers for hashing GrVkSampler
    static Key GenerateKey(const GrSamplerState&);

private:
    GrD3DSampler(GrD3DGpu* gpu, D3D12_CPU_DESCRIPTOR_HANDLE samplerDescriptor)
        : fGpu(gpu)
        , fSampler(samplerDescriptor) {}

    GrD3DGpu* fGpu;
    D3D12_CPU_DESCRIPTOR_HANDLE fSampler;
};

#endif
