/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrAtlasRenderTask.h"

#include "src/core/SkBlendModePriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/tessellate/GrPathStencilCoverOp.h"

GrAtlasRenderTask::GrAtlasRenderTask(GrRecordingContext* rContext, GrAuditTrail* auditTrail,
                                     sk_sp<GrArenas> arenas,
                                     std::unique_ptr<GrDynamicAtlas> dynamicAtlas)
        : GrOpsTask(rContext->priv().drawingManager(),
                    dynamicAtlas->writeView(*rContext->priv().caps()), auditTrail,
                    std::move(arenas))
        , fDynamicAtlas(std::move(dynamicAtlas)) {
}

bool GrAtlasRenderTask::addPath(const SkMatrix& viewMatrix, const SkPath& path, bool antialias,
                                SkIPoint pathDevTopLeft, int widthInAtlas, int heightInAtlas,
                                bool transposedInAtlas, SkIPoint16* locationInAtlas) {
    SkASSERT(!fDynamicAtlas->isInstantiated());  // Paths can't be added after instantiate().
    SkASSERT(this->isEmpty());

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

    // Concatenate this path onto our uber path that matches its fill and AA types.
    SkPath* uberPath = this->getUberPath(path.getFillType(), antialias);
    uberPath->moveTo(locationInAtlas->x(), locationInAtlas->y());  // Implicit moveTo(0,0).
    uberPath->addPath(path, pathToAtlasMatrix);
    return true;
}

bool GrAtlasRenderTask::canShareAtlasTexture(GrAtlasRenderTask* task) {
    // Check that the sizes mach.
    if (fDynamicAtlas->currentWidth() != task->fDynamicAtlas->currentWidth() ||
        fDynamicAtlas->currentHeight() != task->fDynamicAtlas->currentHeight()) {
        return false;
    }
    // Check for common dependents. If these atlases share a common dependent, it means a single
    // draw op samples from them both, so they can't share a texture. The DAG should otherwise
    // guarantee that render tasks never use more than 1 atlas in other cases.
    SkASSERT(this->dependents().size() > 0);
    SkASSERT(task->dependents().size() > 0);
    for (GrRenderTask* a : task->dependents()) {
        for (GrRenderTask* b : this->dependents()) {
            if (a == b) {
                return false;
            }
        }
    }
    return true;
}

void GrAtlasRenderTask::instantiate(GrOnFlushResourceProvider* onFlushRP,
                                    sk_sp<GrTexture> backingTexture) {
    // We don't add our ops until instantiate, at which point we know the atlas is done being built.
    SkASSERT(this->isEmpty());

    fDynamicAtlas->instantiate(onFlushRP, std::move(backingTexture));

    const GrCaps& caps = *onFlushRP->caps();
    SkRect atlasRect = SkRect::Make(SkIRect::MakeSize(fDynamicAtlas->drawBounds()));

    // Add our ops now that we know the atlas is done being built.
    if (caps.performColorClearsAsDraws() || caps.performStencilClearsAsDraws()) {
        this->setColorLoadOp(GrLoadOp::kDiscard);
        this->setInitialStencilContent(GrOpsTask::StencilContent::kDontCare);

        constexpr static GrUserStencilSettings kClearStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kAlways,
                0xffff,
                GrUserStencilOp::kReplace,
                GrUserStencilOp::kReplace,
                0xffff>());

        this->stencilAtlasRect(onFlushRP->recordingContext(), atlasRect, SK_PMColor4fTRANSPARENT,
                               &kClearStencil);
    } else {
        this->setColorLoadOp(GrLoadOp::kClear);
        this->setInitialStencilContent(GrOpsTask::StencilContent::kUserBitsCleared);
    }
    this->target(0)->asRenderTargetProxy()->setNeedsStencil();

    // Add ops to stencil the atlas paths.
    for (auto antialias : {false, true}) {
        for (auto fillType : {SkPathFillType::kWinding, SkPathFillType::kEvenOdd}) {
            SkPath* uberPath = this->getUberPath(fillType, antialias);
            if (uberPath->isEmpty()) {
                continue;
            }
            uberPath->setFillType(fillType);
            GrAAType aaType = (antialias) ? GrAAType::kMSAA : GrAAType::kNone;
            auto op = GrOp::Make<GrPathStencilCoverOp>(
                    onFlushRP->recordingContext(), SkMatrix::I(), *uberPath, GrPaint(), aaType,
                    GrTessellationPathRenderer::PathFlags::kStencilOnly, atlasRect);
            this->addAtlasDrawOp(std::move(op), antialias, caps);
        }
    }

    // Finally, draw a fullscreen rect to cover our stencilled paths and convert them into alpha
    // coverage masks.
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

        // This is the final op in the surfaceDrawContext. Since Ganesh is planning to discard the
        // stencil values anyway, there is no need to reset the stencil values back to 0.
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
        atlasRect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    }
    this->stencilAtlasRect(onFlushRP->recordingContext(), atlasRect, SK_PMColor4fWHITE, stencil);

    this->setClippedContentBounds(SkIRect::MakeSize(fDynamicAtlas->drawBounds()));
}

void GrAtlasRenderTask::stencilAtlasRect(GrRecordingContext* rContext, const SkRect& rect,
                                         const SkPMColor4f& color,
                                         const GrUserStencilSettings* stencil) {
    GrPaint paint;
    paint.setColor4f(color);
    paint.setXPFactory(SkBlendMode_AsXPFactory(SkBlendMode::kSrc));
    GrQuad quad(rect);
    DrawQuad drawQuad{quad, quad, GrQuadAAFlags::kAll};
    auto op = GrFillRectOp::Make(rContext, std::move(paint), GrAAType::kMSAA, &drawQuad, stencil);
    this->addAtlasDrawOp(std::move(op), true/*usesMSAA*/, *rContext->priv().caps());
}

void GrAtlasRenderTask::addAtlasDrawOp(GrOp::Owner op, bool usesMSAA, const GrCaps& caps) {
    auto drawOp = static_cast<GrDrawOp*>(op.get());
#ifdef SK_DEBUG
    drawOp->fAddDrawOpCalled = true;
    drawOp->visitProxies([&](GrSurfaceProxy* p, GrMipmapped mipmapped) {
        SK_ABORT("Atlas ops cannot have proxies.");
    });
#endif

    auto processorAnalysis = drawOp->finalize(caps, nullptr,
                                              GrColorTypeClampType(fDynamicAtlas->colorType()));
    SkASSERT(!processorAnalysis.requiresDstTexture());
    SkASSERT(!processorAnalysis.usesNonCoherentHWBlending());

    drawOp->setClippedBounds(drawOp->bounds());
    this->recordOp(std::move(op), usesMSAA, processorAnalysis, nullptr, nullptr, caps);
}

bool GrAtlasRenderTask::onExecute(GrOpFlushState* flushState) {
    if (!this->GrOpsTask::onExecute(flushState)) {
        return false;
    }
    if (this->target(0)->requiresManualMSAAResolve()) {
        auto nativeRect = GrNativeRect::MakeIRectRelativeTo(
                GrDynamicAtlas::kTextureOrigin,
                this->target(0)->backingStoreDimensions().height(),
                SkIRect::MakeSize(fDynamicAtlas->drawBounds()));
        flushState->gpu()->resolveRenderTarget(this->target(0)->peekRenderTarget(), nativeRect);
    }
    return true;
}
