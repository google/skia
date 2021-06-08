/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

#include "include/pathops/SkPathOps.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/geometry/GrWangsFormula.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "src/gpu/tessellate/GrDrawAtlasPathOp.h"
#include "src/gpu/tessellate/GrPathInnerTriangulateOp.h"
#include "src/gpu/tessellate/GrPathStencilCoverOp.h"
#include "src/gpu/tessellate/GrStrokeTessellateOp.h"

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

GrPathRenderer::CanDrawPath GrTessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    const GrStyledShape& shape = *args.fShape;
    if (args.fAAType == GrAAType::kCoverage ||
        shape.style().hasPathEffect() ||
        args.fViewMatrix->hasPerspective() ||
        shape.style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style ||
        shape.inverseFilled() ||
        args.fHasUserStencilSettings ||
        !args.fProxy->canUseStencil(*args.fCaps)) {
        return CanDrawPath::kNo;
    }
    if (shape.style().strokeRec().getStyle() != SkStrokeRec::kStroke_Style) {
        // On platforms that don't have native support for indirect draws and/or hardware
        // tessellation, we find that the default path renderer can draw fills faster sometimes. Let
        // fills fall through to the default renderer on these platforms for now.
        // (crbug.com/1163441, skbug.com/11138, skbug.com/11139)
        if (!args.fCaps->nativeDrawIndirectSupport() &&
            !args.fCaps->shaderCaps()->tessellationSupport() &&
            // Is the path cacheable? TODO: This check is outdated. Remove it next.
            shape.hasUnstyledKey()) {
            return CanDrawPath::kNo;
        }
    }
    return CanDrawPath::kYes;
}

static GrOp::Owner make_op(GrRecordingContext* rContext, const GrSurfaceContext* surfaceContext,
                           GrTessellationPathRenderer::PathFlags pathFlags, GrAAType aaType,
                           const SkRect& shapeDevBounds, const SkMatrix& viewMatrix,
                           const GrStyledShape& shape, GrPaint&& paint) {
    constexpr static auto kLinearizationPrecision =
            GrTessellationPathRenderer::kLinearizationPrecision;
    constexpr static auto kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;
    SkPath path;
    shape.asPath(&path);

    // Find the worst-case log2 number of line segments that a curve in this path might need to be
    // divided into.
    int worstCaseResolveLevel = GrWangsFormula::worst_case_cubic_log2(kLinearizationPrecision,
                                                                      shapeDevBounds.width(),
                                                                      shapeDevBounds.height());
    if (worstCaseResolveLevel > kMaxResolveLevel) {
        // The path is too large for our internal indirect draw shaders. Crop it to the viewport.
        auto viewport = SkRect::MakeIWH(surfaceContext->width(), surfaceContext->height());
        float inflationRadius = 1;
        const SkStrokeRec& stroke = shape.style().strokeRec();
        if (stroke.getStyle() == SkStrokeRec::kHairline_Style) {
            inflationRadius += SkStrokeRec::GetInflationRadius(stroke.getJoin(), stroke.getMiter(),
                                                               stroke.getCap(), 1);
        } else if (stroke.getStyle() != SkStrokeRec::kFill_Style) {
            inflationRadius += stroke.getInflationRadius() * viewMatrix.getMaxScale();
        }
        viewport.outset(inflationRadius, inflationRadius);

        SkPath viewportPath;
        viewportPath.addRect(viewport);
        // Perform the crop in device space so it's a simple rect-path intersection.
        path.transform(viewMatrix);
        if (!Op(viewportPath, path, kIntersect_SkPathOp, &path)) {
            // The crop can fail if the PathOps encounter NaN or infinities. Return true
            // because drawing nothing is acceptable behavior for FP overflow.
            return nullptr;
        }

        // Transform the path back to its own local space.
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            return nullptr;  // Singular view matrix. Nothing would have drawn anyway. Return null.
        }
        path.transform(inverse);
        path.setIsVolatile(true);

        SkRect newDevBounds;
        viewMatrix.mapRect(&newDevBounds, path.getBounds());
        worstCaseResolveLevel = GrWangsFormula::worst_case_cubic_log2(kLinearizationPrecision,
                                                                      newDevBounds.width(),
                                                                      newDevBounds.height());
        // kMaxResolveLevel should be large enough to tessellate paths the size of any screen we
        // might encounter.
        SkASSERT(worstCaseResolveLevel <= kMaxResolveLevel);
    }

    if (!shape.style().isSimpleFill()) {
        const SkStrokeRec& stroke = shape.style().strokeRec();
        SkASSERT(stroke.getStyle() != SkStrokeRec::kStrokeAndFill_Style);
        return GrOp::Make<GrStrokeTessellateOp>(rContext, aaType, viewMatrix, path, stroke,
                                                std::move(paint));
    } else {
        SkRect devBounds;
        viewMatrix.mapRect(&devBounds, path.getBounds());
        int numVerbs = path.countVerbs();
        if (numVerbs > 0) {
            // Check if the path is large and/or simple enough that we can triangulate the inner fan
            // on the CPU. This is our fastest approach. It allows us to stencil only the curves,
            // and then fill the inner fan directly to the final render target, thus drawing the
            // majority of pixels in a single render pass.
            float gpuFragmentWork = devBounds.height() * devBounds.width();
            float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
            constexpr static float kCpuWeight = 512;
            constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
            if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
                return GrOp::Make<GrPathInnerTriangulateOp>(rContext, viewMatrix, path,
                                                            std::move(paint), aaType, pathFlags,
                                                            devBounds);
            }
        }
        return GrOp::Make<GrPathStencilCoverOp>(rContext, viewMatrix, path, std::move(paint),
                                                aaType, pathFlags, devBounds);
    }
}

bool GrTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;

    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, args.fShape->bounds());

    // See if the path is small and simple enough to atlas instead of drawing directly.
    //
    // NOTE: The atlas uses alpha8 coverage even for msaa render targets. We could theoretically
    // render the sample mask to an integer texture, but such a scheme would probably require
    // GL_EXT_post_depth_coverage, which appears to have low adoption.
    SkIRect devIBounds;
    SkIPoint16 locationInAtlas;
    bool transposedInAtlas;
    if (this->tryAddPathToAtlas(*args.fContext->priv().caps(), *args.fViewMatrix, *args.fShape,
                                devBounds, args.fAAType, &devIBounds, &locationInAtlas,
                                &transposedInAtlas)) {
        // The atlas is not compatible with DDL. We should only be using it on direct contexts.
        SkASSERT(args.fContext->asDirectContext());
        auto op = GrOp::Make<GrDrawAtlasPathOp>(args.fContext,
                surfaceDrawContext->numSamples(), sk_ref_sp(fAtlas.textureProxy()),
                devIBounds, locationInAtlas, transposedInAtlas, *args.fViewMatrix,
                std::move(args.fPaint));
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    if (auto op = make_op(args.fContext, surfaceDrawContext, PathFlags::kNone, args.fAAType,
                          devBounds, *args.fViewMatrix, *args.fShape, std::move(args.fPaint))) {
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    }
    return true;
}

bool GrTessellationPathRenderer::tryAddPathToAtlas(
        const GrCaps& caps, const SkMatrix& viewMatrix, const GrStyledShape& shape,
        const SkRect& devBounds, GrAAType aaType, SkIRect* devIBounds, SkIPoint16* locationInAtlas,
        bool* transposedInAtlas) {
    if (!shape.style().isSimpleFill()) {
        return false;
    }

    if (!fMaxAtlasPathWidth) {
        return false;
    }

    if (!caps.multisampleDisableSupport() && GrAAType::kNone == aaType) {
        return false;
    }

    // Transpose tall paths in the atlas. Since we limit ourselves to small-area paths, this
    // guarantees that every atlas entry has a small height, which lends very well to efficient pow2
    // atlas packing.
    devBounds.roundOut(devIBounds);
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

    if (!fAtlas.addRect(maxDimenstion, minDimension, locationInAtlas)) {
        return false;
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
    SkPath path;
    shape.asPath(&path);
    SkPath* uberPath = this->getAtlasUberPath(path.getFillType(), GrAAType::kNone != aaType);
    uberPath->moveTo(locationInAtlas->x(), locationInAtlas->y());  // Implicit moveTo(0,0).
    uberPath->addPath(path, atlasMatrix);
    return true;
}

void GrTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GrSurfaceDrawContext* surfaceDrawContext = args.fSurfaceDrawContext;
    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;
    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, args.fShape->bounds());
    if (auto op = make_op(args.fContext, surfaceDrawContext, PathFlags::kStencilOnly, aaType,
                          devBounds, *args.fViewMatrix, *args.fShape, GrPaint())) {
        surfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    }
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
