
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSoftwarePathRenderer.h"
#include "GrContext.h"
#include "GrSWMaskHelper.h"
#include "GrVertexBuffer.h"

////////////////////////////////////////////////////////////////////////////////
bool GrSoftwarePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (nullptr == fContext) {
        return false;
    }
    if (args.fStroke->isDashed()) {
        return false;
    }
    return true;
}

namespace {

////////////////////////////////////////////////////////////////////////////////
// gets device coord bounds of path (not considering the fill) and clip. The
// path bounds will be a subset of the clip bounds. returns false if
// path bounds would be empty.
bool get_path_and_clip_bounds(const GrPipelineBuilder* pipelineBuilder,
                              const SkPath& path,
                              const SkMatrix& matrix,
                              SkIRect* devPathBounds,
                              SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    const GrRenderTarget* rt = pipelineBuilder->getRenderTarget();
    if (nullptr == rt) {
        return false;
    }

    pipelineBuilder->clip().getConservativeBounds(rt->width(), rt->height(), devClipBounds);

    if (devClipBounds->isEmpty()) {
        *devPathBounds = SkIRect::MakeWH(rt->width(), rt->height());
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
void draw_around_inv_path(GrDrawTarget* target,
                          GrPipelineBuilder* pipelineBuilder,
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
        target->drawNonAARect(*pipelineBuilder, color, SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fTop,
                  devPathBounds.fLeft, devPathBounds.fBottom);
        target->drawNonAARect(*pipelineBuilder, color, SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.iset(devPathBounds.fRight, devPathBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fBottom);
        target->drawNonAARect(*pipelineBuilder, color, SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fBottom,
                  devClipBounds.fRight, devClipBounds.fBottom);
        target->drawNonAARect(*pipelineBuilder, color, SkMatrix::I(), rect, invert);
    }
}

}

////////////////////////////////////////////////////////////////////////////////
// return true on success; false on failure
bool GrSoftwarePathRenderer::onDrawPath(const DrawPathArgs& args) {
    if (nullptr == fContext) {
        return false;
    }

    SkIRect devPathBounds, devClipBounds;
    if (!get_path_and_clip_bounds(args.fPipelineBuilder, *args.fPath,
                                  *args.fViewMatrix, &devPathBounds, &devClipBounds)) {
        if (args.fPath->isInverseFillType()) {
            draw_around_inv_path(args.fTarget, args.fPipelineBuilder, args.fColor,
                                 *args.fViewMatrix, devClipBounds, devPathBounds);
        }
        return true;
    }

    SkAutoTUnref<GrTexture> texture(
            GrSWMaskHelper::DrawPathMaskToTexture(fContext, *args.fPath, *args.fStroke,
                                                  devPathBounds,
                                                  args.fAntiAlias, args.fViewMatrix));
    if (nullptr == texture) {
        return false;
    }

    GrSWMaskHelper::DrawToTargetWithPathMask(texture, args.fTarget, args.fPipelineBuilder,
                                             args.fColor, *args.fViewMatrix, devPathBounds);

    if (args.fPath->isInverseFillType()) {
        draw_around_inv_path(args.fTarget, args.fPipelineBuilder, args.fColor, *args.fViewMatrix,
                             devClipBounds, devPathBounds);
    }

    return true;
}
