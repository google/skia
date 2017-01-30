/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkPath.h"
#include "SkShadowTessellator.h"

/**
*  Gaussian color filter -- produces a Gaussian ramp based on the color's B value,
*                           then blends with the color's G value.
*                           Final result is black with alpha of Gaussian(B)*G.
*                           The assumption is that the original color's alpha is 1.
*/
class SK_API SkGaussianColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make() {
        return sk_sp<SkColorFilter>(new SkGaussianColorFilter);
    }

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGaussianColorFilter)

protected:
    void flatten(SkWriteBuffer&) const override {}

private:
    SkGaussianColorFilter() : INHERITED() {}

    typedef SkColorFilter INHERITED;
};

void SkGaussianColorFilter::filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const {
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];

        SkScalar factor = SK_Scalar1 - SkGetPackedB32(c) / 255.f;
        factor = SkScalarExp(-factor * factor * 4) - 0.018f;

        dst[i] = SkPackARGB32(factor*SkGetPackedG32(c), 0, 0, 0);
    }
}

sk_sp<SkFlattenable> SkGaussianColorFilter::CreateProc(SkReadBuffer&) {
    return Make();
}

#ifndef SK_IGNORE_TO_STRING
void SkGaussianColorFilter::toString(SkString* str) const {
    str->append("SkGaussianColorFilter ");
}
#endif

#if SK_SUPPORT_GPU
#include "effects/GrBlurredEdgeFragmentProcessor.h"

sk_sp<GrFragmentProcessor> SkGaussianColorFilter::asFragmentProcessor(GrContext*,
                                                                      SkColorSpace*) const {
    return GrBlurredEdgeFP::Make(GrBlurredEdgeFP::kGaussian_Mode);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
static const float kHeightFactor = 1.0f / 128.0f;
static const float kGeomFactor = 64.0f;

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& lightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {

    SkPath xformedPath;
    // TODO: handle transforming the path as part of the tessellator
    path.transform(canvas->getTotalMatrix(), &xformedPath);
    canvas->save();
    canvas->resetMatrix();

    bool transparent = SkToBool(flags & SkShadowFlags::kTransparentOccluder_ShadowFlag);

    if (ambientAlpha > 0) {
        SkScalar radius = occluderHeight * kHeightFactor * kGeomFactor;
        SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(occluderHeight*kHeightFactor, 0.0f)));
        // umbraColor is the interior value, penumbraColor the exterior value.
        // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
        // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
        // the final alpha.
        SkColor  umbraColor = SkColorSetARGB(255, 0, ambientAlpha*255.9999f, umbraAlpha*255.9999f);
        SkColor  penumbraColor = SkColorSetARGB(255, 0, ambientAlpha*255.9999f, 0);

        sk_sp<SkShadowVertices> vertices =
                SkShadowVertices::MakeAmbient(xformedPath, radius, umbraColor, penumbraColor,
                                              transparent);
        SkPaint paint;
        paint.setColor(color);
        paint.setColorFilter(SkGaussianColorFilter::Make());
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode, vertices->vertexCount(),
                             vertices->positions(), nullptr, vertices->colors(),
                             vertices->indices(), vertices->indexCount(), paint);
    }

    if (spotAlpha > 0) {
        float zRatio = SkTPin(occluderHeight / (lightPos.fZ - occluderHeight), 0.0f, 0.95f);
        SkScalar radius = lightRadius * zRatio;

        // Compute the scale and translation for the spot shadow.
        const SkScalar scale = lightPos.fZ / (lightPos.fZ - occluderHeight);

        SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
        const SkVector spotOffset = SkVector::Make(zRatio*(center.fX - lightPos.fX),
                                                   zRatio*(center.fY - lightPos.fY));

        SkColor  umbraColor = SkColorSetARGB(255, 0, spotAlpha*255.9999f, 255);
        SkColor  penumbraColor = SkColorSetARGB(255, 0, spotAlpha*255.9999f, 0);
        sk_sp<SkShadowVertices> vertices =
                SkShadowVertices::MakeSpot(xformedPath, scale, spotOffset, radius, umbraColor,
                                           penumbraColor, transparent);
        SkPaint paint;
        paint.setColor(color);
        paint.setColorFilter(SkGaussianColorFilter::Make());
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode, vertices->vertexCount(),
                             vertices->positions(), nullptr, vertices->colors(),
                             vertices->indices(), vertices->indexCount(), paint);
    }

    canvas->restore();
}
