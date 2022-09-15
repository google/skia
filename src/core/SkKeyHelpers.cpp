/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "include/core/SkData.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkRuntimeEffectDictionary.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"

constexpr SkPMColor4f kErrorColor = { 1, 0, 0, 1 };
#endif

#define VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID) \
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, dict->getUniforms(codeSnippetID));)

//--------------------------------------------------------------------------------------------------

void PassthroughShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                        SkPaintParamsKeyBuilder* builder,
                                        SkPipelineDataGatherer* gatherer) {
#ifdef SK_GRAPHITE_ENABLED
    builder->beginBlock(SkBuiltInCodeSnippetID::kPassthroughShader);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

void PassthroughBlenderBlock::BeginBlock(const SkKeyContext& keyContext,
                                         SkPaintParamsKeyBuilder* builder,
                                         SkPipelineDataGatherer* gatherer) {
#ifdef SK_GRAPHITE_ENABLED
    builder->beginBlock(SkBuiltInCodeSnippetID::kPassthroughBlender);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_solid_uniform_data(const SkShaderCodeDictionary* dict,
                            const SkPMColor4f& premulColor,
                            SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kSolidColorShader)
    gatherer->write(premulColor);

    gatherer->addFlags(dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kSolidColorShader));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void SolidColorShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                       SkPaintParamsKeyBuilder* builder,
                                       SkPipelineDataGatherer* gatherer,
                                       const SkPMColor4f& premulColor) {
#ifdef SK_GRAPHITE_ENABLED
    if (gatherer) {
        auto dict = keyContext.dict();

        add_solid_uniform_data(dict, premulColor, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kSolidColorShader);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_linear_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                      SkBuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)
    int stops = codeSnippetID == SkBuiltInCodeSnippetID::kLinearGradientShader4 ? 4 : 8;

    SkM44 lmInverse;
    bool wasInverted = gradData.fLocalMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);
    gatherer->write(gradData.fColor4fs, stops);
    gatherer->write(gradData.fOffsets, stops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    gatherer->write(static_cast<int>(gradData.fTM));

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_radial_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                      SkBuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)
    int stops = codeSnippetID == SkBuiltInCodeSnippetID::kRadialGradientShader4 ? 4 : 8;

    SkM44 lmInverse;
    bool wasInverted = gradData.fLocalMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);
    gatherer->write(gradData.fColor4fs, stops);
    gatherer->write(gradData.fOffsets, stops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(static_cast<int>(gradData.fTM));

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_sweep_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                     SkBuiltInCodeSnippetID codeSnippetID,
                                     const GradientShaderBlocks::GradientData& gradData,
                                     SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)
    int stops = codeSnippetID == SkBuiltInCodeSnippetID::kSweepGradientShader4 ? 4 : 8;

    SkM44 lmInverse;
    bool wasInverted = gradData.fLocalMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);
    gatherer->write(gradData.fColor4fs, stops);
    gatherer->write(gradData.fOffsets, stops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fBias);
    gatherer->write(gradData.fScale);
    gatherer->write(static_cast<int>(gradData.fTM));

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

void add_conical_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                       SkBuiltInCodeSnippetID codeSnippetID,
                                       const GradientShaderBlocks::GradientData& gradData,
                                       SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID)
    int stops = codeSnippetID == SkBuiltInCodeSnippetID::kConicalGradientShader4 ? 4 : 8;

    SkM44 lmInverse;
    bool wasInverted = gradData.fLocalMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);
    gatherer->write(gradData.fColor4fs, stops);
    gatherer->write(gradData.fOffsets, stops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(gradData.fRadii[1]);
    gatherer->write(static_cast<int>(gradData.fTM));

    gatherer->addFlags(dict->getSnippetRequirementFlags(codeSnippetID));
};

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

GradientShaderBlocks::GradientData::GradientData(SkShader::GradientType type,
                                                 int numStops)
        : fType(type)
        , fPoints{{0.0f, 0.0f}, {0.0f, 0.0f}}
        , fRadii{0.0f, 0.0f}
        , fBias(0.0f)
        , fScale(0.0f)
        , fTM(SkTileMode::kClamp)
        , fNumStops(numStops) {
    sk_bzero(fColor4fs, sizeof(fColor4fs));
    sk_bzero(fOffsets, sizeof(fOffsets));
}

GradientShaderBlocks::GradientData::GradientData(SkShader::GradientType type,
                                                 const SkM44& localMatrix,
                                                 SkPoint point0, SkPoint point1,
                                                 float radius0, float radius1,
                                                 float bias, float scale,
                                                 SkTileMode tm,
                                                 int numStops,
                                                 SkColor4f* color4fs,
                                                 float* offsets)
        : fType(type)
        , fLocalMatrix(localMatrix)
        , fBias(bias)
        , fScale(scale)
        , fTM(tm)
        , fNumStops(std::min(numStops, kMaxStops)) {
    SkASSERT(fNumStops >= 1);

    fPoints[0] = point0;
    fPoints[1] = point1;
    fRadii[0] = radius0;
    fRadii[1] = radius1;
    memcpy(fColor4fs, color4fs, fNumStops * sizeof(SkColor4f));
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
        fColor4fs[i] = fColor4fs[fNumStops-1];
        fOffsets[i] = fOffsets[fNumStops-1];
    }
}

void GradientShaderBlocks::BeginBlock(const SkKeyContext& keyContext,
                                      SkPaintParamsKeyBuilder *builder,
                                      SkPipelineDataGatherer* gatherer,
                                      const GradientData& gradData) {
#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();
    SkBuiltInCodeSnippetID codeSnippetID = SkBuiltInCodeSnippetID::kSolidColorShader;
    switch (gradData.fType) {
        case SkShader::kLinear_GradientType:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? SkBuiltInCodeSnippetID::kLinearGradientShader4
                                    : SkBuiltInCodeSnippetID::kLinearGradientShader8;
            if (gatherer) {
                add_linear_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShader::kRadial_GradientType:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? SkBuiltInCodeSnippetID::kRadialGradientShader4
                                    : SkBuiltInCodeSnippetID::kRadialGradientShader8;
            if (gatherer) {
                add_radial_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShader::kSweep_GradientType:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? SkBuiltInCodeSnippetID::kSweepGradientShader4
                                    : SkBuiltInCodeSnippetID::kSweepGradientShader8;
            if (gatherer) {
                add_sweep_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShader::GradientType::kConical_GradientType:
            codeSnippetID = gradData.fNumStops <= 4
                                    ? SkBuiltInCodeSnippetID::kConicalGradientShader4
                                    : SkBuiltInCodeSnippetID::kConicalGradientShader8;
            if (gatherer) {
                add_conical_gradient_uniform_data(dict, codeSnippetID, gradData, gatherer);
            }
            break;
        case SkShader::GradientType::kColor_GradientType:
        case SkShader::GradientType::kNone_GradientType:
        default:
            SkASSERT(0);
            break;
    }

    builder->beginBlock(codeSnippetID);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_localmatrixshader_uniform_data(const SkShaderCodeDictionary* dict,
                                        const SkM44& localMatrix,
                                        SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kLocalMatrixShader)

    SkM44 lmInverse;
    bool wasInverted = localMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(lmInverse);

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kLocalMatrixShader));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void LocalMatrixShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                        SkPaintParamsKeyBuilder* builder,
                                        SkPipelineDataGatherer* gatherer,
                                        const LMShaderData& lmShaderData) {

#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();
    // When extracted into SkShaderInfo::SnippetEntries the children will appear after their
    // parent. Thus, the parent's uniform data must appear in the uniform block before the
    // uniform data of the children.
    if (gatherer) {
        add_localmatrixshader_uniform_data(dict, lmShaderData.fLocalMatrix, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kLocalMatrixShader);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_image_uniform_data(const SkShaderCodeDictionary* dict,
                            const ImageShaderBlock::ImageData& imgData,
                            SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kImageShader)

    SkMatrix lmInverse;
    bool wasInverted = imgData.fLocalMatrix.invert(&lmInverse);  // TODO: handle failure up stack
    if (!wasInverted) {
        lmInverse.setIdentity();
    }

    gatherer->write(SkM44(lmInverse));
    gatherer->write(imgData.fSubset);
    gatherer->write(static_cast<int>(imgData.fTileModes[0]));
    gatherer->write(static_cast<int>(imgData.fTileModes[1]));
    gatherer->write(imgData.fTextureProxy->dimensions().fWidth);
    gatherer->write(imgData.fTextureProxy->dimensions().fHeight);

    gatherer->addFlags(dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kImageShader));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

ImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                       SkTileMode tileModeX,
                                       SkTileMode tileModeY,
                                       SkRect subset,
                                       const SkMatrix& localMatrix)
    : fSampling(sampling)
    , fTileModes{tileModeX, tileModeY}
    , fSubset(subset)
    , fLocalMatrix(localMatrix) {
}

void ImageShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                  SkPaintParamsKeyBuilder* builder,
                                  SkPipelineDataGatherer* gatherer,
                                  const ImageData& imgData) {

#ifdef SK_GRAPHITE_ENABLED
    // TODO: allow through lazy proxies
    if (gatherer && !imgData.fTextureProxy) {
        // TODO: At some point the pre-compile path should also be creating a texture
        // proxy (i.e., we can remove the 'pipelineData' in the above test).
        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, kErrorColor);
        return;
    }

    auto dict = keyContext.dict();
    if (gatherer) {
        gatherer->add(imgData.fSampling,
                      imgData.fTileModes,
                      imgData.fTextureProxy);

        add_image_uniform_data(dict, imgData, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kImageShader);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_blendshader_uniform_data(const SkShaderCodeDictionary* dict,
                                  SkBlendMode bm,
                                  SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kBlendShader)
    gatherer->write(SkTo<int>(bm));

    gatherer->addFlags(dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kBlendShader));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void BlendShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                  SkPaintParamsKeyBuilder *builder,
                                  SkPipelineDataGatherer* gatherer,
                                  const BlendShaderData& blendData) {

#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();
    // When extracted into SkShaderInfo::SnippetEntries the children will appear after their
    // parent. Thus, the parent's uniform data must appear in the uniform block before the
    // uniform data of the children.
    if (gatherer) {
        add_blendshader_uniform_data(dict, blendData.fBM, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kBlendShader);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_matrix_colorfilter_uniform_data(const SkShaderCodeDictionary* dict,
                                         const MatrixColorFilterBlock::MatrixColorFilterData& data,
                                         SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kMatrixColorFilter)
    gatherer->write(data.fMatrix);
    gatherer->write(data.fTranslate);
    gatherer->write(static_cast<int>(data.fInHSLA));

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kMatrixColorFilter));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void MatrixColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                        SkPaintParamsKeyBuilder* builder,
                                        SkPipelineDataGatherer* gatherer,
                                        const MatrixColorFilterData& matrixCFData) {
#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();

    if (gatherer) {
        add_matrix_colorfilter_uniform_data(dict, matrixCFData, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kMatrixColorFilter);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------

#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_blend_colorfilter_uniform_data(const SkShaderCodeDictionary* dict,
                                        const BlendColorFilterBlock::BlendColorFilterData& data,
                                        SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kBlendColorFilter)
    gatherer->write(SkTo<int>(data.fBlendMode));
    gatherer->write(data.fSrcColor);

    gatherer->addFlags(dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kBlendColorFilter));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void BlendColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                       SkPaintParamsKeyBuilder* builder,
                                       SkPipelineDataGatherer* gatherer,
                                       const BlendColorFilterData& data) {
#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();

    if (gatherer) {
        add_blend_colorfilter_uniform_data(dict, data, gatherer);
    }

    builder->beginBlock(SkBuiltInCodeSnippetID::kBlendColorFilter);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------
void ComposeColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                         SkPaintParamsKeyBuilder* builder,
                                         SkPipelineDataGatherer* gatherer) {
#ifdef SK_GRAPHITE_ENABLED
    builder->beginBlock(SkBuiltInCodeSnippetID::kComposeColorFilter);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------
void GaussianColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                          SkPaintParamsKeyBuilder* builder,
                                          SkPipelineDataGatherer* gatherer) {
#ifdef SK_GRAPHITE_ENABLED
    builder->beginBlock(SkBuiltInCodeSnippetID::kGaussianColorFilter);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED

namespace {

void add_table_colorfilter_uniform_data(const SkShaderCodeDictionary* dict,
                                        const TableColorFilterBlock::TableColorFilterData& data,
                                        SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kTableColorFilter)

    gatherer->addFlags(dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kTableColorFilter));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

TableColorFilterBlock::TableColorFilterData::TableColorFilterData() {}

void TableColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                       SkPaintParamsKeyBuilder* builder,
                                       SkPipelineDataGatherer* gatherer,
                                       const TableColorFilterData& data) {
#ifdef SK_GRAPHITE_ENABLED
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

    builder->beginBlock(SkBuiltInCodeSnippetID::kTableColorFilter);
#endif // SK_GRAPHITE_ENABLED
}

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
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

void add_shaderbasedblender_uniform_data(const SkShaderCodeDictionary* dict,
                                         SkBlendMode bm,
                                         SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, SkBuiltInCodeSnippetID::kShaderBasedBlender)
    gatherer->write(SkTo<int>(bm));

    gatherer->addFlags(
            dict->getSnippetRequirementFlags(SkBuiltInCodeSnippetID::kShaderBasedBlender));
}

} // anonymous namespace

#endif // SK_GRAPHITE_ENABLED

void BlendModeBlock::BeginBlock(const SkKeyContext& keyContext,
                                SkPaintParamsKeyBuilder *builder,
                                SkPipelineDataGatherer* gatherer,
                                SkBlendMode bm) {

#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();

    if (bm <= SkBlendMode::kLastCoeffMode) {
        builder->setBlendInfo(get_blend_info(bm));

        builder->beginBlock(SkBuiltInCodeSnippetID::kFixedFunctionBlender);
        static_assert(SkTFitsIn<uint8_t>(SkBlendMode::kLastMode));
        builder->addByte(static_cast<uint8_t>(bm));
    } else {
        // TODO: set up the correct blend info
        builder->setBlendInfo({});

        if (gatherer) {
            add_shaderbasedblender_uniform_data(dict, bm, gatherer);
        }

        builder->beginBlock(SkBuiltInCodeSnippetID::kShaderBasedBlender);
    }
#endif // SK_GRAPHITE_ENABLED
}

void PrimitiveBlendModeBlock::BeginBlock(const SkKeyContext& keyContext,
                                         SkPaintParamsKeyBuilder *builder,
                                         SkPipelineDataGatherer* gatherer,
                                         SkBlendMode pbm) {

#ifdef SK_GRAPHITE_ENABLED
    auto dict = keyContext.dict();
    // Unlike in the usual blendmode case, the primitive blend mode will always be implemented
    // via shader-based blending.
    if (gatherer) {
        add_shaderbasedblender_uniform_data(dict, pbm, gatherer);
    }
    builder->beginBlock(SkBuiltInCodeSnippetID::kShaderBasedBlender);
#endif // SK_GRAPHITE_ENABLED
}

RuntimeShaderBlock::ShaderData::ShaderData(sk_sp<const SkRuntimeEffect> effect)
        : fEffect(std::move(effect)) {}

RuntimeShaderBlock::ShaderData::ShaderData(sk_sp<const SkRuntimeEffect> effect,
                                           const SkMatrix& localMatrix,
                                           sk_sp<const SkData> uniforms)
        : fEffect(std::move(effect))
        , fLocalMatrix(localMatrix)
        , fUniforms(std::move(uniforms)) {}

static bool skdata_matches(const SkData* a, const SkData* b) {
    // Returns true if both SkData objects hold the same contents, or if they are both null.
    // (SkData::equals supports passing null, and returns false.)
    return a ? a->equals(b) : (a == b);
}

bool RuntimeShaderBlock::ShaderData::operator==(const ShaderData& rhs) const {
    return fEffect == rhs.fEffect &&
           fLocalMatrix == rhs.fLocalMatrix &&
           skdata_matches(fUniforms.get(), rhs.fUniforms.get());
}

#ifdef SK_GRAPHITE_ENABLED
static void add_effect_to_recorder(skgpu::graphite::Recorder* recorder,
                                   int codeSnippetID,
                                   sk_sp<const SkRuntimeEffect> effect) {
    recorder->priv().runtimeEffectDictionary()->set(codeSnippetID, std::move(effect));
}

static void gather_runtime_effect_uniforms(SkSpan<const SkRuntimeEffect::Uniform> rtsUniforms,
                                           SkSpan<const SkUniform> graphiteUniforms,
                                           int graphiteStartingIndex,
                                           const SkData* uniformData,
                                           SkPipelineDataGatherer* gatherer) {
    // Collect all the other uniforms from the provided SkData.
    const uint8_t* uniformBase = uniformData->bytes();
    for (size_t index = 0; index < rtsUniforms.size(); ++index) {
        // The runtime shader SkShaderSnippet burns index 0 on the local matrix, so adjust our index
        // to compensate. (Color filters and blenders don't need any adjustment and pass zero.)
        int graphiteIndex = index + graphiteStartingIndex;
        const SkUniform& skUniform = graphiteUniforms[graphiteIndex];
        // Get a pointer to the offset in our data for this uniform.
        const uint8_t* uniformPtr = uniformBase + rtsUniforms[index].offset;
        // Pass the uniform data to the gatherer.
        gatherer->write(skUniform.type(), skUniform.count(), uniformPtr);
    }
}
#endif

void RuntimeShaderBlock::BeginBlock(const SkKeyContext& keyContext,
                                    SkPaintParamsKeyBuilder* builder,
                                    SkPipelineDataGatherer* gatherer,
                                    const ShaderData& shaderData) {
#ifdef SK_GRAPHITE_ENABLED
    SkShaderCodeDictionary* dict = keyContext.dict();
    int codeSnippetID = dict->findOrCreateRuntimeEffectSnippet(shaderData.fEffect.get());

    add_effect_to_recorder(keyContext.recorder(), codeSnippetID, shaderData.fEffect);

    if (gatherer) {
        const SkShaderSnippet* entry = dict->getEntry(codeSnippetID);
        SkASSERT(entry);

        SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, entry->fUniforms);)
        gatherer->addFlags(entry->fSnippetRequirementFlags);

        // Pass the local matrix inverse so we can use local coordinates.
        SkMatrix inverseLocalMatrix;
        if (!shaderData.fLocalMatrix.invert(&inverseLocalMatrix)) {
            inverseLocalMatrix.setIdentity();
        }
        gatherer->write(SkM44(inverseLocalMatrix));

        gather_runtime_effect_uniforms(shaderData.fEffect->uniforms(),
                                       entry->fUniforms,
                                       /*graphiteStartingIndex=*/1,
                                       shaderData.fUniforms.get(),
                                       gatherer);
    }

    builder->beginBlock(codeSnippetID);
#endif  // SK_GRAPHITE_ENABLED
}

RuntimeColorFilterBlock::ColorFilterData::ColorFilterData(sk_sp<const SkRuntimeEffect> effect)
        : fEffect(std::move(effect)) {}

RuntimeColorFilterBlock::ColorFilterData::ColorFilterData(sk_sp<const SkRuntimeEffect> effect,
                                                          sk_sp<const SkData> uniforms)
        : fEffect(std::move(effect))
        , fUniforms(std::move(uniforms)) {}

bool RuntimeColorFilterBlock::ColorFilterData::operator==(const ColorFilterData& rhs) const {
    return fEffect == rhs.fEffect &&
           skdata_matches(fUniforms.get(), rhs.fUniforms.get());
}

void RuntimeColorFilterBlock::BeginBlock(const SkKeyContext& keyContext,
                                         SkPaintParamsKeyBuilder* builder,
                                         SkPipelineDataGatherer* gatherer,
                                         const ColorFilterData& filterData) {
#ifdef SK_GRAPHITE_ENABLED
    SkShaderCodeDictionary* dict = keyContext.dict();
    int codeSnippetID = dict->findOrCreateRuntimeEffectSnippet(filterData.fEffect.get());

    add_effect_to_recorder(keyContext.recorder(), codeSnippetID, filterData.fEffect);

    if (gatherer) {
        const SkShaderSnippet* entry = dict->getEntry(codeSnippetID);
        SkASSERT(entry);

        SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, entry->fUniforms);)
        gatherer->addFlags(entry->fSnippetRequirementFlags);

        gather_runtime_effect_uniforms(filterData.fEffect->uniforms(),
                                       entry->fUniforms,
                                       /*graphiteStartingIndex=*/0,
                                       filterData.fUniforms.get(),
                                       gatherer);
    }

    builder->beginBlock(codeSnippetID);
#endif  // SK_GRAPHITE_ENABLED
}
