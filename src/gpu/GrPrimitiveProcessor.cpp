/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPrimitiveProcessor.h"

#include "src/gpu/GrFragmentProcessor.h"

/**
 * We specialize the vertex or fragment coord transform code for these matrix types, and where
 * the transform code is applied.
 */
enum SampleFlag {
    kExplicitlySampled_Flag      = 0b0001,  // GrFP::isSampledWithExplicitCoords()

    kNone_SampleMatrix_Flag      = 0b0010, // GrFP::sampleUsage()::hasMatrix() == false
    kUniform_SampleMatrix_Flag   = 0b0100, // GrFP::sampleUsage()::hasUniformMatrix()
    kVariable_SampleMatrix_Flag  = 0b0110, // GrFP::sampleUsage()::hasVariableMatrix()

    // Currently, sample(matrix) only specializes on no-perspective or general.
    // FIXME add new flags as more matrix types are supported.
    kPersp_Matrix_Flag           = 0b1000, // GrFP::sampleUsage()::fHasPerspective
};

GrPrimitiveProcessor::GrPrimitiveProcessor(ClassID classID) : GrProcessor(classID) {}

const GrPrimitiveProcessor::TextureSampler& GrPrimitiveProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t GrPrimitiveProcessor::ComputeCoordTransformsKey(const GrFragmentProcessor& fp) {
    // This is highly coupled with the code in GrGLSLGeometryProcessor::collectTransforms().

    uint32_t key = 0;
    if (fp.isSampledWithExplicitCoords()) {
        key |= kExplicitlySampled_Flag;
    }

    switch(fp.sampleUsage().fKind) {
        case SkSL::SampleUsage::Kind::kNone:
            key |= kNone_SampleMatrix_Flag;
            break;
        case SkSL::SampleUsage::Kind::kUniform:
            key |= kUniform_SampleMatrix_Flag;
            break;
        case SkSL::SampleUsage::Kind::kVariable:
            key |= kVariable_SampleMatrix_Flag;
            break;
    }
    if (fp.sampleUsage().fHasPerspective) {
        key |= kPersp_Matrix_Flag;
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

GrPrimitiveProcessor::TextureSampler::TextureSampler(GrSamplerState samplerState,
                                                     const GrBackendFormat& backendFormat,
                                                     const GrSwizzle& swizzle) {
    this->reset(samplerState, backendFormat, swizzle);
}

void GrPrimitiveProcessor::TextureSampler::reset(GrSamplerState samplerState,
                                                 const GrBackendFormat& backendFormat,
                                                 const GrSwizzle& swizzle) {
    fSamplerState = samplerState;
    fSamplerState.setFilterMode(clamp_filter(backendFormat.textureType(), samplerState.filter()));
    fBackendFormat = backendFormat;
    fSwizzle = swizzle;
    fIsInitialized = true;
}
