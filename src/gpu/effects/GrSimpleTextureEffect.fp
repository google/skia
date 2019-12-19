/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D image;
in half4x4 matrix;
layout(key) in SkBlendMode mode;

@constructorParams {
    SkAlphaType alphaType,
    GrSamplerState samplerParams
}

@coordTransform(image) {
    matrix
}

@samplerParams(image) {
    samplerParams
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy> proxy,
                                                     SkAlphaType alphaType,
                                                     const SkMatrix& matrix,
                                                     SkBlendMode mode = SkBlendMode::kSrcIn) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), matrix, (int)mode, alphaType,
                    GrSamplerState(GrSamplerState::WrapMode::kClamp, GrSamplerState::Filter::kNearest)));
    }

    /* clamp mode */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy> proxy,
                                                     SkAlphaType alphaType,
                                                     const SkMatrix& matrix,
                                                     GrSamplerState::Filter filter,
                                                     SkBlendMode mode = SkBlendMode::kSrcIn) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), matrix, (int)mode,  alphaType,
                                      GrSamplerState(GrSamplerState::WrapMode::kClamp, filter)));
     }

    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy> proxy,
                                                     SkAlphaType alphaType,
                                                     const SkMatrix& matrix,
                                                     const GrSamplerState& p,
                                                     SkBlendMode mode = SkBlendMode::kSrcIn) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), matrix, (int)mode, alphaType, p));
    }
}

@optimizationFlags {
    ModulateForSamplerOptFlags(alphaType,
            samplerParams.wrapModeX() == GrSamplerState::WrapMode::kClampToBorder ||
            samplerParams.wrapModeY() == GrSamplerState::WrapMode::kClampToBorder)
}

void main() {
    sk_OutColor = blend(mode, sample(image, sk_TransformedCoords2D[0]), sk_InColor);
}

@test(testData) {
    auto [proxy, ct, at] = testData->randomProxy();
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
    SkBlendMode mode = static_cast<SkBlendMode>(testData->fRandom->nextULessThan(static_cast<uint32_t>(SkBlendMode::kLastMode) + 1));

    return GrSimpleTextureEffect::Make(std::move(proxy), at, matrix, params);
}
