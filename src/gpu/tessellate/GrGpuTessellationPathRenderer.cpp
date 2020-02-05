/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrGpuTessellationPathRenderer.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrTessellatePathOp.h"

constexpr static SkISize kAtlasInitialSize{512, 512};
constexpr static int kMaxAtlasSize = 2048;

GrGpuTessellationPathRenderer::GrGpuTessellationPathRenderer(const GrCaps& caps) : fAtlas(
        GrColorType::kAlpha_8, GrDynamicAtlas::InternalMultisample::kYes, kAtlasInitialSize,
        std::min(kMaxAtlasSize, caps.maxPreferredRenderTargetSize()), caps) {
    this->resetAtlasUberPaths();
}

void GrGpuTessellationPathRenderer::resetAtlasUberPaths() {
    for (auto fillType : {SkPathFillType::kEvenOdd, SkPathFillType::kWinding}) {
        SkPath* uberPath = this->getAtlasUberPath(fillType);
        uberPath->reset();
        uberPath->setFillType(fillType);
    }
}

GrPathRenderer::CanDrawPath GrGpuTessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    // This class should not have been added to the chain without tessellation support.
    SkASSERT(args.fCaps->shaderCaps()->tessellationSupport());
    if (!args.fShape->style().isSimpleFill() || args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective()) {
        return CanDrawPath::kNo;
    }
    if (GrAAType::kCoverage == args.fAAType) {
        SkASSERT(1 == args.fProxy->numSamples());
        if (!args.fProxy->canUseMixedSamples(*args.fCaps)) {
            return CanDrawPath::kNo;
        }
    }
    SkPath path;
    args.fShape->asPath(&path);
    if (SkPathPriv::ConicWeightCnt(path)) {
        return CanDrawPath::kNo;
    }
    return CanDrawPath::kYes;
}

bool GrGpuTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GrRenderTargetContext* renderTargetContext = args.fRenderTargetContext;
    GrOpMemoryPool* pool = args.fContext->priv().opMemoryPool();
    SkPath path;
    args.fShape->asPath(&path);

    // See if the path is small and simple enough to atlas instead of drawing directly.
    //
    // NOTE: The atlas is always antialiased (regardless of args.fAAType), and gets rendered as
    // alpha8 coverage even for msaa render targets.
    SkIRect devIBounds;
    SkIVector devToAtlasOffset;
    if (this->tryAddPathToAtlas(*args.fViewMatrix, path, &devIBounds, &devToAtlasOffset)) {
        auto op = pool->allocate<GrDrawAtlasPathOp>(
                renderTargetContext->numSamples(), sk_ref_sp(fAtlas.textureProxy()),
                devIBounds, devToAtlasOffset, *args.fViewMatrix, std::move(args.fPaint));
        renderTargetContext->addDrawOp(*args.fClip, std::move(op));
        return true;
    }

    auto op = pool->allocate<GrTessellatePathOp>(
            *args.fViewMatrix, path, std::move(args.fPaint), args.fAAType);
    renderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

bool GrGpuTessellationPathRenderer::tryAddPathToAtlas(const SkMatrix& viewMatrix,
                                                      const SkPath& path, SkIRect* devIBounds,
                                                      SkIVector* devToAtlasOffset) {
    // Check if the path is too complex to justify mapping and copying the points on CPU.
    if (path.countPoints() > 150) {
        return false;
    }

    // Check if the path is too large for an atlas.
    SkRect devBounds;
    viewMatrix.mapRect(&devBounds, path.getBounds());
    if (devBounds.height() * devBounds.width() > 100 * 100 ||
        std::max(devBounds.height(), devBounds.width()) > kMaxAtlasSize / 2) {
        return false;
    }

    devBounds.roundOut(devIBounds);
    if (!fAtlas.addRect(*devIBounds, devToAtlasOffset)) {
        return false;
    }

    // Concatenate this path onto our uber path corresponding to its fill type.
    SkPath* uberPath = this->getAtlasUberPath(path.getFillType());
    auto [uberPts, uberWeights] = SkPathPriv::GrowForVerbsInPath(uberPath, path);
    SkMatrix atlasMatrix = viewMatrix;
    atlasMatrix.postTranslate(devToAtlasOffset->x(), devToAtlasOffset->y());
    atlasMatrix.mapPoints(uberPts, SkPathPriv::PointData(path), path.countPoints());
    memcpy(uberWeights, SkPathPriv::ConicWeightData(path), SkPathPriv::ConicWeightCnt(path));
    return true;
}

void GrGpuTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkPath path;
    args.fShape->asPath(&path);

    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    auto op = args.fContext->priv().opMemoryPool()->allocate<GrTessellatePathOp>(
            *args.fViewMatrix, path, GrPaint(), aaType, GrTessellatePathOp::Flags::kStencilOnly);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
}

void GrGpuTessellationPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                             const uint32_t* opsTaskIDs, int numOpsTaskIDs) {
    if (!fAtlas.drawBounds().isEmpty()) {
        this->renderAtlas(onFlushRP);
        fAtlas.reset(kAtlasInitialSize, *onFlushRP->caps());
        this->resetAtlasUberPaths();
    }
#ifdef SK_DEBUG
    for (const SkPath& uberPath : fAtlasUberPaths) {
        SkASSERT(uberPath.isEmpty());
    }
#endif
}

constexpr static GrUserStencilSettings kTestStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kKeep,
        GrUserStencilOp::kKeep,
        0xffff>());

constexpr static GrUserStencilSettings kTestAndResetStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>());

void GrGpuTessellationPathRenderer::renderAtlas(GrOnFlushResourceProvider* onFlushRP) {
    auto rtc = fAtlas.instantiate(onFlushRP);
    if (!rtc) {
        return;
    }

    // Add ops to stencil the atlas paths.
    for (const SkPath& uberPath : fAtlasUberPaths) {
        if (uberPath.isEmpty()) {
            continue;
        }
        auto op = onFlushRP->opMemoryPool()->allocate<GrTessellatePathOp>(
                SkMatrix::I(), uberPath, GrPaint(), GrAAType::kMSAA,
                GrTessellatePathOp::Flags::kStencilOnly);
        rtc->addDrawOp(GrNoClip(), std::move(op));
    }

    // The next draw will be the final op in the renderTargetContext. So if Ganesh is planning
    // to discard the stencil values anyway, then we might not actually need to reset the
    // stencil values back to zero.
    bool mustResetStencil = !onFlushRP->caps()->discardStencilValuesAfterRenderPass() ||
                            rtc->numSamples() <= 1;  // Need a stencil reset for mixed samples.

    // Draw a fullscreen rect to convert our stencilled paths into alpha coverage masks.
    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    SkRect drawRect = SkRect::MakeIWH(fAtlas.drawBounds().width(), fAtlas.drawBounds().height());
    rtc->priv().stencilRect(GrNoClip(), (mustResetStencil) ? &kTestAndResetStencil : &kTestStencil,
                            std::move(paint), GrAA::kYes, SkMatrix::I(), drawRect, nullptr);

    if (rtc->asSurfaceProxy()->requiresManualMSAAResolve()) {
        onFlushRP->addTextureResolveTask(sk_ref_sp(rtc->asTextureProxy()),
                                         GrSurfaceProxy::ResolveFlags::kMSAA);
    }
}
