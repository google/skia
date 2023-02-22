/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/KeyHelpers.h"

#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ReadWriteSwizzle.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/shaders/SkImageShader.h"

constexpr SkPMColor4f kErrorColor = { 1, 0, 0, 1 };

#define VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID) \
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, dict->getUniforms(codeSnippetID));)

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------

void PassthroughShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kPassthroughShader);
}

//--------------------------------------------------------------------------------------------------

void PassthroughBlenderBlock::BeginBlock(const KeyContext& keyContext,
                                         PaintParamsKeyBuilder* builder,
                                         PipelineDataGatherer* gatherer) {
    builder->beginBlock(BuiltInCodeSnippetID::kPassthroughBlender);
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
// All the gradients share a common postamble of:
//   tilemode
//   colorSpace
//   doUnPremul
void add_gradient_postamble(const GradientShaderBlocks::GradientData& gradData,
                            PipelineDataGatherer* gatherer) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;

    static_assert(static_cast<int>(ColorSpace::kLab)   == 2);
    static_assert(static_cast<int>(ColorSpace::kOKLab) == 3);
    static_assert(static_cast<int>(ColorSpace::kLCH)   == 4);
    static_assert(static_cast<int>(ColorSpace::kOKLCH) == 5);
    static_assert(static_cast<int>(ColorSpace::kHSL)   == 7);
    static_assert(static_cast<int>(ColorSpace::kHWB)   == 8);

    bool inputPremul = static_cast<bool>(gradData.fInterpolation.fInPremul);

    gatherer->write(static_cast<int>(gradData.fTM));
    gatherer->write(static_cast<int>(gradData.fInterpolation.fColorSpace));
    gatherer->write(static_cast<int>(inputPremul));
}

void add_linear_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                      BuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)
    size_t stops = codeSnippetID == BuiltInCodeSnippetID::kLinearGradientShader4 ? 4 : 8;

    gatherer->writeArray({gradData.fColors, stops});
    gatherer->writeArray({gradData.fOffsets, stops});
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
    size_t stops = codeSnippetID == BuiltInCodeSnippetID::kRadialGradientShader4 ? 4 : 8;

    gatherer->writeArray({gradData.fColors, stops});
    gatherer->writeArray({gradData.fOffsets, stops});
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
    size_t stops = codeSnippetID == BuiltInCodeSnippetID::kSweepGradientShader4 ? 4 : 8;

    gatherer->writeArray({gradData.fColors, stops});
    gatherer->writeArray({gradData.fOffsets, stops});
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
    size_t stops = codeSnippetID == BuiltInCodeSnippetID::kConicalGradientShader4 ? 4 : 8;

    gatherer->writeArray({gradData.fColors, stops});
    gatherer->writeArray({gradData.fOffsets, stops});
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
                                                 const SkGradientShader::Interpolation& interp)
        : fType(type)
        , fBias(bias)
        , fScale(scale)
        , fTM(tm)
        , fNumStops(std::min(numStops, kMaxStops))
        , fInterpolation(interp) {
    SkASSERT(fNumStops >= 1);

    fPoints[0] = point0;
    fPoints[1] = point1;
    fRadii[0] = radius0;
    fRadii[1] = radius1;
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
    for (int i = fNumStops ; i < kMaxStops; ++i) {
        fColors[i] = fColors[fNumStops-1];
        fOffsets[i] = fOffsets[fNumStops-1];
    }
}

void GradientShaderBlocks::BeginBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder *builder,
                                      PipelineDataGatherer* gatherer,
                                      const GradientData& gradData) {
    auto dict = keyContext.dict();
    BuiltInCodeSnippetID codeSnippetID = BuiltInCodeSnippetID::kSolidColorShader;
    switch (gradData.fType) {
        case SkShaderBase::GradientType::kLinear:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? BuiltInCodeSnippetID::kLinearGradientShader4
                                    : BuiltInCodeSnippetID::kLinearGradientShader8;
            if (gatherer) {
                add_linear_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kRadial:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? BuiltInCodeSnippetID::kRadialGradientShader4
                                    : BuiltInCodeSnippetID::kRadialGradientShader8;
            if (gatherer) {
                add_radial_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kSweep:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? BuiltInCodeSnippetID::kSweepGradientShader4
                                    : BuiltInCodeSnippetID::kSweepGradientShader8;
            if (gatherer) {
                add_sweep_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShaderBase::GradientType::kConical:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? BuiltInCodeSnippetID::kConicalGradientShader4
                                    : BuiltInCodeSnippetID::kConicalGradientShader8;
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

    gatherer->write(SkTo<int>(steps.flags.mask()));
    gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.srcTF)));
    gatherer->write(SkTo<int>(skcms_TransferFunction_getType(&steps.dstTFInv)));
    gatherer->writeHalfArray({&steps.srcTF.g, kNumXferFnCoeffs});
    gatherer->writeHalfArray({&steps.dstTFInv.g, kNumXferFnCoeffs});

    SkMatrix gamutTransform;
    gamutTransform.set9(steps.src_to_dst_matrix);
    gatherer->writeHalf(gamutTransform);
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

void PorterDuffBlendShaderBlock::BeginBlock(const KeyContext& keyContext,
                                            PaintParamsKeyBuilder* builder,
                                            PipelineDataGatherer* gatherer,
                                            const PorterDuffBlendShaderData& blendData) {
    auto dict = keyContext.dict();
    // When extracted into ShaderInfo::SnippetEntries the children will appear after their
    // parent. Thus, the parent's uniform data must appear in the uniform block before the
    // uniform data of the children.
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kPorterDuffBlendShader)
        SkASSERT(blendData.fPorterDuffConstants.size() == 4);
        gatherer->write(SkSLType::kHalf4, blendData.fPorterDuffConstants.data());
        gatherer->addFlags(
                dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kPorterDuffBlendShader));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kPorterDuffBlendShader);
}

//--------------------------------------------------------------------------------------------------

void BlendShaderBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer,
                                  const BlendShaderData& blendData) {
    auto dict = keyContext.dict();
    // When extracted into ShaderInfo::SnippetEntries the children will appear after their
    // parent. Thus, the parent's uniform data must appear in the uniform block before the
    // uniform data of the children.
    if (gatherer) {
        VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kBlendShader)
        gatherer->write(SkTo<int>(blendData.fBM));

        gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kBlendShader));
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendShader);
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

namespace {

void add_blend_colorfilter_uniform_data(const ShaderCodeDictionary* dict,
                                        const BlendColorFilterBlock::BlendColorFilterData& data,
                                        PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kBlendColorFilter)
    gatherer->write(SkTo<int>(data.fBlendMode));
    gatherer->write(data.fSrcColor);

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kBlendColorFilter));
}

} // anonymous namespace

void BlendColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const BlendColorFilterData* data) {
    auto dict = keyContext.dict();

    if (gatherer) {
        add_blend_colorfilter_uniform_data(dict, *data, gatherer);
    }

    builder->beginBlock(BuiltInCodeSnippetID::kBlendColorFilter);
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

    gatherer->addFlags(dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kTableColorFilter));
}

} // anonymous namespace

TableColorFilterBlock::TableColorFilterData::TableColorFilterData() {}

void TableColorFilterBlock::BeginBlock(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       const TableColorFilterData& data) {
    auto dict = keyContext.dict();

    if (gatherer) {
        if (!data.fTextureProxy) {
            // We're dropping the color filter here!
            PassthroughShaderBlock::BeginBlock(keyContext, builder, gatherer);
            return;
        }

        static const SkTileMode kTileModes[2] = { SkTileMode::kClamp, SkTileMode::kClamp };
        gatherer->add(SkSamplingOptions(), kTileModes, data.fTextureProxy);

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
namespace {

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
/*>> No coverage, input color unknown <<*/
static constexpr skgpu::BlendInfo gBlendTable[kNumCoeffModes] = {
        /* clear */      make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
        /* src */        make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
        /* dst */        make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
        /* src-over */   make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
        /* dst-over */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
        /* src-in */     make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
        /* dst-in */     make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
        /* src-out */    make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
        /* dst-out */    make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
        /* src-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
        /* dst-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
        /* xor */        make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
        /* plus */       make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
        /* modulate */   make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC)
};

const skgpu::BlendInfo& get_blend_info(SkBlendMode bm) {
    if (bm <= SkBlendMode::kLastCoeffMode) {
        return gBlendTable[(int) bm];
    }

    return gBlendTable[(int) SkBlendMode::kSrc];
}

void add_shaderbasedblender_uniform_data(const ShaderCodeDictionary* dict,
                                         SkBlendMode bm,
                                         PipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kShaderBasedBlender)
    gatherer->write(SkTo<int>(bm));

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(BuiltInCodeSnippetID::kShaderBasedBlender));
}

} // anonymous namespace

void BlendModeBlock::BeginBlock(const KeyContext& keyContext,
                                PaintParamsKeyBuilder *builder,
                                PipelineDataGatherer* gatherer,
                                SkBlendMode bm) {

    auto dict = keyContext.dict();

    if (bm <= SkBlendMode::kLastCoeffMode) {
        builder->setBlendInfo(get_blend_info(bm));

        builder->beginBlock(BuiltInCodeSnippetID::kFixedFunctionBlender);
        static_assert(SkTFitsIn<uint8_t>(SkBlendMode::kLastMode));
        builder->addByte(static_cast<uint8_t>(bm));
    } else {
        // TODO: set up the correct blend info
        builder->setBlendInfo({});

        if (gatherer) {
            add_shaderbasedblender_uniform_data(dict, bm, gatherer);
        }

        builder->beginBlock(BuiltInCodeSnippetID::kShaderBasedBlender);
    }
}

void PrimitiveBlendModeBlock::BeginBlock(const KeyContext& keyContext,
                                         PaintParamsKeyBuilder *builder,
                                         PipelineDataGatherer* gatherer,
                                         SkBlendMode bm) {
    auto dict = keyContext.dict();
    // Unlike in the usual blendmode case, the primitive blend mode will always be implemented
    // via shader-based blending.
    if (gatherer) {
        add_shaderbasedblender_uniform_data(dict, bm, gatherer);
    }
    builder->beginBlock(BuiltInCodeSnippetID::kPrimitiveColorShaderBasedBlender);
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
