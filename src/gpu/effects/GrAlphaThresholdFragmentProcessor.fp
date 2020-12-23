/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
in fragmentProcessor  maskFP;
in uniform half innerThreshold;
in uniform half outerThreshold;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    ((outerThreshold >= 1.0) ? kPreservesOpaqueInput_OptimizationFlag : kNone_OptimizationFlags)
}

half4 main() {
    half4 color = sample(inputFP);
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
    return color;
}

@test(testData) {
    // Make the inner and outer thresholds be in [0, 1].
    float outerThresh = testData->fRandom->nextUScalar1();
    float innerThresh = testData->fRandom->nextUScalar1();
    std::unique_ptr<GrFragmentProcessor> inputChild, maskChild;
    if (testData->fRandom->nextBool()) {
        inputChild = GrProcessorUnitTest::MakeChildFP(testData);
    }
    maskChild = GrProcessorUnitTest::MakeChildFP(testData);

    return GrAlphaThresholdFragmentProcessor::Make(std::move(inputChild), std::move(maskChild),
                                                   innerThresh, outerThresh);
}
