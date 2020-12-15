/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStencilClip.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrDrawPathOp.h"
#include "src/gpu/ops/GrStencilAndCoverPathRenderer.h"
#include "src/gpu/ops/GrStencilPathOp.h"

GrPathRenderer* GrStencilAndCoverPathRenderer::Create(GrResourceProvider* resourceProvider,
                                                      const GrCaps& caps) {
    if (caps.shaderCaps()->pathRenderingSupport() && !caps.avoidStencilBuffers()) {
        return new GrStencilAndCoverPathRenderer(resourceProvider);
    } else {
        return nullptr;
    }
}

GrStencilAndCoverPathRenderer::GrStencilAndCoverPathRenderer(GrResourceProvider* resourceProvider)
    : fResourceProvider(resourceProvider) {
}

static bool has_matrix(const GrFragmentProcessor& fp) {
    if (fp.sampleUsage().hasMatrix()) {
        return true;
    }
    for (int i = fp.numChildProcessors() - 1; i >= 0; --i) {
        if (fp.childProcessor(i) && has_matrix(*fp.childProcessor(i))) {
            return true;
        }
    }
    return false;
}

GrPathRenderer::CanDrawPath
GrStencilAndCoverPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkASSERT(!args.fTargetIsWrappedVkSecondaryCB);
    // GrPath doesn't support hairline paths. An arbitrary path effect could produce a hairline
    // path.
    if (args.fShape->style().strokeRec().isHairlineStyle() ||
        args.fShape->style().hasNonDashPathEffect() ||
        args.fHasUserStencilSettings ||
        !args.fShape->hasUnstyledKey()) {
        return CanDrawPath::kNo;
    }
    if (GrAAType::kCoverage == args.fAAType && !args.fProxy->canUseMixedSamples(*args.fCaps)) {
        // We rely on a mixed sampled stencil buffer to implement coverage AA.
        return CanDrawPath::kNo;
    }
    // The lack of vertex shaders means we can't move transform matrices into the vertex shader. We
    // could do the transform in the fragment processor, but that would be very slow, so instead we
    // just avoid using this path renderer in the face of transformed FPs.
    if (args.fPaint && args.fPaint->hasColorFragmentProcessor()) {
        if (has_matrix(*args.fPaint->getColorFragmentProcessor())) {
            return CanDrawPath::kNo;
        }
    }
    return CanDrawPath::kYes;
}

static sk_sp<GrPath> get_gr_path(GrResourceProvider* resourceProvider, const GrStyledShape& shape) {
    GrUniqueKey key;
    bool isVolatile;
    GrPath::ComputeKey(shape, &key, &isVolatile);
    sk_sp<GrPath> path;
    if (!isVolatile) {
        path = resourceProvider->findByUniqueKey<GrPath>(key);
    }
    if (!path) {
        SkPath skPath;
        shape.asPath(&skPath);
        path = resourceProvider->createPath(skPath, shape.style());
        if (!isVolatile) {
            resourceProvider->assignUniqueKeyToResource(key, path.get());
        }
    } else {
#ifdef SK_DEBUG
        SkPath skPath;
        shape.asPath(&skPath);
        SkASSERT(path->isEqualTo(skPath, shape.style()));
#endif
    }
    return path;
}

void GrStencilAndCoverPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrStencilAndCoverPathRenderer::onStencilPath");
    sk_sp<GrPath> p(get_gr_path(fResourceProvider, *args.fShape));
    args.fRenderTargetContext->stencilPath(args.fClip, args.fDoStencilMSAA, *args.fViewMatrix,
                                           std::move(p));
}

bool GrStencilAndCoverPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrStencilAndCoverPathRenderer::onDrawPath");
    SkASSERT(!args.fShape->style().strokeRec().isHairlineStyle());

    const SkMatrix& viewMatrix = *args.fViewMatrix;

    // Any AA will use stencil MSAA
    GrAAType stencilAAType = GrAAType::kNone != args.fAAType ? GrAAType::kMSAA : GrAAType::kNone;

    sk_sp<GrPath> path(get_gr_path(fResourceProvider, *args.fShape));

    if (args.fShape->inverseFilled()) {
        SkMatrix vmi;
        if (!viewMatrix.invert(&vmi)) {
            return true;
        }

        SkRect devBounds = SkRect::MakeIWH(args.fRenderTargetContext->width(),
                                           args.fRenderTargetContext->height()); // Inverse fill.

        // fake inverse with a stencil and cover
        GrAppliedClip appliedClip(args.fRenderTargetContext->dimensions());
        if (args.fClip && args.fClip->apply(
                args.fContext, args.fRenderTargetContext, stencilAAType, true, &appliedClip,
                &devBounds) == GrClip::Effect::kClippedOut) {
            return true;
        }
        GrStencilClip stencilClip(args.fRenderTargetContext->dimensions(),
                                  appliedClip.stencilStackID());
        if (appliedClip.scissorState().enabled()) {
            SkAssertResult(stencilClip.fixedClip().setScissor(appliedClip.scissorState().rect()));
        }
        if (appliedClip.windowRectsState().enabled()) {
            stencilClip.fixedClip().setWindowRectangles(appliedClip.windowRectsState().windows(),
                                                        appliedClip.windowRectsState().mode());
        }
        // Just ignore the analytic FPs (if any) during the stencil pass. They will still clip the
        // final draw and it is meaningless to multiply by coverage when drawing to stencil.
        args.fRenderTargetContext->stencilPath(&stencilClip, GrAA(stencilAAType == GrAAType::kMSAA),
                                               viewMatrix, std::move(path));

        {
            static constexpr GrUserStencilSettings kInvertedCoverPass(
                GrUserStencilSettings::StaticInit<
                    0x0000,
                    // We know our rect will hit pixels outside the clip and the user bits will
                    // be 0 outside the clip. So we can't just fill where the user bits are 0. We
                    // also need to check that the clip bit is set.
                    GrUserStencilTest::kEqualIfInClip,
                    0xffff,
                    GrUserStencilOp::kKeep,
                    GrUserStencilOp::kZero,
                    0xffff>()
            );

            SkRect coverBounds;
            // mapRect through persp matrix may not be correct
            if (!viewMatrix.hasPerspective()) {
                vmi.mapRect(&coverBounds, devBounds);
                // theoretically could set bloat = 0, instead leave it because of matrix inversion
                // precision.
                SkScalar bloat = viewMatrix.getMaxScale() * SK_ScalarHalf;
                coverBounds.outset(bloat, bloat);
            } else {
                coverBounds = devBounds;
            }
            const SkMatrix& coverMatrix = !viewMatrix.hasPerspective() ? viewMatrix : SkMatrix::I();
            const SkMatrix& localMatrix = !viewMatrix.hasPerspective() ? SkMatrix::I() : vmi;

            // We have to suppress enabling MSAA for mixed samples or we will get seams due to
            // coverage modulation along the edge where two triangles making up the rect meet.
            GrAA coverAA = GrAA(args.fAAType == GrAAType::kMSAA);
            args.fRenderTargetContext->stencilRect(args.fClip, &kInvertedCoverPass,
                                                   std::move(args.fPaint), coverAA, coverMatrix,
                                                   coverBounds, &localMatrix);
        }
    } else {
        GrOp::Owner op = GrDrawPathOp::Make(
                args.fContext, viewMatrix, std::move(args.fPaint),
                GrAA(stencilAAType == GrAAType::kMSAA), std::move(path));
        args.fRenderTargetContext->addDrawOp(args.fClip, std::move(op));
    }

    return true;
}
