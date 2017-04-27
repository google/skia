/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D image;
in uniform GrColorSpaceXform colorXform;
in mat4 matrix;

@fields {
    GrCoordTransform fImageCoordTransform;
}

@initializers {
    fImageCoordTransform(resourceProvider, SkMatrix::I(), image.get())
}

@class {
    static OptimizationFlags ModulationFlags(GrPixelConfig config) {
        if (GrPixelConfigIsOpaque(config)) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                   kPreservesOpaqueInput_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }
}

@constructor {
    GrSimpleTextureEffect(GrResourceProvider* resourceProvider,
                          sk_sp<GrTextureProxy> proxy,
                          sk_sp<GrColorSpaceXform> colorSpaceXform,
                          const SkMatrix& matrix,
                          GrSamplerParams::FilterMode filterMode)
    : INHERITED(ModulationFlags(proxy->config()))
    , fImageCoordTransform(resourceProvider, matrix, proxy.get())
    , fImage(resourceProvider, std::move(proxy), filterMode)
    , fColorXform(std::move(colorSpaceXform))
    , fMatrix(matrix) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureSampler(&fImage);
        this->initClassID<GrSimpleTextureEffect>();
    }

    GrSimpleTextureEffect(GrResourceProvider* resourceProvider,
                          sk_sp<GrTextureProxy> proxy,
                          sk_sp<GrColorSpaceXform> colorSpaceXform,
                          const SkMatrix& matrix,
                          const GrSamplerParams& params)
    : INHERITED(ModulationFlags(proxy->config()))
    , fImageCoordTransform(resourceProvider, matrix, proxy.get())
    , fImage(resourceProvider, std::move(proxy), params)
    , fColorXform(std::move(colorSpaceXform))
    , fMatrix(matrix) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureSampler(&fImage);
        this->initClassID<GrSimpleTextureEffect>();
    }
}

@make {
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(resourceProvider, std::move(proxy),
                                      std::move(colorSpaceXform), matrix,
                                      GrSamplerParams::kNone_FilterMode));
    }

    /* clamp mode */
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           GrSamplerParams::FilterMode filterMode) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(resourceProvider, std::move(proxy),
                                      std::move(colorSpaceXform), matrix, filterMode));
     }

    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           const GrSamplerParams& p) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(resourceProvider, std::move(proxy),
                                      std::move(colorSpaceXform), matrix, p));
    }
}

@optimizationFlags {
    (kCompatibleWithCoverageAsAlpha_OptimizationFlag | 
    (GrPixelConfigIsOpaque(image->config()) ? kPreservesOpaqueInput_OptimizationFlag : 0))
}

void main() {
    vec4 color = sk_InColor * texture(image, sk_TransformedCoords2D[0]);
    if (colorXform != mat4(1)) {
        color.rgb = clamp((colorXform * vec4(color.rgb, 1.0)).rgb, 0.0, color.a);
    }
    sk_OutColor = color;
}

@test(testData) {
    int texIdx = testData->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[testData->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[testData->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrSamplerParams params(tileModes, testData->fRandom->nextBool()
                                                               ? GrSamplerParams::kBilerp_FilterMode
                                                               : GrSamplerParams::kNone_FilterMode);

    const SkMatrix& matrix = GrTest::TestMatrix(testData->fRandom);
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(testData->fRandom);
    return GrSimpleTextureEffect::Make(testData->resourceProvider(), testData->textureProxy(texIdx),
                                       std::move(colorSpaceXform), matrix);
}