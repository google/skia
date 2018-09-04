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
    static std::unique_ptr<GrFragmentProcessor> Make(const SkMatrix& inverseLocal,
                                                     const SkPoint& start, const SkPoint& end);
}

@cppEnd {
    static SkMatrix pts_to_unit_matrix(const SkPoint& start, const SkPoint& end) {
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
    half t = sk_TransformedCoords2D[0].x;
    sk_OutColor = half4(t, 1, 0, 0); // y = 1 for always valid
}

@test(d) {
    SkPoint points[] = {{d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()},
                        {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()}};

    GrGradientShader::RandomParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeLinear(points, params.fColors4f, params.fColorSpace, params.fStops,
                                     params.fColorCount, params.fTileMode) :
        SkGradientShader::MakeLinear(points, params.fColors, params.fStops,
                                     params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
