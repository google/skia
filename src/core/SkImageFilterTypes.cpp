/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"

namespace skif {

Mapping Mapping::Make(const SkMatrix& ctm, const SkImageFilter* filter) {
    SkMatrix remainder, layer;
    SkSize scale;
    if (ctm.isScaleTranslate() || as_IFB(filter)->canHandleComplexCTM()) {
        // It doesn't matter what type of matrix ctm is, we can have layer space
        // be equivalent to device space.
        remainder = SkMatrix::I();
        layer = ctm;
    } else if (ctm.decomposeScale(&scale, &remainder)) {
        // TODO (michaelludwig) - Should maybe strip out any fractional part of
        // the translation in 'ctm' so that can be incorporated during regular
        // drawing, instead of by resampling the filtered image.
        layer = SkMatrix::MakeScale(scale.fWidth, scale.fHeight);
    } else {
        // Perspective
        remainder = ctm;
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
SkIDirection Mapping::map<SkIDirection>(const SkIDirection& geom, const SkMatrix& matrix) {
    SkVector v = SkVector::Make(SkIntToScalar(geom->fX), SkIntToScalar(geom->fY));
    matrix.mapVectors(&v, 1);
    return SkIVector::Make(SkScalarRoundToInt(v.fX), SkScalarRoundToInt(v.fY));
}

template<>
SkDirection Mapping::map<SkDirection>(const SkDirection& geom, const SkMatrix& matrix) {
    SkVector v = geom;
    matrix.mapVectors(&v, 1);
    return v;
}

} // end namespace skif
