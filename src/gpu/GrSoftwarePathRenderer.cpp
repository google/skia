
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSoftwarePathRenderer.h"
#include "GrContext.h"
#include "GrSWMaskHelper.h"

////////////////////////////////////////////////////////////////////////////////
bool GrSoftwarePathRenderer::canDrawPath(const SkPath& path,
                                         GrPathFill fill,
                                         const GrDrawTarget* target,
                                         bool antiAlias) const {
    if (!antiAlias || NULL == fContext) {
        // TODO: We could allow the SW path to also handle non-AA paths but
        // this would mean that GrDefaultPathRenderer would never be called
        // (since it appears after the SW renderer in the path renderer
        // chain). Some testing would need to be done r.e. performance
        // and consistency of the resulting images before removing
        // the "!antiAlias" clause from the above test
        return false;
    }

    return true;
}

namespace {

////////////////////////////////////////////////////////////////////////////////
// gets device coord bounds of path (not considering the fill) and clip. The
// path bounds will be a subset of the clip bounds. returns false if
// path bounds would be empty.
bool get_path_and_clip_bounds(const GrDrawTarget* target,
                              const SkPath& path,
                              const GrMatrix& matrix,
                              GrIRect* devPathBounds,
                              GrIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    const GrRenderTarget* rt = target->getDrawState().getRenderTarget();
    if (NULL == rt) {
        return false;
    }
    *devPathBounds = GrIRect::MakeWH(rt->width(), rt->height());

    target->getClip()->getConservativeBounds(rt, devClipBounds);

    // TODO: getConservativeBounds already intersects with the
    // render target's bounding box. Remove this next line
    if (!devPathBounds->intersect(*devClipBounds)) {
        return false;
    }

    if (!path.getBounds().isEmpty()) {
        GrRect pathSBounds;
        matrix.mapRect(&pathSBounds, path.getBounds());
        GrIRect pathIBounds;
        pathSBounds.roundOut(&pathIBounds);
        if (!devPathBounds->intersect(pathIBounds)) {
            // set the correct path bounds, as this would be used later.
            *devPathBounds = pathIBounds;
            return false;
        }
    } else {
        *devPathBounds = GrIRect::EmptyIRect();
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void draw_around_inv_path(GrDrawTarget* target,
                          const GrIRect& devClipBounds,
                          const GrIRect& devPathBounds) {
    GrDrawTarget::AutoDeviceCoordDraw adcd(target);
    if (!adcd.succeeded()) {
        return;
    }
    GrRect rect;
    if (devClipBounds.fTop < devPathBounds.fTop) {
        rect.iset(devClipBounds.fLeft, devClipBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fTop);
        target->drawSimpleRect(rect, NULL);
    }
    if (devClipBounds.fLeft < devPathBounds.fLeft) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fTop,
                  devPathBounds.fLeft, devPathBounds.fBottom);
        target->drawSimpleRect(rect, NULL);
    }
    if (devClipBounds.fRight > devPathBounds.fRight) {
        rect.iset(devPathBounds.fRight, devPathBounds.fTop,
                  devClipBounds.fRight, devPathBounds.fBottom);
        target->drawSimpleRect(rect, NULL);
    }
    if (devClipBounds.fBottom > devPathBounds.fBottom) {
        rect.iset(devClipBounds.fLeft, devPathBounds.fBottom,
                  devClipBounds.fRight, devClipBounds.fBottom);
        target->drawSimpleRect(rect, NULL);
    }
}

}

////////////////////////////////////////////////////////////////////////////////
// return true on success; false on failure
bool GrSoftwarePathRenderer::onDrawPath(const SkPath& path,
                                        GrPathFill fill,
                                        const GrVec* translate,
                                        GrDrawTarget* target,
                                        bool antiAlias) {

    if (NULL == fContext) {
        return false;
    }

    GrDrawState* drawState = target->drawState();

    GrMatrix vm = drawState->getViewMatrix();
    if (NULL != translate) {
        vm.postTranslate(translate->fX, translate->fY);
    }

    GrIRect devPathBounds, devClipBounds;
    if (!get_path_and_clip_bounds(target, path, vm,
                                  &devPathBounds, &devClipBounds)) {
        if (GrIsFillInverted(fill)) {
            draw_around_inv_path(target, devClipBounds, devPathBounds);
        }
        return true;
    }

    SkAutoTUnref<GrTexture> texture(
            GrSWMaskHelper::DrawPathMaskToTexture(fContext, path,
                                                  devPathBounds, fill,
                                                  antiAlias, &vm));
    if (NULL == texture) {
        return false;
    }

    GrSWMaskHelper::DrawToTargetWithPathMask(texture, target, devPathBounds);

    if (GrIsFillInverted(fill)) {
        draw_around_inv_path(target, devClipBounds, devPathBounds);
    }

    return true;
}
