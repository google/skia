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
class GrStencilSettings;

class GrProgramInfo {
public:
    GrProgramInfo(int numSamples,
                  int numStencilSamples,
                  const GrBackendFormat& backendFormat,
                  GrSurfaceOrigin origin,
                  const GrPipeline* pipeline,
                  const GrPrimitiveProcessor* primProc,
                  const GrPipeline::FixedDynamicState* fixedDynamicState,
                  const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                  int numDynamicStateArrays,
                  GrPrimitiveType primitiveType)
            : fNumRasterSamples(pipeline->isStencilEnabled() ? numStencilSamples : numSamples)
            , fIsMixedSampled(fNumRasterSamples > numSamples)
            , fBackendFormat(backendFormat)
            , fOrigin(origin)
            , fPipeline(pipeline)
            , fPrimProc(primProc)
            , fFixedDynamicState(fixedDynamicState)
            , fDynamicStateArrays(dynamicStateArrays)
            , fNumDynamicStateArrays(numDynamicStateArrays)
            , fPrimitiveType(primitiveType) {
        SkASSERT(fNumRasterSamples > 0);
        fRequestedFeatures = fPrimProc->requestedFeatures();
        for (int i = 0; i < fPipeline->numFragmentProcessors(); ++i) {
            fRequestedFeatures |= fPipeline->getFragmentProcessor(i).requestedFeatures();
        }
        fRequestedFeatures |= fPipeline->getXferProcessor().requestedFeatures();

        SkDEBUGCODE(this->validate(false);)
        (void) fNumDynamicStateArrays;  // touch this to quiet unused member warnings
    }

    GrProcessor::CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    int numRasterSamples() const { return fNumRasterSamples;  }
    bool isMixedSampled() const { return fIsMixedSampled; }
    // The backend format of the destination render target [proxy]
    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return *fPipeline; }
    const GrPrimitiveProcessor& primProc() const { return *fPrimProc; }
    const GrPipeline::FixedDynamicState* fixedDynamicState() const { return fFixedDynamicState; }

    bool hasDynamicScissors() const {
        return fPipeline->isScissorEnabled() &&
               fDynamicStateArrays && fDynamicStateArrays->fScissorRects;
    }

    const SkIRect& dynamicScissor(int i) const {
        SkASSERT(this->hasDynamicScissors());

        return fDynamicStateArrays->fScissorRects[i];
    }

    bool hasFixedScissor() const { return fPipeline->isScissorEnabled() && fFixedDynamicState; }

    const SkIRect& fixedScissor() const {
        SkASSERT(this->hasFixedScissor());

        return fFixedDynamicState->fScissorRect;
    }

    bool hasDynamicPrimProcTextures() const {
        return fDynamicStateArrays && fDynamicStateArrays->fPrimitiveProcessorTextures;
    }

    const GrSurfaceProxy* const* dynamicPrimProcTextures(int i) const {
        SkASSERT(this->hasDynamicPrimProcTextures());
        SkASSERT(i < fNumDynamicStateArrays);

        return fDynamicStateArrays->fPrimitiveProcessorTextures +
                                                                i * fPrimProc->numTextureSamplers();
    }

    bool hasFixedPrimProcTextures() const {
        return fFixedDynamicState && fFixedDynamicState->fPrimitiveProcessorTextures;
    }

    const GrSurfaceProxy* const* fixedPrimProcTextures() const {
        SkASSERT(this->hasFixedPrimProcTextures());

        return fFixedDynamicState->fPrimitiveProcessorTextures;
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }

    // For Dawn, Metal and Vulkan the number of stencil bits is known a priori so we can
    // create the stencil settings here.
    GrStencilSettings nonGLStencilSettings() const;

    void visitProxies(const GrOp::VisitProxyFunc& fn) const {
        fPipeline->visitProxies(fn);
    }

#ifdef SK_DEBUG
    void validate(bool flushTime) const;
    void checkAllInstantiated() const;
    void checkMSAAAndMIPSAreResolved() const;
    void compatibleWithMeshes(const GrMesh meshes[], int meshCount) const;

    bool isNVPR() const {
        return fPrimProc->isPathRendering() && !fPrimProc->willUseGeoShader() &&
               !fPrimProc->numVertexAttributes() && !fPrimProc->numInstanceAttributes();
    }
#endif

private:
    const int                             fNumRasterSamples;
    const bool                            fIsMixedSampled;
    const GrBackendFormat                 fBackendFormat;
    const GrSurfaceOrigin                 fOrigin;
    const GrPipeline*                     fPipeline;
    const GrPrimitiveProcessor*           fPrimProc;
    const GrPipeline::FixedDynamicState*  fFixedDynamicState;
    const GrPipeline::DynamicStateArrays* fDynamicStateArrays;
    const int                             fNumDynamicStateArrays;
    GrProcessor::CustomFeatures           fRequestedFeatures;
    GrPrimitiveType                       fPrimitiveType;
};

#endif
