/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/PathRenderer.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

#include <utility>

namespace skgpu::ganesh {

#ifdef SK_DEBUG
void PathRenderer::StencilPathArgs::validate() const {
    SkASSERT(fContext);
    SkASSERT(fSurfaceDrawContext);
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

PathRenderer::StencilSupport PathRenderer::getStencilSupport(const GrStyledShape& shape) const {
    SkDEBUGCODE(SkPath path;)
    SkDEBUGCODE(shape.asPath(&path);)
    SkASSERT(shape.style().isSimpleFill());
    SkASSERT(!path.isInverseFillType());
    return this->onGetStencilSupport(shape);
}

bool PathRenderer::drawPath(const DrawPathArgs& args) {
#ifdef SK_DEBUG
    args.validate();
    CanDrawPathArgs canArgs;
    canArgs.fCaps = args.fContext->priv().caps();
    canArgs.fProxy = args.fSurfaceDrawContext->asRenderTargetProxy();
    canArgs.fClipConservativeBounds = args.fClipConservativeBounds;
    canArgs.fViewMatrix = args.fViewMatrix;
    canArgs.fShape = args.fShape;
    canArgs.fPaint = &args.fPaint;
    canArgs.fSurfaceProps = &args.fSurfaceDrawContext->surfaceProps();
    canArgs.fAAType = args.fAAType;
    canArgs.validate();

    canArgs.fHasUserStencilSettings = !args.fUserStencilSettings->isUnused();
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

void PathRenderer::GetPathDevBounds(const SkPath& path,
                                    SkISize devSize,
                                    const SkMatrix& matrix,
                                    SkRect* bounds) {
    if (path.isInverseFillType()) {
        *bounds = SkRect::Make(devSize);
        return;
    }
    *bounds = path.getBounds();
    matrix.mapRect(bounds);
}

void PathRenderer::onStencilPath(const StencilPathArgs& args) {
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
                          args.fSurfaceDrawContext,
                          nullptr,  // clip
                          args.fClipConservativeBounds,
                          args.fViewMatrix,
                          args.fShape,
                          (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone,
                          false};
    this->drawPath(drawArgs);
}

}  // namespace skgpu::ganesh
