/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/AtlasRenderTask.h"

#include "src/core/SkBlendModePriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsTypes.h"
#include "src/gpu/ops/FillRectOp.h"
#include "src/gpu/ops/PathStencilCoverOp.h"

namespace skgpu::v1 {

AtlasRenderTask::AtlasRenderTask(GrRecordingContext* rContext,
                                 sk_sp<GrArenas> arenas,
                                 std::unique_ptr<GrDynamicAtlas> dynamicAtlas)
        : OpsTask(rContext->priv().drawingManager(),
                  dynamicAtlas->writeView(*rContext->priv().caps()),
                  rContext->priv().auditTrail(),
                  std::move(arenas))
        , fDynamicAtlas(std::move(dynamicAtlas)) {
}

bool AtlasRenderTask::addPath(const SkMatrix& viewMatrix, const SkPath& path,
                              SkIPoint pathDevTopLeft, int widthInAtlas, int heightInAtlas,
                              bool transposedInAtlas, SkIPoint16* locationInAtlas) {
    SkASSERT(!this->isClosed());
    SkASSERT(this->isEmpty());
    SkASSERT(!fDynamicAtlas->isInstantiated());  // Paths can't be added after instantiate().

    if (!fDynamicAtlas->addRect(widthInAtlas, heightInAtlas, locationInAtlas)) {
        return false;
    }

    SkMatrix pathToAtlasMatrix = viewMatrix;
    if (transposedInAtlas) {
        std::swap(pathToAtlasMatrix[0], pathToAtlasMatrix[3]);
        std::swap(pathToAtlasMatrix[1], pathToAtlasMatrix[4]);
        float tx=pathToAtlasMatrix.getTranslateX(), ty=pathToAtlasMatrix.getTranslateY();
        pathToAtlasMatrix.setTranslateX(ty - pathDevTopLeft.y() + locationInAtlas->x());
        pathToAtlasMatrix.setTranslateY(tx - pathDevTopLeft.x() + locationInAtlas->y());
    } else {
        pathToAtlasMatrix.postTranslate(locationInAtlas->x() - pathDevTopLeft.x(),
                                        locationInAtlas->y() - pathDevTopLeft.y());
    }

    if (GrFillRuleForSkPath(path) == GrFillRule::kNonzero) {
        fWindingPathList.add(&fPathDrawAllocator, pathToAtlasMatrix, path);
    } else {
        fEvenOddPathList.add(&fPathDrawAllocator, pathToAtlasMatrix, path);
    }
    return true;
}

GrRenderTask::ExpectedOutcome AtlasRenderTask::onMakeClosed(GrRecordingContext* rContext,
                                                            SkIRect* targetUpdateBounds) {
    // We don't add our ops until now, at which point we know the atlas is done being built.
    SkASSERT(this->isEmpty());
    SkASSERT(!fDynamicAtlas->isInstantiated());  // Instantiation happens after makeClosed().

    const GrCaps& caps = *rContext->priv().caps();

    // Set our dimensions now. OpsTask will need them when we add our ops.
    this->target(0)->priv().setLazyDimensions(fDynamicAtlas->drawBounds());
    this->target(0)->asRenderTargetProxy()->setNeedsStencil();
    SkRect drawRect = target(0)->getBoundsRect();

    // Clear the atlas.
    if (caps.performColorClearsAsDraws() || caps.performStencilClearsAsDraws()) {
        this->setColorLoadOp(GrLoadOp::kDiscard);
        this->setInitialStencilContent(StencilContent::kDontCare);

        constexpr static GrUserStencilSettings kClearStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kAlways,
                0xffff,
                GrUserStencilOp::kReplace,
                GrUserStencilOp::kReplace,
                0xffff>());

        this->stencilAtlasRect(rContext, drawRect, SK_PMColor4fTRANSPARENT, &kClearStencil);
    } else {
        this->setColorLoadOp(GrLoadOp::kClear);
        this->setInitialStencilContent(StencilContent::kUserBitsCleared);
    }

    // Add ops to stencil the atlas paths.
    for (const auto* pathList : {&fWindingPathList, &fEvenOddPathList}) {
        if (pathList->pathCount() > 0) {
            auto op = GrOp::Make<PathStencilCoverOp>(
                    rContext,
                    pathList->pathDrawList(),
                    pathList->totalCombinedPathVerbCnt(),
                    pathList->pathCount(),
                    GrPaint(),
                    GrAAType::kMSAA,
                    GrTessellationPathFlags::kStencilOnly,
                    drawRect);
            this->addAtlasDrawOp(std::move(op), caps);
        }
    }

    // Finally, draw a fullscreen rect to cover our stencilled paths.
    const GrUserStencilSettings* stencil;
    if (caps.discardStencilValuesAfterRenderPass()) {
        constexpr static GrUserStencilSettings kTestStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kKeep,
                0xffff>());

        // This is the final op in the task. Since Ganesh is planning to discard the stencil values
        // anyway, there is no need to reset the stencil values back to 0.
        stencil = &kTestStencil;
    } else {
        constexpr static GrUserStencilSettings kTestAndResetStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>());

        // Outset the cover rect to make extra sure we clear every stencil value touched by the
        // atlas.
        drawRect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    }
    this->stencilAtlasRect(rContext, drawRect, SK_PMColor4fWHITE, stencil);

    this->OpsTask::onMakeClosed(rContext, targetUpdateBounds);

    // Don't mark msaa dirty. Since this op defers being closed, the drawing manager's dirty
    // tracking doesn't work anyway. We will just resolve msaa manually during onExecute.
    return ExpectedOutcome::kTargetUnchanged;
}

void AtlasRenderTask::stencilAtlasRect(GrRecordingContext* rContext, const SkRect& rect,
                                       const SkPMColor4f& color,
                                       const GrUserStencilSettings* stencil) {
    GrPaint paint;
    paint.setColor4f(color);
    paint.setXPFactory(SkBlendMode_AsXPFactory(SkBlendMode::kSrc));
    GrQuad quad(rect);
    DrawQuad drawQuad{quad, quad, GrQuadAAFlags::kAll};
    auto op = FillRectOp::Make(rContext, std::move(paint), GrAAType::kMSAA, &drawQuad, stencil);
    this->addAtlasDrawOp(std::move(op), *rContext->priv().caps());
}

void AtlasRenderTask::addAtlasDrawOp(GrOp::Owner op, const GrCaps& caps) {
    SkASSERT(!this->isClosed());

    auto drawOp = static_cast<GrDrawOp*>(op.get());
    SkDEBUGCODE(drawOp->fAddDrawOpCalled = true;)

    auto processorAnalysis = drawOp->finalize(caps, nullptr,
                                              GrColorTypeClampType(fDynamicAtlas->colorType()));
    SkASSERT(!processorAnalysis.requiresDstTexture());
    SkASSERT(!processorAnalysis.usesNonCoherentHWBlending());

    drawOp->setClippedBounds(drawOp->bounds());
    this->recordOp(std::move(op), true/*usesMSAA*/, processorAnalysis, nullptr, nullptr, caps);
}

bool AtlasRenderTask::onExecute(GrOpFlushState* flushState) {
    if (!this->OpsTask::onExecute(flushState)) {
        return false;
    }
    if (this->target(0)->requiresManualMSAAResolve()) {
        // Since atlases don't get closed until they are done being built, the drawingManager
        // doesn't detect that they need an MSAA resolve. Do it here manually.
        auto nativeRect = GrNativeRect::MakeIRectRelativeTo(
                GrDynamicAtlas::kTextureOrigin,
                this->target(0)->backingStoreDimensions().height(),
                SkIRect::MakeSize(fDynamicAtlas->drawBounds()));
        flushState->gpu()->resolveRenderTarget(this->target(0)->peekRenderTarget(), nativeRect);
    }
    return true;
}

} // namespace skgpu::v1
