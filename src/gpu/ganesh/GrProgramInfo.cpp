/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrProgramInfo.h"

#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"

GrProgramInfo::GrProgramInfo(const GrCaps& caps,
                             const GrSurfaceProxyView& targetView,
                             bool usesMSAASurface,
                             const GrPipeline* pipeline,
                             const GrUserStencilSettings* userStencilSettings,
                             const GrGeometryProcessor* geomProc,
                             GrPrimitiveType primitiveType,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp)
        : fNeedsStencil(targetView.asRenderTargetProxy()->needsStencil())
        , fBackendFormat(targetView.proxy()->backendFormat())
        , fOrigin(targetView.origin())
        , fTargetHasVkResolveAttachmentWithInput(
                  targetView.asRenderTargetProxy()->supportsVkInputAttachment() &&
                  ((targetView.asRenderTargetProxy()->numSamples() > 1 &&
                    targetView.asTextureProxy()) ||
                   targetView.asRenderTargetProxy()->numSamples() == 1))
        , fTargetsNumSamples(targetView.asRenderTargetProxy()->numSamples())
        , fPipeline(pipeline)
        , fUserStencilSettings(userStencilSettings)
        , fGeomProc(geomProc)
        , fPrimitiveType(primitiveType)
        , fRenderPassXferBarriers(renderPassXferBarriers)
        , fColorLoadOp(colorLoadOp) {
    SkASSERT(fTargetsNumSamples > 0);
    fNumSamples = fTargetsNumSamples;
    if (fNumSamples == 1 && usesMSAASurface) {
        fNumSamples = caps.internalMultisampleCount(this->backendFormat());
    }
    SkDEBUGCODE(this->validate(false);)
}

GrStencilSettings GrProgramInfo::nonGLStencilSettings() const {
    GrStencilSettings stencil;

    if (this->isStencilEnabled()) {
        stencil.reset(*fUserStencilSettings, this->pipeline().hasStencilClip(), 8);
    }

    return stencil;
}

#ifdef SK_DEBUG
#include "src/gpu/ganesh/GrTexture.h"

void GrProgramInfo::validate(bool flushTime) const {
    if (flushTime) {
        SkASSERT(fPipeline->allProxiesInstantiated());
    }
}

void GrProgramInfo::checkAllInstantiated() const {
    this->pipeline().visitProxies([](GrSurfaceProxy* proxy, skgpu::Mipmapped) {
        SkASSERT(proxy->isInstantiated());
        return true;
    });
}

void GrProgramInfo::checkMSAAAndMIPSAreResolved() const {
    this->pipeline().visitTextureEffects([](const GrTextureEffect& te) {
        GrTexture* tex = te.texture();
        SkASSERT(tex);

        // Ensure mipmaps were all resolved ahead of time by the DAG.
        if (te.samplerState().mipmapped() == skgpu::Mipmapped::kYes &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a
            // mipmap filter. See skbug.com/7094.
            SkASSERT(tex->mipmapped() != skgpu::Mipmapped::kYes || !tex->mipmapsAreDirty());
        }
    });
}

#endif
