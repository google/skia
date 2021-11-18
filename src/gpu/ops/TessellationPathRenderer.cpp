/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/TessellationPathRenderer.h"

#include "include/private/SkVx.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/PathInnerTriangulateOp.h"
#include "src/gpu/ops/PathStencilCoverOp.h"
#include "src/gpu/ops/PathTessellateOp.h"
#include "src/gpu/ops/StrokeTessellateOp.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

namespace {

GrOp::Owner make_non_convex_fill_op(GrRecordingContext* rContext,
                                    SkArenaAlloc* arena,
                                    skgpu::v1::FillPathFlags fillPathFlags,
                                    GrAAType aaType,
                                    const SkRect& drawBounds,
                                    const SkMatrix& viewMatrix,
                                    const SkPath& path,
                                    GrPaint&& paint) {
    SkASSERT(!path.isConvex() || path.isInverseFillType());
    int numVerbs = path.countVerbs();
    if (numVerbs > 0 && !path.isInverseFillType()) {
        // Check if the path is large and/or simple enough that we can triangulate the inner fan
        // on the CPU. This is our fastest approach. It allows us to stencil only the curves,
        // and then fill the inner fan directly to the final render target, thus drawing the
        // majority of pixels in a single render pass.
        float gpuFragmentWork = drawBounds.height() * drawBounds.width();
        float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
        constexpr static float kCpuWeight = 512;
        constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
        if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
            return GrOp::Make<skgpu::v1::PathInnerTriangulateOp>(rContext,
                                                                 viewMatrix,
                                                                 path,
                                                                 std::move(paint),
                                                                 aaType,
                                                                 fillPathFlags,
                                                                 drawBounds);
        }
    }
    return GrOp::Make<skgpu::v1::PathStencilCoverOp>(rContext,
                                                     arena,
                                                     viewMatrix,
                                                     path,
                                                     std::move(paint),
                                                     aaType,
                                                     fillPathFlags,
                                                     drawBounds);
}

} // anonymous namespace

namespace skgpu::v1 {

bool TessellationPathRenderer::IsSupported(const GrCaps& caps) {
    return !caps.avoidStencilBuffers() &&
           caps.drawInstancedSupport() &&
           !caps.disableTessellationPathRenderer();
}

PathRenderer::StencilSupport TessellationPathRenderer::onGetStencilSupport(
        const GrStyledShape& shape) const {
    if (!shape.style().isSimpleFill() || shape.inverseFilled()) {
        // Don't bother with stroke stencilling or inverse fills yet. The Skia API doesn't support
        // clipping by a stroke, and the stencilling code already knows how to invert a fill.
        return kNoSupport_StencilSupport;
    }
    return shape.knownToBeConvex() ? kNoRestriction_StencilSupport : kStencilOnly_StencilSupport;
}

PathRenderer::CanDrawPath TessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    const GrStyledShape& shape = *args.fShape;
    if (args.fAAType == GrAAType::kCoverage ||
        shape.style().hasPathEffect() ||
        args.fViewMatrix->hasPerspective() ||
        shape.style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style ||
        !args.fProxy->canUseStencil(*args.fCaps)) {
        return CanDrawPath::kNo;
    }
    if (!shape.style().isSimpleFill()) {
        if (shape.inverseFilled()) {
            return CanDrawPath::kNo;
        }
        if (shape.style().strokeRec().getWidth() * args.fViewMatrix->getMaxScale() > 10000) {
            // crbug.com/1266446 -- Don't draw massively wide strokes with the tessellator. Since we
            // outset the viewport by stroke width for pre-chopping, astronomically wide strokes can
            // result in an astronomical viewport size, and therefore an exponential explosion chops
            // and memory usage. It is also simply inefficient to tessellate these strokes due to
            // the number of radial edges required. We're better off just converting them to a path
            // after a certain point.
            return CanDrawPath::kNo;
        }
    }
    if (args.fHasUserStencilSettings) {
        // Non-convex paths and strokes use the stencil buffer internally, so they can't support
        // draws with stencil settings.
        if (!shape.style().isSimpleFill() || !shape.knownToBeConvex() || shape.inverseFilled()) {
            return CanDrawPath::kNo;
        }
    }
    return CanDrawPath::kYes;
}

bool TessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    auto sdc = args.fSurfaceDrawContext;

    SkPath path;
    args.fShape->asPath(&path);

    const SkRect pathDevBounds = args.fViewMatrix->mapRect(args.fShape->bounds());
    float n4 = wangs_formula::worst_case_cubic_pow4(kTessellationPrecision,
                                                    pathDevBounds.width(),
                                                    pathDevBounds.height());
    if (n4 > pow4(kMaxTessellationSegmentsPerCurve)) {
        // The path is extremely large. Pre-chop its curves to keep the number of tessellation
        // segments tractable. This will also flatten curves that fall completely outside the
        // viewport.
        SkRect viewport = SkRect::Make(*args.fClipConservativeBounds);
        if (!args.fShape->style().isSimpleFill()) {
            // Outset the viewport to pad for the stroke width.
            const SkStrokeRec& stroke = args.fShape->style().strokeRec();
            float inflationRadius;
            if (stroke.isHairlineStyle()) {
                // SkStrokeRec::getInflationRadius() doesn't handle hairlines robustly. Instead
                // find the inflation of an equivalent stroke in device space with a width of 1.
                inflationRadius = SkStrokeRec::GetInflationRadius(stroke.getJoin(),
                                                                  stroke.getMiter(),
                                                                  stroke.getCap(), 1);
            } else {
                inflationRadius = stroke.getInflationRadius() * args.fViewMatrix->getMaxScale();
            }
            viewport.outset(inflationRadius, inflationRadius);
        }
        path = PreChopPathCurves(kTessellationPrecision, path, *args.fViewMatrix, viewport);
    }

    // Handle strokes first.
    if (!args.fShape->style().isSimpleFill()) {
        SkASSERT(!path.isInverseFillType());  // See onGetStencilSupport().
        SkASSERT(args.fUserStencilSettings->isUnused());
        const SkStrokeRec& stroke = args.fShape->style().strokeRec();
        SkASSERT(stroke.getStyle() != SkStrokeRec::kStrokeAndFill_Style);
        auto op = GrOp::Make<StrokeTessellateOp>(args.fContext, args.fAAType, *args.fViewMatrix,
                                                 path, stroke, std::move(args.fPaint));
        sdc->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    // Handle empty paths.
    if (pathDevBounds.isEmpty()) {
        if (path.isInverseFillType()) {
            args.fSurfaceDrawContext->drawPaint(args.fClip, std::move(args.fPaint),
                                                *args.fViewMatrix);
        }
        return true;
    }

    // Handle convex paths. Make sure to check 'path' for convexity since it may have been
    // pre-chopped, not 'fShape'.
    if (path.isConvex() && !path.isInverseFillType()) {
        auto op = GrOp::Make<PathTessellateOp>(args.fContext,
                                               args.fSurfaceDrawContext->arenaAlloc(),
                                               args.fAAType,
                                               args.fUserStencilSettings,
                                               *args.fViewMatrix,
                                               path,
                                               std::move(args.fPaint),
                                               pathDevBounds);
        sdc->addDrawOp(args.fClip, std::move(op));
        return true;
    }

    SkASSERT(args.fUserStencilSettings->isUnused());  // See onGetStencilSupport().
    const SkRect& drawBounds = path.isInverseFillType()
            ? args.fSurfaceDrawContext->asSurfaceProxy()->backingStoreBoundsRect()
            : pathDevBounds;
    auto op = make_non_convex_fill_op(args.fContext,
                                      args.fSurfaceDrawContext->arenaAlloc(),
                                      FillPathFlags::kNone,
                                      args.fAAType,
                                      drawBounds,
                                      *args.fViewMatrix,
                                      path,
                                      std::move(args.fPaint));
    sdc->addDrawOp(args.fClip, std::move(op));
    return true;
}

void TessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkASSERT(args.fShape->style().isSimpleFill());  // See onGetStencilSupport().
    SkASSERT(!args.fShape->inverseFilled());  // See onGetStencilSupport().

    auto sdc = args.fSurfaceDrawContext;
    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    SkRect pathDevBounds;
    args.fViewMatrix->mapRect(&pathDevBounds, args.fShape->bounds());

    SkPath path;
    args.fShape->asPath(&path);

    float n4 = wangs_formula::worst_case_cubic_pow4(kTessellationPrecision,
                                                    pathDevBounds.width(),
                                                    pathDevBounds.height());
    if (n4 > pow4(kMaxTessellationSegmentsPerCurve)) {
        SkRect viewport = SkRect::Make(*args.fClipConservativeBounds);
        path = PreChopPathCurves(kTessellationPrecision, path, *args.fViewMatrix, viewport);
    }

    // Make sure to check 'path' for convexity since it may have been pre-chopped, not 'fShape'.
    if (path.isConvex()) {
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
        auto op = GrOp::Make<PathTessellateOp>(args.fContext,
                                               args.fSurfaceDrawContext->arenaAlloc(),
                                               aaType,
                                               &kMarkStencil,
                                               *args.fViewMatrix,
                                               path,
                                               std::move(stencilPaint),
                                               pathDevBounds);
        sdc->addDrawOp(args.fClip, std::move(op));
        return;
    }

    auto op = make_non_convex_fill_op(args.fContext,
                                      args.fSurfaceDrawContext->arenaAlloc(),
                                      FillPathFlags::kStencilOnly,
                                      aaType,
                                      pathDevBounds,
                                      *args.fViewMatrix,
                                      path,
                                      GrPaint());
    sdc->addDrawOp(args.fClip, std::move(op));
}

} // namespace skgpu::v1
