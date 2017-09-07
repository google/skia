/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilAndCoverPathRenderer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawPathOp.h"
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrStencilPathOp.h"
#include "GrStyle.h"
#include "ops/GrRectOpFactory.h"

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

bool GrStencilAndCoverPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // GrPath doesn't support hairline paths. An arbitrary path effect could produce a hairline
    // path.
    if (args.fShape->style().strokeRec().isHairlineStyle() ||
        args.fShape->style().hasNonDashPathEffect()) {
        return false;
    }
    if (args.fHasUserStencilSettings) {
        return false;
    }
    // doesn't do per-path AA, relies on the target having MSAA.
    return (GrAAType::kCoverage != args.fAAType);
}

static GrPath* get_gr_path(GrResourceProvider* resourceProvider, const GrShape& shape) {
    GrUniqueKey key;
    bool isVolatile;
    GrPath::ComputeKey(shape, &key, &isVolatile);
    sk_sp<GrPath> path;
    if (!isVolatile) {
        path.reset(
            static_cast<GrPath*>(resourceProvider->findAndRefResourceByUniqueKey(key)));
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
    return path.release();
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
        SkMatrix invert = SkMatrix::I();
        SkRect bounds =
            SkRect::MakeLTRB(0, 0,
                             SkIntToScalar(args.fRenderTargetContext->width()),
                             SkIntToScalar(args.fRenderTargetContext->height()));
        SkMatrix vmi;
        // mapRect through persp matrix may not be correct
        if (!viewMatrix.hasPerspective() && viewMatrix.invert(&vmi)) {
            vmi.mapRect(&bounds);
            // theoretically could set bloat = 0, instead leave it because of matrix inversion
            // precision.
            SkScalar bloat = viewMatrix.getMaxScale() * SK_ScalarHalf;
            bounds.outset(bloat, bloat);
        } else {
            if (!viewMatrix.invert(&invert)) {
                return false;
            }
        }
        const SkMatrix& viewM = viewMatrix.hasPerspective() ? SkMatrix::I() : viewMatrix;

        // fake inverse with a stencil and cover
        args.fRenderTargetContext->priv().stencilPath(*args.fClip, args.fAAType, viewMatrix,
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
            // We have to suppress enabling MSAA for mixed samples or we will get seams due to
            // coverage modulation along the edge where two triangles making up the rect meet.
            GrAAType coverAAType = args.fAAType;
            if (GrAAType::kMixedSamples == coverAAType) {
                coverAAType = GrAAType::kNone;
            }
            args.fRenderTargetContext->addDrawOp(*args.fClip,
                                                 GrRectOpFactory::MakeNonAAFillWithLocalMatrix(
                                                         std::move(args.fPaint), viewM, invert,
                                                         bounds, coverAAType, &kInvertedCoverPass));
        }
    } else {
        std::unique_ptr<GrDrawOp> op =
                GrDrawPathOp::Make(viewMatrix, std::move(args.fPaint), args.fAAType, path.get());
        args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    }

    return true;
}
