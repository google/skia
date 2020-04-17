/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrTessellatePathOp.h"

constexpr static SkISize kAtlasInitialSize{512, 512};
constexpr static int kMaxAtlasSize = 2048;

GrTessellationPathRenderer::GrTessellationPathRenderer(const GrCaps& caps) : fAtlas(
        GrColorType::kAlpha_8, GrDynamicAtlas::InternalMultisample::kYes, kAtlasInitialSize,
        std::min(kMaxAtlasSize, caps.maxPreferredRenderTargetSize()), caps) {
}

GrPathRenderer::CanDrawPath GrTessellationPathRenderer::onCanDrawPath(
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

bool GrTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GrRenderTargetContext* renderTargetContext = args.fRenderTargetContext;
    GrOpMemoryPool* pool = args.fContext->priv().opMemoryPool();
    SkPath path;
    args.fShape->asPath(&path);

    // See if the path is small and simple enough to atlas instead of drawing directly.
    //
    // NOTE: The atlas uses alpha8 coverage even for msaa render targets. We could theoretically
    // render the sample mask to an integer texture, but such a scheme would probably require
    // GL_EXT_post_depth_coverage, which appears to have low adoption.
    SkIRect devIBounds;
    SkIVector devToAtlasOffset;
    if (this->tryAddPathToAtlas(*args.fContext->priv().caps(), *args.fViewMatrix, path,
                                args.fAAType, &devIBounds, &devToAtlasOffset)) {
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

bool GrTessellationPathRenderer::tryAddPathToAtlas(
        const GrCaps& caps, const SkMatrix& viewMatrix, const SkPath& path, GrAAType aaType,
        SkIRect* devIBounds, SkIVector* devToAtlasOffset) {
    if (!caps.multisampleDisableSupport() && GrAAType::kNone == aaType) {
        return false;
    }

    // Atlas paths require their points to be transformed on CPU. Check if the path has too many
    // points to justify this CPU transformation.
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

    SkMatrix atlasMatrix = viewMatrix;
    atlasMatrix.postTranslate(devToAtlasOffset->x(), devToAtlasOffset->y());

    // Concatenate this path onto our uber path that matches its fill and AA types.
    SkPath* uberPath = this->getAtlasUberPath(path.getFillType(), GrAAType::kNone != aaType);
    uberPath->moveTo(devToAtlasOffset->x(), devToAtlasOffset->y());  // Implicit moveTo(0,0).
    uberPath->addPath(path, atlasMatrix);
    return true;
}

void GrTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkPath path;
    args.fShape->asPath(&path);

    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    auto op = args.fContext->priv().opMemoryPool()->allocate<GrTessellatePathOp>(
            *args.fViewMatrix, path, GrPaint(), aaType, GrTessellatePathOp::Flags::kStencilOnly);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
}

void GrTessellationPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                          const uint32_t* opsTaskIDs, int numOpsTaskIDs) {
    if (!fAtlas.drawBounds().isEmpty()) {
        this->renderAtlas(onFlushRP);
        fAtlas.reset(kAtlasInitialSize, *onFlushRP->caps());
    }
    for (SkPath& path : fAtlasUberPaths) {
        path.reset();
    }
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

void GrTessellationPathRenderer::renderAtlas(GrOnFlushResourceProvider* onFlushRP) {
    auto rtc = fAtlas.instantiate(onFlushRP);
    if (!rtc) {
        return;
    }

    // Add ops to stencil the atlas paths.
    for (auto antialias : {false, true}) {
        for (auto fillType : {SkPathFillType::kWinding, SkPathFillType::kEvenOdd}) {
            SkPath* uberPath = this->getAtlasUberPath(fillType, antialias);
            if (uberPath->isEmpty()) {
                continue;
            }
            uberPath->setFillType(fillType);
            GrAAType aaType = (antialias) ? GrAAType::kMSAA : GrAAType::kNone;
            auto op = onFlushRP->opMemoryPool()->allocate<GrTessellatePathOp>(
                    SkMatrix::I(), *uberPath, GrPaint(), aaType,
                    GrTessellatePathOp::Flags::kStencilOnly);
            rtc->addDrawOp(GrNoClip(), std::move(op));
        }
    }

    // Finally, draw a fullscreen rect to convert our stencilled paths into alpha coverage masks.
    auto fillRectFlags = GrFillRectOp::InputFlags::kNone;

    // This will be the final op in the renderTargetContext. So if Ganesh is planning to discard the
    // stencil values anyway, then we might not actually need to reset the stencil values back to 0.
    bool mustResetStencil = !onFlushRP->caps()->discardStencilValuesAfterRenderPass();

    if (rtc->numSamples() <= 1) {
        // We are mixed sampled. We need to enable conservative raster and ensure stencil values get
        // reset in order to avoid artifacts along the diagonal of the atlas.
        fillRectFlags |= GrFillRectOp::InputFlags::kConservativeRaster;
        mustResetStencil = true;
    }

    SkRect coverRect = SkRect::MakeIWH(fAtlas.drawBounds().width(), fAtlas.drawBounds().height());
    const GrUserStencilSettings* stencil;
    if (mustResetStencil) {
        // Outset the cover rect in case there are T-junctions in the path bounds.
        coverRect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    } else {
        stencil = &kTestStencil;
    }

    GrQuad coverQuad(coverRect);
    DrawQuad drawQuad{coverQuad, coverQuad, GrQuadAAFlags::kAll};

    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);

    auto coverOp = GrFillRectOp::Make(rtc->surfPriv().getContext(), std::move(paint),
                                      GrAAType::kMSAA, &drawQuad, stencil, fillRectFlags);
    rtc->addDrawOp(GrNoClip(), std::move(coverOp));

    if (rtc->asSurfaceProxy()->requiresManualMSAAResolve()) {
        onFlushRP->addTextureResolveTask(sk_ref_sp(rtc->asTextureProxy()),
                                         GrSurfaceProxy::ResolveFlags::kMSAA);
    }
}
