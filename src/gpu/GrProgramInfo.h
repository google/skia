/*
* Copyright 2018 Google LLC
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
    }

    int numSamples() const { return fNumSamples;  }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return fPipeline; }
    const GrPrimitiveProcessor& primProc() const { return fPrimProc; }
    const GrPipeline::FixedDynamicState* fixedDynamicState() const { return fFixedDynamicState; }
    const GrPipeline::DynamicStateArrays* dynamicStateArrays() const { return fDynamicStateArrays; }

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

#if 0
    void validate() const {
        const GrTextureProxy* const* primProcProxies = nullptr;
        if (fDynamicStateArrays && fDynamicStateArrays->fPrimitiveProcessorTextures) {
            primProcProxies = fDynamicStateArrays->fPrimitiveProcessorTextures;
        } else if (fFixedDynamicState) {
            primProcProxies = fFixedDynamicState->fPrimitiveProcessorTextures;
        }

        SkASSERT(SkToBool(primProcProxies) == SkToBool(fPrimProc.numTextureSamplers()));
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
