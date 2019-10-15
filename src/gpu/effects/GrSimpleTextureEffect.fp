/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D image;
in GrColorType srcColorType;
in half4x4 matrix;

@constructorParams {
    GrSamplerState samplerParams
}

@coordTransform(image) {
    matrix
}

@samplerParams(image) {
    samplerParams
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     GrColorType srcColorType,
                                                     const SkMatrix& matrix) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), srcColorType, matrix,
                    GrSamplerState(GrSamplerState::WrapMode::kClamp, GrSamplerState::Filter::kNearest)));
    }

    /* clamp mode */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     GrColorType srcColorType,
                                                     const SkMatrix& matrix,
                                                     GrSamplerState::Filter filter) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), srcColorType, matrix,
                                      GrSamplerState(GrSamplerState::WrapMode::kClamp, filter)));
     }

    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                                     GrColorType srcColorType,
                                                     const SkMatrix& matrix,
                                                     const GrSamplerState& p) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), srcColorType, matrix, p));
    }
}

@optimizationFlags {
    ModulateForSamplerOptFlags(srcColorType,
            samplerParams.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
            samplerParams.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder)
}

void main() {
    sk_OutColor = sk_InColor * sample(image, sk_TransformedCoords2D[0]);
}

@test(testData) {
    int texIdx = testData->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                               : GrProcessorUnitTest::kAlphaTextureIdx;
    GrSamplerState::WrapMode wrapModes[2];
    GrTest::TestWrapModes(testData->fRandom, wrapModes);
    if (!testData->caps()->npotTextureTileSupport()) {
        // Performing repeat sampling on npot textures will cause asserts on HW
        // that lacks support.
        wrapModes[0] = GrSamplerState::WrapMode::kClamp;
        wrapModes[1] = GrSamplerState::WrapMode::kClamp;
    }

    GrSamplerState params(wrapModes, testData->fRandom->nextBool()
                                                               ? GrSamplerState::Filter::kBilerp
                                                               : GrSamplerState::Filter::kNearest);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    return GrSimpleTextureEffect::Make(testData->textureProxy(texIdx),
                                       testData->textureProxyColorType(texIdx), matrix, params);
}
