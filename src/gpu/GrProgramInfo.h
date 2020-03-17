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

class GrStencilSettings;

class GrProgramInfo {
public:
    GrProgramInfo(int numSamples,
                  int numStencilSamples,
                  const GrBackendFormat& backendFormat,
                  GrSurfaceOrigin origin,
                  const GrPipeline* pipeline,
                  const GrPrimitiveProcessor* primProc,
                  GrPrimitiveType primitiveType,
                  uint8_t tessellationPatchVertexCount = 0)
            : fNumRasterSamples(pipeline->isStencilEnabled() ? numStencilSamples : numSamples)
            , fIsMixedSampled(fNumRasterSamples > numSamples)
            , fBackendFormat(backendFormat)
            , fOrigin(origin)
            , fPipeline(pipeline)
            , fPrimProc(primProc)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount) {
        SkASSERT(fNumRasterSamples > 0);
        SkASSERT((GrPrimitiveType::kPatches == fPrimitiveType) ==
                 (fTessellationPatchVertexCount > 0));
        fRequestedFeatures = fPrimProc->requestedFeatures();
        for (int i = 0; i < fPipeline->numFragmentProcessors(); ++i) {
            fRequestedFeatures |= fPipeline->getFragmentProcessor(i).requestedFeatures();
        }
        fRequestedFeatures |= fPipeline->getXferProcessor().requestedFeatures();

        SkDEBUGCODE(this->validate(false);)
    }

    GrProcessor::CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    int numRasterSamples() const { return fNumRasterSamples;  }
    bool isMixedSampled() const { return fIsMixedSampled; }
    // The backend format of the destination render target [proxy]
    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return *fPipeline; }
    const GrPrimitiveProcessor& primProc() const { return *fPrimProc; }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    uint8_t tessellationPatchVertexCount() const {
        SkASSERT(GrPrimitiveType::kPatches == fPrimitiveType);
        return fTessellationPatchVertexCount;
    }

    uint16_t primitiveTypeKey() const {
        return ((uint16_t)fPrimitiveType << 8) | fTessellationPatchVertexCount;
    }

    // For Dawn, Metal and Vulkan the number of stencil bits is known a priori so we can
    // create the stencil settings here.
    GrStencilSettings nonGLStencilSettings() const;

    // Invokes the visitor function on all FP proxies in the pipeline. The caller is responsible
    // to call the visitor on its own primProc proxies.
    void visitFPProxies(const GrOp::VisitProxyFunc& func) const { fPipeline->visitProxies(func); }

#ifdef SK_DEBUG
    void validate(bool flushTime) const;
    void checkAllInstantiated() const;
    void checkMSAAAndMIPSAreResolved() const;

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
    GrProcessor::CustomFeatures           fRequestedFeatures;
    GrPrimitiveType                       fPrimitiveType;
    uint8_t                               fTessellationPatchVertexCount;  // GrPrimType::kPatches.
};

#endif
