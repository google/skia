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
    SkASSERT(GrAAType::kCoverage != fAAType);
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

static GrPathRenderer::StencilSupport get_min_stencil_support(GrPathRenderer::DrawType drawType) {
    using StencilSupport = GrPathRenderer::StencilSupport;
    GR_STATIC_ASSERT(StencilSupport::kNoSupport < StencilSupport::kStencilOnly);
    GR_STATIC_ASSERT(StencilSupport::kStencilOnly < StencilSupport::kNoRestriction);

    switch (drawType) {
        using DrawType = GrPathRenderer::DrawType;
        case DrawType::kStencil:
            return StencilSupport::kStencilOnly;
        case DrawType::kStencilAndColor:
            return StencilSupport::kNoRestriction;
        case DrawType::kColor:
            return StencilSupport::kNoSupport;
    }

    SK_ABORT("Invalid GrPathRenderer::DrawType.");
    return StencilSupport::kNoSupport;
}

GrPathRenderer::CanDrawPath GrPathRenderer::canDrawPath(
        const CanDrawPathArgs& args, DrawType drawType, StencilSupport* outStencilSupport) const {
    SkDEBUGCODE(args.validate();)

    *outStencilSupport = StencilSupport::kNoSupport;
    if (DrawType::kColor != drawType) {
        *outStencilSupport = this->getStencilSupport(*args.fShape);
        if (*outStencilSupport < get_min_stencil_support(drawType)) {
            return CanDrawPath::kNo;
        }
    }

    return this->onCanDrawPath(args, drawType);
}

bool GrPathRenderer::drawPath(const DrawPathArgs& args) {
#ifdef SK_DEBUG
    args.validate();
    SkASSERT(!(args.fAAType == GrAAType::kMSAA &&
               GrFSAAType::kUnifiedMSAA != args.fRenderTargetContext->fsaaType()));
    SkASSERT(!(args.fAAType == GrAAType::kMixedSamples &&
               GrFSAAType::kMixedSamples != args.fRenderTargetContext->fsaaType()));
    if (!args.fUserStencilSettings->isUnused()) {
        SkPath path;
        args.fShape->asPath(&path);
        SkASSERT(args.fShape->style().isSimpleFill());
        SkASSERT(StencilSupport::kNoRestriction == this->getStencilSupport(*args.fShape));
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
                          args.fAAType,
                          false};
    this->drawPath(drawArgs);
}
