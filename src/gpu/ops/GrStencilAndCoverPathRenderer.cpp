/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverPathRenderer.h"
#include "GrCaps.h"
#include "GrDrawPathOp.h"
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrRecordingContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrShape.h"
#include "GrStencilClip.h"
#include "GrStencilPathOp.h"
#include "GrStyle.h"
#include "ops/GrFillRectOp.h"

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

GrPathRenderer::CanDrawPath
GrStencilAndCoverPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkASSERT(!args.fTargetIsWrappedVkSecondaryCB);
    // GrPath doesn't support hairline paths. An arbitrary path effect could produce a hairline
    // path.
    if (args.fShape->style().strokeRec().isHairlineStyle() ||
        args.fShape->style().hasNonDashPathEffect()) {
        return CanDrawPath::kNo;
    }
    if (args.fHasUserStencilSettings) {
        return CanDrawPath::kNo;
    }
    // doesn't do per-path AA, relies on the target having MSAA.
    if (GrAAType::kCoverage == args.fAAType) {
        return CanDrawPath::kNo;
    }
    return CanDrawPath::kYes;
}

static sk_sp<GrPath> get_gr_path(GrResourceProvider* resourceProvider, const GrShape& shape) {
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
    args.fRenderTargetContext->priv().stencilPath(*args.fClip, args.fAAType,
                                                  *args.fViewMatrix, p.get());
}

bool GrStencilAndCoverPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrStencilAndCoverPathRenderer::onDrawPath");
    SkASSERT(!args.fShape->style().strokeRec().isHairlineStyle());

    const SkMatrix& viewMatrix = *args.fViewMatrix;


    sk_sp<GrPath> path(get_gr_path(fResourceProvider, *args.fShape));

    if (args.fShape->inverseFilled()) {
        SkMatrix vmi;
        if (!viewMatrix.invert(&vmi)) {
            return true;
        }

        SkRect devBounds = SkRect::MakeIWH(args.fRenderTargetContext->width(),
                                           args.fRenderTargetContext->height()); // Inverse fill.

        // fake inverse with a stencil and cover
        GrAppliedClip appliedClip;
        if (!args.fClip->apply(args.fContext, args.fRenderTargetContext,
                               GrAATypeIsHW(args.fAAType), true, &appliedClip, &devBounds)) {
            return true;
        }
        GrStencilClip stencilClip(appliedClip.stencilStackID());
        if (appliedClip.scissorState().enabled()) {
            stencilClip.fixedClip().setScissor(appliedClip.scissorState().rect());
        }
        if (appliedClip.windowRectsState().enabled()) {
            stencilClip.fixedClip().setWindowRectangles(appliedClip.windowRectsState().windows(),
                                                        appliedClip.windowRectsState().mode());
        }
        // Just ignore the analytic FPs (if any) during the stencil pass. They will still clip the
        // final draw and it is meaningless to multiply by coverage when drawing to stencil.
        args.fRenderTargetContext->priv().stencilPath(stencilClip, args.fAAType, viewMatrix,
                                                      path.get());

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
            GrAAType coverAAType = args.fAAType;
            if (GrAAType::kMixedSamples == coverAAType) {
                coverAAType = GrAAType::kNone;
            }
            // This is a non-coverage aa rect operation
            SkASSERT(coverAAType == GrAAType::kNone || coverAAType == GrAAType::kMSAA);
            std::unique_ptr<GrDrawOp> op = GrFillRectOp::MakeWithLocalMatrix(
                                                         args.fContext, std::move(args.fPaint),
                                                         coverAAType, coverMatrix, localMatrix,
                                                         coverBounds, &kInvertedCoverPass);

            args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
        }
    } else {
        std::unique_ptr<GrDrawOp> op =
                GrDrawPathOp::Make(args.fContext, viewMatrix, std::move(args.fPaint),
                                   args.fAAType, path.get());
        args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    }

    return true;
}
