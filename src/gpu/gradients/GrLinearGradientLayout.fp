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

void main() {
    half t = half(sk_TransformedCoords2D[0].x);
    sk_OutColor = half4(t, 1, 0, 0); // y = 1 for always valid
}

//////////////////////////////////////////////////////////////////////////////

@header {
    #include "SkLinearGradient.h"
    #include "GrGradientShader.h"
}

// The linear gradient never rejects a pixel so it doesn't change opacity
@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkLinearGradient& gradient,
                                                     const GrFPArgs& args);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrLinearGradientLayout::Make(
            const SkLinearGradient& grad, const GrFPArgs& args) {
        SkMatrix matrix;
        if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }
        matrix.postConcat(grad.getGradientMatrix());
        return std::unique_ptr<GrFragmentProcessor>(new GrLinearGradientLayout(matrix));
    }
}

//////////////////////////////////////////////////////////////////////////////

@test(d) {
    SkScalar scale = GrGradientShader::RandomParams::kGradientScale;
    SkPoint points[] = {{d->fRandom->nextRangeScalar(0.0f, scale),
                         d->fRandom->nextRangeScalar(0.0f, scale)},
                        {d->fRandom->nextRangeScalar(0.0f, scale),
                         d->fRandom->nextRangeScalar(0.0f, scale)}};

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
