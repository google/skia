/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"

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

static constexpr SkScalar kSnapEpsilon = 1.f / (1 << 6); // ~ .015

static SkScalar snap_to_zero(SkScalar v) {
    return SkScalarNearlyZero(v, kSnapEpsilon) ? 0.f : v;
}

static SkScalar snap_to_one(SkScalar v) {
    return SkScalarNearlyEqual(v, 1.f, kSnapEpsilon) ? 1.f : v;
}

static SkScalar snap_to_int(SkScalar v) {
    SkScalar iv = SkScalarRoundToScalar(v);
    return SkScalarNearlyEqual(v, iv, kSnapEpsilon) ? iv : v;
}

// Snap components of the incoming matrix to match a scale translate matrix, cleaning up floating
// point error that may have crept in that makes integer translations and scale factors slightly not
static SkMatrix make_clean_matrix(const SkMatrix& ctm) {
    return SkMatrix::MakeAll(
            snap_to_int(ctm.get(0)), snap_to_int(ctm.get(1)), snap_to_int(ctm.get(2)),
            snap_to_int(ctm.get(3)), snap_to_int(ctm.get(4)), snap_to_int(ctm.get(5)),
            snap_to_zero(ctm.get(6)), snap_to_zero(ctm.get(7)), snap_to_one(ctm.get(8)));
}

namespace skif {

Mapping Mapping::DecomposeCTM(const SkMatrix& ctm, const SkImageFilter* filter) {
    SkMatrix m = ctm; //make_clean_matrix(ctm);
    SkMatrix remainder, layer;
    SkSize scale;
    if (!filter || m.isScaleTranslate() || as_IFB(filter)->canHandleComplexCTM()) {
        // It doesn't matter what type of matrix ctm is, we can have layer space be equivalent to
        // device space.
        remainder = SkMatrix::I();
        layer = m;
    } else if (m.decomposeScale(&scale, &remainder)) {
        // TODO (michaelludwig) - Should maybe strip out any fractional part of the translation in
        // 'ctm' so that can be incorporated during regular drawing, instead of by resampling the
        // filtered image.
        // FIXME: remainder * layer doesn't always equal m, so we may fail later isScaleTarnslate tests, etc.
        layer = SkMatrix::Scale(scale.fWidth, scale.fHeight);
    } else {
        // Perspective
        // TODO (michaelludwig) - Should investigate choosing a scale factor for the layer matrix
        // that minimizes the aliasing in the final draw.
        remainder = m;
        layer = SkMatrix::I();
    }
    return Mapping(remainder, layer);
}

// Instantiate map specializations for the 6 geometric types used during filtering
template<>
SkIRect Mapping::map<SkIRect>(const SkIRect& geom, const SkMatrix& matrix) {
    return matrix.mapRect(SkRect::Make(geom)).roundOut();
}

template<>
SkRect Mapping::map<SkRect>(const SkRect& geom, const SkMatrix& matrix) {
    return matrix.mapRect(geom);
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

} // end namespace skif
