/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPrimitiveProcessor.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"

/**
 * We specialize the vertex or fragment coord transform code for these matrix types, and where
 * the transform code is applied.
 */
enum SampleFlag {
    kExplicitlySampled_Flag          = 0b00001,  // GrFP::isSampledWithExplicitCoords()

    kLegacyCoordTransform_Flag       = 0b00010, // !GrFP::coordTransform(i)::isNoOp()

    kNone_SampleMatrix_Flag          = 0b00100, // GrFP::sampleMatrix()::isNoOp()
    kConstUniform_SampleMatrix_Flag  = 0b01000, // GrFP::sampleMatrix()::isConstUniform()
    kVariable_SampleMatrix_Flag      = 0b01100, // GrFP::sampleMatrix()::isVariable()

    // Currently, sample(matrix) only specializes on no-perspective or general.
    // FIXME add new flags as more matrix types are supported.
    kPersp_Matrix_Flag               = 0b10000, // GrFP::sampleMatrix()::fHasPerspective
};

GrPrimitiveProcessor::GrPrimitiveProcessor(ClassID classID) : GrProcessor(classID) {}

const GrPrimitiveProcessor::TextureSampler& GrPrimitiveProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t GrPrimitiveProcessor::computeCoordTransformsKey(const GrFragmentProcessor& fp) const {
    // This is highly coupled with the code in GrGLSLGeometryProcessor::collectTransforms().
    // At this point, all effects do not use really coord transforms; they may implicitly report
    // a noop coord transform but this does not impact the key.
    SkASSERT(fp.numCoordTransforms() == 0 ||
             (fp.numCoordTransforms() == 1 && fp.coordTransform(0).isNoOp()));

    uint32_t key = 0;
    if (fp.isSampledWithExplicitCoords()) {
        key |= kExplicitlySampled_Flag;
    }

    switch(fp.sampleMatrix().fKind) {
        case SkSL::SampleMatrix::Kind::kNone:
            key |= kNone_SampleMatrix_Flag;
            break;
        case SkSL::SampleMatrix::Kind::kConstantOrUniform:
            key |= kConstUniform_SampleMatrix_Flag;
            break;
        case SkSL::SampleMatrix::Kind::kVariable:
            key |= kVariable_SampleMatrix_Flag;
            break;
    }
    if (fp.sampleMatrix().fHasPerspective) {
        key |= kPersp_Matrix_Flag;
    }

    return key;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline GrSamplerState::Filter clamp_filter(GrTextureType type,
                                                  GrSamplerState::Filter requestedFilter) {
    if (GrTextureTypeHasRestrictedSampling(type)) {
        return std::min(requestedFilter, GrSamplerState::Filter::kBilerp);
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
