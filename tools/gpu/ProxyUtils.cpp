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
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/image/SkImage_Base.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#endif

namespace sk_gpu_test {

GrTextureProxy* GetTextureImageProxy(SkImage* image, GrRecordingContext* rContext) {
    if (!as_IB(image)->isGaneshBacked() || as_IB(image)->isYUVA()) {
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
    auto [view, ct] = skgpu::ganesh::AsView(rContext, image, skgpu::Mipmapped::kNo);
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
                                                GrCPixmap pixmap) {
    if (dContext->abandoned()) {
        return {};
    }

    const GrCaps* caps = dContext->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(pixmap.colorType(), renderable);
    if (!format.isValid()) {
        return {};
    }
    skgpu::Swizzle swizzle = caps->getReadSwizzle(format, pixmap.colorType());

    sk_sp<GrTextureProxy> proxy;
    proxy = dContext->priv().proxyProvider()->createProxy(format,
                                                          pixmap.dimensions(),
                                                          renderable,
                                                          /*sample count*/ 1,
                                                          skgpu::Mipmapped::kNo,
                                                          SkBackingFit::kExact,
                                                          skgpu::Budgeted::kYes,
                                                          GrProtected::kNo,
                                                          /*label=*/"TextureProxyViewFromData");
    if (!proxy) {
        return {};
    }
    GrSurfaceProxyView view(proxy, origin, swizzle);
    auto sContext = dContext->priv().makeSC(std::move(view), pixmap.colorInfo());
    if (!sContext) {
        return {};
    }
    if (!sContext->writePixels(dContext, pixmap, {0, 0})) {
        return {};
    }
    return sContext->readSurfaceView();
}

#if defined(SK_GANESH)
GrProgramInfo* CreateProgramInfo(const GrCaps* caps,
                                 SkArenaAlloc* arena,
                                 const GrSurfaceProxyView& writeView,
                                 bool usesMSAASurface,
                                 GrAppliedClip&& appliedClip,
                                 const GrDstProxyView& dstProxyView,
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
                                                     &appliedClip, stencilSettings, *caps,
                                                     GrClampType::kAuto, &analysisColor);
    SkASSERT(!analysis.requiresDstTexture());

    return GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps, arena, writeView, usesMSAASurface,
                                                       std::move(appliedClip), dstProxyView,
                                                       geomProc, std::move(processors),
                                                       primitiveType, renderPassXferBarriers,
                                                       colorLoadOp, flags, stencilSettings);
}
#endif // defined(SK_GANESH)

}  // namespace sk_gpu_test
