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
    return SkToBool(fTexProvider);
}

namespace {

////////////////////////////////////////////////////////////////////////////////
// gets device coord bounds of path (not considering the fill) and clip. The
// path bounds will be a subset of the clip bounds. returns false if
// path bounds would be empty.
bool get_path_and_clip_bounds(int width, int height,
                              const GrClip& clip,
                              const SkPath& path,
                              const SkMatrix& matrix,
                              SkIRect* devPathBounds,
                              SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    clip.getConservativeBounds(width, height, devClipBounds);

    if (devClipBounds->isEmpty()) {
        *devPathBounds = SkIRect::MakeWH(width, height);
        return false;
    }

    if (!path.getBounds().isEmpty()) {
        SkRect pathSBounds;
        matrix.mapRect(&pathSBounds, path.getBounds());
        SkIRect pathIBounds;
        pathSBounds.roundOut(&pathIBounds);
        *devPathBounds = *devClipBounds;
        if (!devPathBounds->intersect(pathIBounds)) {
            // set the correct path bounds, as this would be used later.
            *devPathBounds = pathIBounds;
            return false;
        }
    } else {
        *devPathBounds = SkIRect::EmptyIRect();
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

    GrPipelineBuilder pipelineBuilder(*paint, drawContext->isUnifiedMultisampled());
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

    SkIRect devPathBounds, devClipBounds;
    if (!get_path_and_clip_bounds(args.fDrawContext->width(), args.fDrawContext->height(),
                                  *args.fClip, *args.fPath,
                                  *args.fViewMatrix, &devPathBounds, &devClipBounds)) {
        if (args.fPath->isInverseFillType()) {
            DrawAroundInvPath(args.fDrawContext, args.fPaint, args.fUserStencilSettings,
                              *args.fClip, args.fColor,
                              *args.fViewMatrix, devClipBounds, devPathBounds);
        }
        return true;
    }

    SkAutoTUnref<GrTexture> texture(
            GrSWMaskHelper::DrawPathMaskToTexture(fTexProvider, *args.fPath, *args.fStyle,
                                                  devPathBounds,
                                                  args.fAntiAlias, args.fViewMatrix));
    if (nullptr == texture) {
        return false;
    }

    GrSWMaskHelper::DrawToTargetWithPathMask(texture, args.fDrawContext, args.fPaint,
                                             args.fUserStencilSettings,
                                             *args.fClip, args.fColor, *args.fViewMatrix,
                                             devPathBounds);

    if (args.fPath->isInverseFillType()) {
        DrawAroundInvPath(args.fDrawContext, args.fPaint, args.fUserStencilSettings,
                          *args.fClip, args.fColor,
                          *args.fViewMatrix, devClipBounds, devPathBounds);
    }

    return true;
}
