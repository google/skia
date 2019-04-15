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
    half t = half(length(sk_TransformedCoords2D[0]));
    sk_OutColor = half4(t, 1, 0, 0); // y = 1 for always valid
}

//////////////////////////////////////////////////////////////////////////////

@header {
    #include "SkRadialGradient.h"
    #include "../GrGradientShader.h"
}

// The radial gradient never rejects a pixel so it doesn't change opacity
@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkRadialGradient& gradient,
                                                     const GrFPArgs& args);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrRadialGradientLayout::Make(
            const SkRadialGradient& grad, const GrFPArgs& args) {
        SkMatrix matrix;
        if (!grad.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }
        matrix.postConcat(grad.getGradientMatrix());
        return std::unique_ptr<GrFragmentProcessor>(new GrRadialGradientLayout(matrix));
    }
}

//////////////////////////////////////////////////////////////////////////////

@test(d) {
    SkScalar scale = GrGradientShader::RandomParams::kGradientScale;
    std::unique_ptr<GrFragmentProcessor> fp;
    GrTest::TestAsFPArgs asFPArgs(d);
    do {
        GrGradientShader::RandomParams params(d->fRandom);
        SkPoint center = {d->fRandom->nextRangeScalar(0.0f, scale),
                          d->fRandom->nextRangeScalar(0.0f, scale)};
        SkScalar radius = d->fRandom->nextRangeScalar(0.0f, scale);
        sk_sp<SkShader> shader = params.fUseColors4f
                         ? SkGradientShader::MakeRadial(center, radius, params.fColors4f,
                                                        params.fColorSpace, params.fStops,
                                                        params.fColorCount, params.fTileMode)
                         : SkGradientShader::MakeRadial(center, radius, params.fColors,
                                                        params.fStops, params.fColorCount,
                                                        params.fTileMode);
        // Degenerate params can create an Empty (non-null) shader, where fp will be nullptr
        fp = shader ? as_SB(shader)->asFragmentProcessor(asFPArgs.args()) : nullptr;
    } while (!fp);
    return fp;
}
