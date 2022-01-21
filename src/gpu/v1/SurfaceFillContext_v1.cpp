/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v1/SurfaceFillContext_v1.h"

#include "include/private/GrImageContext.h"
#include "src/gpu/GrDstProxyView.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/ops/ClearOp.h"
#include "src/gpu/ops/FillRectOp.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

#define ASSERT_SINGLE_OWNER        SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED        if (fContext->abandoned()) { return; }

class AutoCheckFlush {
public:
    AutoCheckFlush(GrDrawingManager* drawingManager) : fDrawingManager(drawingManager) {
        SkASSERT(fDrawingManager);
    }
    ~AutoCheckFlush() { fDrawingManager->flushIfNecessary(); }

private:
    GrDrawingManager* fDrawingManager;
};

namespace skgpu::v1 {

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// OpsTask to be picked up and added to by SurfaceFillContext lower in the call
// stack. When this occurs with a closed OpsTask, a new one will be allocated
// when the SurfaceFillContext attempts to use it (via getOpsTask).
SurfaceFillContext::SurfaceFillContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       const GrColorInfo& colorInfo,
                                       bool flushTimeOpsTask)
        : skgpu::SurfaceFillContext(rContext,
                                    std::move(readView),
                                    std::move(writeView),
                                    std::move(colorInfo))
        , fFlushTimeOpsTask(flushTimeOpsTask) {
    fOpsTask = sk_ref_sp(rContext->priv().drawingManager()->getLastOpsTask(this->asSurfaceProxy()));

    SkDEBUGCODE(this->validate();)
}

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("v1::SurfaceFillContext", "fillRectWithFP", fContext);

    AutoCheckFlush acf(this->drawingManager());

    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto op = FillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                        SkRect::Make(dstRect));
    this->addDrawOp(std::move(op));
}

void SurfaceFillContext::addDrawOp(GrOp::Owner owner) {
    GrDrawOp* op = static_cast<GrDrawOp*>(owner.get());
    GrClampType clampType = GrColorTypeClampType(this->colorInfo().colorType());
    auto clip = GrAppliedClip::Disabled();
    const GrCaps& caps = *this->caps();
    GrProcessorSet::Analysis analysis = op->finalize(caps, &clip, clampType);
    SkASSERT(!op->usesStencil());
    SkASSERT(!analysis.requiresDstTexture());
    SkRect bounds = owner->bounds();
    // We shouldn't have coverage AA or hairline draws in fill contexts.
    SkASSERT(!op->hasAABloat() && !op->hasZeroArea());
    if (!bounds.intersect(this->asSurfaceProxy()->getBoundsRect())) {
        return;
    }
    op->setClippedBounds(op->bounds());
    SkDEBUGCODE(op->fAddDrawOpCalled = true;)

    GrDstProxyView dstProxyView;
    this->getOpsTask()->addDrawOp(fContext->priv().drawingManager(),
                                  std::move(owner),
                                  op->usesMSAA(),
                                  analysis,
                                  std::move(clip),
                                  dstProxyView,
                                  GrTextureResolveManager(this->drawingManager()),
                                  caps);
}

void SurfaceFillContext::ClearToGrPaint(std::array<float, 4> color, GrPaint* paint) {
    paint->setColor4f({color[0], color[1], color[2], color[3]});
    if (color[3] == 1.f) {
        // Can just rely on the src-over blend mode to do the right thing.
        // This may improve batching.
        paint->setPorterDuffXPFactory(SkBlendMode::kSrcOver);
    } else {
        // A clear overwrites the prior color, so even if it's transparent, it behaves as if it
        // were src blended
        paint->setPorterDuffXPFactory(SkBlendMode::kSrc);
    }
}

void SurfaceFillContext::addOp(GrOp::Owner op) {
    GrDrawingManager* drawingMgr = this->drawingManager();
    this->getOpsTask()->addOp(drawingMgr,
                              std::move(op),
                              GrTextureResolveManager(drawingMgr),
                              *this->caps());
}

OpsTask* SurfaceFillContext::getOpsTask() {
    ASSERT_SINGLE_OWNER
    SkDEBUGCODE(this->validate();)

    if (!fOpsTask || fOpsTask->isClosed()) {
        this->replaceOpsTask();
    }
    SkASSERT(!fOpsTask->isClosed());
    return fOpsTask.get();
}

sk_sp<GrRenderTask> SurfaceFillContext::refRenderTask() {
    return sk_ref_sp(this->getOpsTask());
}

OpsTask* SurfaceFillContext::replaceOpsTask() {
    sk_sp<OpsTask> newOpsTask = this->drawingManager()->newOpsTask(
            this->writeSurfaceView(), this->arenas(), fFlushTimeOpsTask);
    this->willReplaceOpsTask(fOpsTask.get(), newOpsTask.get());
    fOpsTask = std::move(newOpsTask);
    return fOpsTask.get();
}

#ifdef SK_DEBUG
void SurfaceFillContext::onValidate() const {
    if (fOpsTask && !fOpsTask->isClosed()) {
        SkASSERT(this->drawingManager()->getLastRenderTask(fWriteView.proxy()) == fOpsTask.get());
    }
}
#endif

void SurfaceFillContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("v1::SurfaceFillContext", "discard", fContext);

    AutoCheckFlush acf(this->drawingManager());

    this->getOpsTask()->discard();
}

void SurfaceFillContext::internalClear(const SkIRect* scissor,
                                       std::array<float, 4> color,
                                       bool upgradePartialToFull) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("v1::SurfaceFillContext", "clear", fContext);

    // There are three ways clears are handled: load ops, native clears, and draws. Load ops are
    // only for fullscreen clears; native clears can be fullscreen or with scissors if the backend
    // supports then. Drawing an axis-aligned rect is the fallback path.
    GrScissorState scissorState(this->asSurfaceProxy()->backingStoreDimensions());
    if (scissor && !scissorState.set(*scissor)) {
        // The clear is offscreen, so skip it (normally this would be handled by addDrawOp,
        // except clear ops are not draw ops).
        return;
    }

    // If we have a scissor but it's okay to clear beyond it for performance reasons, then disable
    // the test. We only do this when the clear would be handled by a load op or natively.
    if (scissorState.enabled() && !this->caps()->performColorClearsAsDraws()) {
        if (upgradePartialToFull && (this->caps()->preferFullscreenClears() ||
                                     this->caps()->shouldInitializeTextures())) {
            // TODO: wrt the shouldInitializeTextures path, it would be more performant to
            // only clear the entire target if we knew it had not been cleared before. As
            // is this could end up doing a lot of redundant clears.
            scissorState.setDisabled();
        } else {
            // Unlike with stencil clears, we also allow clears up to the logical dimensions of the
            // render target to overflow into any approx-fit padding of the backing store dimensions
            scissorState.relaxTest(this->dimensions());
        }
    }

    if (!scissorState.enabled()) {
        // This is a fullscreen clear, so could be handled as a load op. Regardless, we can also
        // discard all prior ops in the current task since the color buffer will be overwritten.
        auto opsTask = this->getOpsTask();
        if (opsTask->resetForFullscreenClear(this->canDiscardPreviousOpsOnFullClear()) &&
            !this->caps()->performColorClearsAsDraws()) {
            color = this->writeSurfaceView().swizzle().applyTo(color);
            // The op list was emptied and native clears are allowed, so just use the load op
            opsTask->setColorLoadOp(GrLoadOp::kClear, color);
            return;
        } else {
            // Will use an op for the clear, reset the load op to discard since the op will
            // blow away the color buffer contents
            opsTask->setColorLoadOp(GrLoadOp::kDiscard);
        }
    }

    // At this point we are either a partial clear or a fullscreen clear that couldn't be applied
    // as a load op.
    bool clearAsDraw = this->caps()->performColorClearsAsDraws() ||
                       (scissorState.enabled() && this->caps()->performPartialClearsAsDraws());
    if (clearAsDraw) {
        GrPaint paint;
        ClearToGrPaint(color, &paint);
        auto op = FillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                            SkRect::Make(scissorState.rect()));
        this->addDrawOp(std::move(op));
    } else {
        color = this->writeSurfaceView().swizzle().applyTo(color);
        this->addOp(ClearOp::MakeColor(fContext, scissorState, color));
    }
}

bool SurfaceFillContext::blitTexture(GrSurfaceProxyView view,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    SkASSERT(view.asTextureProxy());
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    if (!GrClipSrcRectAndDstPoint(this->dimensions(),
                                  view.dimensions(),
                                  srcRect,
                                  dstPoint,
                                  &clippedSrcRect,
                                  &clippedDstPoint)) {
        return false;
    }

    auto fp = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
    auto dstRect = SkIRect::MakePtSize(clippedDstPoint, clippedSrcRect.size());
    auto srcRectF = SkRect::Make(clippedSrcRect);
    this->fillRectToRectWithFP(srcRectF, dstRect, std::move(fp));
    return true;
}

} // namespace skgpu::v1
