/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawContextPriv.h"
#include "GrDrawPathBatch.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrStencilPathBatch.h"
#include "GrStyle.h"
#include "batches/GrRectBatchFactory.h"

GrPathRenderer* GrStencilAndCoverPathRenderer::Create(GrResourceProvider* resourceProvider,
                                                      const GrCaps& caps) {
    if (caps.shaderCaps()->pathRenderingSupport()) {
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
    if (args.fAntiAlias) {
        return args.fIsStencilBufferMSAA;
    } else {
        return true; // doesn't do per-path AA, relies on the target having MSAA
    }
}

static GrPath* get_gr_path(GrResourceProvider* resourceProvider, const SkPath& skPath,
                           const GrStyle& style) {
    GrUniqueKey key;
    bool isVolatile;
    GrPath::ComputeKey(skPath, style, &key, &isVolatile);
    SkAutoTUnref<GrPath> path(
        static_cast<GrPath*>(resourceProvider->findAndRefResourceByUniqueKey(key)));
    if (!path) {
        path.reset(resourceProvider->createPath(skPath, style));
        if (!isVolatile) {
            resourceProvider->assignUniqueKeyToResource(key, path);
        }
    } else {
        SkASSERT(path->isEqualTo(skPath, style));
    }
    return path.release();
}

void GrStencilAndCoverPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrStencilAndCoverPathRenderer::onStencilPath");
    SkASSERT(!args.fIsAA || args.fDrawContext->isStencilBufferMultisampled());
    SkPath path;
    args.fShape->asPath(&path);

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Make());
    paint.setAntiAlias(args.fIsAA);

    const GrPipelineBuilder pipelineBuilder(paint, args.fIsAA);

    SkAutoTUnref<GrPath> p(get_gr_path(fResourceProvider, path, GrStyle::SimpleFill()));
    args.fDrawContext->drawContextPriv().stencilPath(pipelineBuilder, *args.fClip,
                                                     *args.fViewMatrix, p, p->getFillType());
}

bool GrStencilAndCoverPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrStencilAndCoverPathRenderer::onDrawPath");
    SkASSERT(!args.fPaint->isAntiAlias() || args.fDrawContext->isStencilBufferMultisampled());
    SkASSERT(!args.fShape->style().strokeRec().isHairlineStyle());

    const SkMatrix& viewMatrix = *args.fViewMatrix;

    SkPath path;
    args.fShape->asPath(&path);

    SkAutoTUnref<GrPath> p(get_gr_path(fResourceProvider, path, args.fShape->style()));

    if (path.isInverseFillType()) {
        static constexpr GrUserStencilSettings kInvertedCoverPass(
            GrUserStencilSettings::StaticInit<
                0x0000,
                // We know our rect will hit pixels outside the clip and the user bits will be 0
                // outside the clip. So we can't just fill where the user bits are 0. We also need
                // to check that the clip bit is set.
                GrUserStencilTest::kEqualIfInClip,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kZero,
                0xffff>()
        );

        // fake inverse with a stencil and cover
        {
            GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fPaint->isAntiAlias());
            pipelineBuilder.setUserStencil(&kInvertedCoverPass);

            args.fDrawContext->drawContextPriv().stencilPath(pipelineBuilder, *args.fClip,
                                                             viewMatrix, p, p->getFillType());
        }

        SkMatrix invert = SkMatrix::I();
        SkRect bounds =
            SkRect::MakeLTRB(0, 0,
                             SkIntToScalar(args.fDrawContext->width()),
                             SkIntToScalar(args.fDrawContext->height()));
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

        SkAutoTUnref<GrDrawBatch> coverBatch(
                GrRectBatchFactory::CreateNonAAFill(args.fColor, viewM, bounds, nullptr,
                                                    &invert));

        {
            GrPipelineBuilder pipelineBuilder(*args.fPaint,
                                              args.fPaint->isAntiAlias() &&
                                              !args.fDrawContext->hasMixedSamples());
            pipelineBuilder.setUserStencil(&kInvertedCoverPass);

            args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, coverBatch);
        }
    } else {
        static constexpr GrUserStencilSettings kCoverPass(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>()
        );

        SkAutoTUnref<GrDrawBatch> batch(
                GrDrawPathBatch::Create(viewMatrix, args.fColor, p->getFillType(), p));

        GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fPaint->isAntiAlias());
        pipelineBuilder.setUserStencil(&kCoverPass);
        if (args.fAntiAlias) {
            SkASSERT(args.fDrawContext->isStencilBufferMultisampled());
            pipelineBuilder.enableState(GrPipelineBuilder::kHWAntialias_Flag);
        }

        args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);
    }

    return true;
}
