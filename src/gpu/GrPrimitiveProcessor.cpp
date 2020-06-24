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
    kExplicitlySampled_Flag          = 0b0000001,  // GrFP::isSampledWithExplicitCoords()

    kLegacyCoordTransform_Flag       = 0b0000010, // !GrFP::coordTransform(i)::isNoOp()

    kNone_SampleMatrix_Flag          = 0b0000100, // GrFP::sampleMatrix()::isNoOp()
    kConstUniform_SampleMatrix_Flag  = 0b0001000, // GrFP::sampleMatrix()::isConstUniform()
    kVariable_SampleMatrix_Flag      = 0b0001100, // GrFP::sampleMatrix()::isVariable()

    // Legacy coord transforms specialize on identity, S+T, no-perspective, and general matrix types
    // FIXME these (and kLegacyCoordTransform) can be removed once all FPs no longer use them
    kLCT_ScaleTranslate_Matrix_Flag  = 0b0010000, // GrFP::coordTransform(i)::isScaleTranslate()
    kLCT_NoPersp_Matrix_Flag         = 0b0100000, // !GrFP::coordTransform(i)::hasPerspective()
    kLCT_General_Matrix_Flag         = 0b0110000, // any other matrix type

    // Currently, sample(matrix) only specializes on no-perspective or general.
    // FIXME add new flags as more matrix types are supported.
    kPersp_Matrix_Flag               = 0b1000000, // GrFP::sampleMatrix()::fHasPerspective
};

GrPrimitiveProcessor::GrPrimitiveProcessor(ClassID classID) : GrProcessor(classID) {}

const GrPrimitiveProcessor::TextureSampler& GrPrimitiveProcessor::textureSampler(int i) const {
    SkASSERT(i >= 0 && i < this->numTextureSamplers());
    return this->onTextureSampler(i);
}

uint32_t GrPrimitiveProcessor::computeCoordTransformsKey(const GrFragmentProcessor& fp) const {
    // This is highly coupled with the code in GrGLSLGeometryProcessor::emitTransforms().
    // At this point, all effects either don't use legacy coord transforms, or only use 1.
    SkASSERT(fp.numCoordTransforms() <= 1);

    uint32_t key = 0;
    if (fp.isSampledWithExplicitCoords()) {
        key |= kExplicitlySampled_Flag;
    }
    if (fp.numCoordTransforms() > 0) {
        const GrCoordTransform& coordTransform = fp.coordTransform(0);
        if (!coordTransform.isNoOp()) {
            // A true identity matrix shouldn't result in a coord transform; proxy normalization
            // and flipping will eventually present as a scale+translate matrix.
            SkASSERT(!coordTransform.matrix().isIdentity() || coordTransform.normalize() ||
                     coordTransform.reverseY());
            key |= kLegacyCoordTransform_Flag;
            if (coordTransform.matrix().isScaleTranslate()) {
                key |= kLCT_ScaleTranslate_Matrix_Flag;
            } else if (!coordTransform.matrix().hasPerspective()) {
                key |= kLCT_NoPersp_Matrix_Flag;
            } else {
                key |= kLCT_General_Matrix_Flag;
            }
        }
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
