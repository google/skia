in uniform sampler2D image;
in uniform colorSpaceXform colorXform;
in uniform sampler2D mask;
in uniform float innerThreshold;
in uniform float outerThreshold;

@class {
    inline OptimizationFlags optFlags(float outerThreshold);
}

@constructorParams {
    const SkIRect& bounds
}

@make {
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> image,
                                           sk_sp<GrColorSpaceXform> colorXform,
                                           sk_sp<GrTextureProxy> mask,
                                           float innerThreshold,
                                           float outerThreshold,
                                           const SkIRect& bounds) {
        return sk_sp<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(image,
                                                                                colorXform,
                                                                                mask,
                                                                                innerThreshold,
                                                                                outerThreshold,
                                                                                bounds));
    }
}

@coordTransform(image) {
    SkMatrix::I()
}

@coordTransform(mask) {
    SkMatrix::MakeTrans(SkIntToScalar(-bounds.x()), SkIntToScalar(-bounds.y()))
}

@header {
    #include "GrColorSpaceXform.h"
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
    vec4 color = texture(image, sk_TransformedCoords2D[0], colorXform);
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
                                std::move(bmpProxy),
                                colorSpaceXform,
                                std::move(maskProxy),
                                innerThresh, outerThresh,
                                bounds);
}