/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 in half4x4 gradientMatrix;

 @coordTransform {
    gradientMatrix
 }

 @make {
    static std::unique_ptr<GrFragmentProcessor> Make(
        const SkMatrix& inverseLocal, const SkPoint& center, SkScalar radius);
 }

 @cppEnd {
    static SkMatrix rad_to_unit_matrix(const SkPoint& center, SkScalar radius) {
        SkScalar inv = SkScalarInvert(radius);

        SkMatrix matrix;
        matrix.setTranslate(-center.fX, -center.fY);
        matrix.postScale(inv, inv);
        return matrix;
    }

    std::unique_ptr<GrFragmentProcessor> GrRadialGradientLayout::Make(
            const SkMatrix& inverseLocal, const SkPoint& center, SkScalar radius) {
        SkMatrix matrix = inverseLocal;
        matrix.postConcat(rad_to_unit_matrix(center, radius));
        return std::unique_ptr<GrFragmentProcessor>(new GrRadialGradientLayout(matrix));
    }
 }

 void main() {
    sk_OutColor = half4(length(sk_TransformedCoords2D[0].xy));
 }
