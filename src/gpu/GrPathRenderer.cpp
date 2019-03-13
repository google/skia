/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderer.h"
#include "GrCaps.h"
#include "GrPaint.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrShape.h"
#include "GrUserStencilSettings.h"
#include "SkDrawProcs.h"

#ifdef SK_DEBUG
void GrPathRenderer::StencilPathArgs::validate() const {
    SkASSERT(fContext);
    SkASSERT(fRenderTargetContext);
    SkASSERT(fClipConservativeBounds);
    SkASSERT(fViewMatrix);
    SkASSERT(fShape);
    SkASSERT(fShape->style().isSimpleFill());
    SkPath path;
    fShape->asPath(&path);
    SkASSERT(!path.isInverseFillType());
}
#endif

//////////////////////////////////////////////////////////////////////////////

GrPathRenderer::GrPathRenderer() {}

GrPathRenderer::StencilSupport GrPathRenderer::getStencilSupport(const GrShape& shape) const {
    SkDEBUGCODE(SkPath path;)
    SkDEBUGCODE(shape.asPath(&path);)
    SkASSERT(shape.style().isSimpleFill());
    SkASSERT(!path.isInverseFillType());
    return this->onGetStencilSupport(shape);
}

bool GrPathRenderer::drawPath(const DrawPathArgs& args) {
#ifdef SK_DEBUG
    args.validate();
    CanDrawPathArgs canArgs;
    canArgs.fCaps = args.fContext->priv().caps();
    canArgs.fClipConservativeBounds = args.fClipConservativeBounds;
    canArgs.fViewMatrix = args.fViewMatrix;
    canArgs.fShape = args.fShape;
    canArgs.fAATypeFlags = args.fAATypeFlags;
    canArgs.fTargetIsWrappedVkSecondaryCB = args.fRenderTargetContext->wrapsVkSecondaryCB();
    canArgs.validate();

    canArgs.fHasUserStencilSettings = !args.fUserStencilSettings->isUnused();
    if (AATypeFlags::kMixedSampledStencilThenCover & canArgs.fAATypeFlags) {
        SkASSERT(GrFSAAType::kMixedSamples == args.fRenderTargetContext->fsaaType());
    }
    SkASSERT(CanDrawPath::kNo != this->canDrawPath(canArgs));
    if (!args.fUserStencilSettings->isUnused()) {
        SkPath path;
        args.fShape->asPath(&path);
        SkASSERT(args.fShape->style().isSimpleFill());
        SkASSERT(kNoRestriction_StencilSupport == this->getStencilSupport(*args.fShape));
    }
#endif
    return this->onDrawPath(args);
}

bool GrPathRenderer::IsStrokeHairlineOrEquivalent(const GrStyle& style, const SkMatrix& matrix,
                                                  SkScalar* outCoverage) {
    if (style.pathEffect()) {
        return false;
    }
    const SkStrokeRec& stroke = style.strokeRec();
    if (stroke.isHairlineStyle()) {
        if (outCoverage) {
            *outCoverage = SK_Scalar1;
        }
        return true;
    }
    return stroke.getStyle() == SkStrokeRec::kStroke_Style &&
           SkDrawTreatAAStrokeAsHairline(stroke.getWidth(), matrix, outCoverage);
}


void GrPathRenderer::GetPathDevBounds(const SkPath& path,
                                      int devW, int devH,
                                      const SkMatrix& matrix,
                                      SkRect* bounds) {
    if (path.isInverseFillType()) {
        *bounds = SkRect::MakeWH(SkIntToScalar(devW), SkIntToScalar(devH));
        return;
    }
    *bounds = path.getBounds();
    matrix.mapRect(bounds);
}

void GrPathRenderer::onStencilPath(const StencilPathArgs& args) {
    static constexpr GrUserStencilSettings kIncrementStencil(
            GrUserStencilSettings::StaticInit<
                    0xffff,
                    GrUserStencilTest::kAlways,
                    0xffff,
                    GrUserStencilOp::kReplace,
                    GrUserStencilOp::kReplace,
                    0xffff>()
    );

    GrPaint paint;
    DrawPathArgs drawArgs{args.fContext,
                          std::move(paint),
                          &kIncrementStencil,
                          args.fRenderTargetContext,
                          nullptr,  // clip
                          args.fClipConservativeBounds,
                          args.fViewMatrix,
                          args.fShape,
                          (GrAA::kYes == args.fDoStencilMSAA)
                                  ? AATypeFlags::kMSAA
                                  : AATypeFlags::kNone,
                          false};
    this->drawPath(drawArgs);
}
