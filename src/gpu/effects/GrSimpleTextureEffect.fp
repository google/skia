/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform sampler2D image;
in uniform colorSpaceXform colorXform;
in mat4 matrix;

@fields {
    GrCoordTransform fImageCoordTransform;
}

@initializers {
    fImageCoordTransform(SkMatrix::I(), image.get())
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
    GrSimpleTextureEffect(sk_sp<GrTextureProxy> proxy,
                          sk_sp<GrColorSpaceXform> colorSpaceXform,
                          const SkMatrix& matrix,
                          GrSamplerParams::FilterMode filterMode)
    : INHERITED(ModulationFlags(proxy->config()))
    , fImageCoordTransform(matrix, proxy.get())
    , fImage(std::move(proxy), filterMode)
    , fColorXform(std::move(colorSpaceXform))
    , fMatrix(matrix) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureSampler(&fImage);
        this->initClassID<GrSimpleTextureEffect>();
    }

    GrSimpleTextureEffect(sk_sp<GrTextureProxy> proxy,
                          sk_sp<GrColorSpaceXform> colorSpaceXform,
                          const SkMatrix& matrix,
                          const GrSamplerParams& params)
    : INHERITED(ModulationFlags(proxy->config()))
    , fImageCoordTransform(matrix, proxy.get())
    , fImage(std::move(proxy), params)
    , fColorXform(std::move(colorSpaceXform))
    , fMatrix(matrix) {
        this->addCoordTransform(&fImageCoordTransform);
        this->addTextureSampler(&fImage);
        this->initClassID<GrSimpleTextureEffect>();
    }
}

@make {
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), std::move(colorSpaceXform), matrix,
                                      GrSamplerParams::kNone_FilterMode));
    }

    /* clamp mode */
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           GrSamplerParams::FilterMode filterMode) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), std::move(colorSpaceXform), matrix,
                                      filterMode));
     }

    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           const SkMatrix& matrix,
                                           const GrSamplerParams& p) {
        return sk_sp<GrFragmentProcessor>(
            new GrSimpleTextureEffect(std::move(proxy), std::move(colorSpaceXform), matrix, p));
    }
}

@optimizationFlags {
    (kCompatibleWithCoverageAsAlpha_OptimizationFlag |
    (GrPixelConfigIsOpaque(image->config()) ? kPreservesOpaqueInput_OptimizationFlag : 0))
}

void main() {
    sk_OutColor = sk_InColor * texture(image, sk_TransformedCoords2D[0], colorXform);
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
    return GrSimpleTextureEffect::Make(testData->textureProxy(texIdx), std::move(colorSpaceXform),
                                       matrix);
}
