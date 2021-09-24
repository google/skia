/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkMatrixPriv.h"

// Both [I]Vectors and Sk[I]Sizes are transformed as non-positioned values, i.e. go through
// mapVectors() not mapPoints().
static SkIVector map_as_vector(int32_t x, int32_t y, const SkMatrix& matrix) {
    SkVector v = SkVector::Make(SkIntToScalar(x), SkIntToScalar(y));
    matrix.mapVectors(&v, 1);
    return SkIVector::Make(SkScalarRoundToInt(v.fX), SkScalarRoundToInt(v.fY));
}

static SkVector map_as_vector(SkScalar x, SkScalar y, const SkMatrix& matrix) {
    SkVector v = SkVector::Make(x, y);
    matrix.mapVectors(&v, 1);
    return v;
}

namespace skif {

bool Mapping::decomposeCTM(const SkMatrix& ctm, const SkImageFilter* filter,
                           const skif::ParameterSpace<SkPoint>& representativePt) {
    SkMatrix remainder, layer;
    SkSize decomposed;
    using MatrixCapability = SkImageFilter_Base::MatrixCapability;
    MatrixCapability capability =
            filter ? as_IFB(filter)->getCTMCapability() : MatrixCapability::kComplex;
    if (capability == MatrixCapability::kTranslate) {
        // Apply the entire CTM post-filtering
        remainder = ctm;
        layer = SkMatrix::I();
    } else if (ctm.isScaleTranslate() || capability == MatrixCapability::kComplex) {
        // Either layer space can be anything (kComplex) - or - it can be scale+translate, and the
        // ctm is. In both cases, the layer space can be equivalent to device space.
        remainder = SkMatrix::I();
        layer = ctm;
    } else if (ctm.decomposeScale(&decomposed, &remainder)) {
        // This case implies some amount of sampling post-filtering, either due to skew or rotation
        // in the original matrix. As such, keep the layer matrix as simple as possible.
        layer = SkMatrix::Scale(decomposed.fWidth, decomposed.fHeight);
    } else {
        // Perspective, which has a non-uniform scaling effect on the filter. Pick a single scale
        // factor that best matches where the filter will be evaluated.
        SkScalar scale = SkMatrixPriv::DifferentialAreaScale(ctm, SkPoint(representativePt));
        if (SkScalarIsFinite(scale) && !SkScalarNearlyZero(scale)) {
            // Now take the sqrt to go from an area scale factor to a scaling per X and Y
            // FIXME: It would be nice to be able to choose a non-uniform scale.
            scale = SkScalarSqrt(scale);
        } else {
            // The representative point was behind the W = 0 plane, so don't factor out any scale.
            // NOTE: This makes remainder and layer the same as the MatrixCapability::Translate case
            scale = 1.f;
        }

        remainder = ctm;
        remainder.preScale(SkScalarInvert(scale), SkScalarInvert(scale));
        layer = SkMatrix::Scale(scale, scale);
    }

    SkMatrix invRemainder;
    if (!remainder.invert(&invRemainder)) {
        // Under floating point arithmetic, it's possible to decompose an invertible matrix into
        // a scaling matrix and a remainder and have the remainder be non-invertible. Generally
        // when this happens the scale factors are so large and the matrix so ill-conditioned that
        // it's unlikely that any drawing would be reasonable, so failing to make a layer is okay.
        return false;
    } else {
        fParamToLayerMatrix = layer;
        fLayerToDevMatrix = remainder;
        fDevToLayerMatrix = invRemainder;
        return true;
    }
}

bool Mapping::adjustLayerSpace(const SkMatrix& layer) {
    SkMatrix invLayer;
    if (!layer.invert(&invLayer)) {
        return false;
    }
    fParamToLayerMatrix.postConcat(layer);
    fDevToLayerMatrix.postConcat(layer);
    fLayerToDevMatrix.preConcat(invLayer);
    return true;
}

// Instantiate map specializations for the 6 geometric types used during filtering
template<>
SkRect Mapping::map<SkRect>(const SkRect& geom, const SkMatrix& matrix) {
    return matrix.mapRect(geom);
}

template<>
SkIRect Mapping::map<SkIRect>(const SkIRect& geom, const SkMatrix& matrix) {
    SkRect mapped = matrix.mapRect(SkRect::Make(geom));
    mapped.inset(1e-3f, 1e-3f);
    return mapped.roundOut();
}

template<>
SkIPoint Mapping::map<SkIPoint>(const SkIPoint& geom, const SkMatrix& matrix) {
    SkPoint p = SkPoint::Make(SkIntToScalar(geom.fX), SkIntToScalar(geom.fY));
    matrix.mapPoints(&p, 1);
    return SkIPoint::Make(SkScalarRoundToInt(p.fX), SkScalarRoundToInt(p.fY));
}

template<>
SkPoint Mapping::map<SkPoint>(const SkPoint& geom, const SkMatrix& matrix) {
    SkPoint p;
    matrix.mapPoints(&p, &geom, 1);
    return p;
}

template<>
IVector Mapping::map<IVector>(const IVector& geom, const SkMatrix& matrix) {
    return IVector(map_as_vector(geom.fX, geom.fY, matrix));
}

template<>
Vector Mapping::map<Vector>(const Vector& geom, const SkMatrix& matrix) {
    return Vector(map_as_vector(geom.fX, geom.fY, matrix));
}

template<>
SkISize Mapping::map<SkISize>(const SkISize& geom, const SkMatrix& matrix) {
    SkIVector v = map_as_vector(geom.fWidth, geom.fHeight, matrix);
    return SkISize::Make(v.fX, v.fY);
}

template<>
SkSize Mapping::map<SkSize>(const SkSize& geom, const SkMatrix& matrix) {
    SkVector v = map_as_vector(geom.fWidth, geom.fHeight, matrix);
    return SkSize::Make(v.fX, v.fY);
}

FilterResult FilterResult::resolveToBounds(const LayerSpace<SkIRect>& newBounds) const {
    // NOTE(michaelludwig) - This implementation is based on the assumption that an image resolved
    // to 'newBounds' will be decal tiled and that the current image is decal tiled. Because of this
    // simplification, the resolved image is always a subset of 'fImage' that matches the
    // intersection of 'newBounds' and 'layerBounds()' so no rendering/copying is needed.
    LayerSpace<SkIRect> tightBounds = newBounds;
    if (!fImage || !tightBounds.intersect(this->layerBounds())) {
        return {}; //  Fully transparent
    }

    // Calculate offset from old origin to new origin, representing the relative subset in the image
    LayerSpace<IVector> originShift = tightBounds.topLeft() - fOrigin;

    auto subsetImage = fImage->makeSubset(SkIRect::MakeXYWH(originShift.x(), originShift.y(),
                                          tightBounds.width(), tightBounds.height()));
    return {std::move(subsetImage), tightBounds.topLeft()};
}

} // end namespace skif
