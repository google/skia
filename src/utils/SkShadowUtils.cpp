/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "SkGaussianEdgeShader.h"
#include "SkPath.h"
//#include "../effects/shadows/SkAmbientShadowMaskFilter.h"
//#include "../effects/shadows/SkSpotShadowMaskFilter.h"

#include "SkShadowTessellator.h"

static const float kHeightFactor = 1.0f / 128.0f;
static const float kGeomFactor = 64.0f;

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& lightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {

    if (ambientAlpha > 0) {
        SkScalar radius = occluderHeight * kHeightFactor * kGeomFactor;
        SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(occluderHeight * kHeightFactor, 0.0f)));
        // umbraColor is the interior value, penumbraColor the exterior value.
        // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
        // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
        // the final alpha.
        SkColor  umbraColor = SkColorSetARGB(ambientAlpha*255.9999f, 0, 0, umbraAlpha*255.9999f);
        SkColor  penumbraColor = SkColorSetARGB(ambientAlpha*255.9999f, 0, 0, 0);

        SkAmbientShadowTessellator tess(path, radius, umbraColor, penumbraColor,
                                        SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag));

        SkPaint paint;
        paint.setColor(color);
        // TODO: make shader work with drawVertices?
        paint.setShader(SkGaussianEdgeShader::Make());
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode, tess.vertexCount(), tess.positions(),
                             nullptr, tess.colors(), tess.indices(), tess.indexCount(), paint);
    }

    if (spotAlpha > 0) {
        float zRatio = SkTPin(occluderHeight / (lightPos.fZ - occluderHeight), 0.0f, 0.95f);
        SkScalar radius = lightRadius * zRatio;

        // Compute the scale and translation for the spot shadow.
        const SkScalar scale = lightPos.fZ / (lightPos.fZ - occluderHeight);

        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        const SkVector spotOffset = SkVector::Make(zRatio*(center.fX - lightPos.fX),
                                                   zRatio*(center.fY - lightPos.fY));

        SkColor  umbraColor = SkColorSetARGB(spotAlpha*255.9999f, 0, 0, 255);
        SkColor  penumbraColor = SkColorSetARGB(spotAlpha*255.9999f, 0, 0, 0);
        SkSpotShadowTessellator tess(path, scale, spotOffset, radius, umbraColor, penumbraColor,
                                     SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag));

        SkPaint paint;
        paint.setColor(color);
        // TODO: make shader work with drawVertices?
        paint.setShader(SkGaussianEdgeShader::Make());
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode, tess.vertexCount(), tess.positions(),
                             nullptr, tess.colors(), tess.indices(), tess.indexCount(), paint);
    }

    //SkPaint newPaint;
    //newPaint.setColor(color);
    //newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderHeight, ambientAlpha, flags));
    //canvas->drawPath(path, newPaint);
    //newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
    //                                                    spotAlpha, flags));
    //canvas->drawPath(path, newPaint);
}
