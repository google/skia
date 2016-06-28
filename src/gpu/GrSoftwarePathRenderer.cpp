/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSoftwarePathRenderer.h"
#include "GrAuditTrail.h"
#include "GrClip.h"
#include "GrSWMaskHelper.h"
#include "GrTextureProvider.h"
#include "batches/GrRectBatchFactory.h"

////////////////////////////////////////////////////////////////////////////////
bool GrSoftwarePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // Pass on any style that applies. The caller will apply the style if a suitable renderer is
    // not found and try again with the new GrShape.
    return !args.fShape->style().applies() && SkToBool(fTexProvider);
}

namespace {

////////////////////////////////////////////////////////////////////////////////
// gets device coord bounds of path (not considering the fill) and clip. The
// path bounds will be a subset of the clip bounds. returns false if
// path bounds would be empty.
bool get_shape_and_clip_bounds(int width, int height,
                               const GrClip& clip,
                               const GrShape& shape,
                               const SkMatrix& matrix,
                               SkIRect* devShapeBounds,
                               SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    clip.getConservativeBounds(width, height, devClipBounds);

    if (devClipBounds->isEmpty()) {
        *devShapeBounds = SkIRect::MakeWH(width, height);
        return false;
    }
    SkRect shapeBounds = shape.styledBounds();
    if (!shapeBounds.isEmpty()) {
        SkRect shapeSBounds;
        matrix.mapRect(&shapeSBounds, shapeBounds);
        SkIRect shapeIBounds;
        shapeSBounds.roundOut(&shapeIBounds);
        *devShapeBounds = *devClipBounds;
        if (!devShapeBounds->intersect(shapeIBounds)) {
            // set the correct path bounds, as this would be used later.
            *devShapeBounds = shapeIBounds;
            return false;
        }
    } else {
        *devShapeBounds = SkIRect::EmptyIRect();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

}

void GrSoftwarePathRenderer::DrawNonAARect(GrDrawContext* drawContext,
                                           const GrPaint* paint,
                                           const GrUserStencilSettings* userStencilSettings,
                                           const GrClip& clip,
                                           GrColor color,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const SkMatrix& localMatrix) {
    SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(color, viewMatrix, rect,
                                                                        nullptr, &localMatrix));

    GrPipelineBuilder pipelineBuilder(*paint, drawContext->mustUseHWAA(*paint));
    pipelineBuilder.setUserStencil(userStencilSettings);

    drawContext->drawBatch(pipelineBuilder, clip, batch);
}

void GrSoftwarePathRenderer::DrawAroundInvPath(GrDrawContext* drawContext,
                                               const GrPaint* paint,
                                               const GrUserStencilSettings* userStencilSettings,
                                               const GrClip& clip,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkIRect& devClipBounds,
                                               const SkIRect& devPathBounds) {
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

    SkRect rect;
    if (devClipBounds.fTop < devPathBounds.fTop) {
        rect.iset(devClipBounds.fLeft, devClipBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fTop);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip, color,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fTop,
                  devPathBounds.fLeft, devPathBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip, color,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.iset(devPathBounds.fRight, devPathBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip, color,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fBottom,
                  devClipBounds.fRight, devClipBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip, color,
                      SkMatrix::I(), rect, invert);
    }
}

////////////////////////////////////////////////////////////////////////////////
// return true on success; false on failure
bool GrSoftwarePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrSoftwarePathRenderer::onDrawPath");
    if (!fTexProvider) {
        return false;
    }

    // We really need to know if the shape will be inverse filled or not
    bool inverseFilled = false;
    SkTLazy<GrShape> tmpShape;
    SkASSERT(!args.fShape->style().applies())
    inverseFilled = args.fShape->inverseFilled();

    SkIRect devShapeBounds, devClipBounds;
    if (!get_shape_and_clip_bounds(args.fDrawContext->width(), args.fDrawContext->height(),
                                   *args.fClip, *args.fShape,
                                   *args.fViewMatrix, &devShapeBounds, &devClipBounds)) {
        if (inverseFilled) {
            DrawAroundInvPath(args.fDrawContext, args.fPaint, args.fUserStencilSettings,
                              *args.fClip, args.fColor,
                              *args.fViewMatrix, devClipBounds, devShapeBounds);

        }
        return true;
    }

    SkAutoTUnref<GrTexture> texture(
            GrSWMaskHelper::DrawShapeMaskToTexture(fTexProvider, *args.fShape, devShapeBounds,
                                                   args.fAntiAlias, args.fViewMatrix));
    if (nullptr == texture) {
        return false;
    }

    GrSWMaskHelper::DrawToTargetWithShapeMask(texture, args.fDrawContext, args.fPaint,
                                              args.fUserStencilSettings,
                                              *args.fClip, args.fColor, *args.fViewMatrix,
                                              devShapeBounds);

    if (inverseFilled) {
        DrawAroundInvPath(args.fDrawContext, args.fPaint, args.fUserStencilSettings,
                          *args.fClip, args.fColor,
                          *args.fViewMatrix, devClipBounds, devShapeBounds);
    }

    return true;
}
