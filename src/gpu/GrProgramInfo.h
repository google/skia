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

class GrMesh;

class GrProgramInfo {
public:
    GrProgramInfo(const GrOpFlushState*, const GrPipeline&, const GrPrimitiveProcessor&,
                  const GrPipeline::FixedDynamicState*, const GrPipeline::DynamicStateArrays*,
                  int numDynamicStateArrays);

    GrProcessor::CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    // Returns the number of samples the rasterizer will compute. Because we support mixed samples,
    // this won't necssarily match the number of samples in the render target.
    // NOTE: If MSAA is disabled, we still return the number of samples found in the stencil and/or
    // color buffer.
    int numRasterSamples() const { return fNumRasterSamples;  }
    bool isMixedSampled() const { return fIsMixedSampled;  }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return fPipeline; }
    const GrPrimitiveProcessor& primProc() const { return fPrimProc; }
    const GrPipeline::FixedDynamicState* fixedDynamicState() const { return fFixedDynamicState; }

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
        SkASSERT(i < fNumDynamicStateArrays);

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
    void checkAllInstantiated() const;
    void checkMSAAAndMIPSAreResolved() const;
    void compatibleWithMeshes(const GrMesh meshes[], int meshCount) const;

    bool isNVPR() const {
        return fPrimProc.isPathRendering() && !fPrimProc.willUseGeoShader() &&
               !fPrimProc.numVertexAttributes() && !fPrimProc.numInstanceAttributes();
    }
#endif

private:
    const int                             fNumRasterSamples;
    const bool                            fIsMixedSampled;
    const GrSurfaceOrigin                 fOrigin;
    const GrPipeline&                     fPipeline;
    const GrPrimitiveProcessor&           fPrimProc;
    const GrPipeline::FixedDynamicState*  fFixedDynamicState;
    const GrPipeline::DynamicStateArrays* fDynamicStateArrays;
    const int                             fNumDynamicStateArrays;
    GrProcessor::CustomFeatures           fRequestedFeatures;
};

#endif
