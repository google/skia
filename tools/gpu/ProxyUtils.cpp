/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ProxyUtils.h"

#include "include/core/SkColor.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrImageContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrPixmap.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/image/SkImage_Base.h"

namespace sk_gpu_test {

GrTextureProxy* GetTextureImageProxy(SkImage* image, GrRecordingContext* rContext) {
    if (!image->isTextureBacked() || as_IB(image)->isYUVA()) {
        return nullptr;
    }
    if (!rContext) {
        // If the image is backed by a recording context we'll use that.
        GrImageContext* iContext = as_IB(image)->context();
        SkASSERT(iContext);
        rContext = iContext->priv().asRecordingContext();
        if (!rContext) {
            return nullptr;
        }
    }
    auto [view, ct] = as_IB(image)->asView(rContext, GrMipmapped::kNo);
    if (!view) {
        // With the above checks we expect this to succeed unless there is a context mismatch.
        SkASSERT(!image->isValid(rContext));
        return nullptr;
    }
    GrSurfaceProxy* proxy = view.proxy();
    SkASSERT(proxy->asTextureProxy());
    return proxy->asTextureProxy();
}

GrSurfaceProxyView MakeTextureProxyViewFromData(GrDirectContext* dContext,
                                                GrRenderable renderable,
                                                GrSurfaceOrigin origin,
                                                GrPixmap pixmap) {
    if (dContext->abandoned()) {
        return {};
    }

    const GrCaps* caps = dContext->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(pixmap.colorType(), renderable);
    if (!format.isValid()) {
        return {};
    }
    GrSwizzle swizzle = caps->getReadSwizzle(format, pixmap.colorType());

    sk_sp<GrTextureProxy> proxy;
    proxy = dContext->priv().proxyProvider()->createProxy(format,
                                                          pixmap.dimensions(),
                                                          renderable,
                                                          /*sample count*/ 1,
                                                          GrMipmapped::kNo,
                                                          SkBackingFit::kExact,
                                                          SkBudgeted::kYes,
                                                          GrProtected::kNo);
    if (!proxy) {
        return {};
    }
    GrSurfaceProxyView view(proxy, origin, swizzle);
    auto sContext = GrSurfaceContext::Make(dContext, std::move(view), pixmap.colorInfo());
    if (!sContext) {
        return {};
    }
    if (!sContext->writePixels(dContext, pixmap, {0, 0})) {
        return {};
    }
    return sContext->readSurfaceView();
}

GrProgramInfo* CreateProgramInfo(const GrCaps* caps,
                                 SkArenaAlloc* arena,
                                 const GrSurfaceProxyView& writeView,
                                 GrAppliedClip&& appliedClip,
                                 const GrXferProcessor::DstProxyView& dstProxyView,
                                 GrGeometryProcessor* geomProc,
                                 SkBlendMode blendMode,
                                 GrPrimitiveType primitiveType,
                                 GrXferBarrierFlags renderPassXferBarriers,
                                 GrLoadOp colorLoadOp,
                                 GrPipeline::InputFlags flags,
                                 const GrUserStencilSettings* stencilSettings) {

    GrProcessorSet processors = GrProcessorSet(blendMode);

    SkPMColor4f analysisColor = { 0, 0, 0, 1 }; // opaque black

    SkDEBUGCODE(auto analysis =) processors.finalize(analysisColor,
                                                     GrProcessorAnalysisCoverage::kSingleChannel,
                                                     &appliedClip, stencilSettings, false,
                                                     *caps, GrClampType::kAuto, &analysisColor);
    SkASSERT(!analysis.requiresDstTexture());

    return GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps, arena, writeView,
                                                       std::move(appliedClip), dstProxyView,
                                                       geomProc, std::move(processors),
                                                       primitiveType, renderPassXferBarriers,
                                                       colorLoadOp, flags, stencilSettings);
}


}  // namespace sk_gpu_test
