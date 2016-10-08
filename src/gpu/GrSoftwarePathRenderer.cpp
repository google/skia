/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSoftwarePathRenderer.h"
#include "GrAuditTrail.h"
#include "GrClip.h"
#include "GrPipelineBuilder.h"
#include "GrGpuResourcePriv.h"
#include "GrSWMaskHelper.h"
#include "GrTextureProvider.h"
#include "batches/GrRectBatchFactory.h"

////////////////////////////////////////////////////////////////////////////////
bool GrSoftwarePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // Pass on any style that applies. The caller will apply the style if a suitable renderer is
    // not found and try again with the new GrShape.
    return !args.fShape->style().applies() && SkToBool(fTexProvider);
}

////////////////////////////////////////////////////////////////////////////////
static bool get_unclipped_shape_dev_bounds(const GrShape& shape, const SkMatrix& matrix,
                                           SkIRect* devBounds) {
    SkRect shapeBounds = shape.styledBounds();
    if (shapeBounds.isEmpty()) {
        return false;
    }
    SkRect shapeDevBounds;
    matrix.mapRect(&shapeDevBounds, shapeBounds);
    shapeDevBounds.roundOut(devBounds);
    return true;
}

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
static bool get_shape_and_clip_bounds(int width, int height,
                                      const GrClip& clip,
                                      const GrShape& shape,
                                      const SkMatrix& matrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* clippedDevShapeBounds,
                                      SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    clip.getConservativeBounds(width, height, devClipBounds);

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::EmptyIRect();
        *clippedDevShapeBounds = SkIRect::EmptyIRect();
        return false;
    }
    if (!clippedDevShapeBounds->intersect(*devClipBounds, *unclippedDevShapeBounds)) {
        *clippedDevShapeBounds = SkIRect::EmptyIRect();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void GrSoftwarePathRenderer::DrawNonAARect(GrDrawContext* drawContext,
                                           const GrPaint& paint,
                                           const GrUserStencilSettings& userStencilSettings,
                                           const GrClip& clip,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const SkMatrix& localMatrix) {
    SkAutoTUnref<GrDrawBatch> batch(GrRectBatchFactory::CreateNonAAFill(paint.getColor(),
                                                                        viewMatrix, rect,
                                                                        nullptr, &localMatrix));

    GrPipelineBuilder pipelineBuilder(paint, drawContext->mustUseHWAA(paint));
    pipelineBuilder.setUserStencil(&userStencilSettings);

    drawContext->drawBatch(pipelineBuilder, clip, batch);
}

void GrSoftwarePathRenderer::DrawAroundInvPath(GrDrawContext* drawContext,
                                               const GrPaint& paint,
                                               const GrUserStencilSettings& userStencilSettings,
                                               const GrClip& clip,
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
        DrawNonAARect(drawContext, paint, userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fTop,
                  devPathBounds.fLeft, devPathBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.iset(devPathBounds.fRight, devPathBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip,
                      SkMatrix::I(), rect, invert);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fBottom,
                  devClipBounds.fRight, devClipBounds.fBottom);
        DrawNonAARect(drawContext, paint, userStencilSettings, clip,
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
    SkASSERT(!args.fShape->style().applies());
    inverseFilled = args.fShape->inverseFilled();

    SkIRect unclippedDevShapeBounds, clippedDevShapeBounds, devClipBounds;
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = fAllowCaching && !inverseFilled && args.fViewMatrix->preservesAxisAlignment() &&
                    args.fShape->hasUnstyledKey() && args.fAntiAlias;

    if (!get_shape_and_clip_bounds(args.fDrawContext->width(), args.fDrawContext->height(),
                                   *args.fClip, *args.fShape,
                                   *args.fViewMatrix, &unclippedDevShapeBounds,
                                   &clippedDevShapeBounds,
                                   &devClipBounds)) {
        if (inverseFilled) {
            DrawAroundInvPath(args.fDrawContext, *args.fPaint, *args.fUserStencilSettings,
                              *args.fClip,
                              *args.fViewMatrix, devClipBounds, unclippedDevShapeBounds);

        }
        return true;
    }

    const SkIRect* boundsForMask = &clippedDevShapeBounds;
    if (useCache) {
        // Use the cache only if >50% of the path is visible.
        int unclippedWidth = unclippedDevShapeBounds.width();
        int unclippedHeight = unclippedDevShapeBounds.height();
        int unclippedArea = unclippedWidth * unclippedHeight;
        int clippedArea = clippedDevShapeBounds.width() * clippedDevShapeBounds.height();
        int maxTextureSize = args.fDrawContext->caps()->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            boundsForMask = &unclippedDevShapeBounds;
        }
    }

    GrUniqueKey maskKey;
    struct KeyData {
        SkScalar fFractionalTranslateX;
        SkScalar fFractionalTranslateY;
    };

    if (useCache) {
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = args.fViewMatrix->get(SkMatrix::kMScaleX);
        SkScalar sy = args.fViewMatrix->get(SkMatrix::kMScaleY);
        SkScalar kx = args.fViewMatrix->get(SkMatrix::kMSkewX);
        SkScalar ky = args.fViewMatrix->get(SkMatrix::kMSkewY);
        SkScalar tx = args.fViewMatrix->get(SkMatrix::kMTransX);
        SkScalar ty = args.fViewMatrix->get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&maskKey, kDomain, 5 + args.fShape->unstyledKeySize());
        builder[0] = SkFloat2Bits(sx);
        builder[1] = SkFloat2Bits(sy);
        builder[2] = SkFloat2Bits(kx);
        builder[3] = SkFloat2Bits(ky);
        builder[4] = fracX | (fracY >> 8);
        args.fShape->writeUnstyledKey(&builder[5]);
    }

    sk_sp<GrTexture> texture;
    if (useCache) {
        texture.reset(args.fResourceProvider->findAndRefTextureByUniqueKey(maskKey));
    }
    if (!texture) {
         GrSWMaskHelper::TextureType type = useCache ? GrSWMaskHelper::TextureType::kExactFit
                                                     : GrSWMaskHelper::TextureType::kApproximateFit;
         texture.reset(GrSWMaskHelper::DrawShapeMaskToTexture(fTexProvider, *args.fShape,
                                                              *boundsForMask, args.fAntiAlias,
                                                              type, args.fViewMatrix));
         if (!texture) {
             return false;
         }
         if (useCache) {
             texture->resourcePriv().setUniqueKey(maskKey);
         }
    }

    GrSWMaskHelper::DrawToTargetWithShapeMask(texture.get(), args.fDrawContext, *args.fPaint,
                                              *args.fUserStencilSettings,
                                              *args.fClip, *args.fViewMatrix,
                                              SkIPoint {boundsForMask->fLeft, boundsForMask->fTop},
                                              *boundsForMask);

    if (inverseFilled) {
        DrawAroundInvPath(args.fDrawContext, *args.fPaint, *args.fUserStencilSettings,
                          *args.fClip,
                          *args.fViewMatrix, devClipBounds, unclippedDevShapeBounds);
    }

    return true;
}
