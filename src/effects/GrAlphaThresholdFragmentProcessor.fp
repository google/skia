/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D image;
in uniform GrColorSpaceXform colorXform;
in uniform sampler2D mask;
in uniform float innerThreshold;
in uniform float outerThreshold;

@class {
    inline OptimizationFlags OptFlags(float outerThreshold);
}

@constructorParams {
    GrResourceProvider* resourceProvider,
    const SkIRect& bounds
}

@make {
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> image, 
                                           sk_sp<GrColorSpaceXform> colorXform,
                                           sk_sp<GrTextureProxy> mask, 
                                           float innerThreshold,
                                           float outerThreshold,
                                           const SkIRect& bounds) {
        return sk_sp<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(image, colorXform, mask, innerThreshold, outerThreshold, resourceProvider, bounds));
    }
}

@fields {
    GrCoordTransform fImageCoordTransform;
    GrCoordTransform fMaskCoordTransform;
}

@initializers {
    fImageCoordTransform(resourceProvider, SkMatrix::I(), image.get()),
    fMaskCoordTransform(resourceProvider,
                        SkMatrix::MakeTrans(SkIntToScalar(-bounds.x()), SkIntToScalar(-bounds.y())),
                        mask.get())
}

@constructorCode {
    this->addCoordTransform(&fImageCoordTransform);
    this->addCoordTransform(&fMaskCoordTransform);
}

@body {
    inline GrFragmentProcessor::OptimizationFlags GrAlphaThresholdFragmentProcessor::OptFlags(
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
    vec4 color = texture(image, sk_TransformedCoords2D[0]);
    if (colorXform != mat4(1)) {
        color.rgb = clamp((colorXform * vec4(color.rgb, 1.0)).rgb, 0.0, color.a);
    }
    vec4 mask_color = texture(mask, sk_TransformedCoords2D[1]);
    if (mask_color.a < 0.5) {
        if (color.a > outerThreshold) {
            float scale = outerThreshold / color.a;
            color.rgb *= scale;
            color.a = outerThreshold;
        }
    } else if (color.a < innerThreshold) {
        float scale = innerThreshold / max(0.001, color.a);
        color.rgb *= scale;
        color.a = innerThreshold;
    }
    sk_OutColor = color;
}

@test(testData) {
    sk_sp<GrTextureProxy> bmpProxy = testData->textureProxy(GrProcessorUnitTest::kSkiaPMTextureIdx);
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
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(testData->fRandom);
    return GrAlphaThresholdFragmentProcessor::Make(
                                testData->resourceProvider(),
                                std::move(bmpProxy),
                                colorSpaceXform,
                                std::move(maskProxy),
                                innerThresh, outerThresh,
                                bounds);    
}
