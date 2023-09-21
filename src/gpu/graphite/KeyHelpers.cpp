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
#include "src/gpu/graphite/YUVATextureProxies.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkBlendShader.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkCoordClampShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkPerlinNoiseShaderImpl.h"
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

void add_dst_read_sample_uniform_data(const ShaderCodeDictionary* dict,
                                      PipelineDataGatherer* gatherer,
                                      sk_sp<TextureProxy> dstTexture,
                                      SkIPoint dstOffset) {
    static const SkTileMode kTileModes[2] = {SkTileMode::kClamp, SkTileMode::kClamp};
    gatherer->add(SkSamplingOptions(), kTileModes, dstTexture);

    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kDstReadSample)

    SkV4 coords{static_cast<float>(dstOffset.x()),
                static_cast<float>(dstOffset.y()),
                1.0f / dstTexture->dimensions().width(),
                1.0f / dstTexture->dimensions().height()};
    gatherer->write(coords);
}

} // anonymous namespace

void DstReadSampleBlock::BeginBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    sk_sp<TextureProxy> dstTexture,
                                    SkIPoint dstOffset) {
    if (gatherer) {
        add_dst_read_sample_uniform_data(
                keyContext.dict(), gatherer, std::move(dstTexture), dstOffset);
    }
    builder->beginBlock(BuiltInCodeSnippetID::kDstReadSample);
}

void DstReadFetchBlock::BeginBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer) {
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kDstReadFetch)
    }
    builder->beginBlock(BuiltInCodeSnippetID::kDstReadFetch);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_gradient_preamble(const GradientShaderBlocks::GradientData& gradData,
                           PipelineDataGatherer* gatherer) {
    constexpr int kInternalStopLimit = GradientShaderBlocks::GradientData::kNumInternalStorageStops;

    if (gradData.fNumStops <= kInternalStopLimit) {
        if (gradData.fNumStops <= 4) {
            // Round up to 4 stops.
            gatherer->writeArray({gradData.fColors, 4});
            // The offsets are packed into a single float4 to save space.
            gatherer->write(SkSLType::kFloat4, &gradData.fOffsets);
        } else if (gradData.fNumStops <= 8) {
            // Round up to 8 stops.
            gatherer->writeArray({gradData.fColors, 8});
            // The offsets are packed into a float4 array to save space.
            gatherer->writeArray(SkSLType::kFloat4, &gradData.fOffsets, 2);
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
        case SkShaderBase::GradientType::kNone:
        default:
            SkDEBUGFAIL("Expected a gradient shader, but it wasn't one.");
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
    // We have 7 source coefficients and 7 destination coefficients. We pass them via a 4x4 matrix;
    // the first two columns hold the source values, and the second two hold the destination.
    // (The final value of each 8-element group is ignored.)
    // In std140, this arrangement is much more efficient than a simple array of scalars.
    SkM44 coeffs;

    gatherer->write(SkTo<int>(steps.flags.mask()));

    if (steps.flags.linearize) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.srcTF)));
        coeffs.setCol(0, {steps.srcTF.g, steps.srcTF.a, steps.srcTF.b, steps.srcTF.c});
        coeffs.setCol(1, {steps.srcTF.d, steps.srcTF.e, steps.srcTF.f, 0.0f});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
    }

    SkMatrix gamutTransform;
    if (steps.flags.gamut_transform) {
        // TODO: it seems odd to copy this into an SkMatrix just to write it to the gatherer
        gamutTransform.set9(steps.src_to_dst_matrix);
    }
    gatherer->writeHalf(gamutTransform);

    if (steps.flags.encode) {
        gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.dstTFInv)));
        coeffs.setCol(2, {steps.dstTFInv.g, steps.dstTFInv.a, steps.dstTFInv.b, steps.dstTFInv.c});
        coeffs.setCol(3, {steps.dstTFInv.d, steps.dstTFInv.e, steps.dstTFInv.f, 0.0f});
    } else {
        gatherer->write(SkTo<int>(skcms_TFType::skcms_TFType_Invalid));
    }

    gatherer->writeHalf(coeffs);
}

void add_image_uniform_data(const ShaderCodeDictionary* dict,
                            const ImageShaderBlock::ImageData& imgData,
                            PipelineDataGatherer* gatherer) {
    SkASSERT(!imgData.fSampling.useCubic);
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kImageShader)

    gatherer->write(SkPoint::Make(imgData.fTextureProxy->dimensions().fWidth,
                                  imgData.fTextureProxy->dimensions().fHeight));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
    gatherer->write(SkTo<int>(imgData.fReadSwizzle));

    add_color_space_uniforms(imgData.fSteps, gatherer);
}


void add_cubic_image_uniform_data(const ShaderCodeDictionary* dict,
                                  const ImageShaderBlock::ImageData& imgData,
                                  PipelineDataGatherer* gatherer) {
    SkASSERT(imgData.fSampling.useCubic);
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCubicImageShader)

    gatherer->write(SkPoint::Make(imgData.fTextureProxy->dimensions().fWidth,
                                  imgData.fTextureProxy->dimensions().fHeight));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    const SkCubicResampler& cubic = imgData.fSampling.cubic;
    gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));
    gatherer->write(SkTo<int>(imgData.fReadSwizzle));

    add_color_space_uniforms(imgData.fSteps, gatherer);
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
        // proxy (i.e., we can remove the 'gatherer' in the above test).
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

void ImageShaderBlock::BeginCubicBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const ImageData* imgData) {
    SkASSERT(!gatherer == !imgData);

    // TODO: allow through lazy proxies
    if (gatherer && !imgData->fTextureProxy) {
        // TODO: At some point the pre-compile path should also be creating a texture
        // proxy (i.e., we can remove the 'gatherer' in the above test).
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        return;
    }

    auto dict = keyContext.dict();
    if (gatherer) {
        gatherer->add(imgData->fSampling,
                      imgData->fTileModes,
                      imgData->fTextureProxy);

        add_cubic_image_uniform_data(dict, *imgData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kCubicImageShader);
}

//--------------------------------------------------------------------------------------------------

// makes use of ImageShader functions, above
namespace {

void add_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                const YUVImageShaderBlock::ImageData& imgData,
                                PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kYUVImageShader)

    gatherer->write(imgData.fImgSize);
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes[0]));
    gatherer->write(SkTo<int>(imgData.fTileModes[1]));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
    gatherer->write(imgData.fSampling.useCubic);
    if (imgData.fSampling.useCubic) {
        const SkCubicResampler& cubic = imgData.fSampling.cubic;
        gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));
    } else {
        gatherer->writeHalf(SkM44());
    }

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->write(imgData.fYUVtoRGBTranslate);

    add_color_space_uniforms(imgData.fSteps, gatherer);
}

} // anonymous namespace

YUVImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                          SkTileMode tileModeX,
                                          SkTileMode tileModeY,
                                          SkRect subset)
        : fSampling(sampling)
        , fTileModes{tileModeX, tileModeY}
        , fSubset(subset) {
    SkASSERT(fSteps.flags.mask() == 0);   // By default, the colorspace should have no effect
}

void YUVImageShaderBlock::BeginBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer,
                                     const ImageData* imgData) {
    SkASSERT(!gatherer == !imgData);

    // TODO: allow through lazy proxies
    if (gatherer &&
        (!imgData->fTextureProxies[0] || !imgData->fTextureProxies[1] ||
         !imgData->fTextureProxies[2] || !imgData->fTextureProxies[3])) {
        // TODO: At some point the pre-compile path should also be creating a texture
        // proxy (i.e., we can remove the 'pipelineData' in the above test).
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        return;
    }

    auto dict = keyContext.dict();
    if (gatherer) {
        for (int i = 0; i < 4; ++i) {
            gatherer->add(imgData->fSampling,
                          imgData->fTileModes,
                          imgData->fTextureProxies[i]);
        }

        add_yuv_image_uniform_data(dict, *imgData, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kYUVImageShader);
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
}

} // anonymous namespace

void DitherShaderBlock::BeginBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer,
                                   const DitherData* ditherData) {
    if (gatherer) {
        static const SkBitmap gLUT = skgpu::MakeDitherLUT();

        sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), gLUT);
        if (!proxy) {
            SKGPU_LOG_W("Couldn't create dither shader's LUT");

            PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
            return;
        }

        add_dither_uniform_data(keyContext.dict(), *ditherData, gatherer);

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
        VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kBlendShader)
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendShader);
}

//--------------------------------------------------------------------------------------------------

void BlendModeBlenderBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       SkBlendMode blendMode) {
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kBlendModeBlender)
        gatherer->write(SkTo<int>(blendMode));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendModeBlender);
}

//--------------------------------------------------------------------------------------------------

void CoeffBlenderBlock::BeginBlock(const KeyContext& keyContext,
                                   PaintParamsKeyBuilder* builder,
                                   PipelineDataGatherer* gatherer,
                                   SkSpan<const float> coeffs) {
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kCoeffBlender)
        SkASSERT(coeffs.size() == 4);
        gatherer->write(SkSLType::kHalf4, coeffs.data());
    }

    builder->beginBlock(BuiltInCodeSnippetID::kCoeffBlender);
}

//--------------------------------------------------------------------------------------------------

void PrimitiveColorBlock::BeginBlock(const KeyContext& keyContext,
                                     PaintParamsKeyBuilder* builder,
                                     PipelineDataGatherer* gatherer) {
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kPrimitiveColor)
    }

    builder->beginBlock(BuiltInCodeSnippetID::kPrimitiveColor);
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
              BlendModeBlenderBlock::BeginBlock(keyContext, builder, gatherer, bm);
              builder->endBlock();
          },
          /* addSrcToKey= */ [&]() -> void {
              SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, srcColor);
              builder->endBlock();
          },
          /* addDstToKey= */ [&]() -> void {
              PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
              builder->endBlock();
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

        gather_runtime_effect_uniforms(shaderData.fEffect->uniforms(),
                                       entry->fUniforms,
                                       shaderData.fUniforms.get(),
                                       gatherer);
    }

    builder->beginBlock(codeSnippetID);
}

// ==================================================================

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkBlendModeBlender* blender) {
    SkASSERT(blender);

    AddModeBlend(keyContext, builder, gatherer, blender->mode());
}

static void add_children_to_key(const KeyContext& keyContext,
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
                    SolidColorShaderBlock::BeginBlock(
                            childContext, builder, gatherer, {0, 0, 0, 0});
                    builder->endBlock();
                    break;

                case ChildType::kColorFilter:
                    // A "passthrough" color filter returns the input color as-is.
                    PriorOutputBlock::BeginBlock(childContext, builder, gatherer);
                    builder->endBlock();
                    break;

                case ChildType::kBlender:
                    // A "passthrough" blender performs `blend_src_over(src, dest)`.
                    BlendModeBlenderBlock::BeginBlock(
                            childContext, builder, gatherer, SkBlendMode::kSrcOver);
                    builder->endBlock();
                    break;
            }
        }
    }
}
static void add_to_key(const KeyContext& keyContext,
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

void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkBlender* blender) {
    if (!blender) {
        return;
    }
    switch (as_BB(blender)->type()) {
#define M(type)                                                    \
    case SkBlenderBase::BlenderType::k##type:                      \
        add_to_key(keyContext,                                     \
                   builder,                                        \
                   gatherer,                                       \
                   static_cast<const Sk##type##Blender*>(blender)); \
        return;
        SK_ALL_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

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
    ColorSpaceTransformBlock::ColorSpaceTransformData data(
            filter->src().get(), kAlphaType, filter->dst().get(), kAlphaType);
    ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer, &data);
    builder->endBlock();
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
    GaussianColorFilterBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkMatrixColorFilter* filter) {
    SkASSERT(filter);

    bool inHSLA = filter->domain() == SkMatrixColorFilter::Domain::kHSLA;
    MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(filter->matrix(), inHSLA);

    MatrixColorFilterBlock::BeginBlock(keyContext, builder, gatherer, &matrixCFData);
    builder->endBlock();
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
        PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
        builder->endBlock();
        return;
    }

    TableColorFilterBlock::TableColorFilterData data(std::move(proxy));

    TableColorFilterBlock::BeginBlock(keyContext, builder, gatherer, data);
    builder->endBlock();
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
                            ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer,
                                                                 &data1);
                            builder->endBlock();
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
                ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer, &data2);
                builder->endBlock();
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
        PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
        builder->endBlock();
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

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkCTMShader*) {
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
    // TODO(michaelludwig) implement this when clipShader() is implemented
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorShader* shader) {
    SkASSERT(shader);

    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                      SkColor4f::FromColor(shader->color()).premul());
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColor4Shader* shader) {
    SkASSERT(shader);

    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, shader->color().premul());
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorFilterShader* shader) {
    SkASSERT(shader);

    Compose(keyContext, builder, gatherer,
            /* emitInnerToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->shader().get());
            },
            /* emitOuterToKey= */ [&]() -> void {
                AddToKey(keyContext, builder, gatherer, shader->filter().get());
            });
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkCoordClampShader* shader) {
    SkASSERT(shader);

    CoordClampShaderBlock::CoordClampData data(shader->subset());

    CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
        AddToKey(keyContext, builder, gatherer, shader->shader().get());
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkEmptyShader*) {
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
    builder->endBlock();
}

static void add_yuv_image_to_key(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 const SkImageShader* origShader,
                                 sk_sp<const SkImage> imageToDraw,
                                 SkSamplingOptions sampling) {
    SkASSERT(!imageToDraw->isAlphaOnly());

    const YUVATextureProxies& yuvaProxies =
            static_cast<const Image_YUVA*>(imageToDraw.get())->yuvaProxies();
    const SkYUVAInfo& yuvaInfo = yuvaProxies.yuvaInfo();

    YUVImageShaderBlock::ImageData imgData(sampling, origShader->tileModeX(),
                                           origShader->tileModeY(), origShader->subset());
    imgData.fImgSize = { (float)imageToDraw->width(), (float)imageToDraw->height() };
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        memset(&imgData.fChannelSelect[i], 0, sizeof(SkColor4f));
    }
    int textureCount = 0;
    SkYUVAInfo::YUVALocations yuvaLocations = yuvaProxies.yuvaLocations();
    for (int locIndex = 0; locIndex < SkYUVAInfo::kYUVAChannelCount; ++locIndex) {
        auto [yuvPlane, yuvChannel] = yuvaLocations[locIndex];
        if (yuvPlane >= 0) {
            SkASSERT(locIndex == textureCount);
            TextureProxyView view = yuvaProxies.makeView(yuvPlane);
            imgData.fTextureProxies[locIndex] = view.refProxy();
            imgData.fChannelSelect[locIndex][static_cast<int>(yuvChannel)] = 1.0f;
            ++textureCount;
        }
    }
    SkASSERT(textureCount == 3 || textureCount == 4);
    // If the format has no alpha, we still need to set the proxy to something
    if (textureCount == 3) {
        imgData.fTextureProxies[3] = imgData.fTextureProxies[0];
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

    if (!origShader->isRaw()) {
        imgData.fSteps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                                imageToDraw->alphaType(),
                                                keyContext.dstColorInfo().colorSpace(),
                                                keyContext.dstColorInfo().alphaType());
    }

    // The YUV formats can encode their own origin including reflection and rotation,
    // so we need to wrap our block in an additional local matrix transform.
    SkMatrix originMatrix = yuvaInfo.originMatrix();
    LocalMatrixShaderBlock::LMShaderData lmShaderData(originMatrix);

    KeyContextWithLocalMatrix newContext(keyContext, originMatrix);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);

        YUVImageShaderBlock::BeginBlock(newContext, builder, gatherer, &imgData);
        builder->endBlock();

    builder->endBlock();
}

static skgpu::graphite::ReadSwizzle swizzle_class_to_read_enum(const skgpu::Swizzle& swizzle) {
    if (swizzle == skgpu::Swizzle::RGBA()) {
        return skgpu::graphite::ReadSwizzle::kRGBA;
    } else if (swizzle == skgpu::Swizzle::RGB1()) {
        return skgpu::graphite::ReadSwizzle::kRGB1;
    } else if (swizzle == skgpu::Swizzle("rrrr")) {
        return skgpu::graphite::ReadSwizzle::kRRRR;
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
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                          kErrorColor);
        builder->endBlock();
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

    skgpu::Mipmapped mipmapped = (newSampling.mipmap != SkMipmapMode::kNone)
                                     ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    auto [view, _] = AsView(keyContext.recorder(), imageToDraw.get(), mipmapped);

    ImageShaderBlock::ImageData imgData(shader->sampling(), shader->tileModeX(),
                                        shader->tileModeY(), shader->subset(),
                                        ReadSwizzle::kRGBA);
    imgData.fSampling = newSampling;
    imgData.fTextureProxy = view.refProxy();
    skgpu::Swizzle readSwizzle = view.swizzle();
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (imageToDraw->isAlphaOnly()) {
        readSwizzle = skgpu::Swizzle::Concat(readSwizzle, skgpu::Swizzle("000a"));
    }
    imgData.fReadSwizzle = swizzle_class_to_read_enum(readSwizzle);

    auto addImageSampling = [&]() -> void {
        if (imgData.fSampling.useCubic) {
            ImageShaderBlock::BeginCubicBlock(keyContext, builder, gatherer, &imgData);
        } else {
            ImageShaderBlock::BeginBlock(keyContext, builder, gatherer, &imgData);
        }
        builder->endBlock();
    };

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
                  /* addSrcToKey= */ addImageSampling,
                  /* addDstToKey= */ [&]() -> void {
                      SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                                        keyContext.paintColor());
                      builder->endBlock();
                  });
            return;
        }
    }

    addImageSampling();
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
    if (!matrix.isIdentity()) {

        LocalMatrixShaderBlock::LMShaderData lmShaderData(matrix);

        KeyContextWithLocalMatrix newContext(keyContext, matrix);

        LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);

        AddToKey(newContext, builder, gatherer, wrappedShader);

        builder->endBlock();
    } else  {
        AddToKey(keyContext, builder, gatherer, wrappedShader);
    }
}

// If either of these change then the corresponding change must also be made in the SkSL
// perlin_noise_shader function.
static_assert((int)SkPerlinNoiseShader::kFractalNoise_Type ==
              (int)PerlinNoiseShaderBlock::Type::kFractalNoise);
static_assert((int)SkPerlinNoiseShader::kTurbulence_Type ==
              (int)PerlinNoiseShaderBlock::Type::kTurbulence);
static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkPerlinNoiseShader* shader) {
    SkASSERT(shader);
    SkASSERT(shader->numOctaves());

    SkMatrix totalMatrix = keyContext.local2Dev().asM33();
    if (keyContext.localMatrix()) {
        totalMatrix.preConcat(*keyContext.localMatrix());
    }

    SkMatrix invTotal;
    bool result = totalMatrix.invert(&invTotal);
    if (!result) {
        SKGPU_LOG_W("Couldn't invert totalMatrix for PerlinNoiseShader");

        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        builder->endBlock();
        return;
    }

    std::unique_ptr<SkPerlinNoiseShader::PaintingData> paintingData =
        shader->getPaintingData(totalMatrix);
    paintingData->generateBitmaps();

    sk_sp<TextureProxy> perm = RecorderPriv::CreateCachedProxy(
            keyContext.recorder(), paintingData->getPermutationsBitmap());

    sk_sp<TextureProxy> noise =
            RecorderPriv::CreateCachedProxy(keyContext.recorder(), paintingData->getNoiseBitmap());

    if (!perm || !noise) {
        SKGPU_LOG_W("Couldn't create tables for PerlinNoiseShader");

        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        builder->endBlock();
        return;
    }

    PerlinNoiseShaderBlock::PerlinNoiseData data(
            static_cast<PerlinNoiseShaderBlock::Type>(shader->noiseType()),
            paintingData->fBaseFrequency,
            shader->numOctaves(),
            {paintingData->fStitchDataInit.fWidth, paintingData->fStitchDataInit.fHeight});

    data.fPermutationsProxy = std::move(perm);
    data.fNoiseProxy = std::move(noise);

    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). Remember: this matrix (shader2World) is going to be
    // inverted before being applied.
    SkMatrix shader2Local =
            SkMatrix::Translate(-1 + totalMatrix.getTranslateX(), -1 + totalMatrix.getTranslateY());
    shader2Local.postConcat(invTotal);

    LocalMatrixShaderBlock::LMShaderData lmShaderData(shader2Local);

    KeyContextWithLocalMatrix newContext(keyContext, shader2Local);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);
        PerlinNoiseShaderBlock::BeginBlock(newContext, builder, gatherer, &data);
        builder->endBlock();
    builder->endBlock();

}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkPictureShader* shader) {
    SkASSERT(shader);

    Recorder* recorder = keyContext.recorder();
    const Caps* caps = recorder->priv().caps();

    // TODO: We'll need additional plumbing to get the correct props from our callers. In
    // particular we'll need to expand the keyContext to have the surfaceProps, the dstColorType
    // and dstColorSpace.
    SkSurfaceProps props{};

    SkMatrix totalM = keyContext.local2Dev().asM33();
    if (keyContext.localMatrix()) {
        totalM.preConcat(*keyContext.localMatrix());
    }
    auto info = SkPictureShader::CachedImageInfo::Make(shader->tile(),
                                                       totalM,
                                                       /* dstColorType= */ kRGBA_8888_SkColorType,
                                                       /* dstColorSpace= */ nullptr,
                                                       caps->maxTextureSize(),
                                                       props);
    if (!info.success) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        builder->endBlock();
        return;
    }

    // TODO: right now we're explicitly not caching here. We could expand the ImageProvider
    // API to include already Graphite-backed images, add a Recorder-local cache or add
    // rendered-picture images to the global cache.
    sk_sp<SkImage> img = info.makeImage(
            SkSurfaces::RenderTarget(recorder, info.imageInfo, skgpu::Mipmapped::kNo, &info.props),
            shader->picture().get());
    if (!img) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        builder->endBlock();
        return;
    }

    const auto shaderLM = SkMatrix::Scale(1.f/info.tileScale.width(), 1.f/info.tileScale.height());
    sk_sp<SkShader> imgShader = img->makeShader(shader->tileModeX(), shader->tileModeY(),
                                                SkSamplingOptions(shader->filter()), &shaderLM);
    if (!imgShader) {
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        builder->endBlock();
        return;
    }

    AddToKey(keyContext, builder, gatherer, imgShader.get());
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

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTransformShader* shader) {
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTriColorShader* shader) {
    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
    builder->endBlock();
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
            ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer, &data);
            builder->endBlock();
        });
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
        case ColorSpace::kLCH:
        case ColorSpace::kOKLCH:
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
    // (via the compose block) so that the localMatrix block can treat them as
    // one child.
    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                GradientShaderBlocks::BeginBlock(keyContext, builder, gatherer, gradData);
                builder->endBlock();
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer, &data);
                builder->endBlock();
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

    sk_sp<TextureProxy> proxy;

    if (shader->getColorCount() > GradientShaderBlocks::GradientData::kNumInternalStorageStops) {
        if (shader->cachedBitmap().empty()) {
            SkBitmap colorsAndOffsetsBitmap = create_color_and_offset_bitmap(
                    shader->getColorCount(), colors, shader->getPositions());
            if (colorsAndOffsetsBitmap.empty()) {
                SKGPU_LOG_W("Couldn't create GradientShader's color and offset bitmap");

                SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
                builder->endBlock();
                return;
            }
            shader->setCachedBitmap(colorsAndOffsetsBitmap);
        }

        proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), shader->cachedBitmap());
        if (!proxy) {
            SKGPU_LOG_W("Couldn't create GradientShader's color and offset bitmap proxy");

            SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
            builder->endBlock();
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
                                            shader->getColorCount(),
                                            colors,
                                            shader->getPositions(),
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


void AddToKey(const KeyContext& keyContext,
              PaintParamsKeyBuilder* builder,
              PipelineDataGatherer* gatherer,
              const SkShader* shader) {
    if (!shader) {
        return;
    }
    switch (as_SB(shader)->type()) {
#define M(type)                                                        \
    case SkShaderBase::ShaderType::k##type:                             \
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


} // namespace skgpu::graphite
