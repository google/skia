/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "tools/gpu/ProxyUtils.h"

namespace sk_gpu_test {

GrSurfaceProxyView MakeTextureProxyViewFromData(GrDirectContext* dContext,
                                                GrRenderable renderable,
                                                GrSurfaceOrigin origin,
                                                const GrImageInfo& imageInfo,
                                                const void* data,
                                                size_t rowBytes) {
    if (dContext->abandoned()) {
        return {};
    }

    const GrCaps* caps = dContext->priv().caps();

    const GrBackendFormat format = caps->getDefaultBackendFormat(imageInfo.colorType(), renderable);
    if (!format.isValid()) {
        return {};
    }
    GrSwizzle swizzle = caps->getReadSwizzle(format, imageInfo.colorType());

    sk_sp<GrTextureProxy> proxy;
    proxy = dContext->priv().proxyProvider()->createProxy(format, imageInfo.dimensions(),
                                                          renderable, 1, GrMipmapped::kNo,
                                                          SkBackingFit::kExact, SkBudgeted::kYes,
                                                          GrProtected::kNo);
    if (!proxy) {
        return {};
    }
    GrSurfaceProxyView view(proxy, origin, swizzle);
    auto sContext = GrSurfaceContext::Make(dContext, std::move(view), imageInfo.colorInfo());
    if (!sContext) {
        return {};
    }
    if (!sContext->writePixels(dContext, imageInfo, data, rowBytes, {0, 0})) {
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
