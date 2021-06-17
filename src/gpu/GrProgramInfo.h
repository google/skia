/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramInfo_DEFINED
#define GrProgramInfo_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrPipeline.h"

class GrStencilSettings;

class GrProgramInfo {
public:
    GrProgramInfo(const GrSurfaceProxyView& targetView,
                  const GrPipeline* pipeline,
                  const GrUserStencilSettings* userStencilSettings,
                  const GrGeometryProcessor* geomProc,
                  GrPrimitiveType primitiveType,
                  uint8_t tessellationPatchVertexCount,
                  GrXferBarrierFlags renderPassXferBarriers,
                  GrLoadOp colorLoadOp)
            : fNumSamples(targetView.asRenderTargetProxy()->numSamples())
            , fNeedsStencil(targetView.asRenderTargetProxy()->needsStencil())
            , fBackendFormat(targetView.proxy()->backendFormat())
            , fOrigin(targetView.origin())
            , fTargetSupportsVkResolveLoad(
                      targetView.asRenderTargetProxy()->numSamples() > 1 &&
                      targetView.asTextureProxy() &&
                      targetView.asRenderTargetProxy()->supportsVkInputAttachment())
            , fPipeline(pipeline)
            , fUserStencilSettings(userStencilSettings)
            , fGeomProc(geomProc)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount)
            , fRenderPassXferBarriers(renderPassXferBarriers)
            , fColorLoadOp(colorLoadOp) {
        SkASSERT(this->numSamples() > 0);
        SkASSERT((GrPrimitiveType::kPatches == fPrimitiveType) ==
                 (fTessellationPatchVertexCount > 0));
        fRequestedFeatures = fGeomProc->requestedFeatures();
        for (int i = 0; i < fPipeline->numFragmentProcessors(); ++i) {
            fRequestedFeatures |= fPipeline->getFragmentProcessor(i).requestedFeatures();
        }
        fRequestedFeatures |= fPipeline->getXferProcessor().requestedFeatures();

        SkDEBUGCODE(this->validate(false);)
    }

    GrProcessor::CustomFeatures requestedFeatures() const { return fRequestedFeatures; }

    int numSamples() const { return fNumSamples; }
    int needsStencil() const { return fNeedsStencil; }
    bool isStencilEnabled() const {
        return fUserStencilSettings != &GrUserStencilSettings::kUnused ||
               fPipeline->hasStencilClip();
    }
    const GrUserStencilSettings* userStencilSettings() const { return fUserStencilSettings; }
    // The backend format of the destination render target [proxy]
    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrSurfaceOrigin origin() const { return fOrigin;  }
    const GrPipeline& pipeline() const { return *fPipeline; }
    const GrGeometryProcessor& geomProc() const { return *fGeomProc; }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    uint8_t tessellationPatchVertexCount() const {
        SkASSERT(GrPrimitiveType::kPatches == fPrimitiveType);
        return fTessellationPatchVertexCount;
    }

    bool targetSupportsVkResolveLoad() const { return fTargetSupportsVkResolveLoad; }

    GrXferBarrierFlags renderPassBarriers() const { return fRenderPassXferBarriers; }

    GrLoadOp colorLoadOp() const { return fColorLoadOp; }

    uint16_t primitiveTypeKey() const {
        return ((uint16_t)fPrimitiveType << 8) | fTessellationPatchVertexCount;
    }

    // For Dawn, Metal and Vulkan the number of stencil bits is known a priori so we can
    // create the stencil settings here.
    GrStencilSettings nonGLStencilSettings() const;

    // Invokes the visitor function on all FP proxies in the pipeline. The caller is responsible
    // to call the visitor on its own primProc proxies.
    void visitFPProxies(const GrVisitProxyFunc& func) const { fPipeline->visitProxies(func); }

#ifdef SK_DEBUG
    void validate(bool flushTime) const;
    void checkAllInstantiated() const;
    void checkMSAAAndMIPSAreResolved() const;
#endif

private:
    const int                             fNumSamples;
    const bool                            fNeedsStencil;
    const GrBackendFormat                 fBackendFormat;
    const GrSurfaceOrigin                 fOrigin;
    const bool                            fTargetSupportsVkResolveLoad;
    const GrPipeline*                     fPipeline;
    const GrUserStencilSettings*          fUserStencilSettings;
    const GrGeometryProcessor*            fGeomProc;
    GrProcessor::CustomFeatures           fRequestedFeatures;
    GrPrimitiveType                       fPrimitiveType;
    uint8_t                               fTessellationPatchVertexCount;  // GrPrimType::kPatches.
    GrXferBarrierFlags                    fRenderPassXferBarriers;
    GrLoadOp                              fColorLoadOp;
};

#endif
