/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrDrawPathBatch.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrRenderTarget.h"
#include "GrResourceProvider.h"
#include "GrStrokeInfo.h"
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
    if (args.fStroke->isHairlineStyle()) {
        return false;
    }
    if (!args.fIsStencilDisabled) {
        return false;
    }
    if (args.fAntiAlias) {
        return args.fIsStencilBufferMSAA;
    } else {
        return true; // doesn't do per-path AA, relies on the target having MSAA
    }
}

static GrPath* get_gr_path(GrResourceProvider* resourceProvider, const SkPath& skPath,
                           const GrStrokeInfo& stroke) {
    GrUniqueKey key;
    bool isVolatile;
    GrPath::ComputeKey(skPath, stroke, &key, &isVolatile);
    SkAutoTUnref<GrPath> path(
        static_cast<GrPath*>(resourceProvider->findAndRefResourceByUniqueKey(key)));
    if (!path) {
        path.reset(resourceProvider->createPath(skPath, stroke));
        if (!isVolatile) {
            resourceProvider->assignUniqueKeyToResource(key, path);
        }
    } else {
        SkASSERT(path->isEqualTo(skPath, stroke));
    }
    return path.release();
}

void GrStencilAndCoverPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fTarget->getAuditTrail(),
                              "GrStencilAndCoverPathRenderer::onStencilPath");
    SkASSERT(!args.fPath->isInverseFillType());
    SkAutoTUnref<GrPath> p(get_gr_path(fResourceProvider, *args.fPath, *args.fStroke));
    args.fTarget->stencilPath(*args.fPipelineBuilder, *args.fViewMatrix, p, p->getFillType());
}

bool GrStencilAndCoverPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fTarget->getAuditTrail(),
                              "GrStencilAndCoverPathRenderer::onDrawPath");
    SkASSERT(!args.fStroke->isHairlineStyle());
    const SkPath& path = *args.fPath;
    GrPipelineBuilder* pipelineBuilder = args.fPipelineBuilder;
    const SkMatrix& viewMatrix = *args.fViewMatrix;

    SkASSERT(pipelineBuilder->getStencil().isDisabled());

    if (args.fAntiAlias) {
        SkASSERT(pipelineBuilder->getRenderTarget()->isStencilBufferMultisampled());
        pipelineBuilder->enableState(GrPipelineBuilder::kHWAntialias_Flag);
    }

    SkAutoTUnref<GrPath> p(get_gr_path(fResourceProvider, path, *args.fStroke));

    if (path.isInverseFillType()) {
        GR_STATIC_CONST_SAME_STENCIL(kInvertedStencilPass,
            kKeep_StencilOp,
            kZero_StencilOp,
            // We know our rect will hit pixels outside the clip and the user bits will be 0
            // outside the clip. So we can't just fill where the user bits are 0. We also need to
            // check that the clip bit is set.
            kEqualIfInClip_StencilFunc,
            0xffff,
            0x0000,
            0xffff);

        pipelineBuilder->setStencil(kInvertedStencilPass);

        // fake inverse with a stencil and cover
        args.fTarget->stencilPath(*pipelineBuilder, viewMatrix, p, p->getFillType());

        SkMatrix invert = SkMatrix::I();
        SkRect bounds =
            SkRect::MakeLTRB(0, 0, SkIntToScalar(pipelineBuilder->getRenderTarget()->width()),
                             SkIntToScalar(pipelineBuilder->getRenderTarget()->height()));
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
        if (pipelineBuilder->getRenderTarget()->hasMixedSamples()) {
            pipelineBuilder->disableState(GrPipelineBuilder::kHWAntialias_Flag);
        }

        SkAutoTUnref<GrDrawBatch> batch(
                GrRectBatchFactory::CreateNonAAFill(args.fColor, viewM, bounds, nullptr,
                                                    &invert));
        args.fTarget->drawBatch(*pipelineBuilder, batch);
    } else {
        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
            kZero_StencilOp,
            kKeep_StencilOp,
            kNotEqual_StencilFunc,
            0xffff,
            0x0000,
            0xffff);

        pipelineBuilder->setStencil(kStencilPass);
        SkAutoTUnref<GrDrawPathBatchBase> batch(
                GrDrawPathBatch::Create(viewMatrix, args.fColor, p->getFillType(), p));
        args.fTarget->drawPathBatch(*pipelineBuilder, batch);
    }

    pipelineBuilder->stencil()->setDisabled();
    return true;
}
