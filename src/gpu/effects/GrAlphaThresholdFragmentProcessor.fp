/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? inputFP;
in uniform sampler2D mask;
in uniform half innerThreshold;
in uniform half outerThreshold;

@constructorParams {
    const SkIRect& bounds
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     GrSurfaceProxyView mask,
                                                     float innerThreshold,
                                                     float outerThreshold,
                                                     const SkIRect& bounds) {
        return std::unique_ptr<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(
                std::move(inputFP), std::move(mask), innerThreshold, outerThreshold, bounds));
    }
}

@coordTransform(mask) {
    SkMatrix::Translate(SkIntToScalar(-bounds.x()), SkIntToScalar(-bounds.y()))
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (kCompatibleWithCoverageAsAlpha_OptimizationFlag |
     ((outerThreshold >= 1.0) ? kPreservesOpaqueInput_OptimizationFlag : kNone_OptimizationFlags))
}

void main() {
    half4 color = sample(inputFP, sk_InColor);
    half4 mask_color = sample(mask, sk_TransformedCoords2D[0]);
    if (mask_color.a < 0.5) {
        if (color.a > outerThreshold) {
            half scale = outerThreshold / color.a;
            color.rgb *= scale;
            color.a = outerThreshold;
        }
    } else if (color.a < innerThreshold) {
        half scale = innerThreshold / max(0.001, color.a);
        color.rgb *= scale;
        color.a = innerThreshold;
    }
    sk_OutColor = color;
}

@test(testData) {
    auto [maskView, ct, at] = testData->randomAlphaOnlyView();
    // Make the inner and outer thresholds be in (0, 1) exclusive and be sorted correctly.
    float innerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    float outerThresh = testData->fRandom->nextUScalar1() * .99f + 0.005f;
    const int kMaxWidth = 1000;
    const int kMaxHeight = 1000;
    uint32_t width = testData->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = testData->fRandom->nextULessThan(kMaxHeight);
    uint32_t x = testData->fRandom->nextULessThan(kMaxWidth - width);
    uint32_t y = testData->fRandom->nextULessThan(kMaxHeight - height);
    SkIRect bounds = SkIRect::MakeXYWH(x, y, width, height);

    return GrAlphaThresholdFragmentProcessor::Make(/*inputFP=*/nullptr, std::move(maskView),
                                                   innerThresh, outerThresh, bounds);
}
