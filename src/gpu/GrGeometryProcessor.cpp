/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGeometryProcessor.h"

#include "src/gpu/GrFragmentProcessor.h"


GrGeometryProcessor::GrGeometryProcessor(ClassID classID) : GrProcessor(classID) {}

const GrGeometryProcessor::TextureSampler& GrGeometryProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t GrGeometryProcessor::ComputeCoordTransformsKey(const GrFragmentProcessor& fp) {
    // This is highly coupled with the code in ProgramImpl::collectTransforms().
    uint32_t key = static_cast<uint32_t>(fp.sampleUsage().kind()) << 1;
    // This needs to be updated if GP starts specializing varyings on additional matrix types.
    if (fp.sampleUsage().hasPerspective()) {
        key |= 0b1;
    }
    return key;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline GrSamplerState::Filter clamp_filter(GrTextureType type,
                                                  GrSamplerState::Filter requestedFilter) {
    if (GrTextureTypeHasRestrictedSampling(type)) {
        return std::min(requestedFilter, GrSamplerState::Filter::kLinear);
    }
    return requestedFilter;
}

GrGeometryProcessor::TextureSampler::TextureSampler(GrSamplerState samplerState,
                                                    const GrBackendFormat& backendFormat,
                                                    const GrSwizzle& swizzle) {
    this->reset(samplerState, backendFormat, swizzle);
}

void GrGeometryProcessor::TextureSampler::reset(GrSamplerState samplerState,
                                                const GrBackendFormat& backendFormat,
                                                const GrSwizzle& swizzle) {
    fSamplerState = samplerState;
    fSamplerState.setFilterMode(clamp_filter(backendFormat.textureType(), samplerState.filter()));
    fBackendFormat = backendFormat;
    fSwizzle = swizzle;
    fIsInitialized = true;
}
