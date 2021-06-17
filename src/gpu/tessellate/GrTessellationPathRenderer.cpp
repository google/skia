/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

#include "include/private/SkVx.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/generated/GrDeviceSpaceEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrPathInnerTriangulateOp.h"
#include "src/gpu/tessellate/GrPathStencilCoverOp.h"
#include "src/gpu/tessellate/GrPathTessellateOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"
#include "src/gpu/tessellate/shaders/GrModulateAtlasCoverageFP.h"

constexpr static SkISize kAtlasInitialSize{512, 512};
constexpr static int kMaxAtlasSize = 2048;

constexpr static auto kAtlasAlpha8Type = GrColorType::kAlpha_8;

// The atlas is only used for small-area paths, which means at least one dimension of every path is
// guaranteed to be quite small. So if we transpose tall paths, then every path will have a small
// height, which lends very well to efficient pow2 atlas packing.
constexpr static auto kAtlasAlgorithm = GrDynamicAtlas::RectanizerAlgorithm::kPow2;

// Ensure every path in the atlas falls in or below the 128px high rectanizer band.
constexpr static int kMaxAtlasPathHeight = 128;

bool GrTessellationPathRenderer::IsSupported(const GrCaps& caps) {
    return !caps.avoidStencilBuffers() &&
           caps.drawInstancedSupport() &&
           caps.shaderCaps()->vertexIDSupport() &&
           !caps.disableTessellationPathRenderer();
}

GrTessellationPathRenderer::GrTessellationPathRenderer(GrRecordingContext* rContext)
        : fAtlas(kAtlasAlpha8Type, GrDynamicAtlas::InternalMultisample::kYes, kAtlasInitialSize,
                 std::min(kMaxAtlasSize, rContext->priv().caps()->maxPreferredRenderTargetSize()),
                 *rContext->priv().caps(), kAtlasAlgorithm) {
    const GrCaps& caps = *rContext->priv().caps();
    auto atlasFormat = caps.getDefaultBackendFormat(kAtlasAlpha8Type, GrRenderable::kYes);
    if (rContext->asDirectContext() &&  // The atlas doesn't support DDL yet.
        caps.internalMultisampleCount(atlasFormat) > 1) {
        fMaxAtlasPathWidth = fAtlas.maxAtlasSize() / 2;  // Enable the atlas.
    }
}

GrPathRenderer::StencilSupport GrTessellationPathRenderer::onGetStencilSupport(
        const GrStyledShape& shape) const {
    if (!shape.style().isSimpleFill()) {
        // Don't bother with stroke stencilling yet. Skia probably shouldn't support this at all
        // since you can't clip by a stroke.
        return kNoSupport_StencilSupport;
    }
    return shape.knownToBeConvex() ? kNoRestriction_StencilSupport : kStencilOnly_StencilSupport;
}

GrPathRenderer::CanDrawPath GrTessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    const GrStyledShape& shape = *args.fShape;
    if (args.fAAType == GrAAType::kCoverage ||
        shape.style().hasPathEffect() ||
        args.fViewMatrix->hasPerspective() ||
        shape.style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style ||
        shape.inverseFilled() ||
        !args.fProxy->canUseStencil(*args.fCaps)) {
        return CanDrawPath::kNo;
    }
    if (args.fHasUserStencilSettings) {
        // Non-convex paths and strokes use the stencil buffer internally, so they can't support
        // draws with stencil settings.
        if (!shape.style().isSimpleFill() || !shape.knownToBeConvex()) {
            return CanDrawPath::kNo;
        }
    }
    return CanDrawPath::kYes;
}

static GrOp::Owner make_non_convex_fill_op(GrRecordingContext* rContext,
                                           GrTessellationPathRenderer::PathFlags pathFlags,
                                           GrAAType aaType, const SkRect& pathDevBounds,
                                           const SkMatrix& viewMatrix, const SkPath& path,
                                           GrPaint&& paint) {
    SkASSERT(!path.isConvex());
    int numVerbs = path.countVerbs();
    if (numVerbs > 0) {
        // Check if the path is large and/or simple enough that we can triangulate the inner fan
        // on the CPU. This is our fastest approach. It allows us to stencil only the curves,
        // and then fill the inner fan directly to the final render target, thus drawing the
        // majority of pixels in a single render pass.
        float gpuFragmentWork = pathDevBounds.height() * pathDevBounds.width();
        float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
        constexpr static float kCpuWeight = 512;
        constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
        if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
            return GrOp::Make<GrPathInnerTriangulateOp>(rContext, viewMatrix, path,
                                                        std::move(paint), aaType, pathFlags,
                                                        pathDevBounds);
        }
    }
    return GrOp::Make<GrPathStencilCoverOp>(rContext, viewMatrix, path, std::move(paint), aaType,
                                            pathFlags, pathDevBounds);
}

bool GrTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;

    SkPath path;
    args.fShape->asPath(&path);

    // Handle strokes first.
    if (!args.fShape->style().isSimpleFill()) {
        SkASSERT(args.fUserStencilSettings->isUnused());
        const SkStrokeRec& stroke = args.fShape->style().strokeRec();
        SkASSERT(stroke.getStyle() != SkStrokeRec::kStrokeAndFill_Style);
        auto op = GrOp::Make<GrStrokeTessellateOp>(args.fContext, args.fAAType, *args.fViewMatrix,
                                                   path, stroke, std::move(args.fPaint));
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    SkRect pathDevBounds;
    args.fViewMatrix->mapRect(&pathDevBounds, args.fShape->bounds());

    // See if the path is small and simple enough to atlas instead of drawing directly.
    //
    // NOTE: The atlas uses alpha8 coverage even for msaa render targets. We could theoretically
    // render the sample mask to an integer texture, but such a scheme would probably require
    // GL_EXT_post_depth_coverage, which appears to have low adoption.
    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    if (args.fUserStencilSettings->isUnused() &&
        this->tryAddPathToAtlas(*args.fContext->priv().caps(), *args.fViewMatrix, path,
                                pathDevBounds, args.fAAType != GrAAType::kNone, &devIBounds,
                                &locationInAtlas, &transposedInAtlas)) {
        // The atlas is not compatible with DDL. We should only be using it on direct contexts.
        SkASSERT(args.fContext->asDirectContext());
        auto op = GrOp::Make<GrDrawAtlasPathOp>(args.fContext, surfaceDrawContext->numSamples(),
                                                sk_ref_sp(fAtlas.textureProxy()), devIBounds,
                                                locationInAtlas, transposedInAtlas,
                                                *args.fViewMatrix, std::move(args.fPaint));
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    // Handle convex paths only if we couldn't fit them in the atlas. We give the atlas priority in
    // an effort to reduce DMSAA triggers.
    if (args.fShape->knownToBeConvex()) {
        auto op = GrOp::Make<GrPathTessellateOp>(args.fContext, *args.fViewMatrix, path,
                                                 std::move(args.fPaint), args.fAAType,
                                                 args.fUserStencilSettings, pathDevBounds);
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    SkASSERT(args.fUserStencilSettings->isUnused());  // See onGetStencilSupport().
    auto op = make_non_convex_fill_op(args.fContext, PathFlags::kNone, args.fAAType, pathDevBounds,
                                      *args.fViewMatrix, path, std::move(args.fPaint));
    surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

void GrTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkASSERT(args.fShape->style().isSimpleFill());  // See onGetStencilSupport().

    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;
    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    SkRect pathDevBounds;
    args.fViewMatrix->mapRect(&pathDevBounds, args.fShape->bounds());

    SkPath path;
    args.fShape->asPath(&path);

    if (args.fShape->knownToBeConvex()) {
        constexpr static GrUserStencilSettings kMarkStencil(
            GrUserStencilSettings::StaticInit<
                0x0001,
                GrUserStencilTest::kAlways,
                0xffff,
                GrUserStencilOp::kReplace,
                GrUserStencilOp::kKeep,
                0xffff>());

        GrPaint stencilPaint;
        stencilPaint.setXPFactory(GrDisableColorXPFactory::Get());
        auto op = GrOp::Make<GrPathTessellateOp>(args.fContext, *args.fViewMatrix, path,
                                                 std::move(stencilPaint), aaType, &kMarkStencil,
                                                 pathDevBounds);
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return;
    }

    auto op = make_non_convex_fill_op(args.fContext, PathFlags::kStencilOnly, aaType, pathDevBounds,
                                      *args.fViewMatrix, path, GrPaint());
    surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
}

GrFPResult GrTessellationPathRenderer::makeAtlasClipFP(const SkIRect& drawBounds,
                                                       const SkMatrix& viewMatrix,
                                                       const SkPath& path, GrAA aa,
                                                       std::unique_ptr<GrFragmentProcessor> inputFP,
                                                       const GrCaps& caps) {
    if (viewMatrix.hasPerspective()) {
        return GrFPFailure(std::move(inputFP));
    }
    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    // tryAddPathToAtlas() ignores inverseness of the fill. See getAtlasUberPath().
    if (!this->tryAddPathToAtlas(caps, viewMatrix, path, viewMatrix.mapRect(path.getBounds()),
                                 aa != GrAA::kNo, &devIBounds, &locationInAtlas,
                                 &transposedInAtlas)) {
        // The path is too big, or the atlas ran out of room.
        return GrFPFailure(std::move(inputFP));
    }
    SkMatrix atlasMatrix;
    auto [atlasX, atlasY] = locationInAtlas;
    if (!transposedInAtlas) {
        atlasMatrix = SkMatrix::Translate(atlasX - devIBounds.left(), atlasY - devIBounds.top());
    } else {
        atlasMatrix.setAll(0, 1, atlasX - devIBounds.top(),
                           1, 0, atlasY - devIBounds.left(),
                           0, 0, 1);
    }
    auto flags = GrModulateAtlasCoverageFP::Flags::kNone;
    if (path.isInverseFillType()) {
        flags |= GrModulateAtlasCoverageFP::Flags::kInvertCoverage;
    }
    if (!devIBounds.contains(drawBounds)) {
        flags |= GrModulateAtlasCoverageFP::Flags::kCheckBounds;
        // At this point in time we expect callers to tighten the scissor for "kIntersect" clips, as
        // opposed to us having to check the path bounds. Feel free to remove this assert if that
        // ever changes.
        SkASSERT(path.isInverseFillType());
    }
    return GrFPSuccess(std::make_unique<GrModulateAtlasCoverageFP>(flags, std::move(inputFP),
                                                                   fAtlas.surfaceProxyView(caps),
                                                                   atlasMatrix, devIBounds));
}

void GrTessellationPathRenderer::AtlasPathKey::set(const SkMatrix& m, bool antialias,
                                                   const SkPath& path) {
    using grvx::float2;
    fAffineMatrix[0] = m.getScaleX();
    fAffineMatrix[1] = m.getSkewX();
    fAffineMatrix[2] = m.getSkewY();
    fAffineMatrix[3] = m.getScaleY();
    float2 translate = {m.getTranslateX(), m.getTranslateY()};
    float2 subpixelPosition = translate - skvx::floor(translate);
    float2 subpixelPositionKey = skvx::trunc(subpixelPosition *
                                             GrPathTessellator::kLinearizationPrecision);
    skvx::cast<uint8_t>(subpixelPositionKey).store(fSubpixelPositionKey);
    fAntialias = antialias;
    fFillRule = (uint8_t)GrFillRuleForSkPath(path);  // Fill rule doesn't affect the path's genID.
    fPathGenID = path.getGenerationID();
}

bool GrTessellationPathRenderer::tryAddPathToAtlas(const GrCaps& caps, const SkMatrix& viewMatrix,
                                                   const SkPath& path, const SkRect& pathDevBounds,
                                                   bool antialias, SkIRect* devIBounds,
                                                   SkIPoint16* locationInAtlas,
                                                   bool* transposedInAtlas) {
    SkASSERT(!viewMatrix.hasPerspective());  // See onCanDrawPath().

    if (!fMaxAtlasPathWidth) {
        return false;
    }

    if (!caps.multisampleDisableSupport() && !antialias) {
        return false;
    }

    // Transpose tall paths in the atlas. Since we limit ourselves to small-area paths, this
    // guarantees that every atlas entry has a small height, which lends very well to efficient pow2
    // atlas packing.
    pathDevBounds.roundOut(devIBounds);
    int maxDimenstion = devIBounds->width();
    int minDimension = devIBounds->height();
    *transposedInAtlas = minDimension > maxDimenstion;
    if (*transposedInAtlas) {
        std::swap(minDimension, maxDimenstion);
    }

    // Check if the path is too large for an atlas. Since we transpose paths in the atlas so height
    // is always "minDimension", limiting to kMaxAtlasPathHeight^2 pixels guarantees height <=
    // kMaxAtlasPathHeight, while also allowing paths that are very wide and short.
    if ((uint64_t)maxDimenstion * minDimension > kMaxAtlasPathHeight * kMaxAtlasPathHeight ||
        maxDimenstion > fMaxAtlasPathWidth) {
        return false;
    }

    // Check if this path is already in the atlas. This is mainly for clip paths.
    AtlasPathKey atlasPathKey;
    if (!path.isVolatile()) {
        atlasPathKey.set(viewMatrix, antialias, path);
        if (const SkIPoint16* existingLocation = fAtlasPathCache.find(atlasPathKey)) {
            *locationInAtlas = *existingLocation;
            return true;
        }
    }

    if (!fAtlas.addRect(maxDimenstion, minDimension, locationInAtlas)) {
        return false;
    }

    // Remember this path's location in the atlas, in case it gets drawn again.
    if (!path.isVolatile()) {
        fAtlasPathCache.set(atlasPathKey, *locationInAtlas);
    }

    SkMatrix atlasMatrix = viewMatrix;
    if (*transposedInAtlas) {
        std::swap(atlasMatrix[0], atlasMatrix[3]);
        std::swap(atlasMatrix[1], atlasMatrix[4]);
        float tx=atlasMatrix.getTranslateX(), ty=atlasMatrix.getTranslateY();
        atlasMatrix.setTranslateX(ty - devIBounds->y() + locationInAtlas->x());
        atlasMatrix.setTranslateY(tx - devIBounds->x() + locationInAtlas->y());
    } else {
        atlasMatrix.postTranslate(locationInAtlas->x() - devIBounds->x(),
                                  locationInAtlas->y() - devIBounds->y());
    }

    // Concatenate this path onto our uber path that matches its fill and AA types.
    SkPath* uberPath = this->getAtlasUberPath(path.getFillType(), antialias);
    uberPath->moveTo(locationInAtlas->x(), locationInAtlas->y());  // Implicit moveTo(0,0).
    uberPath->addPath(path, atlasMatrix);
    return true;
}

void GrTessellationPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                          SkSpan<const uint32_t> /* taskIDs */) {
    if (!fAtlas.drawBounds().isEmpty()) {
        this->renderAtlas(onFlushRP);
        fAtlas.reset(kAtlasInitialSize, *onFlushRP->caps());
    }
    for (SkPath& path : fAtlasUberPaths) {
        path.reset();
    }
    fAtlasPathCache.reset();
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

    SkRect atlasRect = SkRect::MakeIWH(fAtlas.drawBounds().width(), fAtlas.drawBounds().height());

    // Add ops to stencil the atlas paths.
    for (auto antialias : {false, true}) {
        for (auto fillType : {SkPathFillType::kWinding, SkPathFillType::kEvenOdd}) {
            SkPath* uberPath = this->getAtlasUberPath(fillType, antialias);
            if (uberPath->isEmpty()) {
                continue;
            }
            uberPath->setFillType(fillType);
            GrAAType aaType = (antialias) ? GrAAType::kMSAA : GrAAType::kNone;
            auto op = GrOp::Make<GrPathStencilCoverOp>(onFlushRP->recordingContext(), SkMatrix::I(),
                                                       *uberPath, GrPaint(), aaType,
                                                       PathFlags::kStencilOnly, atlasRect);
            rtc->addDrawOp(nullptr, std::move(op));
        }
    }

    // Finally, draw a fullscreen rect to convert our stencilled paths into alpha coverage masks.
    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    const GrUserStencilSettings* stencil;
    if (onFlushRP->caps()->discardStencilValuesAfterRenderPass()) {
        // This is the final op in the surfaceDrawContext. Since Ganesh is planning to discard the
        // stencil values anyway, there is no need to reset the stencil values back to 0.
        stencil = &kTestStencil;
    } else {
        // Outset the cover rect in case there are T-junctions in the path bounds.
        atlasRect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    }
    rtc->stencilRect(nullptr, stencil, std::move(paint), GrAA::kYes, SkMatrix::I(), atlasRect);

    if (rtc->asSurfaceProxy()->requiresManualMSAAResolve()) {
        onFlushRP->addTextureResolveTask(sk_ref_sp(rtc->asTextureProxy()),
                                         GrSurfaceProxy::ResolveFlags::kMSAA);
    }
}
