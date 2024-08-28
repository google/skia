/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramInfo_DEFINED
#define GrProgramInfo_DEFINED

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"

#include <cstdint>

class GrGeometryProcessor;
class GrStencilSettings;
class GrSurfaceProxyView;
enum GrSurfaceOrigin : int;
enum class GrXferBarrierFlags;

class GrProgramInfo {
public:
    GrProgramInfo(const GrCaps& caps,
                  const GrSurfaceProxyView& targetView,
                  bool usesMSAASurface,
                  const GrPipeline* pipeline,
                  const GrUserStencilSettings* userStencilSettings,
                  const GrGeometryProcessor* geomProc,
                  GrPrimitiveType primitiveType,
                  GrXferBarrierFlags renderPassXferBarriers,
                  GrLoadOp colorLoadOp);

    int numSamples() const { return fNumSamples; }
    int needsStencil() const { return fNeedsStencil; }
    bool isStencilEnabled() const {
        return fUserStencilSettings != &GrUserStencilSettings::kUnused ||
               fPipeline->hasStencilClip();
    }
    const GrUserStencilSettings* userStencilSettings() const { return fUserStencilSettings; }
    // The backend format of the destination render target [proxy]
    const GrBackendFormat& backendFormat() const { return fBackendFormat; }
    GrSurfaceOrigin origin() const { return fOrigin; }
    const GrPipeline& pipeline() const { return *fPipeline; }
    const GrGeometryProcessor& geomProc() const { return *fGeomProc; }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }

    bool targetHasVkResolveAttachmentWithInput() const {
        return fTargetHasVkResolveAttachmentWithInput;
    }

    int targetsNumSamples() const { return fTargetsNumSamples; }

    GrXferBarrierFlags renderPassBarriers() const { return fRenderPassXferBarriers; }

    GrLoadOp colorLoadOp() const { return fColorLoadOp; }

    uint16_t primitiveTypeKey() const {
        return (uint16_t) fPrimitiveType;
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
    int                                   fNumSamples;
    bool                                  fNeedsStencil;
    GrBackendFormat                       fBackendFormat;
    GrSurfaceOrigin                       fOrigin;
    bool                                  fTargetHasVkResolveAttachmentWithInput;
    int                                   fTargetsNumSamples;
    const GrPipeline*                     fPipeline;
    const GrUserStencilSettings*          fUserStencilSettings;
    const GrGeometryProcessor*            fGeomProc;
    GrPrimitiveType                       fPrimitiveType;
    GrXferBarrierFlags                    fRenderPassXferBarriers;
    GrLoadOp                              fColorLoadOp;
};

#endif
