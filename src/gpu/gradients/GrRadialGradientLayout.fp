/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
#include "GrGradientShader.h"
}

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
    half t = length(sk_TransformedCoords2D[0]);
    sk_OutColor = half4(t, 1, 0, 0); // y = 1 for always valid
 }

@test(d) {
    sk_sp<SkShader> shader;
    do {
        GrGradientShader::RandomParams params(d->fRandom);
        SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
        SkScalar radius = d->fRandom->nextUScalar1();
        shader = params.fUseColors4f
                         ? SkGradientShader::MakeRadial(center, radius, params.fColors4f,
                                                        params.fColorSpace, params.fStops,
                                                        params.fColorCount, params.fTileMode)
                         : SkGradientShader::MakeRadial(center, radius, params.fColors,
                                                        params.fStops, params.fColorCount,
                                                        params.fTileMode);
    } while (!shader);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
