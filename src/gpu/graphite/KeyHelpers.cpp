/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyHelpers.h"

#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"

constexpr SkPMColor4f kErrorColor = { 1, 0, 0, 1 };

#define VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID) \
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, dict->getUniforms(codeSnippetID));)

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------

void PriorOutputBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kPriorOutput);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_solid_uniform_data(const ShaderCodeDictionary* dict,
                            const SkPMColor4f& premulColor,
                            PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kSolidColorShader)
    gatherer->write(premulColor);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kSolidColorShader));
}

} // anonymous namespace

void SolidColorShaderBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const SkPMColor4f& premulColor) {
    if (gatherer) {
        auto dict = keyContext.dict();

        add_solid_uniform_data(dict, premulColor, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kSolidColorShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_gradient_preamble(const GradientShaderBlocks::GradientData& gradData,
                           PipelineDataGatherer* gatherer) {
    constexpr int kInternalStopLimit = GradientShaderBlocks::GradientData::kNumInternalStorageStops;

    if (gradData.fNumStops <= kInternalStopLimit) {
        int stops = gradData.fNumStops <= 4 ? 4 : 8;

        gatherer->writeArray({gradData.fColors, stops});
        gatherer->writeArray({gradData.fOffsets, stops});
    }
}


// All the gradients share a common postamble of:
//   numStops - for texture-based gradients
//   tilemode
//   colorSpace
//   doUnPremul
void add_gradient_postamble(const GradientShaderBlocks::GradientData& gradData,
                            PipelineDataGatherer* gatherer) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;

    constexpr int kInternalStopLimit = GradientShaderBlocks::GradientData::kNumInternalStorageStops;

    static_assert(static_cast<int>(ColorSpace::kLab)   == 2);
    static_assert(static_cast<int>(ColorSpace::kOKLab) == 3);
    static_assert(static_cast<int>(ColorSpace::kLCH)   == 4);
    static_assert(static_cast<int>(ColorSpace::kOKLCH) == 5);
    static_assert(static_cast<int>(ColorSpace::kHSL)   == 7);
    static_assert(static_cast<int>(ColorSpace::kHWB)   == 8);

    bool inputPremul = static_cast<bool>(gradData.fInterpolation.fInPremul);

    if (gradData.fNumStops > kInternalStopLimit) {
        gatherer->write(gradData.fNumStops);
    }

    gatherer->write(static_cast<int>(gradData.fTM));
    gatherer->write(static_cast<int>(gradData.fInterpolation.fColorSpace));
    gatherer->write(static_cast<int>(inputPremul));
}

void add_linear_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                      BuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    add_gradient_postamble(gradData, gatherer);

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_radial_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                      BuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fRadii[0]);
    add_gradient_postamble(gradData, gatherer);

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_sweep_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                     BuiltInCodeSnippetID codeSnippetID,
                                     const GradientShaderBlocks::GradientData& gradData,
                                     PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fBias);
    gatherer->write(gradData.fScale);
    add_gradient_postamble(gradData, gatherer);

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_conical_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                       BuiltInCodeSnippetID codeSnippetID,
                                       const GradientShaderBlocks::GradientData& gradData,
                                       PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(gradData.fRadii[1]);
    add_gradient_postamble(gradData, gatherer);

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

} // anonymous namespace

GradientShaderBlocks::GradientData::GradientData(SkShaderBase::GradientType type, int numStops)
        : fType(type)
        , fPoints{{0.0f, 0.0f}, {0.0f, 0.0f}}
        , fRadii{0.0f, 0.0f}
        , fBias(0.0f)
        , fScale(0.0f)
        , fTM(SkTileMode::kClamp)
        , fNumStops(numStops) {
    sk_bzero(fColors, sizeof(fColors));
    sk_bzero(fOffsets, sizeof(fOffsets));
}

GradientShaderBlocks::GradientData::GradientData(SkShaderBase::GradientType type,
                                                 SkPoint point0, SkPoint point1,
                                                 float radius0, float radius1,
                                                 float bias, float scale,
                                                 SkTileMode tm,
                                                 int numStops,
                                                 const SkPMColor4f* colors,
                                                 float* offsets,
                                                 sk_sp<TextureProxy> colorsAndOffsetsProxy,
                                                 const SkGradientShader::Interpolation& interp)
        : fType(type)
        , fBias(bias)
        , fScale(scale)
        , fTM(tm)
        , fNumStops(numStops)
        , fInterpolation(interp) {
    SkASSERT(fNumStops >= 1);

    fPoints[0] = point0;
    fPoints[1] = point1;
    fRadii[0] = radius0;
    fRadii[1] = radius1;

    if (fNumStops <= kNumInternalStorageStops) {
        memcpy(fColors, colors, fNumStops * sizeof(SkColor4f));
        if (offsets) {
            memcpy(fOffsets, offsets, fNumStops * sizeof(float));
        } else {
            for (int i = 0; i < fNumStops; ++i) {
                fOffsets[i] = SkIntToFloat(i) / (fNumStops-1);
            }
        }

        // Extend the colors and offset, if necessary, to fill out the arrays
        // TODO: this should be done later when the actual code snippet has been selected!!
        for (int i = fNumStops ; i < kNumInternalStorageStops; ++i) {
            fColors[i] = fColors[fNumStops-1];
            fOffsets[i] = fOffsets[fNumStops-1];
        }
    } else {
        fColorsAndOffsetsProxy = std::move(colorsAndOffsetsProxy);
        SkASSERT(fColorsAndOffsetsProxy);
    }
}

void GradientShaderBlocks::BeginBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder *builder,
                                      PipelineDataGatherer* gatherer,
                                      const GradientData& gradData) {
    auto dict = keyContext.dict();

    if (gradData.fNumStops > GradientData::kNumInternalStorageStops && gatherer) {
        SkASSERT(gradData.fColorsAndOffsetsProxy);

        static constexpr SkSamplingOptions kNearest(SkFilterMode::kNearest, SkMipmapMode::kNone);
        static constexpr SkTileMode kClampTiling[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
        gatherer->add(kNearest, kClampTiling, gradData.fColorsAndOffsetsProxy);
    }

    BuiltInCodeSnippetID codeSnippetID = BuiltInCodeSnippetID::kSolidColorShader;
    switch (gradData.fType) {
        case SkShaderBase::GradientType::kLinear:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kLinearGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kLinearGradientShader8
                                              : BuiltInCodeSnippetID::kLinearGradientShaderTexture;
            if (gatherer) {
                add_linear_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kRadial:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kRadialGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kRadialGradientShader8
                                              : BuiltInCodeSnippetID::kRadialGradientShaderTexture;
            if (gatherer) {
                add_radial_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kSweep:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kSweepGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kSweepGradientShader8
                                              : BuiltInCodeSnippetID::kSweepGradientShaderTexture;
            if (gatherer) {
                add_sweep_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kConical:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kConicalGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kConicalGradientShader8
                                              : BuiltInCodeSnippetID::kConicalGradientShaderTexture;
            if (gatherer) {
                add_conical_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kColor:
        case SkShaderBase::GradientType::kNone:
        default:
            SkASSERT(0);
            break;
    }

    builder->beginBlock(codeSnippetID);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_localmatrixshader_uniform_data(const ShaderCodeDictionary* dict,
                                        const SkM44& localMatrix,
                                        PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kLocalMatrixShader)

    SkM44 lmInverse;
    bool wasInverted = localMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kLocalMatrixShader));
}

} // anonymous namespace

void LocalMatrixShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const LMShaderData* lmShaderData) {
    SkASSERT(!gatherer == !lmShaderData);

    auto dict = keyContext.dict();
    // When extracted into ShaderInfo::SnippetEntries the children will appear after their
    // parent. Thus, the parent's uniform data must appear in the uniform block before the
    // uniform data of the children.
    if (gatherer) {
        add_localmatrixshader_uniform_data(dict, lmShaderData->fLocalMatrix, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kLocalMatrixShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_color_space_uniforms(const SkColorSpaceXformSteps& steps, PipelineDataGatherer* gatherer) {
    static constexpr int kNumXferFnCoeffs = 7;
    static constexpr float kEmptyXferFn[kNumXferFnCoeffs] = {};

    gatherer->write(SkTo<int>(steps.flags.mask()));

    if (steps.flags.linearize) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.srcTF)));
        gatherer->writeHalfArray({&steps.srcTF.g, kNumXferFnCoeffs});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
        gatherer->writeHalfArray({kEmptyXferFn, kNumXferFnCoeffs});
    }

    SkMatrix gamutTransform;
    if (steps.flags.gamut_transform) {
        // TODO: it seems odd to copy this into an SkMatrix just to write it to the gatherer
        gamutTransform.set9(steps.src_to_dst_matrix);
    }
    gatherer->writeHalf(gamutTransform);

    if (steps.flags.encode) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.dstTFInv)));
        gatherer->writeHalfArray({&steps.dstTFInv.g, kNumXferFnCoeffs});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
        gatherer->writeHalfArray({kEmptyXferFn, kNumXferFnCoeffs});
    }
}

void add_image_uniform_data(const ShaderCodeDictionary* dict,
                            const ImageShaderBlock::ImageData& imgData,
                            PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kImageShader)

    gatherer->write(SkPoint::Make(imgData.fTextureProxy->dimensions().fWidth,
                                  imgData.fTextureProxy->dimensions().fHeight));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
    gatherer->write(imgData.fSampling.useCubic);
    if (imgData.fSampling.useCubic) {
        const SkCubicResampler& cubic = imgData.fSampling.cubic;
        gatherer->write(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));
    } else {
        gatherer->write(SkM44());
    }
    gatherer->write(SkTo<int>(imgData.fReadSwizzle));

    add_color_space_uniforms(imgData.fSteps, gatherer);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kImageShader));
}

} // anonymous namespace

ImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                       SkTileMode tileModeX,
                                       SkTileMode tileModeY,
                                       SkRect subset,
                                       ReadSwizzle readSwizzle)
        : fSampling(sampling)
        , fTileModes{tileModeX, tileModeY}
        , fSubset(subset)
        , fReadSwizzle(readSwizzle) {
    SkASSERT(fSteps.flags.mask() == 0);   // By default, the colorspace should have no effect
}

void ImageShaderBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer,
                                  const ImageData* imgData) {
    SkASSERT(!gatherer == !imgData);

    // TODO: allow through lazy proxies
    if (gatherer && !imgData->fTextureProxy) {
        // TODO: At some point the pre-compile path should also be creating a texture
        // proxy (i.e., we can remove the 'pipelineData' in the above test).
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        return;
    }

    auto dict = keyContext.dict();
    if (gatherer) {
        gatherer->add(imgData->fSampling,
                      imgData->fTileModes,
                      imgData->fTextureProxy);

        add_image_uniform_data(dict, *imgData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kImageShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_coordclamp_uniform_data(const ShaderCodeDictionary* dict,
                                 const CoordClampShaderBlock::CoordClampData& clampData,
                                 PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCoordClampShader)

    gatherer->write(clampData.fSubset);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kCoordClampShader));
}

} // anonymous namespace

void CoordClampShaderBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const CoordClampData* clampData) {
    SkASSERT(!gatherer == !clampData);

    auto dict = keyContext.dict();
    if (gatherer) {
        add_coordclamp_uniform_data(dict, *clampData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kCoordClampShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_dither_uniform_data(const ShaderCodeDictionary* dict,
                             const DitherShaderBlock::DitherData& ditherData,
                             PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kDitherShader)

    gatherer->writeHalf(ditherData.fRange);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kDitherShader));
}

} // anonymous namespace

void DitherShaderBlock::BeginBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer,
                                   const DitherData* ditherData) {
    SkASSERT(!gatherer == !ditherData);

    auto dict = keyContext.dict();
    if (gatherer) {
        static const SkBitmap gLUT = skgpu::MakeDitherLUT();

        sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), gLUT);
        if (!proxy) {
            SKGPU_LOG_W("Couldn't create dither shader's LUT");

            PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
            return;
        }

        add_dither_uniform_data(dict, *ditherData, gatherer);

        static constexpr SkSamplingOptions kNearest(SkFilterMode::kNearest, SkMipmapMode::kNone);
        static constexpr SkTileMode kRepeatTiling[2] = { SkTileMode::kRepeat, SkTileMode::kRepeat };

        gatherer->add(kNearest, kRepeatTiling, std::move(proxy));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kDitherShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_perlin_noise_uniform_data(const ShaderCodeDictionary* dict,
                                   const PerlinNoiseShaderBlock::PerlinNoiseData& noiseData,
                                   PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kPerlinNoiseShader)

    gatherer->write(noiseData.fBaseFrequency);
    gatherer->write(noiseData.fStitchData);
    gatherer->write(static_cast<int>(noiseData.fType));
    gatherer->write(noiseData.fNumOctaves);
    gatherer->write(static_cast<int>(noiseData.stitching()));

    static const SkTileMode kRepeatXTileModes[2] = { SkTileMode::kRepeat, SkTileMode::kClamp };
    static const SkSamplingOptions kNearestSampling { SkFilterMode::kNearest };

    gatherer->add(kNearestSampling, kRepeatXTileModes, noiseData.fPermutationsProxy);
    gatherer->add(kNearestSampling, kRepeatXTileModes, noiseData.fNoiseProxy);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kPerlinNoiseShader));
}

} // anonymous namespace

void PerlinNoiseShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const PerlinNoiseData* noiseData) {
    SkASSERT(!gatherer == !noiseData);

    auto dict = keyContext.dict();
    if (gatherer) {
        add_perlin_noise_uniform_data(dict, *noiseData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kPerlinNoiseShader);
}

//--------------------------------------------------------------------------------------------------

void BlendShaderBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) {
    if (gatherer) {
        auto dict = keyContext.dict();
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kBlendShader)
        gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kBlendShader));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendShader);
}

//--------------------------------------------------------------------------------------------------

void BlendModeBlenderBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       SkBlendMode blendMode) {
    if (gatherer) {
        auto dict = keyContext.dict();
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kBlendModeBlender)
        gatherer->write(SkTo<int>(blendMode));
        gatherer->addFlags(
                dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kBlendModeBlender));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendModeBlender);
}

//--------------------------------------------------------------------------------------------------

void CoeffBlenderBlock::BeginBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer,
                                   SkSpan<const float> coeffs) {
    if (gatherer) {
        auto dict = keyContext.dict();
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCoeffBlender)
        SkASSERT(coeffs.size() == 4);
        gatherer->write(SkSLType::kHalf4, coeffs.data());
        gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kCoeffBlender));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kCoeffBlender);
}

//--------------------------------------------------------------------------------------------------

void PrimitiveColorBlock::BeginBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer) {
    if (gatherer) {
        auto dict = keyContext.dict();
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kPrimitiveColor)
        gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kPrimitiveColor));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kPrimitiveColor);
}

//--------------------------------------------------------------------------------------------------

void ColorFilterShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer) {
    auto dict = keyContext.dict();

    if (gatherer) {
        gatherer->addFlags(
                dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kColorFilterShader));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kColorFilterShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_matrix_colorfilter_uniform_data(const ShaderCodeDictionary* dict,
                                         const MatrixColorFilterBlock::MatrixColorFilterData& data,
                                         PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kMatrixColorFilter)
    gatherer->write(data.fMatrix);
    gatherer->write(data.fTranslate);
    gatherer->write(static_cast<int>(data.fInHSLA));

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kMatrixColorFilter));
}

} // anonymous namespace

void MatrixColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const MatrixColorFilterData* matrixCFData) {
    SkASSERT(!gatherer == !matrixCFData);

    auto dict = keyContext.dict();

    if (gatherer) {
        add_matrix_colorfilter_uniform_data(dict, *matrixCFData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kMatrixColorFilter);
}

//--------------------------------------------------------------------------------------------------
void ComposeColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                         PaintParamsKeyBuilder* builder,
                                         PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kComposeColorFilter);
}

//--------------------------------------------------------------------------------------------------
void GaussianColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                          PaintParamsKeyBuilder* builder,
                                          PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kGaussianColorFilter);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_table_colorfilter_uniform_data(const ShaderCodeDictionary* dict,
                                        const TableColorFilterBlock::TableColorFilterData& data,
                                        PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kTableColorFilter)

    static const SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
    gatherer->add(SkSamplingOptions(), kTileModes, data.fTextureProxy);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kTableColorFilter));
}

} // anonymous namespace

void TableColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const TableColorFilterData& data) {
    auto dict = keyContext.dict();

    if (gatherer) {
        if (!data.fTextureProxy) {
            // We're dropping the color filter here!
            PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
            return;
        }

        add_table_colorfilter_uniform_data(dict, data, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kTableColorFilter);
}

//--------------------------------------------------------------------------------------------------
namespace {

void add_color_space_xform_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData* data,
        PipelineDataGatherer* gatherer) {

    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformColorFilter)
    add_color_space_uniforms(data->fSteps, gatherer);

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kColorSpaceXformColorFilter));
}

}  // anonymous namespace

ColorSpaceTransformBlock::ColorSpaceTransformData::ColorSpaceTransformData(const SkColorSpace* src,
                                                                           SkAlphaType srcAT,
                                                                           const SkColorSpace* dst,
                                                                           SkAlphaType dstAT)
        : fSteps(src, srcAT, dst, dstAT) {}

void ColorSpaceTransformBlock::BeginBlock(const KeyContext& keyContext,
                                          PaintParamsKeyBuilder* builder,
                                          PipelineDataGatherer* gatherer,
                                          const ColorSpaceTransformData* data) {
    if (gatherer) {
        add_color_space_xform_uniform_data(keyContext.dict(), data, gatherer);
    }
    builder->beginBlock(BuiltInCodeSnippetID::kColorSpaceXformColorFilter);
}

//--------------------------------------------------------------------------------------------------

void AddDstBlendBlock(const KeyContext& keyContext,
                      PaintParamsKeyBuilder* builder,
                      PipelineDataGatherer* gatherer,
                      const SkBlender* blender) {
    BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);

    // src -- prior output
    PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
    // dst -- surface color
    // TODO(b/238757201): Use the surface color as the destination by replacing
    // SolidColorShaderBlock with a DstColorBlock
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 1, 1, 1});
    builder->endBlock();
    // blender -- shader based blending
    as_BB(blender)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();  // BlendShaderBlock
}

void AddPrimitiveBlendBlock(const KeyContext& keyContext,
                            PaintParamsKeyBuilder* builder,
                            PipelineDataGatherer* gatherer,
                            const SkBlender* blender) {
    BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);

    // src -- prior output
    PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
    // dst -- primitive color
    PrimitiveColorBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
    // blender -- shader based blending
    as_BB(blender)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();  // BlendShaderBlock
}

void AddColorBlendBlock(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* builder,
                        PipelineDataGatherer* gatherer,
                        SkBlendMode bm,
                        const SkPMColor4f& srcColor) {
    BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);

    // src -- solid color
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, srcColor);
    builder->endBlock();
    // dst -- prior output
    PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
    // blender -- shader based blending
    BlendModeBlenderBlock::BeginBlock(keyContext, builder, gatherer, bm);
    builder->endBlock();

    builder->endBlock();  // BlendShaderBlock
}

RuntimeEffectBlock::ShaderData::ShaderData(sk_sp<const SkRuntimeEffect> effect)
        : fEffect(std::move(effect)) {}

RuntimeEffectBlock::ShaderData::ShaderData(sk_sp<const SkRuntimeEffect> effect,
                                           sk_sp<const SkData> uniforms)
        : fEffect(std::move(effect))
        , fUniforms(std::move(uniforms)) {}

static bool skdata_matches(const SkData* a, const SkData* b) {
    // Returns true if both SkData objects hold the same contents, or if they are both null.
    // (SkData::equals supports passing null, and returns false.)
    return a ? a->equals(b) : (a == b);
}

bool RuntimeEffectBlock::ShaderData::operator==(const ShaderData& rhs) const {
    return fEffect == rhs.fEffect && skdata_matches(fUniforms.get(), rhs.fUniforms.get());
}

static void gather_runtime_effect_uniforms(SkSpan<const SkRuntimeEffect::Uniform> rtsUniforms,
                                           SkSpan<const Uniform> graphiteUniforms,
                                           const SkData* uniformData,
                                           PipelineDataGatherer* gatherer) {
    // Collect all the other uniforms from the provided SkData.
    const uint8_t* uniformBase = uniformData->bytes();
    for (size_t index = 0; index < rtsUniforms.size(); ++index) {
        const Uniform& uniform = graphiteUniforms[index];
        // Get a pointer to the offset in our data for this uniform.
        const uint8_t* uniformPtr = uniformBase + rtsUniforms[index].offset;
        // Pass the uniform data to the gatherer.
        gatherer->write(uniform, uniformPtr);
    }
}

void RuntimeEffectBlock::BeginBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    const ShaderData& shaderData) {
    ShaderCodeDictionary* dict = keyContext.dict();
    int codeSnippetID = dict->findOrCreateRuntimeEffectSnippet(shaderData.fEffect.get());

    keyContext.rtEffectDict()->set(codeSnippetID, shaderData.fEffect);

    if (gatherer) {
        const ShaderSnippet* entry = dict->getEntry(codeSnippetID);
        SkASSERT(entry);

        SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, entry->fUniforms);)
        gatherer->addFlags(entry->fSnippetRequirementFlags);

        gather_runtime_effect_uniforms(shaderData.fEffect->uniforms(),
                                       entry->fUniforms,
                                       shaderData.fUniforms.get(),
                                       gatherer);
    }

    builder->beginBlock(codeSnippetID);
}

} // namespace skgpu::graphite
