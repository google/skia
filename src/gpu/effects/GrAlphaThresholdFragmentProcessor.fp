/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? inputFP;
in fragmentProcessor  maskFP;
in uniform half innerThreshold;
in uniform half outerThreshold;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (kCompatibleWithCoverageAsAlpha_OptimizationFlag |
     ((outerThreshold >= 1.0) ? kPreservesOpaqueInput_OptimizationFlag : kNone_OptimizationFlags))
}

void main() {
    half4 color = sample(inputFP, sk_InColor);
    half4 mask_color = sample(maskFP);
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
    std::unique_ptr<GrFragmentProcessor> inputChild, maskChild;
    if (testData->fRandom->nextBool()) {
        inputChild = GrProcessorUnitTest::MakeChildFP(testData);
    }
    maskChild = GrProcessorUnitTest::MakeChildFP(testData);

    return GrAlphaThresholdFragmentProcessor::Make(std::move(inputChild), std::move(maskChild),
                                                   innerThresh, outerThresh);
}
