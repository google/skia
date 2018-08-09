/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlSampler_DEFINED
#define GrMtlSampler_DEFINED

#import <metal/metal.h>

class GrSamplerState;
class GrMtlGpu;

// This class only acts as a wrapper for a MTLSamplerState object for now, but will be more useful
// once we start caching sampler states.
class GrMtlSampler {
public:
    static GrMtlSampler* Create(const GrMtlGpu* gpu, const GrSamplerState&, uint32_t maxMipLevel);

    id<MTLSamplerState> mtlSamplerState() const { return fMtlSamplerState; }

private:
    GrMtlSampler(id<MTLSamplerState> mtlSamplerState) : fMtlSamplerState(mtlSamplerState) {}

    id<MTLSamplerState> fMtlSamplerState;
};

#endif
