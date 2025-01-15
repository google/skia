/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/SurfaceFillContext.h"

#include "include/core/SkBlendMode.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrTextureResolveManager.h"
#include "src/gpu/ganesh/GrTracing.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrRect.h"
#include "src/gpu/ganesh/ops/ClearOp.h"
#include "src/gpu/ganesh/ops/FillRectOp.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"

#include <cstdint>

#define ASSERT_SINGLE_OWNER        SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED        if (fContext->abandoned()) { return; }

namespace skgpu::ganesh {

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// OpsTask to be picked up and added to by SurfaceFillContext lower in the call
// stack. When this occurs with a closed OpsTask, a new one will be allocated
// when the SurfaceFillContext attempts to use it (via getOpsTask).
SurfaceFillContext::SurfaceFillContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       const GrColorInfo& colorInfo)
            : SurfaceContext(rContext, std::move(readView), colorInfo)
            , fWriteView(std::move(writeView)) {
    SkASSERT(this->asSurfaceProxy() == fWriteView.proxy());
    SkASSERT(this->origin() == fWriteView.origin());

    fOpsTask = sk_ref_sp(rContext->priv().drawingManager()->getLastOpsTask(this->asSurfaceProxy()));

    SkDEBUGCODE(this->validate();)
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

void SurfaceFillContext::discard() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceFillContext", "discard", fContext);

    this->getOpsTask()->discard();
}

void SurfaceFillContext::resolveMSAA() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceFillContext", "resolveMSAA", fContext);

    this->drawingManager()->newTextureResolveRenderTask(this->asSurfaceProxyRef(),
                                                        GrSurfaceProxy::ResolveFlags::kMSAA,
                                                        *this->caps());
}

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceFillContext", "fillRectWithFP", fContext);

    GrPaint paint;
    paint.setColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto op = FillRectOp::MakeNonAARect(fContext, std::move(paint), SkMatrix::I(),
                                        SkRect::Make(dstRect));
    this->addDrawOp(std::move(op));
}

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        const SkMatrix& localMatrix,
                                        std::unique_ptr<GrFragmentProcessor> fp) {
    fp = GrMatrixEffect::Make(localMatrix, std::move(fp));
    this->fillRectWithFP(dstRect, std::move(fp));
}

bool SurfaceFillContext::blitTexture(GrSurfaceProxyView view,
                                     const SkIRect& srcRect,
                                     const SkIPoint& dstPoint) {
    SkASSERT(view.asTextureProxy());

    SkIPoint clippedDstPoint = dstPoint;
    SkIRect clippedSrcRect = srcRect;
    if (!GrClipSrcRectAndDstPoint(this->dimensions(),
                                  &clippedDstPoint,
                                  view.dimensions(),
                                  &clippedSrcRect)) {
        return false;
    }

    SkIRect clippedDstRect = SkIRect::MakePtSize(clippedDstPoint, clippedSrcRect.size());

    auto fp = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
    this->fillRectToRectWithFP(SkRect::Make(clippedSrcRect), clippedDstRect, std::move(fp));
    return true;
}

sk_sp<GrRenderTask> SurfaceFillContext::refRenderTask() {
    return sk_ref_sp(this->getOpsTask());
}

OpsTask* SurfaceFillContext::replaceOpsTask() {
    sk_sp<OpsTask> newOpsTask = this->drawingManager()->newOpsTask(this->writeSurfaceView(),
                                                                   this->arenas());
    this->willReplaceOpsTask(fOpsTask.get(), newOpsTask.get());
    fOpsTask = std::move(newOpsTask);
    return fOpsTask.get();
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

void SurfaceFillContext::internalClear(const SkIRect* scissor,
                                       std::array<float, 4> color,
                                       bool upgradePartialToFull) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_CREATE_TRACE_MARKER_CONTEXT("SurfaceFillContext", "clear", fContext);

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

#ifdef SK_DEBUG
void SurfaceFillContext::onValidate() const {
    if (fOpsTask && !fOpsTask->isClosed()) {
        SkASSERT(this->drawingManager()->getLastRenderTask(fWriteView.proxy()) == fOpsTask.get());
    }
}
#endif

}  // namespace skgpu::ganesh
