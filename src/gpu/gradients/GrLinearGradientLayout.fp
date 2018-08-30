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
        const SkMatrix& inverseLocal,
        const SkPoint& start, const SkPoint& end);
}

@cppEnd {
    static SkMatrix pts_to_unit_matrix(const SkPoint& start,
                                       const SkPoint& end) {
        SkVector    vec = end - start;
        SkScalar    mag = vec.length();
        SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

        vec.scale(inv);
        SkMatrix matrix;
        matrix.setSinCos(-vec.fY, vec.fX, start.fX, start.fY);
        matrix.postTranslate(-start.fX, -start.fY);
        matrix.postScale(inv, inv);
        return matrix;
    }

    std::unique_ptr<GrFragmentProcessor> GrLinearGradientLayout::Make(
            const SkMatrix& inverseLocal, const SkPoint& start, const SkPoint& end) {
        SkMatrix matrix = inverseLocal;
        matrix.postConcat(pts_to_unit_matrix(start, end));
        return std::unique_ptr<GrFragmentProcessor>(new GrLinearGradientLayout(matrix));
    }
}

void main() {
    sk_OutColor = half4(sk_TransformedCoords2D[0].x);
}
