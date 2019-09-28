/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D mask;
in uniform half innerThreshold;
in uniform half outerThreshold;

@class {
    inline OptimizationFlags optFlags(float outerThreshold);
}

@constructorParams {
    const SkIRect& bounds
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> mask,
                                                     float innerThreshold,
                                                     float outerThreshold,
                                                     const SkIRect& bounds) {
        return std::unique_ptr<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(
                mask, innerThreshold, outerThreshold, bounds));
    }
}

@coordTransform(mask) {
    SkMatrix::MakeTrans(SkIntToScalar(-bounds.x()), SkIntToScalar(-bounds.y()))
}

@cpp {
    inline GrFragmentProcessor::OptimizationFlags GrAlphaThresholdFragmentProcessor::optFlags(
                                                                             float outerThreshold) {
        if (outerThreshold >= 1.0) {
            return kPreservesOpaqueInput_OptimizationFlag |
                   kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }
}

void main() {
    half4 color = sk_InColor;
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
    sk_sp<GrTextureProxy> maskProxy = testData->textureProxy(GrProcessorUnitTest::kAlphaTextureIdx);
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
    return GrAlphaThresholdFragmentProcessor::Make(std::move(maskProxy), innerThresh, outerThresh,
                                                   bounds);
}
