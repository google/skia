/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/TessellationPathRenderer.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrDisableColorXP.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/FillPathFlags.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/PathInnerTriangulateOp.h"
#include "src/gpu/ganesh/ops/PathStencilCoverOp.h"
#include "src/gpu/ganesh/ops/PathTessellateOp.h"
#include "src/gpu/ganesh/ops/StrokeTessellateOp.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <utility>

class GrRecordingContext;
class SkArenaAlloc;

namespace {

using namespace skgpu::tess;

GrOp::Owner make_non_convex_fill_op(GrRecordingContext* rContext,
                                    SkArenaAlloc* arena,
                                    skgpu::ganesh::FillPathFlags fillPathFlags,
                                    GrAAType aaType,
                                    const SkRect& drawBounds,
                                    const SkIRect& clipBounds,
                                    const SkMatrix& viewMatrix,
                                    const SkPath& path,
                                    GrPaint&& paint) {
    SkASSERT(!path.isConvex() || path.isInverseFillType());
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    int numVerbs = path.countVerbs();
    if (numVerbs > 0 && !path.isInverseFillType()) {
        // Check if the path is large and/or simple enough that we can triangulate the inner fan
        // on the CPU. This is our fastest approach. It allows us to stencil only the curves,
        // and then fill the inner fan directly to the final render target, thus drawing the
        // majority of pixels in a single render pass.
        SkRect clippedDrawBounds = SkRect::Make(clipBounds);
        if (clippedDrawBounds.intersect(drawBounds)) {
            float gpuFragmentWork = clippedDrawBounds.height() * clippedDrawBounds.width();
            float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
            constexpr static float kCpuWeight = 512;
            constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
            if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
                return GrOp::Make<skgpu::ganesh::PathInnerTriangulateOp>(rContext,
                                                                         viewMatrix,
                                                                         path,
                                                                         std::move(paint),
                                                                         aaType,
                                                                         fillPathFlags,
                                                                         drawBounds);
            }
        } // we should be clipped out when the GrClip is analyzed, so just return the default op
    }
#endif

    return GrOp::Make<skgpu::ganesh::PathStencilCoverOp>(
            rContext, arena, viewMatrix, path, std::move(paint), aaType, fillPathFlags, drawBounds);
}

} // anonymous namespace

namespace skgpu::ganesh {

namespace {

// `chopped_path` may be null, in which case no chopping actually happens. Returns true on success,
// false on failure (chopping not allowed).
bool ChopPathIfNecessary(const SkMatrix& viewMatrix,
                         const GrStyledShape& shape,
                         const SkIRect& clipConservativeBounds,
                         const SkStrokeRec& stroke,
                         SkPath* chopped_path) {
    const SkRect pathDevBounds = viewMatrix.mapRect(shape.bounds());
    float n4 = wangs_formula::worst_case_cubic_p4(tess::kPrecision,
                                                  pathDevBounds.width(),
                                                  pathDevBounds.height());
    if (n4 > tess::kMaxSegmentsPerCurve_p4 && shape.segmentMask() != SkPath::kLine_SegmentMask) {
        // The path is extremely large. Pre-chop its curves to keep the number of tessellation
        // segments tractable. This will also flatten curves that fall completely outside the
        // viewport.
        SkRect viewport = SkRect::Make(clipConservativeBounds);
        if (!shape.style().isSimpleFill()) {
            // Outset the viewport to pad for the stroke width.
            float inflationRadius;
            if (stroke.isHairlineStyle()) {
                // SkStrokeRec::getInflationRadius() doesn't handle hairlines robustly. Instead
                // find the inflation of an equivalent stroke in device space with a width of 1.
                inflationRadius = SkStrokeRec::GetInflationRadius(stroke.getJoin(),
                                                                  stroke.getMiter(),
                                                                  stroke.getCap(), 1);
            } else {
                inflationRadius = stroke.getInflationRadius() * viewMatrix.getMaxScale();
            }
            viewport.outset(inflationRadius, inflationRadius);
        }
        if (wangs_formula::worst_case_cubic(
                     tess::kPrecision,
                     viewport.width(),
                     viewport.height()) > kMaxSegmentsPerCurve) {
            return false;
        }
        if (chopped_path) {
            *chopped_path = PreChopPathCurves(tess::kPrecision, *chopped_path, viewMatrix,
                                              viewport);
        }
    }
    return true;
}

} // anonymous namespace

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

    // By passing in null for the chopped-path no chopping happens. Rather this returns whether
    // chopping is possible.
    if (!ChopPathIfNecessary(*args.fViewMatrix, shape, *args.fClipConservativeBounds,
                             shape.style().strokeRec(), nullptr)) {
        return CanDrawPath::kNo;
    }

    return CanDrawPath::kYes;
}

bool TessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    auto sdc = args.fSurfaceDrawContext;

    SkPath path;
    args.fShape->asPath(&path);

    // onDrawPath() should only be called if ChopPathIfNecessary() succeeded.
    SkAssertResult(ChopPathIfNecessary(*args.fViewMatrix, *args.fShape,
                                       *args.fClipConservativeBounds,
                                       args.fShape->style().strokeRec(), &path));

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
    const SkRect pathDevBounds = args.fViewMatrix->mapRect(args.fShape->bounds());
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
                                      *args.fClipConservativeBounds,
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

    float n4 = wangs_formula::worst_case_cubic_p4(tess::kPrecision,
                                                  pathDevBounds.width(),
                                                  pathDevBounds.height());
    if (n4 > tess::kMaxSegmentsPerCurve_p4) {
        SkRect viewport = SkRect::Make(*args.fClipConservativeBounds);
        path = PreChopPathCurves(tess::kPrecision, path, *args.fViewMatrix, viewport);
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
                                      *args.fClipConservativeBounds,
                                      *args.fViewMatrix,
                                      path,
                                      GrPaint());
    sdc->addDrawOp(args.fClip, std::move(op));
}

}  // namespace skgpu::ganesh
