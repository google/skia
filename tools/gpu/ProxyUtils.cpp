/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/SkGr.h"
#include "tools/gpu/ProxyUtils.h"

namespace sk_gpu_test {

sk_sp<GrTextureProxy> MakeTextureProxyFromData(GrContext* context,
                                               GrRenderable renderable,
                                               GrSurfaceOrigin origin,
                                               const GrImageInfo& imageInfo,
                                               const void* data,
                                               size_t rowBytes) {
    if (context->priv().abandoned()) {
        return nullptr;
    }

    const GrCaps* caps = context->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(imageInfo.colorType(), renderable);
    if (!format.isValid()) {
        return nullptr;
    }
    GrSwizzle swizzle = caps->getReadSwizzle(format, imageInfo.colorType());

    sk_sp<GrTextureProxy> proxy;
    proxy = context->priv().proxyProvider()->createProxy(
            format, imageInfo.dimensions(), swizzle, renderable, 1, GrMipMapped::kNo,
            SkBackingFit::kExact, SkBudgeted::kYes, GrProtected::kNo);
    if (!proxy) {
        return nullptr;
    }
    GrSurfaceProxyView view(proxy, origin, swizzle);
    auto sContext = GrSurfaceContext::Make(context, std::move(view), imageInfo.colorType(),
                                           imageInfo.alphaType(), imageInfo.refColorSpace());
    if (!sContext) {
        return nullptr;
    }
    if (!sContext->writePixels(imageInfo, data, rowBytes, {0, 0}, context)) {
        return nullptr;
    }
    return proxy;
}

GrProgramInfo* CreateProgramInfo(const GrCaps* caps,
                                 SkArenaAlloc* arena,
                                 const GrSurfaceProxyView* dstView,
                                 GrAppliedClip&& appliedClip,
                                 const GrXferProcessor::DstProxyView& dstProxyView,
                                 GrGeometryProcessor* geomProc,
                                 SkBlendMode blendMode,
                                 GrPrimitiveType primitiveType,
                                 GrPipeline::InputFlags flags,
                                 const GrUserStencilSettings* stencil) {

    GrPipeline::InitArgs initArgs;
    initArgs.fInputFlags = flags;
    initArgs.fUserStencil = stencil;
    initArgs.fCaps = caps;
    initArgs.fDstProxyView = dstProxyView;
    initArgs.fOutputSwizzle = dstView->swizzle();

    GrPipeline::FixedDynamicState* fixedDynamicState = nullptr;

    if (appliedClip.scissorState().enabled()) {
        fixedDynamicState = arena->make<GrPipeline::FixedDynamicState>(
                                                        appliedClip.scissorState().rect());
    }

    GrProcessorSet processors = GrProcessorSet(blendMode);

    SkPMColor4f analysisColor = { 0, 0, 0, 1 }; // opaque black

    SkDEBUGCODE(auto analysis =) processors.finalize(analysisColor,
                                                     GrProcessorAnalysisCoverage::kSingleChannel,
                                                     &appliedClip, stencil, false,
                                                     *caps, GrClampType::kAuto, &analysisColor);
    SkASSERT(!analysis.requiresDstTexture());

    GrPipeline* pipeline = arena->make<GrPipeline>(initArgs,
                                                   std::move(processors),
                                                   std::move(appliedClip));

    GrRenderTargetProxy* dstProxy = dstView->asRenderTargetProxy();
    return arena->make<GrProgramInfo>(dstProxy->numSamples(),
                                      dstProxy->numStencilSamples(),
                                      dstProxy->backendFormat(),
                                      dstView->origin(),
                                      pipeline,
                                      geomProc,
                                      fixedDynamicState,
                                      nullptr, 0,
                                      primitiveType);
}


}  // namespace sk_gpu_test
