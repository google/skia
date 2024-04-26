/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyHelpers.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/Surface.h"
#include "src/base/SkHalf.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkYUVMath.h"
#include "src/effects/colorfilters/SkBlendModeColorFilter.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/effects/colorfilters/SkColorSpaceXformColorFilter.h"
#include "src/effects/colorfilters/SkComposeColorFilter.h"
#include "src/effects/colorfilters/SkGaussianColorFilter.h"
#include "src/effects/colorfilters/SkMatrixColorFilter.h"
#include "src/effects/colorfilters/SkRuntimeColorFilter.h"
#include "src/effects/colorfilters/SkTableColorFilter.h"
#include "src/effects/colorfilters/SkWorkingFormatColorFilter.h"
#include "src/gpu/Blend.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Image_YUVA_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParams.h"
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
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBlendShader.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkCoordClampShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkPerlinNoiseShaderImpl.h"
#include "src/shaders/SkPerlinNoiseShaderType.h"
#include "src/shaders/SkPictureShader.h"
#include "src/shaders/SkRuntimeShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"
#include "src/shaders/SkTriColorShader.h"
#include "src/shaders/SkWorkingColorSpaceShader.h"
#include "src/shaders/gradients/SkConicalGradient.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkRadialGradient.h"
#include "src/shaders/gradients/SkSweepGradient.h"

#define VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID) \
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, dict->getUniforms(codeSnippetID));)

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------

namespace {

void add_solid_uniform_data(const ShaderCodeDictionary* dict,
                            const SkPMColor4f& premulColor,
                            PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kSolidColorShader)
    gatherer->write(premulColor);
}

} // anonymous namespace

void SolidColorShaderBlock::AddBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer,
                                     const SkPMColor4f& premulColor) {
    add_solid_uniform_data(keyContext.dict(), premulColor, gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kSolidColorShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_rgb_paint_color_uniform_data(const ShaderCodeDictionary* dict,
                                      const SkPMColor4f& premulColor,
                                      PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kRGBPaintColor)
    gatherer->writePaintColor(premulColor);
}

void add_alpha_only_paint_color_uniform_data(const ShaderCodeDictionary* dict,
                                             const SkPMColor4f& premulColor,
                                             PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kAlphaOnlyPaintColor)
    gatherer->writePaintColor(premulColor);
}

} // anonymous namespace

void RGBPaintColorBlock::AddBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) {
    add_rgb_paint_color_uniform_data(keyContext.dict(), keyContext.paintColor(), gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kRGBPaintColor);
}

void AlphaOnlyPaintColorBlock::AddBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer) {
    add_alpha_only_paint_color_uniform_data(keyContext.dict(), keyContext.paintColor(), gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kAlphaOnlyPaintColor);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_dst_read_sample_uniform_data(const ShaderCodeDictionary* dict,
                                      PipelineDataGatherer* gatherer,
                                      sk_sp<TextureProxy> dstTexture,
                                      SkIPoint dstOffset) {
    static const SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(SkSamplingOptions(), kTileModes, dstTexture);

    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kDstReadSample)

    SkV4 coords{static_cast<float>(dstOffset.x()),
                static_cast<float>(dstOffset.y()),
                dstTexture ? 1.0f / dstTexture->dimensions().width()  : 1.0f,
                dstTexture ? 1.0f / dstTexture->dimensions().height() : 1.0f };
    gatherer->write(coords);
}

} // anonymous namespace

void DstReadSampleBlock::AddBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer,
                                  sk_sp<TextureProxy> dstTexture,
                                  SkIPoint dstOffset) {
    add_dst_read_sample_uniform_data(keyContext.dict(), gatherer, std::move(dstTexture), dstOffset);

    builder->addBlock(BuiltInCodeSnippetID::kDstReadSample);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_gradient_preamble(const GradientShaderBlocks::GradientData& gradData,
                           PipelineDataGatherer* gatherer) {
    constexpr int kInternalStopLimit = GradientShaderBlocks::GradientData::kNumInternalStorageStops;

    if (gradData.fNumStops <= kInternalStopLimit) {
        if (gradData.fNumStops <= 4) {
            // Round up to 4 stops.
            gatherer->writeArray(SkSpan{gradData.fColors, 4});
            gatherer->write(gradData.fOffsets[0]);
        } else if (gradData.fNumStops <= 8) {
            // Round up to 8 stops.
            gatherer->writeArray(SkSpan{gradData.fColors, 8});
            gatherer->writeArray(SkSpan{gradData.fOffsets, 2});
        } else {
            // Did kNumInternalStorageStops change?
            SkUNREACHABLE;
        }
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

    static_assert(static_cast<int>(ColorSpace::kLab)           == 2);
    static_assert(static_cast<int>(ColorSpace::kOKLab)         == 3);
    static_assert(static_cast<int>(ColorSpace::kOKLabGamutMap) == 4);
    static_assert(static_cast<int>(ColorSpace::kLCH)           == 5);
    static_assert(static_cast<int>(ColorSpace::kOKLCH)         == 6);
    static_assert(static_cast<int>(ColorSpace::kOKLCHGamutMap) == 7);
    static_assert(static_cast<int>(ColorSpace::kHSL)           == 9);
    static_assert(static_cast<int>(ColorSpace::kHWB)           == 10);

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
                                                 const float* offsets,
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
        float* rawOffsets = fOffsets[0].ptr();
        if (offsets) {
            memcpy(rawOffsets, offsets, fNumStops * sizeof(float));
        } else {
            for (int i = 0; i < fNumStops; ++i) {
                rawOffsets[i] = SkIntToFloat(i) / (fNumStops-1);
            }
        }

        // Extend the colors and offset, if necessary, to fill out the arrays.
        // The unrolled binary search implementation assumes excess stops match the last real value.
        for (int i = fNumStops; i < kNumInternalStorageStops; ++i) {
            fColors[i] = fColors[fNumStops-1];
            rawOffsets[i] = rawOffsets[fNumStops-1];
        }
    } else {
        fColorsAndOffsetsProxy = std::move(colorsAndOffsetsProxy);
        SkASSERT(fColorsAndOffsetsProxy);
    }
}

void GradientShaderBlocks::AddBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
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
            add_linear_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            break;
        case SkShaderBase::GradientType::kRadial:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kRadialGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kRadialGradientShader8
                                              : BuiltInCodeSnippetID::kRadialGradientShaderTexture;
            add_radial_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            break;
        case SkShaderBase::GradientType::kSweep:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kSweepGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kSweepGradientShader8
                                              : BuiltInCodeSnippetID::kSweepGradientShaderTexture;
            add_sweep_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            break;
        case SkShaderBase::GradientType::kConical:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kConicalGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kConicalGradientShader8
                                              : BuiltInCodeSnippetID::kConicalGradientShaderTexture;
            add_conical_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            break;
        case SkShaderBase::GradientType::kNone:
        default:
            SkDEBUGFAIL("Expected a gradient shader, but it wasn't one.");
            break;
    }

    builder->addBlock(codeSnippetID);
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
}

} // anonymous namespace

void LocalMatrixShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const LMShaderData& lmShaderData) {

    add_localmatrixshader_uniform_data(keyContext.dict(), lmShaderData.fLocalMatrix, gatherer);

    builder->beginBlock(BuiltInCodeSnippetID::kLocalMatrixShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

static constexpr int kColorSpaceXformFlagAlphaSwizzle = 0x20;

void add_color_space_uniforms(const SkColorSpaceXformSteps& steps,
                              ReadSwizzle readSwizzle,
                              PipelineDataGatherer* gatherer) {
    // We have 7 source coefficients and 7 destination coefficients. We pass them via a 4x4 matrix;
    // the first two columns hold the source values, and the second two hold the destination.
    // (The final value of each 8-element group is ignored.)
    // In std140, this arrangement is much more efficient than a simple array of scalars.
    SkM44 coeffs;

    int colorXformFlags = SkTo<int>(steps.flags.mask());
    if (readSwizzle != ReadSwizzle::kRGBA) {
        // Ensure that we do the gamut step
        SkColorSpaceXformSteps gamutSteps;
        gamutSteps.flags.gamut_transform = true;
        colorXformFlags |= SkTo<int>(gamutSteps.flags.mask());
        if (readSwizzle != ReadSwizzle::kBGRA) {
            // TODO: Maybe add a fullMask() method to XformSteps?
            SkASSERT(colorXformFlags < kColorSpaceXformFlagAlphaSwizzle);
            colorXformFlags |= kColorSpaceXformFlagAlphaSwizzle;
        }
    }
    gatherer->write(colorXformFlags);

    if (steps.flags.linearize) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.srcTF)));
        coeffs.setCol(0, {steps.srcTF.g, steps.srcTF.a, steps.srcTF.b, steps.srcTF.c});
        coeffs.setCol(1, {steps.srcTF.d, steps.srcTF.e, steps.srcTF.f, 0.0f});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
    }

    SkMatrix gamutTransform;
    const float identity[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    // TODO: it seems odd to copy this into an SkMatrix just to write it to the gatherer
    // src_to_dst_matrix is column-major, SkMatrix is row-major.
    const float* m = steps.flags.gamut_transform ? steps.src_to_dst_matrix : identity;
    if (readSwizzle == ReadSwizzle::kRRR1) {
        gamutTransform.setAll(m[0] + m[3] + m[6], 0, 0,
                              m[1] + m[4] + m[7], 0, 0,
                              m[2] + m[5] + m[8], 0, 0);
    } else if (readSwizzle == ReadSwizzle::kBGRA) {
        gamutTransform.setAll(m[6], m[3], m[0],
                              m[7], m[4], m[1],
                              m[8], m[5], m[2]);
    } else if (readSwizzle == ReadSwizzle::k000R) {
        gamutTransform.setAll(0, 0, 0,
                              0, 0, 0,
                              0, 0, 0);
    } else if (steps.flags.gamut_transform) {
        gamutTransform.setAll(m[0], m[3], m[6],
                              m[1], m[4], m[7],
                              m[2], m[5], m[8]);
    }
    gatherer->writeHalf(gamutTransform);

    if (steps.flags.encode) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.dstTFInv)));
        coeffs.setCol(2, {steps.dstTFInv.g, steps.dstTFInv.a, steps.dstTFInv.b, steps.dstTFInv.c});
        coeffs.setCol(3, {steps.dstTFInv.d, steps.dstTFInv.e, steps.dstTFInv.f, 0.0f});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
    }

    // Pack alpha swizzle in the unused coeff entries.
    switch (readSwizzle) {
        case ReadSwizzle::k000R:
            coeffs.setRC(3, 1, 1.f);
            coeffs.setRC(3, 3, 0.f);
            break;
        case ReadSwizzle::kRGB1:
        case ReadSwizzle::kRRR1:
            coeffs.setRC(3, 1, 0.f);
            coeffs.setRC(3, 3, 1.f);
            break;
        default:
            coeffs.setRC(3, 1, 0.f);
            coeffs.setRC(3, 3, 0.f);
            break;
    }

    gatherer->writeHalf(coeffs);
}

void add_image_uniform_data(const ShaderCodeDictionary* dict,
                            const ImageShaderBlock::ImageData& imgData,
                            PipelineDataGatherer* gatherer) {
    SkASSERT(!imgData.fSampling.useCubic);
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));

    add_color_space_uniforms(imgData.fSteps, imgData.fReadSwizzle, gatherer);
}

void add_cubic_image_uniform_data(const ShaderCodeDictionary* dict,
                                  const ImageShaderBlock::ImageData& imgData,
                                  PipelineDataGatherer* gatherer) {
    SkASSERT(imgData.fSampling.useCubic);
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCubicImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    const SkCubicResampler& cubic = imgData.fSampling.cubic;
    gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));

    add_color_space_uniforms(imgData.fSteps, imgData.fReadSwizzle, gatherer);
}

void add_hw_image_uniform_data(const ShaderCodeDictionary* dict,
                               const ImageShaderBlock::ImageData& imgData,
                               PipelineDataGatherer* gatherer) {
    SkASSERT(!imgData.fSampling.useCubic);
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kHWImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));

    add_color_space_uniforms(imgData.fSteps, imgData.fReadSwizzle, gatherer);
}

} // anonymous namespace

ImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                       SkTileMode tileModeX,
                                       SkTileMode tileModeY,
                                       SkISize imgSize,
                                       SkRect subset,
                                       ReadSwizzle readSwizzle)
        : fSampling(sampling)
        , fTileModes{tileModeX, tileModeY}
        , fImgSize(imgSize)
        , fSubset(subset)
        , fReadSwizzle(readSwizzle) {
    SkASSERT(fSteps.flags.mask() == 0);   // By default, the colorspace should have no effect
}

static bool can_do_tiling_in_hw(const Caps* caps, const ImageShaderBlock::ImageData& imgData) {
    if (!caps->clampToBorderSupport() && (imgData.fTileModes[0] == SkTileMode::kDecal ||
                                          imgData.fTileModes[1] == SkTileMode::kDecal)) {
        return false;
    }
    return imgData.fSubset.contains(SkRect::Make(imgData.fImgSize));
}

void ImageShaderBlock::AddBlock(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const ImageData& imgData) {

    if (keyContext.recorder() && !imgData.fTextureProxy) {
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    const Caps* caps = keyContext.caps();
    const bool doTilingInHw = !imgData.fSampling.useCubic && can_do_tiling_in_hw(caps, imgData);

    static constexpr SkTileMode kDefaultTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(imgData.fSampling,
                  doTilingInHw ? imgData.fTileModes : kDefaultTileModes,
                  imgData.fTextureProxy);

    if (doTilingInHw) {
        add_hw_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kHWImageShader);
    } else if (imgData.fSampling.useCubic) {
        add_cubic_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kCubicImageShader);
    } else {
        add_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kImageShader);
    }
}

//--------------------------------------------------------------------------------------------------

// makes use of ImageShader functions, above
namespace {

void add_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                const YUVImageShaderBlock::ImageData& imgData,
                                PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kYUVImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(imgData.fLinearFilterUVInset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
    gatherer->write(SkTo<int>(imgData.fSamplingUV.filter));

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->write(imgData.fYUVtoRGBTranslate);
}

void add_cubic_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                      const YUVImageShaderBlock::ImageData& imgData,
                                      PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCubicYUVImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    const SkCubicResampler& cubic = imgData.fSampling.cubic;
    gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->write(imgData.fYUVtoRGBTranslate);
}

} // anonymous namespace

YUVImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                          SkTileMode tileModeX,
                                          SkTileMode tileModeY,
                                          SkISize imgSize,
                                          SkRect subset)
        : fSampling(sampling)
        , fSamplingUV(sampling)
        , fTileModes{tileModeX, tileModeY}
        , fImgSize(imgSize)
        , fImgSizeUV(imgSize)
        , fSubset(subset) {
}

void YUVImageShaderBlock::AddBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer,
                                   const ImageData& imgData) {
    if (keyContext.recorder() &&
        (!imgData.fTextureProxies[0] || !imgData.fTextureProxies[1] ||
         !imgData.fTextureProxies[2] || !imgData.fTextureProxies[3])) {
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    SkTileMode uvTileModes[2] = { imgData.fTileModes[0] == SkTileMode::kDecal
                                          ? SkTileMode::kClamp : imgData.fTileModes[0],
                                  imgData.fTileModes[1] == SkTileMode::kDecal
                                          ? SkTileMode::kClamp : imgData.fTileModes[1] };
    gatherer->add(imgData.fSampling, imgData.fTileModes, imgData.fTextureProxies[0]);
    gatherer->add(imgData.fSamplingUV, uvTileModes, imgData.fTextureProxies[1]);
    gatherer->add(imgData.fSamplingUV, uvTileModes, imgData.fTextureProxies[2]);
    gatherer->add(imgData.fSampling, imgData.fTileModes, imgData.fTextureProxies[3]);

    if (imgData.fSampling.useCubic) {
        add_cubic_yuv_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kCubicYUVImageShader);
    } else {
        add_yuv_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kYUVImageShader);
    }
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_coordclamp_uniform_data(const ShaderCodeDictionary* dict,
                                 const CoordClampShaderBlock::CoordClampData& clampData,
                                 PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCoordClampShader)

    gatherer->write(clampData.fSubset);
}

} // anonymous namespace

void CoordClampShaderBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const CoordClampData& clampData) {
    add_coordclamp_uniform_data(keyContext.dict(), clampData, gatherer);

    builder->beginBlock(BuiltInCodeSnippetID::kCoordClampShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_dither_uniform_data(const ShaderCodeDictionary* dict,
                             const DitherShaderBlock::DitherData& ditherData,
                             PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kDitherShader)

    gatherer->writeHalf(ditherData.fRange);
}

} // anonymous namespace

void DitherShaderBlock::AddBlock(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 const DitherData& data) {
    add_dither_uniform_data(keyContext.dict(), data, gatherer);

    static constexpr SkSamplingOptions kNearest(SkFilterMode::kNearest, SkMipmapMode::kNone);
    static constexpr SkTileMode kRepeatTiling[2] = { SkTileMode::kRepeat, SkTileMode::kRepeat };

    SkASSERT(data.fLUTProxy || !keyContext.recorder());
    gatherer->add(kNearest, kRepeatTiling, data.fLUTProxy);

    builder->addBlock(BuiltInCodeSnippetID::kDitherShader);
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
}

} // anonymous namespace

void PerlinNoiseShaderBlock::AddBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder* builder,
                                      PipelineDataGatherer* gatherer,
                                      const PerlinNoiseData& noiseData) {
    add_perlin_noise_uniform_data(keyContext.dict(), noiseData, gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kPerlinNoiseShader);
}

//--------------------------------------------------------------------------------------------------

void BlendShaderBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kBlendShader)

    builder->beginBlock(BuiltInCodeSnippetID::kBlendShader);
}

//--------------------------------------------------------------------------------------------------

void BlendModeBlenderBlock::AddBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer,
                                     SkBlendMode blendMode) {
    VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kBlendModeBlender)
    gatherer->write(SkTo<int>(blendMode));

    builder->addBlock(BuiltInCodeSnippetID::kBlendModeBlender);
}

//--------------------------------------------------------------------------------------------------

void CoeffBlenderBlock::AddBlock(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 SkSpan<const float> coeffs) {
    VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kCoeffBlender)
    SkASSERT(coeffs.size() == 4);
    gatherer->writeHalf(SkV4{coeffs[0], coeffs[1], coeffs[2], coeffs[3]});

    builder->addBlock(BuiltInCodeSnippetID::kCoeffBlender);
}

//--------------------------------------------------------------------------------------------------

void ClipShaderBlock::BeginBlock(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kClipShader)

    builder->beginBlock(BuiltInCodeSnippetID::kClipShader);
}

//--------------------------------------------------------------------------------------------------

void ComposeBlock::BeginBlock(const KeyContext& keyContext,
                              PaintParamsKeyBuilder* builder,
                              PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kCompose);
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
}

} // anonymous namespace

void MatrixColorFilterBlock::AddBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder* builder,
                                      PipelineDataGatherer* gatherer,
                                      const MatrixColorFilterData& matrixCFData) {

    add_matrix_colorfilter_uniform_data(keyContext.dict(), matrixCFData, gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kMatrixColorFilter);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_table_colorfilter_uniform_data(const ShaderCodeDictionary* dict,
                                        const TableColorFilterBlock::TableColorFilterData& data,
                                        PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kTableColorFilter)

    static const SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
    gatherer->add(SkSamplingOptions(), kTileModes, data.fTextureProxy);
}

} // anonymous namespace

void TableColorFilterBlock::AddBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer,
                                     const TableColorFilterData& data) {
    SkASSERT(data.fTextureProxy || !keyContext.recorder());

    add_table_colorfilter_uniform_data(keyContext.dict(), data, gatherer);

    builder->addBlock(BuiltInCodeSnippetID::kTableColorFilter);
}

//--------------------------------------------------------------------------------------------------
namespace {

void add_color_space_xform_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData& data,
        PipelineDataGatherer* gatherer) {

    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformColorFilter)
    add_color_space_uniforms(data.fSteps, ReadSwizzle::kRGBA, gatherer);
}

}  // anonymous namespace

ColorSpaceTransformBlock::ColorSpaceTransformData::ColorSpaceTransformData(const SkColorSpace* src,
                                                                           SkAlphaType srcAT,
                                                                           const SkColorSpace* dst,
                                                                           SkAlphaType dstAT)
        : fSteps(src, srcAT, dst, dstAT) {}

void ColorSpaceTransformBlock::AddBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const ColorSpaceTransformData& data) {
    add_color_space_xform_uniform_data(keyContext.dict(), data, gatherer);
    builder->addBlock(BuiltInCodeSnippetID::kColorSpaceXformColorFilter);
}

//--------------------------------------------------------------------------------------------------

void AddBlendModeColorFilter(const KeyContext& keyContext,
                             PaintParamsKeyBuilder* builder,
                             PipelineDataGatherer* gatherer,
                             SkBlendMode bm,
                             const SkPMColor4f& srcColor) {
    Blend(keyContext, builder, gatherer,
          /* addBlendToKey= */ [&] () -> void {
              // Note, we're playing a bit of a game here. By explicitly adding a
              // BlendModeBlenderBlock we're always forcing the SkSL to call 'sk_blend'
              // rather than allowing it to sometimes call 'blend_porter_duff'. This reduces
              // the number of shader combinations and allows the pre-compilation system to more
              // easily match the rendering path.
              BlendModeBlenderBlock::AddBlock(keyContext, builder, gatherer, bm);
          },
          /* addSrcToKey= */ [&]() -> void {
              SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, srcColor);
          },
          /* addDstToKey= */ [&]() -> void {
              builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
          });
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

static void gather_runtime_effect_uniforms(const KeyContext& keyContext,
                                           const SkRuntimeEffect* effect,
                                           SkSpan<const Uniform> graphiteUniforms,
                                           const SkData* uniformData,
                                           PipelineDataGatherer* gatherer) {
    if (!uniformData) {
        return;  // precompiling
    }

    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, graphiteUniforms);)

    SkSpan<const SkRuntimeEffect::Uniform> rtsUniforms = effect->uniforms();

    if (!rtsUniforms.empty() && uniformData) {
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

    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
        SkColorSpace* dstCS = keyContext.dstColorInfo().colorSpace();
        if (!dstCS) {
            dstCS = sk_srgb_linear_singleton(); // turn colorspace conversion into a noop
        }

        // TODO(b/332565302): If the runtime shader only uses one of these
        // transforms, we could upload only one set of uniforms.
        ColorSpaceTransformBlock::ColorSpaceTransformData dstToLinear(dstCS,
                                                                      kUnpremul_SkAlphaType,
                                                                      sk_srgb_linear_singleton(),
                                                                      kUnpremul_SkAlphaType);
        ColorSpaceTransformBlock::ColorSpaceTransformData linearToDst(sk_srgb_linear_singleton(),
                                                                      kUnpremul_SkAlphaType,
                                                                      dstCS,
                                                                      kUnpremul_SkAlphaType);

        add_color_space_uniforms(dstToLinear.fSteps, ReadSwizzle::kRGBA, gatherer);
        add_color_space_uniforms(linearToDst.fSteps, ReadSwizzle::kRGBA, gatherer);
    }
}

void RuntimeEffectBlock::BeginBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    const ShaderData& shaderData) {
    ShaderCodeDictionary* dict = keyContext.dict();
    int codeSnippetID = dict->findOrCreateRuntimeEffectSnippet(shaderData.fEffect.get());

    if (codeSnippetID >= SkKnownRuntimeEffects::kUnknownRuntimeEffectIDStart) {
        keyContext.rtEffectDict()->set(codeSnippetID, shaderData.fEffect);
    }

    const ShaderSnippet* entry = dict->getEntry(codeSnippetID);
    SkASSERT(entry);

    gather_runtime_effect_uniforms(keyContext,
                                   shaderData.fEffect.get(),
                                   entry->fUniforms,
                                   shaderData.fUniforms.get(),
                                   gatherer);

    builder->beginBlock(codeSnippetID);
}

// ==================================================================

namespace {

void add_to_key(const KeyContext& keyContext,
                PaintParamsKeyBuilder* builder,
                PipelineDataGatherer* gatherer,
                const SkBlendModeBlender* blender) {
    SkASSERT(blender);

    AddModeBlend(keyContext, builder, gatherer, blender->mode());
}

// Be sure to keep this function in sync w/ its correlate in FactoryFunctions.cpp
void add_children_to_key(const KeyContext& keyContext,
                         PaintParamsKeyBuilder* builder,
                         PipelineDataGatherer* gatherer,
                         SkSpan<const SkRuntimeEffect::ChildPtr> children,
                         SkSpan<const SkRuntimeEffect::Child> childInfo) {
    SkASSERT(children.size() == childInfo.size());

    using ChildType = SkRuntimeEffect::ChildType;

    KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

    for (size_t index = 0; index < children.size(); ++index) {
        const SkRuntimeEffect::ChildPtr& child = children[index];
        std::optional<ChildType> type = child.type();
        if (type == ChildType::kShader) {
            AddToKey(childContext, builder, gatherer, child.shader());
        } else if (type == ChildType::kColorFilter) {
            AddToKey(childContext, builder, gatherer, child.colorFilter());
        } else if (type == ChildType::kBlender) {
            AddToKey(childContext, builder, gatherer, child.blender());
        } else {
            // We don't have a child effect. Substitute in a no-op effect.
            switch (childInfo[index].type) {
                case ChildType::kShader:
                    // A missing shader returns transparent black
                    SolidColorShaderBlock::AddBlock(childContext, builder, gatherer,
                                                    SK_PMColor4fTRANSPARENT);
                    break;

                case ChildType::kColorFilter:
                    // A "passthrough" color filter returns the input color as-is.
                    builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
                    break;

                case ChildType::kBlender:
                    // A "passthrough" blender performs `blend_src_over(src, dest)`.
                    AddKnownModeBlend(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                    break;
            }
        }
    }
}

void add_to_key(const KeyContext& keyContext,
                PaintParamsKeyBuilder* builder,
                PipelineDataGatherer* gatherer,
                const SkRuntimeBlender* blender) {
    SkASSERT(blender);
    sk_sp<SkRuntimeEffect> effect = blender->effect();
    SkASSERT(effect);
    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            effect->uniforms(),
            blender->uniforms(),
            keyContext.dstColorInfo().colorSpace());
    SkASSERT(uniforms);

    RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                   { effect, std::move(uniforms) });

    add_children_to_key(keyContext, builder, gatherer,
                        blender->children(), effect->children());

    builder->endBlock();
}

void notify_in_use(Recorder* recorder,
                   DrawContext* drawContext,
                   SkSpan<const SkRuntimeEffect::ChildPtr> children) {
    for (const auto& child : children) {
        if (child.type().has_value()) {
            switch (*child.type()) {
                case SkRuntimeEffect::ChildType::kShader:
                    NotifyImagesInUse(recorder, drawContext, child.shader());
                    break;
                case SkRuntimeEffect::ChildType::kColorFilter:
                    NotifyImagesInUse(recorder, drawContext, child.colorFilter());
                    break;
                case SkRuntimeEffect::ChildType::kBlender:
                    NotifyImagesInUse(recorder, drawContext, child.blender());
                    break;
            }
        } // else a null child is a no-op, so cannot sample an image
    }
}

} // anonymous namespace

void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkBlender* blender) {
    if (!blender) {
        return;
    }
    switch (as_BB(blender)->type()) {
#define M(type)                                                     \
    case SkBlenderBase::BlenderType::k##type:                       \
        add_to_key(keyContext,                                      \
                   builder,                                         \
                   gatherer,                                        \
                   static_cast<const Sk##type##Blender*>(blender)); \
        return;
        SK_ALL_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

void NotifyImagesInUse(Recorder* recorder, DrawContext* drawContext, const SkBlender* blender) {
    if (!blender) {
        return;
    }
    if (as_BB(blender)->type() == SkBlenderBase::BlenderType::kRuntime) {
        const auto* rbb = static_cast<const SkRuntimeBlender*>(blender);
        notify_in_use(recorder, drawContext, rbb->children());
    } // else blend mode doesn't reference images
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
static SkPMColor4f map_color(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkPMColor4f color = {c.fR, c.fG, c.fB, c.fA};
    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType, dst, kPremul_SkAlphaType).apply(color.vec());
    return color;
}
static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkBlendModeColorFilter* filter) {
    SkASSERT(filter);

    SkPMColor4f color = map_color(filter->color(), sk_srgb_singleton(),
                                  keyContext.dstColorInfo().colorSpace());

    AddBlendModeColorFilter(keyContext, builder, gatherer, filter->mode(), color);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorSpaceXformColorFilter* filter) {
    SkASSERT(filter);

    constexpr SkAlphaType kAlphaType = kPremul_SkAlphaType;
    ColorSpaceTransformBlock::ColorSpaceTransformData csData(filter->src().get(), kAlphaType,
                                                             filter->dst().get(), kAlphaType);
    ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, csData);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* keyBuilder,
                       PipelineDataGatherer* gatherer,
                       const SkComposeColorFilter* filter) {
    SkASSERT(filter);

    Compose(keyContext, keyBuilder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                AddToKey(keyContext, keyBuilder, gatherer, filter->inner().get());
            },
            /* addOuterToKey= */ [&]() -> void {
                AddToKey(keyContext, keyBuilder, gatherer, filter->outer().get());
            });
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkGaussianColorFilter*) {
    builder->addBlock(BuiltInCodeSnippetID::kGaussianColorFilter);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkMatrixColorFilter* filter) {
    SkASSERT(filter);

    bool inHSLA = filter->domain() == SkMatrixColorFilter::Domain::kHSLA;
    MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(filter->matrix(), inHSLA);

    MatrixColorFilterBlock::AddBlock(keyContext, builder, gatherer, matrixCFData);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkRuntimeColorFilter* filter) {
    SkASSERT(filter);

    sk_sp<SkRuntimeEffect> effect = filter->effect();
    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            effect->uniforms(), filter->uniforms(), keyContext.dstColorInfo().colorSpace());
    SkASSERT(uniforms);

    RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, {effect, std::move(uniforms)});

    add_children_to_key(keyContext, builder, gatherer,
                        filter->children(), effect->children());

    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTableColorFilter* filter) {
    SkASSERT(filter);

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                                                filter->bitmap());
    if (!proxy) {
        SKGPU_LOG_W("Couldn't create TableColorFilter's table");

        // Return the input color as-is.
        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
        return;
    }

    TableColorFilterBlock::TableColorFilterData data(std::move(proxy));

    TableColorFilterBlock::AddBlock(keyContext, builder, gatherer, data);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkWorkingFormatColorFilter* filter) {
    SkASSERT(filter);

    const SkColorInfo& dstInfo = keyContext.dstColorInfo();
    const SkAlphaType dstAT = dstInfo.alphaType();
    sk_sp<SkColorSpace> dstCS = dstInfo.refColorSpace();
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    SkAlphaType workingAT;
    sk_sp<SkColorSpace> workingCS = filter->workingFormat(dstCS, &workingAT);
    SkColorInfo workingInfo(dstInfo.colorType(), workingAT, workingCS);
    KeyContextWithColorInfo workingContext(keyContext, workingInfo);

    // Use two nested compose blocks to chain (dst->working), child, and (working->dst) together
    // while appearing as one block to the parent node.
    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                // Inner compose
                Compose(keyContext, builder, gatherer,
                        /* addInnerToKey= */ [&]() -> void {
                            // Innermost (inner of inner compose)
                            ColorSpaceTransformBlock::ColorSpaceTransformData data1(
                                    dstCS.get(), dstAT, workingCS.get(), workingAT);
                            ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                               data1);
                        },
                        /* addOuterToKey= */ [&]() -> void {
                            // Middle (outer of inner compose)
                            AddToKey(workingContext, builder, gatherer, filter->child().get());
                        });
            },
            /* addOuterToKey= */ [&]() -> void {
                // Outermost (outer of outer compose)
                ColorSpaceTransformBlock::ColorSpaceTransformData data2(
                        workingCS.get(), workingAT, dstCS.get(), dstAT);
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data2);
            });
}

void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkColorFilter* filter) {
    if (!filter) {
        return;
    }
    switch (as_CFB(filter)->type()) {
    case SkColorFilterBase::Type::kNoop:
        // Return the input color as-is.
        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
        return;
#define M(type)                                                        \
    case SkColorFilterBase::Type::k##type:                             \
        add_to_key(keyContext,                                         \
                   builder,                                            \
                   gatherer,                                           \
                   static_cast<const Sk##type##ColorFilter*>(filter)); \
        return;
        SK_ALL_COLOR_FILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

void NotifyImagesInUse(Recorder* recorder, DrawContext* drawContext, const SkColorFilter* filter) {
    if (!filter) {
        return;
    }
    if (as_CFB(filter)->type() == SkColorFilterBase::Type::kCompose) {
        // Recurse to two children
        const auto* cf = static_cast<const SkComposeColorFilter*>(filter);
        NotifyImagesInUse(recorder, drawContext, cf->inner().get());
        NotifyImagesInUse(recorder, drawContext, cf->outer().get());
    } else if (as_CFB(filter)->type() == SkColorFilterBase::Type::kWorkingFormat) {
        // Recurse to one child
        const auto* wfcf = static_cast<const SkWorkingFormatColorFilter*>(filter);
        NotifyImagesInUse(recorder, drawContext, wfcf->child().get());
    } else if (as_CFB(filter)->type() == SkColorFilterBase::Type::kRuntime) {
        // Recurse to all children
        const auto* rcf = static_cast<const SkRuntimeColorFilter*>(filter);
        notify_in_use(recorder, drawContext, rcf->children());
    } // else other color filters do not rely on SkImages
}

// ==================================================================

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkBlendShader* shader) {
    SkASSERT(shader);

    Blend(keyContext, builder, gatherer,
            /* addBlendToKey= */ [&] () -> void {
                AddModeBlend(keyContext, builder, gatherer, shader->mode());
            },
            /* addSrcToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->src().get());
            },
            /* addDstToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->dst().get());
            });
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkBlendShader* shader) {
    // SkBlendShader uses a fixed blend mode, so there's no blender to recurse through
    NotifyImagesInUse(recorder, drawContext, shader->src().get());
    NotifyImagesInUse(recorder, drawContext, shader->dst().get());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkCTMShader* shader) {
    // CTM shaders are always given device coordinates, so we don't have to modify the CTM itself
    // with keyContext's local transform.
    LocalMatrixShaderBlock::LMShaderData lmShaderData(shader->ctm());

    KeyContextWithLocalMatrix newContext(keyContext, shader->ctm());

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, lmShaderData);

        AddToKey(newContext, builder, gatherer, shader->proxyShader().get());

    builder->endBlock();
}
static void notify_in_use(Recorder* recorder, DrawContext* drawContext, const SkCTMShader* shader) {
    NotifyImagesInUse(recorder, drawContext, shader->proxyShader().get());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorShader* shader) {
    SkASSERT(shader);

    SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer,
                                    SkColor4f::FromColor(shader->color()).premul());
}
static void notify_in_use(Recorder*, DrawContext*, const SkColorShader*) {
    // No-op
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColor4Shader* shader) {
    SkASSERT(shader);

    SkPMColor4f color = map_color(shader->color(), shader->colorSpace().get(),
                                  keyContext.dstColorInfo().colorSpace());

    SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, color);
}
static void notify_in_use(Recorder*, DrawContext*, const SkColor4Shader*) {
    // No-op
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorFilterShader* shader) {
    SkASSERT(shader);

    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->shader().get());
            },
            /* addOuterToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->filter().get());
            });
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkColorFilterShader* shader) {
    NotifyImagesInUse(recorder, drawContext, shader->shader().get());
    NotifyImagesInUse(recorder, drawContext, shader->filter().get());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkCoordClampShader* shader) {
    SkASSERT(shader);

    CoordClampShaderBlock::CoordClampData data(shader->subset());

    CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, data);
        AddToKey(keyContext, builder, gatherer, shader->shader().get());
    builder->endBlock();
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkCoordClampShader* shader) {
    NotifyImagesInUse(recorder, drawContext, shader->shader().get());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkEmptyShader*) {
    builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
}
static void notify_in_use(Recorder*, DrawContext*, const SkEmptyShader*) {
    // No-op
}

static void add_yuv_image_to_key(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 const SkImageShader* origShader,
                                 sk_sp<const SkImage> imageToDraw,
                                 SkSamplingOptions sampling) {
    SkASSERT(!imageToDraw->isAlphaOnly());

    const Image_YUVA* yuvaImage = static_cast<const Image_YUVA*>(imageToDraw.get());
    const SkYUVAInfo& yuvaInfo = yuvaImage->yuvaInfo();
    // We would want to add a translation to the local matrix to handle other sitings.
    SkASSERT(yuvaInfo.sitingX() == SkYUVAInfo::Siting::kCentered);
    SkASSERT(yuvaInfo.sitingY() == SkYUVAInfo::Siting::kCentered);
    YUVImageShaderBlock::ImageData imgData(sampling,
                                           origShader->tileModeX(),
                                           origShader->tileModeY(),
                                           imageToDraw->dimensions(),
                                           origShader->subset());
    for (int locIndex = 0; locIndex < SkYUVAInfo::kYUVAChannelCount; ++locIndex) {
        const TextureProxyView& view = yuvaImage->proxyView(locIndex);
        if (view) {
            imgData.fTextureProxies[locIndex] = view.refProxy();
            // The view's swizzle has the data channel for the YUVA location in all slots, so read
            // the 0th slot to determine fChannelSelect
            switch(view.swizzle()[0]) {
                case 'r': imgData.fChannelSelect[locIndex] = {1.f, 0.f, 0.f, 0.f}; break;
                case 'g': imgData.fChannelSelect[locIndex] = {0.f, 1.f, 0.f, 0.f}; break;
                case 'b': imgData.fChannelSelect[locIndex] = {0.f, 0.f, 1.f, 0.f}; break;
                case 'a': imgData.fChannelSelect[locIndex] = {0.f, 0.f, 0.f, 1.f}; break;
                default:
                    imgData.fChannelSelect[locIndex] = {0.f, 0.f, 0.f, 0.f};
                    SkDEBUGFAILF("Unexpected swizzle for YUVA data: %c", view.swizzle()[0]);
                    break;
            }
        } else {
            // Only the A proxy view should be null, in which case we bind the Y proxy view to
            // pass validation and send all 1s for the channel selection to signal opaque alpha.
            SkASSERT(locIndex == 3);
            imgData.fTextureProxies[locIndex] = yuvaImage->proxyView(SkYUVAInfo::kY).refProxy();
            imgData.fChannelSelect[locIndex] = {1.f, 1.f, 1.f, 1.f};
        }
    }

    auto [ssx, ssy] = yuvaImage->uvSubsampleFactors();
    if (ssx > 1 || ssy > 1) {
        // We need to adjust the image size we use for sampling to reflect the actual image size of
        // the UV planes. However, since our coordinates are in Y's texel space we need to scale
        // accordingly.
        const TextureProxyView& view = yuvaImage->proxyView(SkYUVAInfo::kU);
        imgData.fImgSizeUV = {view.dimensions().width()*ssx, view.dimensions().height()*ssy};
        // This promotion of nearest to linear filtering for UV planes exists to mimic
        // libjpeg[-turbo]'s do_fancy_upsampling option. We will filter the subsampled plane,
        // however we want to filter at a fixed point for each logical image pixel to simulate
        // nearest neighbor. In the shader we detect that the UV filtermode doesn't match the Y
        // filtermode, and snap to Y pixel centers.
        if (imgData.fSampling.filter == SkFilterMode::kNearest) {
            imgData.fSamplingUV = SkSamplingOptions(SkFilterMode::kLinear,
                                                    imgData.fSampling.mipmap);
            // Consider a logical image pixel at the edge of the subset. When computing the logical
            // pixel color value we should use a blend of two values from the subsampled plane.
            // Depending on where the subset edge falls in actual subsampled plane, one of those
            // values may come from outside the subset. Hence, we will use the default inset
            // in Y texel space of 1/2. This applies the wrap mode to the subset but allows
            // linear filtering to read pixels that are just outside the subset.
            imgData.fLinearFilterUVInset.fX = 0.5f;
            imgData.fLinearFilterUVInset.fY = 0.5f;
        } else if (imgData.fSampling.filter == SkFilterMode::kLinear) {
            // We need to inset so that we aren't sampling outside the subset, but no farther.
            // Start by mapping the subset to UV texel space
            float scaleX = 1.f/ssx;
            float scaleY = 1.f/ssy;
            SkRect subsetUV = {imgData.fSubset.fLeft  *scaleX,
                               imgData.fSubset.fTop   *scaleY,
                               imgData.fSubset.fRight *scaleX,
                               imgData.fSubset.fBottom*scaleY};
            // Round to UV texel borders
            SkIRect iSubsetUV = subsetUV.roundOut();
            // Inset in UV and map back to Y texel space. This gives us the largest possible
            // inset rectangle that will not sample outside of the subset texels in UV space.
            SkRect insetRectUV = {(iSubsetUV.fLeft  +0.5f)*ssx,
                                  (iSubsetUV.fTop   +0.5f)*ssy,
                                  (iSubsetUV.fRight -0.5f)*ssx,
                                  (iSubsetUV.fBottom-0.5f)*ssy};
            // Compute intersection with original inset
            SkRect insetRect = imgData.fSubset.makeOutset(-0.5f, -0.5f);
            (void) insetRect.intersect(insetRectUV);
            // Compute max inset values to ensure we always remain within the subset.
            imgData.fLinearFilterUVInset = {std::max(insetRect.fLeft - imgData.fSubset.fLeft,
                                                     imgData.fSubset.fRight - insetRect.fRight),
                                            std::max(insetRect.fTop - imgData.fSubset.fTop,
                                                     imgData.fSubset.fBottom - insetRect.fBottom)};
        }
    }

    float yuvM[20];
    SkColorMatrix_YUV2RGB(yuvaInfo.yuvColorSpace(), yuvM);
    // We drop the fourth column entirely since the transformation
    // should not depend on alpha. The fifth column is sent as a separate
    // vector. The fourth row is also dropped entirely because alpha should
    // never be modified.
    SkASSERT(yuvM[3] == 0 && yuvM[8] == 0 && yuvM[13] == 0 && yuvM[18] == 1);
    SkASSERT(yuvM[15] == 0 && yuvM[16] == 0 && yuvM[17] == 0 && yuvM[19] == 0);
    imgData.fYUVtoRGBMatrix.setAll(
        yuvM[ 0], yuvM[ 1], yuvM[ 2],
        yuvM[ 5], yuvM[ 6], yuvM[ 7],
        yuvM[10], yuvM[11], yuvM[12]
    );
    imgData.fYUVtoRGBTranslate = {yuvM[4], yuvM[9], yuvM[14]};

    // The YUV formats can encode their own origin including reflection and rotation,
    // so we need to wrap our block in an additional local matrix transform.
    SkMatrix originMatrix = yuvaInfo.originMatrix();
    LocalMatrixShaderBlock::LMShaderData lmShaderData(originMatrix);

    KeyContextWithLocalMatrix newContext(keyContext, originMatrix);

    SkColorSpaceXformSteps steps;
    SkASSERT(steps.flags.mask() == 0);   // By default, the colorspace should have no effect
    if (!origShader->isRaw()) {
        steps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                       imageToDraw->alphaType(),
                                       keyContext.dstColorInfo().colorSpace(),
                                       keyContext.dstColorInfo().alphaType());
    }
    ColorSpaceTransformBlock::ColorSpaceTransformData data(steps);

    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, lmShaderData);
                    YUVImageShaderBlock::AddBlock(newContext, builder, gatherer, imgData);
                builder->endBlock();
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data);
            });
}

static skgpu::graphite::ReadSwizzle swizzle_class_to_read_enum(const skgpu::Swizzle& swizzle) {
    if (swizzle == skgpu::Swizzle::RGBA()) {
        return skgpu::graphite::ReadSwizzle::kRGBA;
    } else if (swizzle == skgpu::Swizzle::RGB1()) {
        return skgpu::graphite::ReadSwizzle::kRGB1;
    } else if (swizzle == skgpu::Swizzle("rrr1")) {
        return skgpu::graphite::ReadSwizzle::kRRR1;
    } else if (swizzle == skgpu::Swizzle::BGRA()) {
        return skgpu::graphite::ReadSwizzle::kBGRA;
    } else if (swizzle == skgpu::Swizzle("000r")) {
        return skgpu::graphite::ReadSwizzle::k000R;
    } else {
        SKGPU_LOG_W("%s is an unsupported read swizzle. Defaulting to RGBA.\n",
                    swizzle.asString().data());
        return skgpu::graphite::ReadSwizzle::kRGBA;
    }
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkImageShader* shader) {
    SkASSERT(shader);

    auto [ imageToDraw, newSampling ] = GetGraphiteBacked(keyContext.recorder(),
                                                          shader->image().get(),
                                                          shader->sampling());
    if (!imageToDraw) {
        SKGPU_LOG_W("Couldn't convert ImageShader's image to a Graphite-backed image");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }
    if (as_IB(imageToDraw)->isYUVA()) {
        return add_yuv_image_to_key(keyContext,
                                      builder,
                                      gatherer,
                                      shader,
                                      std::move(imageToDraw),
                                      newSampling);
    }

    auto view = AsView(imageToDraw.get());
    SkASSERT(newSampling.mipmap == SkMipmapMode::kNone || view.mipmapped() == Mipmapped::kYes);

    ImageShaderBlock::ImageData imgData(shader->sampling(),
                                        shader->tileModeX(),
                                        shader->tileModeY(),
                                        view.proxy()->dimensions(),
                                        shader->subset(),
                                        ReadSwizzle::kRGBA);
    imgData.fSampling = newSampling;
    imgData.fTextureProxy = view.refProxy();
    skgpu::Swizzle readSwizzle = view.swizzle();
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (imageToDraw->isAlphaOnly()) {
        readSwizzle = skgpu::Swizzle::Concat(readSwizzle, skgpu::Swizzle("000a"));
    }
    imgData.fReadSwizzle = swizzle_class_to_read_enum(readSwizzle);

    if (!shader->isRaw()) {
        imgData.fSteps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                                imageToDraw->alphaType(),
                                                keyContext.dstColorInfo().colorSpace(),
                                                keyContext.dstColorInfo().alphaType());

        if (imageToDraw->isAlphaOnly() && keyContext.scope() != KeyContext::Scope::kRuntimeEffect) {
            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&] () -> void {
                      AddKnownModeBlend(keyContext, builder, gatherer, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
                  });
            return;
        }
    }

    ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
}
static void notify_in_use(Recorder* recorder,
                          DrawContext*,
                          const SkImageShader* shader) {
    auto image = as_IB(shader->image());
    if (!image->isGraphiteBacked()) {
        // If it's not graphite-backed, there's no pending graphite work.
        return;
    }

    // TODO(b/323887207): Once scratch devices are linked to special images and their use needs to
    // be linked to specific draw contexts, that will be passed in here.
    static_cast<Image_Base*>(image)->notifyInUse(recorder);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkLocalMatrixShader* shader) {
    SkASSERT(shader);
    auto wrappedShader = shader->wrappedShader().get();

    // Fold the texture's origin flip into the local matrix so that the image shader doesn't need
    // additional state
    SkMatrix matrix;
    if (as_SB(wrappedShader)->type() == SkShaderBase::ShaderType::kImage) {
        auto imgShader = static_cast<const SkImageShader*>(wrappedShader);
        // If the image is not graphite backed then we can assume the origin will be TopLeft as we
        // require that in the ImageProvider utility. Also Graphite YUV images are assumed to be
        // TopLeft origin.
        // TODO (b/336788317): Fold YUVAImage's origin into this matrix as well.
        auto imgBase = as_IB(imgShader->image());
        if (imgBase->isGraphiteBacked() && !imgBase->isYUVA()) {
            auto imgGraphite = static_cast<Image*>(imgBase);
            SkASSERT(imgGraphite);
            const auto& view = imgGraphite->textureProxyView();
            if (view.origin() == Origin::kBottomLeft) {
                matrix.setScaleY(-1);
                matrix.setTranslateY(view.height());
            }
        }
    }

    matrix.postConcat(shader->localMatrix());
    LocalMatrixShaderBlock::LMShaderData lmShaderData(matrix);

    KeyContextWithLocalMatrix newContext(keyContext, matrix);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, lmShaderData);

        AddToKey(newContext, builder, gatherer, wrappedShader);

    builder->endBlock();
}

static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkLocalMatrixShader* shader) {
    NotifyImagesInUse(recorder, drawContext, shader->wrappedShader().get());
}

// If either of these change then the corresponding change must also be made in the SkSL
// perlin_noise_shader function.
static_assert((int)SkPerlinNoiseShaderType::kFractalNoise ==
              (int)PerlinNoiseShaderBlock::Type::kFractalNoise);
static_assert((int)SkPerlinNoiseShaderType::kTurbulence ==
              (int)PerlinNoiseShaderBlock::Type::kTurbulence);
static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkPerlinNoiseShader* shader) {
    SkASSERT(shader);
    SkASSERT(shader->numOctaves());

    std::unique_ptr<SkPerlinNoiseShader::PaintingData> paintingData = shader->getPaintingData();
    paintingData->generateBitmaps();

    sk_sp<TextureProxy> perm = RecorderPriv::CreateCachedProxy(
            keyContext.recorder(), paintingData->getPermutationsBitmap());

    sk_sp<TextureProxy> noise =
            RecorderPriv::CreateCachedProxy(keyContext.recorder(), paintingData->getNoiseBitmap());

    if (!perm || !noise) {
        SKGPU_LOG_W("Couldn't create tables for PerlinNoiseShader");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    PerlinNoiseShaderBlock::PerlinNoiseData perlinData(
            static_cast<PerlinNoiseShaderBlock::Type>(shader->noiseType()),
            paintingData->fBaseFrequency,
            shader->numOctaves(),
            {paintingData->fStitchDataInit.fWidth, paintingData->fStitchDataInit.fHeight});

    perlinData.fPermutationsProxy = std::move(perm);
    perlinData.fNoiseProxy = std::move(noise);

    PerlinNoiseShaderBlock::AddBlock(keyContext, builder, gatherer, perlinData);
}
static void notify_in_use(Recorder*, DrawContext*, const SkPerlinNoiseShader*) {
    // No-op, perlin noise has no children.
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkPictureShader* shader) {
    SkASSERT(shader);

    Recorder* recorder = keyContext.recorder();
    const Caps* caps = recorder->priv().caps();

    // TODO: We'll need additional plumbing to get the correct props from our callers. In
    // particular we'll need to expand the keyContext to have the surfaceProps.
    SkSurfaceProps props{};

    SkMatrix totalM = keyContext.local2Dev().asM33();
    if (keyContext.localMatrix()) {
        totalM.preConcat(*keyContext.localMatrix());
    }
    auto info = SkPictureShader::CachedImageInfo::Make(shader->tile(),
                                                       totalM,
                                                       keyContext.dstColorInfo().colorType(),
                                                       keyContext.dstColorInfo().colorSpace(),
                                                       caps->maxTextureSize(),
                                                       props);
    if (!info.success) {
        SKGPU_LOG_W("Couldn't access PictureShaders' Image info");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    // TODO: right now we're explicitly not caching here. We could expand the ImageProvider
    // API to include already Graphite-backed images, add a Recorder-local cache or add
    // rendered-picture images to the global cache.
    sk_sp<SkImage> img = info.makeImage(
            SkSurfaces::RenderTarget(recorder, info.imageInfo, skgpu::Mipmapped::kNo, &info.props),
            shader->picture().get());
    if (!img) {
        SKGPU_LOG_W("Couldn't create SkImage for PictureShader");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    const auto shaderLM = SkMatrix::Scale(1.f/info.tileScale.width(), 1.f/info.tileScale.height());
    sk_sp<SkShader> imgShader = img->makeShader(shader->tileModeX(), shader->tileModeY(),
                                                SkSamplingOptions(shader->filter()), &shaderLM);
    if (!imgShader) {
        SKGPU_LOG_W("Couldn't create SkImageShader for PictureShader");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    AddToKey(keyContext, builder, gatherer, imgShader.get());
}
static void notify_in_use(Recorder*, DrawContext*, const SkPictureShader*) {
    // While the SkPicture the shader points to, may have Graphite-backed shaders that need to be
    // notified, that will happen when the picture is rendered into an image in add_to_key
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkRuntimeShader* shader) {
    SkASSERT(shader);
    sk_sp<SkRuntimeEffect> effect = shader->effect();
    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            effect->uniforms(),
            shader->uniformData(keyContext.dstColorInfo().colorSpace()),
            keyContext.dstColorInfo().colorSpace());
    SkASSERT(uniforms);

    RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                   {effect, std::move(uniforms)});

    add_children_to_key(keyContext, builder, gatherer,
                        shader->children(), effect->children());

    builder->endBlock();
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkRuntimeShader* shader) {
    notify_in_use(recorder, drawContext, shader->children());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTransformShader* shader) {
    SKGPU_LOG_W("Raster-only SkShader (SkTransformShader) encountered");
    builder->addBlock(BuiltInCodeSnippetID::kError);
}
static void notify_in_use(Recorder*, DrawContext*, const SkTransformShader*) {
    // no-op
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTriColorShader* shader) {
    SKGPU_LOG_W("Raster-only SkShader (SkTriColorShader) encountered");
    builder->addBlock(BuiltInCodeSnippetID::kError);
}
static void notify_in_use(Recorder*, DrawContext*, const SkTriColorShader*) {
    // no-op
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkWorkingColorSpaceShader* shader) {
    SkASSERT(shader);

    const SkColorInfo& dstInfo = keyContext.dstColorInfo();
    const SkAlphaType dstAT = dstInfo.alphaType();
    sk_sp<SkColorSpace> dstCS = dstInfo.refColorSpace();
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    sk_sp<SkColorSpace> workingCS = shader->workingSpace();
    SkColorInfo workingInfo(dstInfo.colorType(), dstAT, workingCS);
    KeyContextWithColorInfo workingContext(keyContext, workingInfo);

    // Compose the inner shader (in the working space) with a (working->dst) transform:
    Compose(keyContext, builder, gatherer,
        /* addInnerToKey= */ [&]() -> void {
            AddToKey(workingContext, builder, gatherer, shader->shader().get());
        },
        /* addOuterToKey= */ [&]() -> void {
            ColorSpaceTransformBlock::ColorSpaceTransformData data(
                    workingCS.get(), dstAT, dstCS.get(), dstAT);
            ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data);
        });
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkWorkingColorSpaceShader* shader) {
    NotifyImagesInUse(recorder, drawContext, shader->shader().get());
}

static SkBitmap create_color_and_offset_bitmap(int numStops,
                                               const SkPMColor4f* colors,
                                               const float* offsets) {
    SkBitmap colorsAndOffsetsBitmap;

    colorsAndOffsetsBitmap.allocPixels(
            SkImageInfo::Make(numStops, 2, kRGBA_F16_SkColorType, kPremul_SkAlphaType));

    for (int i = 0; i < numStops; i++) {
        // TODO: there should be a way to directly set a premul pixel in a bitmap with
        // a premul color.
        SkColor4f unpremulColor = colors[i].unpremul();
        colorsAndOffsetsBitmap.erase(unpremulColor, SkIRect::MakeXYWH(i, 0, 1, 1));

        float offset = offsets ? offsets[i] : SkIntToFloat(i) / (numStops - 1);
        SkASSERT(offset >= 0.0f && offset <= 1.0f);

        int exponent;
        float mantissa = frexp(offset, &exponent);

        SkHalf halfE = SkFloatToHalf(exponent);
        if ((int)SkHalfToFloat(halfE) != exponent) {
            SKGPU_LOG_W("Encoding gradient to f16 failed");
            return {};
        }

#if defined(SK_DEBUG)
        SkHalf halfM = SkFloatToHalf(mantissa);

        float restored = ldexp(SkHalfToFloat(halfM), (int)SkHalfToFloat(halfE));
        float error = abs(restored - offset);
        SkASSERT(error < 0.001f);
#endif

        // TODO: we're only using 2 of the f16s here. The encoding could be altered to better
        // preserve precision. This encoding yields < 0.001f error for 2^20 evenly spaced stops.
        colorsAndOffsetsBitmap.erase(SkColor4f{mantissa, (float)exponent, 0, 1},
                                     SkIRect::MakeXYWH(i, 1, 1, 1));
    }

    return colorsAndOffsetsBitmap;
}

// Please see GrGradientShader.cpp::make_interpolated_to_dst for substantial comments
// as to why this code is structured this way.
static void make_interpolated_to_dst(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer,
                                     const GradientShaderBlocks::GradientData& gradData,
                                     const SkGradientShader::Interpolation& interp,
                                     SkColorSpace* intermediateCS) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;

    bool inputPremul = static_cast<bool>(interp.fInPremul);

    switch (interp.fColorSpace) {
        case ColorSpace::kLab:
        case ColorSpace::kOKLab:
        case ColorSpace::kOKLabGamutMap:
        case ColorSpace::kLCH:
        case ColorSpace::kOKLCH:
        case ColorSpace::kOKLCHGamutMap:
        case ColorSpace::kHSL:
        case ColorSpace::kHWB:
            inputPremul = false;
            break;
        default:
            break;
    }

    const SkColorInfo& dstColorInfo = keyContext.dstColorInfo();

    SkColorSpace* dstColorSpace =
            dstColorInfo.colorSpace() ? dstColorInfo.colorSpace() : sk_srgb_singleton();

    SkAlphaType intermediateAlphaType = inputPremul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;

    ColorSpaceTransformBlock::ColorSpaceTransformData data(
            intermediateCS, intermediateAlphaType, dstColorSpace, dstColorInfo.alphaType());

    // The gradient block and colorSpace conversion block need to be combined
    // (via the Compose block) so that the localMatrix block can treat them as
    // one child.
    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                GradientShaderBlocks::AddBlock(keyContext, builder, gatherer, gradData);
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data);
            });
}

static void add_gradient_to_key(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const SkGradientBaseShader* shader,
                                SkPoint point0,
                                SkPoint point1,
                                float radius0,
                                float radius1,
                                float bias,
                                float scale) {
    SkColor4fXformer xformedColors(shader, keyContext.dstColorInfo().colorSpace());
    const SkPMColor4f* colors = xformedColors.fColors.begin();
    const float* positions = xformedColors.fPositions;
    const int colorCount = xformedColors.fColors.size();

    sk_sp<TextureProxy> proxy;

    if (colorCount > GradientShaderBlocks::GradientData::kNumInternalStorageStops) {
        if (shader->cachedBitmap().empty()) {
            SkBitmap colorsAndOffsetsBitmap =
                    create_color_and_offset_bitmap(colorCount, colors, positions);
            if (colorsAndOffsetsBitmap.empty()) {
                SKGPU_LOG_W("Couldn't create GradientShader's color and offset bitmap");
                builder->addBlock(BuiltInCodeSnippetID::kError);
                return;
            }
            shader->setCachedBitmap(colorsAndOffsetsBitmap);
        }

        proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), shader->cachedBitmap());
        if (!proxy) {
            SKGPU_LOG_W("Couldn't create GradientShader's color and offset bitmap proxy");
            builder->addBlock(BuiltInCodeSnippetID::kError);
            return;
        }
    }

    GradientShaderBlocks::GradientData data(shader->asGradient(),
                                            point0,
                                            point1,
                                            radius0,
                                            radius1,
                                            bias,
                                            scale,
                                            shader->getTileMode(),
                                            colorCount,
                                            colors,
                                            positions,
                                            std::move(proxy),
                                            shader->getInterpolation());

    make_interpolated_to_dst(keyContext,
                             builder,
                             gatherer,
                             data,
                             shader->getInterpolation(),
                             xformedColors.fIntermediateColorSpace.get());
}

static void add_gradient_to_key(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const SkConicalGradient* shader) {
    add_gradient_to_key(keyContext,
                        builder,
                        gatherer,
                        shader,
                        shader->getStartCenter(),
                        shader->getEndCenter(),
                        shader->getStartRadius(),
                        shader->getEndRadius(),
                        0.0f,
                        0.0f);
}

static void add_gradient_to_key(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const SkLinearGradient* shader) {
    add_gradient_to_key(keyContext,
                        builder,
                        gatherer,
                        shader,
                        shader->start(),
                        shader->end(),
                        0.0f,
                        0.0f,
                        0.0f,
                        0.0f);
}

static void add_gradient_to_key(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const SkRadialGradient* shader) {
    add_gradient_to_key(keyContext,
                        builder,
                        gatherer,
                        shader,
                        shader->center(),
                        { 0.0f, 0.0f },
                        shader->radius(),
                        0.0f,
                        0.0f,
                        0.0f);
}

static void add_gradient_to_key(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const SkSweepGradient* shader) {
    add_gradient_to_key(keyContext,
                        builder,
                        gatherer,
                        shader,
                        shader->center(),
                        { 0.0f, 0.0f },
                        0.0f,
                        0.0f,
                        shader->tBias(),
                        shader->tScale());
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkGradientBaseShader* shader) {
    SkASSERT(shader);
    switch (shader->asGradient()) {
#define M(type)                                                               \
    case SkShaderBase::GradientType::k##type:                                 \
        add_gradient_to_key(keyContext,                                       \
                            builder,                                          \
                            gatherer,                                         \
                            static_cast<const Sk##type##Gradient*>(shader));  \
        return;
        SK_ALL_GRADIENTS(M)
#undef M
        case SkShaderBase::GradientType::kNone:
            SkDEBUGFAIL("Gradient shader says its type is none");
            return;
    }
    SkUNREACHABLE;
}
static void notify_in_use(Recorder*, DrawContext*, const SkGradientBaseShader*) {
    // Gradients do not have children, so no images to notify
}

void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkShader* shader) {
    if (!shader) {
        return;
    }
    switch (as_SB(shader)->type()) {
#define M(type)                                                        \
    case SkShaderBase::ShaderType::k##type:                            \
        add_to_key(keyContext,                                         \
                   builder,                                            \
                   gatherer,                                           \
                   static_cast<const Sk##type##Shader*>(shader)); \
        return;
        SK_ALL_SHADERS(M)
#undef M
    }
    SkUNREACHABLE;
}

void NotifyImagesInUse(Recorder* recorder,
                       DrawContext* drawContext,
                       const SkShader* shader) {
    if (!shader) {
        return;
    }
    switch (as_SB(shader)->type()) {
#define M(type)                                                      \
    case SkShaderBase::ShaderType::k##type:                          \
        notify_in_use(recorder,                                      \
                      drawContext,                                   \
                      static_cast<const Sk##type##Shader*>(shader)); \
        return;
        SK_ALL_SHADERS(M)
#undef M
    }
    SkUNREACHABLE;
}


} // namespace skgpu::graphite
