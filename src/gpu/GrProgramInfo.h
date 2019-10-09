/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramInfo_DEFINED
#define GrProgramInfo_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrPrimitiveProcessor.h"

class GrProgramInfo {
public:
    // TODO: it seems like this object should also get the number of copies in
    // dynamicStateArrays. If that were true a portion of checkAllInstantiated could be moved
    // to validate.
    GrProgramInfo(int numSamples,
                  GrSurfaceOrigin origin,
                  const GrPipeline& pipeline,
                  const GrPrimitiveProcessor& primProc,
                  const GrPipeline::FixedDynamicState* fixedDynamicState,
                  const GrPipeline::DynamicStateArrays* dynamicStateArrays)
            : fNumSamples(numSamples)
            , fOrigin(origin)
            , fPipeline(pipeline)
            , fPrimProc(primProc)
            , fFixedDynamicState(fixedDynamicState)
            , fDynamicStateArrays(dynamicStateArrays) {
        SkDEBUGCODE(this->validate();)
    }

    int numSamples() const { return fNumSamples;  }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return fPipeline; }
    const GrPrimitiveProcessor& primProc() const { return fPrimProc; }
    const GrPipeline::FixedDynamicState* fixedDynamicState() const { return fFixedDynamicState; }
    const GrPipeline::DynamicStateArrays* dynamicStateArrays() const { return fDynamicStateArrays; }

    // TODO: can this be removed?
    const GrTextureProxy* const* primProcProxies() const {
        const GrTextureProxy* const* primProcProxies = nullptr;
        if (fDynamicStateArrays && fDynamicStateArrays->fPrimitiveProcessorTextures) {
            primProcProxies = fDynamicStateArrays->fPrimitiveProcessorTextures;
        } else if (fFixedDynamicState) {
            primProcProxies = fFixedDynamicState->fPrimitiveProcessorTextures;
        }

        SkASSERT(SkToBool(primProcProxies) == SkToBool(fPrimProc.numTextureSamplers()));
        return primProcProxies;
    }

    bool hasDynamicScissors() const {
        return fPipeline.isScissorEnabled() &&
               fDynamicStateArrays && fDynamicStateArrays->fScissorRects;
    }

    const SkIRect& dynamicScissor(int i) const {
        SkASSERT(this->hasDynamicScissors());

        return fDynamicStateArrays->fScissorRects[i];
    }

    bool hasFixedScissor() const { return fPipeline.isScissorEnabled() && fFixedDynamicState; }

    const SkIRect& fixedScissor() const {
        SkASSERT(this->hasFixedScissor());

        return fFixedDynamicState->fScissorRect;
    }

    bool hasDynamicPrimProcTextures() const {
        return fDynamicStateArrays && fDynamicStateArrays->fPrimitiveProcessorTextures;
    }

    const GrTextureProxy* const* dynamicPrimProcTextures(int i) const {
        SkASSERT(this->hasDynamicPrimProcTextures());

        return fDynamicStateArrays->fPrimitiveProcessorTextures +
                                                                i * fPrimProc.numTextureSamplers();
    }

    bool hasFixedPrimProcTextures() const {
        return fFixedDynamicState && fFixedDynamicState->fPrimitiveProcessorTextures;
    }

    const GrTextureProxy* const* fixedPrimProcTextures() const {
        SkASSERT(this->hasFixedPrimProcTextures());

        return fFixedDynamicState->fPrimitiveProcessorTextures;
    }

#ifdef SK_DEBUG
    void validate() const;
    void checkAllInstantiated(int meshCount) const;
    void checkMSAAAndMIPSAreResolved(int meshCount) const;

    bool isNVPR() const {
        return fPrimProc.isPathRendering() && !fPrimProc.willUseGeoShader() &&
               !fPrimProc.numVertexAttributes() && !fPrimProc.numInstanceAttributes();
    }

    // TODO: calculate this once in the ctor and use more widely
    GrProcessor::CustomFeatures requestedFeatures() const {
        GrProcessor::CustomFeatures requestedFeatures = fPrimProc.requestedFeatures();
        for (int i = 0; i < fPipeline.numFragmentProcessors(); ++i) {
            requestedFeatures |= fPipeline.getFragmentProcessor(i).requestedFeatures();
        }
        requestedFeatures |= fPipeline.getXferProcessor().requestedFeatures();
        return requestedFeatures;
    }
#endif

private:
    const int                             fNumSamples;
    const GrSurfaceOrigin                 fOrigin;
    const GrPipeline&                     fPipeline;
    const GrPrimitiveProcessor&           fPrimProc;
    const GrPipeline::FixedDynamicState*  fFixedDynamicState;
    const GrPipeline::DynamicStateArrays* fDynamicStateArrays;
};

#endif
