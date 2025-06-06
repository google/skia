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
#include "include/core/SkM44.h"
#include "include/core/SkScalar.h"
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
#include "src/gpu/graphite/Image_Base_Graphite.h"
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
#include "src/gpu/graphite/Surface_Graphite.h"
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
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkRadialGradient.h"
#include "src/shaders/gradients/SkSweepGradient.h"

using namespace skia_private;

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------

namespace {

// Automatically calls beginStruct() with the required alignment and endStruct() when it is deleted.
// Automatically registers uniform expectations in debug builds.
class ScopedUniformWriter {
public:
    ScopedUniformWriter(PipelineDataGatherer* gatherer,
                        const ShaderCodeDictionary* dict,
                        BuiltInCodeSnippetID codeSnippetID)
            : ScopedUniformWriter(gatherer, dict->getEntry(codeSnippetID)) {}

    ~ScopedUniformWriter() {
        if (fGatherer) {
            fGatherer->endStruct();
        }
    }

private:
    ScopedUniformWriter(PipelineDataGatherer* gatherer, const ShaderSnippet* snippet)
#if defined(SK_DEBUG)
        : fValidator(gatherer, snippet->fUniforms, SkToBool(snippet->fUniformStructName))
#endif
    {
        if (snippet->fUniformStructName) {
            gatherer->beginStruct(snippet->fRequiredAlignment);
            fGatherer = gatherer;
        } else {
            fGatherer = nullptr;
        }
    }

    PipelineDataGatherer* fGatherer;
    SkDEBUGCODE(UniformExpectationsValidator fValidator;)
};

#define BEGIN_WRITE_UNIFORMS(gatherer, dict, codeSnippetID) \
    ScopedUniformWriter scope{gatherer, dict, codeSnippetID};

void add_solid_uniform_data(const ShaderCodeDictionary* dict,
                            const SkPMColor4f& premulColor,
                            PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kSolidColorShader)
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
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kRGBPaintColor)
    gatherer->writePaintColor(premulColor);
}

void add_alpha_only_paint_color_uniform_data(const ShaderCodeDictionary* dict,
                                             const SkPMColor4f& premulColor,
                                             PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kAlphaOnlyPaintColor)
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
                            int bufferOffset,
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
        if (gradData.fUseStorageBuffer) {
            gatherer->write(bufferOffset);
        }
    }

    gatherer->write(static_cast<int>(gradData.fTM));
    gatherer->write(static_cast<int>(gradData.fInterpolation.fColorSpace));
    gatherer->write(static_cast<int>(inputPremul));
}

void add_linear_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                      BuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      int bufferOffset,
                                      PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    add_gradient_postamble(gradData, bufferOffset, gatherer);
};

void add_radial_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                      BuiltInCodeSnippetID codeSnippetID,
                                      const GradientShaderBlocks::GradientData& gradData,
                                      int bufferOffset,
                                      PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    add_gradient_postamble(gradData, bufferOffset, gatherer);
};

void add_sweep_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                     BuiltInCodeSnippetID codeSnippetID,
                                     const GradientShaderBlocks::GradientData& gradData,
                                     int bufferOffset,
                                     PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, codeSnippetID)

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fBias);
    gatherer->write(gradData.fScale);
    add_gradient_postamble(gradData, bufferOffset, gatherer);
};

void add_conical_gradient_uniform_data(const ShaderCodeDictionary* dict,
                                       BuiltInCodeSnippetID codeSnippetID,
                                       const GradientShaderBlocks::GradientData& gradData,
                                       int bufferOffset,
                                       PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, codeSnippetID)

    float dRadius = gradData.fRadii[1] - gradData.fRadii[0];
    bool isRadial = SkPoint::Distance(gradData.fPoints[1], gradData.fPoints[0])
                                      < SK_ScalarNearlyZero;

    // When a == 0, encode invA == 1 for radial case, and invA == 0 for linear edge case.
    float a = 0;
    float invA = 1;
    if (!isRadial) {
        a = 1 - dRadius * dRadius;
        if (std::abs(a) > SK_ScalarNearlyZero) {
            invA = 1.0 / (2.0 * a);
        } else {
            a = 0;
            invA = 0;
        }
    } else {
        // Since radius0 is being scaled by 1 / dRadius, and the original radius
        // is always positive, this gives us the original sign of dRadius.
        dRadius = gradData.fRadii[0] > 0 ? 1 : -1;
    }

    add_gradient_preamble(gradData, gatherer);
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(dRadius);
    gatherer->write(a);
    gatherer->write(invA);
    add_gradient_postamble(gradData, bufferOffset, gatherer);
};

} // anonymous namespace

// Writes the color and offset data directly in the gatherer gradient buffer and returns the
// offset the data begins at in the buffer.
static int write_color_and_offset_bufdata(int numStops,
                                           const SkPMColor4f* colors,
                                           const float* offsets,
                                           const SkGradientBaseShader* shader,
                                           PipelineDataGatherer* gatherer) {
    auto [dstData, bufferOffset] = gatherer->allocateGradientData(numStops, shader);
    if (dstData) {
        // Data doesn't already exist so we need to write it.
        // Writes all offset data, then color data. This way when binary searching through the
        // offsets, there is better cache locality.
        for (int i = 0, colorIdx = numStops; i < numStops; i++, colorIdx+=4) {
            float offset = offsets ? offsets[i] : SkIntToFloat(i) / (numStops - 1);
            SkASSERT(offset >= 0.0f && offset <= 1.0f);

            dstData[i] = offset;
            dstData[colorIdx + 0] = colors[i].fR;
            dstData[colorIdx + 1] = colors[i].fG;
            dstData[colorIdx + 2] = colors[i].fB;
            dstData[colorIdx + 3] = colors[i].fA;
        }
    }

    return bufferOffset;
}

GradientShaderBlocks::GradientData::GradientData(SkShaderBase::GradientType type,
                                                 int numStops,
                                                 bool useStorageBuffer)
        : fType(type)
        , fPoints{{0.0f, 0.0f}, {0.0f, 0.0f}}
        , fRadii{0.0f, 0.0f}
        , fBias(0.0f)
        , fScale(0.0f)
        , fTM(SkTileMode::kClamp)
        , fNumStops(numStops)
        , fUseStorageBuffer(useStorageBuffer)
        , fSrcColors(nullptr)
        , fSrcOffsets(nullptr) {
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
                                                 const SkGradientBaseShader* shader,
                                                 sk_sp<TextureProxy> colorsAndOffsetsProxy,
                                                 bool useStorageBuffer,
                                                 const SkGradientShader::Interpolation& interp)
        : fType(type)
        , fBias(bias)
        , fScale(scale)
        , fTM(tm)
        , fNumStops(numStops)
        , fUseStorageBuffer(useStorageBuffer)
        , fSrcColors(colors)
        , fSrcOffsets(offsets)
        , fSrcShader(shader)
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
        if (!fUseStorageBuffer) {
            fColorsAndOffsetsProxy = std::move(colorsAndOffsetsProxy);
            SkASSERT(fColorsAndOffsetsProxy);
        }
    }
}

void GradientShaderBlocks::AddBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    const GradientData& gradData) {
    auto dict = keyContext.dict();

    int bufferOffset = 0;
    if (gradData.fNumStops > GradientData::kNumInternalStorageStops && keyContext.recorder()) {
        if (gradData.fUseStorageBuffer) {
            bufferOffset = write_color_and_offset_bufdata(gradData.fNumStops,
                                                          gradData.fSrcColors,
                                                          gradData.fSrcOffsets,
                                                          gradData.fSrcShader,
                                                          gatherer);
        } else {
            SkASSERT(gradData.fColorsAndOffsetsProxy);
            gatherer->add(gradData.fColorsAndOffsetsProxy,
                          {SkFilterMode::kNearest, SkTileMode::kClamp});
        }
    }

    BuiltInCodeSnippetID codeSnippetID = BuiltInCodeSnippetID::kSolidColorShader;
    switch (gradData.fType) {
        case SkShaderBase::GradientType::kLinear:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kLinearGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kLinearGradientShader8
                        : gradData.fUseStorageBuffer
                            ? BuiltInCodeSnippetID::kLinearGradientShaderBuffer
                            : BuiltInCodeSnippetID::kLinearGradientShaderTexture;
            add_linear_gradient_uniform_data(dict, codeSnippetID, gradData, bufferOffset, gatherer);
            break;
        case SkShaderBase::GradientType::kRadial:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kRadialGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kRadialGradientShader8
                        : gradData.fUseStorageBuffer
                            ? BuiltInCodeSnippetID::kRadialGradientShaderBuffer
                            : BuiltInCodeSnippetID::kRadialGradientShaderTexture;
            add_radial_gradient_uniform_data(dict, codeSnippetID, gradData, bufferOffset, gatherer);
            break;
        case SkShaderBase::GradientType::kSweep:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kSweepGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kSweepGradientShader8
                        : gradData.fUseStorageBuffer
                            ? BuiltInCodeSnippetID::kSweepGradientShaderBuffer
                            : BuiltInCodeSnippetID::kSweepGradientShaderTexture;
            add_sweep_gradient_uniform_data(dict, codeSnippetID, gradData, bufferOffset, gatherer);
            break;
        case SkShaderBase::GradientType::kConical:
            codeSnippetID =
                    gradData.fNumStops <= 4 ? BuiltInCodeSnippetID::kConicalGradientShader4
                    : gradData.fNumStops <= 8 ? BuiltInCodeSnippetID::kConicalGradientShader8
                        : gradData.fUseStorageBuffer
                            ? BuiltInCodeSnippetID::kConicalGradientShaderBuffer
                            : BuiltInCodeSnippetID::kConicalGradientShaderTexture;
            add_conical_gradient_uniform_data(dict, codeSnippetID, gradData, bufferOffset, gatherer);
            break;
        case SkShaderBase::GradientType::kNone:
        default:
            SkDEBUGFAIL("Expected a gradient shader, but it wasn't one.");
            break;
    }

    builder->addBlock(codeSnippetID);
}

//--------------------------------------------------------------------------------------------------

void LocalMatrixShaderBlock::BeginBlock(const KeyContext& keyContext,
                                        PaintParamsKeyBuilder* builder,
                                        PipelineDataGatherer* gatherer,
                                        const LMShaderData& lmShaderData) {
    const ShaderCodeDictionary* dict = keyContext.dict();
    const SkMatrix& m = lmShaderData.fLocalMatrix;

    if (lmShaderData.fLocalMatrix.hasPerspective()) {
        // Perspective local matrices are rare enough and add enough extra instructions that it's
        // worth specializing since it has to perform a per-pixel division.
        builder->beginBlock(BuiltInCodeSnippetID::kLocalMatrixShaderPersp);
        BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kLocalMatrixShaderPersp)
        gatherer->write(m);
    } else {
        // For an affine 2D transform, we only need to upload the upper 2x2 and XY translation.
        builder->beginBlock(BuiltInCodeSnippetID::kLocalMatrixShader);

        BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kLocalMatrixShader)
        // The upper 2x2 is expected to be in column major order, but SkMatrix is 3x3 row major.
        gatherer->write(SkV4{m.getScaleX(), m.getSkewY(),
                             m.getSkewX(),  m.getScaleY()});
        gatherer->write(SkV2{m.getTranslateX(), m.getTranslateY()});
    }
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_color_space_uniforms(const SkColorSpaceXformSteps& steps,
                              bool has_ootf,
                              ReadSwizzle readSwizzle,
                              PipelineDataGatherer* gatherer) {
    SkMatrix gamutTransform;
    const float identity[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    // TODO: it seems odd to copy this into an SkMatrix just to write it to the gatherer
    // fSrcToDstMatrix is column-major, SkMatrix is row-major.
    const float* m = steps.fFlags.gamut_transform ? steps.fSrcToDstMatrix : identity;
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
    } else if (steps.fFlags.gamut_transform) {
        gamutTransform.setAll(m[0], m[3], m[6],
                              m[1], m[4], m[7],
                              m[2], m[5], m[8]);
    }
    gatherer->writeHalf(gamutTransform);

    // To encode whether to do premul/unpremul or make the output opaque, we use
    // srcDEF_args.w and dstDEF_args.w:
    // - identity: {0, 1}
    // - do unpremul: {-1, 1}
    // - do premul: {0, 0}
    // - do both: {-1, 0}
    // - alpha swizzle 1: {1, 1}
    // - alpha swizzle r: {1, 0}
    const bool alphaSwizzleR = readSwizzle == ReadSwizzle::k000R;
    const bool alphaSwizzle1 = readSwizzle == ReadSwizzle::kRGB1 ||
                               readSwizzle == ReadSwizzle::kRRR1;

    // It doesn't make sense to unpremul/premul in opaque cases, but we might get a request to
    // anyways, which we can just ignore.
    const bool unpremul = alphaSwizzle1 ? false : steps.fFlags.unpremul;
    const bool premul = alphaSwizzle1 ? false : steps.fFlags.premul;

    const float srcW = unpremul ? -1.f :
                       (alphaSwizzleR || alphaSwizzle1) ? 1.f :
                                                          0.f;
    const float dstW = (premul || alphaSwizzleR) ? 0.f : 1.f;

    // To encode which transfer function to apply, we use the src and dst gamma values:
    // - identity: 0
    // - sRGB: g > 0
    // - PQ: -2
    // - HLG: -1
    if (steps.fFlags.linearize) {
        const skcms_TFType type = skcms_TransferFunction_getType(&steps.fSrcTF);
        const float srcG = type == skcms_TFType_sRGBish ? steps.fSrcTF.g :
                           type == skcms_TFType_PQish ? -2.f :
                           type == skcms_TFType_HLGish ? -1.f :
                                                         0.f;
        gatherer->write(SkV4{srcG, steps.fSrcTF.a, steps.fSrcTF.b, steps.fSrcTF.c});
        gatherer->write(SkV4{steps.fSrcTF.d, steps.fSrcTF.e, steps.fSrcTF.f, srcW});
    } else {
        gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f});
        gatherer->write(SkV4{0.f, 0.f, 0.f, srcW});
    }

    if (steps.fFlags.encode) {
        const skcms_TFType type = skcms_TransferFunction_getType(&steps.fDstTFInv);
        const float dstG = type == skcms_TFType_sRGBish ? steps.fDstTFInv.g :
                           type == skcms_TFType_PQish ? -2.f :
                           type == skcms_TFType_HLGinvish ? -1.f :
                                                            0.f;
        gatherer->write(SkV4{dstG, steps.fDstTFInv.a, steps.fDstTFInv.b, steps.fDstTFInv.c});
        gatherer->write(SkV4{steps.fDstTFInv.d, steps.fDstTFInv.e, steps.fDstTFInv.f, dstW});
    } else {
        gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f});
        gatherer->write(SkV4{0.f, 0.f, 0.f, dstW});
    }

    if (has_ootf) {
        // TODO(https://issues.skia.org/issues/420956739): Populate OOTF parameters.
        gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f});
        gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f});
    }
}

void add_image_uniform_data(const ShaderCodeDictionary* dict,
                            const ImageShaderBlock::ImageData& imgData,
                            PipelineDataGatherer* gatherer) {
    SkASSERT(!imgData.fSampling.useCubic);
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes.first));
    gatherer->write(SkTo<int>(imgData.fTileModes.second));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
}

void add_clamp_image_uniform_data(const ShaderCodeDictionary* dict,
                                  const ImageShaderBlock::ImageData& imgData,
                                  PipelineDataGatherer* gatherer) {
    SkASSERT(!imgData.fSampling.useCubic);
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kImageShaderClamp)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));

    // Matches GrTextureEffect::kLinearInset, to make sure we don't touch an outer row or column
    // with a weight of 0 when linear filtering.
    const float kLinearInset = 0.5f + 0.00001f;

    // The subset should clamp texel coordinates to an inset subset to prevent sampling neighboring
    // texels when coords fall exactly at texel boundaries.
    SkRect subsetInsetClamp = imgData.fSubset;
    if (imgData.fSampling.filter == SkFilterMode::kNearest) {
        subsetInsetClamp.roundOut(&subsetInsetClamp);
    }
    subsetInsetClamp.inset(kLinearInset, kLinearInset);
    gatherer->write(subsetInsetClamp);
}

void add_cubic_image_uniform_data(const ShaderCodeDictionary* dict,
                                  const ImageShaderBlock::ImageData& imgData,
                                  PipelineDataGatherer* gatherer) {
    SkASSERT(imgData.fSampling.useCubic);
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCubicImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes.first));
    gatherer->write(SkTo<int>(imgData.fTileModes.second));
    const SkCubicResampler& cubic = imgData.fSampling.cubic;
    gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));
}

bool can_do_tiling_in_hw(const Caps* caps, const ImageShaderBlock::ImageData& imgData) {
    if (!caps->clampToBorderSupport() && (imgData.fTileModes.first == SkTileMode::kDecal ||
                                          imgData.fTileModes.second == SkTileMode::kDecal)) {
        return false;
    }
    return imgData.fSubset.contains(SkRect::Make(imgData.fImgSize));
}

void add_sampler_data_to_key(PaintParamsKeyBuilder* builder, const SamplerDesc& samplerDesc) {
    if (samplerDesc.isImmutable()) {
        builder->addData(samplerDesc.asSpan());
    } else {
        // Means we have a regular dynamic sampler. Append a default SamplerDesc to convey this,
        // allowing the key to maintain and convey sampler binding order.
        builder->addData({});
    }
}

} // anonymous namespace

ImageShaderBlock::ImageData::ImageData(const SkSamplingOptions& sampling,
                                       SkTileMode tileModeX,
                                       SkTileMode tileModeY,
                                       SkISize imgSize,
                                       SkRect subset,
                                       ImmutableSamplerInfo immutableSamplerInfo)
        : fSampling(sampling)
        , fTileModes{tileModeX, tileModeY}
        , fImgSize(imgSize)
        , fSubset(subset)
        , fImmutableSamplerInfo(immutableSamplerInfo) {
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

    if (doTilingInHw) {
        CoordNormalizeShaderBlock::CoordNormalizeData data(SkSize::Make(imgData.fImgSize));
        CoordNormalizeShaderBlock::BeginBlock(keyContext, builder, gatherer, data);
        builder->beginBlock(BuiltInCodeSnippetID::kHWImageShader);
    } else if (imgData.fSampling.useCubic) {
        add_cubic_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->beginBlock(BuiltInCodeSnippetID::kCubicImageShader);
    } else if (imgData.fTileModes.first == SkTileMode::kClamp &&
               imgData.fTileModes.second == SkTileMode::kClamp) {
        add_clamp_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->beginBlock(BuiltInCodeSnippetID::kImageShaderClamp);
    } else {
        add_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->beginBlock(BuiltInCodeSnippetID::kImageShader);
    }

    static constexpr std::pair<SkTileMode, SkTileMode> kDefaultTileModes =
            {SkTileMode::kClamp, SkTileMode::kClamp};

    // Image shaders must append immutable sampler data (or '0' in the more common case where
    // regular samplers are used).
    // TODO(b/392623124): In precompile mode (fTextureProxy == null), we still have a need for
    // immutable samplers, which must be passed in somehow.
    ImmutableSamplerInfo info = imgData.fTextureProxy
            ? caps->getImmutableSamplerInfo(imgData.fTextureProxy->textureInfo())
            : imgData.fImmutableSamplerInfo;
    SamplerDesc samplerDesc {imgData.fSampling,
                             doTilingInHw ? imgData.fTileModes : kDefaultTileModes,
                             info};
    gatherer->add(imgData.fTextureProxy, samplerDesc);
    add_sampler_data_to_key(builder, samplerDesc);

    builder->endBlock();

    if (doTilingInHw) {
        // Additional block for coord normalization.
        builder->endBlock();
    }
}

//--------------------------------------------------------------------------------------------------

// makes use of ImageShader functions, above
namespace {

void add_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                const YUVImageShaderBlock::ImageData& imgData,
                                PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kYUVImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(imgData.fLinearFilterUVInset);
    gatherer->write(SkTo<int>(imgData.fTileModes.first));
    gatherer->write(SkTo<int>(imgData.fTileModes.second));
    gatherer->write(SkTo<int>(imgData.fSampling.filter));
    gatherer->write(SkTo<int>(imgData.fSamplingUV.filter));

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->writeHalf(imgData.fYUVtoRGBTranslate);
}

void add_cubic_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                      const YUVImageShaderBlock::ImageData& imgData,
                                      PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCubicYUVImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);
    gatherer->write(SkTo<int>(imgData.fTileModes.first));
    gatherer->write(SkTo<int>(imgData.fTileModes.second));
    const SkCubicResampler& cubic = imgData.fSampling.cubic;
    gatherer->writeHalf(SkImageShader::CubicResamplerMatrix(cubic.B, cubic.C));

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->writeHalf(imgData.fYUVtoRGBTranslate);
}

void add_hw_yuv_image_uniform_data(const ShaderCodeDictionary* dict,
                                   const YUVImageShaderBlock::ImageData& imgData,
                                   PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kHWYUVImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);

    SkPoint linearFilterUVInset = imgData.fLinearFilterUVInset;
    // We sign-encode whether we need to adjust the UV coords by applying `fLinearFilterUVInset` for
    // nearest neighbor filtering in `linearFilterUVInset.fX`.
    if (imgData.fSampling.filter == SkFilterMode::kNearest) {
        linearFilterUVInset.fX = -linearFilterUVInset.fX;
    }
    // We sign-encode whether we need clamping for subset or mismatched Y/UV plane size draws in
    // `linearFilterUVInset.fY` - only clamp tiling modes are supported though.
    if (!imgData.fSubset.contains(SkRect::Make(imgData.fImgSize)) ||
        imgData.fImgSize != imgData.fImgSizeUV) {
        SkASSERT(imgData.fTileModes.first == SkTileMode::kClamp &&
                 imgData.fTileModes.second == SkTileMode::kClamp);
        linearFilterUVInset.fY = -linearFilterUVInset.fY;
    }
    gatherer->write(linearFilterUVInset);

    for (int i = 0; i < 4; ++i) {
        gatherer->writeHalf(imgData.fChannelSelect[i]);
    }
    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    gatherer->writeHalf(imgData.fYUVtoRGBTranslate);
}

void add_hw_yuv_no_swizzle_image_uniform_data(const ShaderCodeDictionary* dict,
                                              const YUVImageShaderBlock::ImageData& imgData,
                                              PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kHWYUVNoSwizzleImageShader)

    gatherer->write(SkSize::Make(1.f/imgData.fImgSize.width(), 1.f/imgData.fImgSize.height()));
    gatherer->write(SkSize::Make(1.f/imgData.fImgSizeUV.width(), 1.f/imgData.fImgSizeUV.height()));
    gatherer->write(imgData.fSubset);

    SkPoint linearFilterUVInset = imgData.fLinearFilterUVInset;
    // We sign-encode whether we need to adjust the UV coords by applying `fLinearFilterUVInset` for
    // nearest neighbor filtering in `linearFilterUVInset.fX`.
    if (imgData.fSampling.filter == SkFilterMode::kNearest) {
        linearFilterUVInset.fX = -linearFilterUVInset.fX;
    }
    // We sign-encode whether we need clamping for subset or mismatched Y/UV plane size draws in
    // `linearFilterUVInset.fY` - only clamp tiling modes are supported though.
    if (!imgData.fSubset.contains(SkRect::Make(imgData.fImgSize)) ||
        imgData.fImgSize != imgData.fImgSizeUV) {
        SkASSERT(imgData.fTileModes.first == SkTileMode::kClamp &&
                 imgData.fTileModes.second == SkTileMode::kClamp);
        linearFilterUVInset.fY = -linearFilterUVInset.fY;
    }
    gatherer->write(linearFilterUVInset);

    gatherer->writeHalf(imgData.fYUVtoRGBMatrix);
    SkV4 yuvToRGBXlateAlphaParam = {
        imgData.fYUVtoRGBTranslate.fX,
        imgData.fYUVtoRGBTranslate.fY,
        imgData.fYUVtoRGBTranslate.fZ,
        imgData.fAlphaParam
    };
    gatherer->writeHalf(yuvToRGBXlateAlphaParam);
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

static bool can_do_yuv_tiling_in_hw(const Caps* caps,
                                    const YUVImageShaderBlock::ImageData& imgData) {
    if (!caps->clampToBorderSupport() && (imgData.fTileModes.first == SkTileMode::kDecal ||
                                          imgData.fTileModes.second == SkTileMode::kDecal)) {
        return false;
    }
    // Use the HW tiling shader variant if we're drawing the full rect with matched Y and UV plane
    // sizes and any tiling mode, or if we're drawing a subset with clamp tiling mode.
    return (imgData.fSubset.contains(SkRect::Make(imgData.fImgSize)) &&
            imgData.fImgSize == imgData.fImgSizeUV) ||
           (imgData.fTileModes.first == SkTileMode::kClamp &&
            imgData.fTileModes.second == SkTileMode::kClamp);
}

static bool no_yuv_swizzle(const YUVImageShaderBlock::ImageData& imgData) {
    // Y_U_V or U_Y_V format, reading from R channel for each texture
    if (imgData.fChannelSelect[0].x == 1 &&
        imgData.fChannelSelect[1].x == 1 &&
        imgData.fChannelSelect[2].x == 1 &&
        imgData.fChannelSelect[3].x == 1) {
        return true;
    }

    return false;
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

    const Caps* caps = keyContext.caps();
    const bool doTilingInHw = !imgData.fSampling.useCubic && can_do_yuv_tiling_in_hw(caps, imgData);
    const bool noYUVSwizzle = no_yuv_swizzle(imgData);

    auto uvTileModes = std::make_pair(imgData.fTileModes.first == SkTileMode::kDecal
                                            ? SkTileMode::kClamp : imgData.fTileModes.first,
                                      imgData.fTileModes.second == SkTileMode::kDecal
                                            ? SkTileMode::kClamp : imgData.fTileModes.second);
    gatherer->add(imgData.fTextureProxies[0], {imgData.fSampling, imgData.fTileModes});
    gatherer->add(imgData.fTextureProxies[1], {imgData.fSamplingUV, uvTileModes});
    gatherer->add(imgData.fTextureProxies[2], {imgData.fSamplingUV, uvTileModes});
    gatherer->add(imgData.fTextureProxies[3], {imgData.fSampling, imgData.fTileModes});

    if (doTilingInHw && noYUVSwizzle) {
        add_hw_yuv_no_swizzle_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kHWYUVNoSwizzleImageShader);
    } else if (doTilingInHw) {
        add_hw_yuv_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kHWYUVImageShader);
    } else if (imgData.fSampling.useCubic) {
        add_cubic_yuv_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kCubicYUVImageShader);
    } else {
        add_yuv_image_uniform_data(keyContext.dict(), imgData, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kYUVImageShader);
    }
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_coord_normalize_uniform_data(const ShaderCodeDictionary* dict,
                                      const CoordNormalizeShaderBlock::CoordNormalizeData& data,
                                      PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCoordNormalizeShader)

    gatherer->write(data.fInvDimensions);
}

} // anonymous namespace

void CoordNormalizeShaderBlock::BeginBlock(const KeyContext& keyContext,
                                           PaintParamsKeyBuilder* builder,
                                           PipelineDataGatherer* gatherer,
                                           const CoordNormalizeData& data) {
    add_coord_normalize_uniform_data(keyContext.dict(), data, gatherer);

    builder->beginBlock(BuiltInCodeSnippetID::kCoordNormalizeShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_coordclamp_uniform_data(const ShaderCodeDictionary* dict,
                                 const CoordClampShaderBlock::CoordClampData& clampData,
                                 PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kCoordClampShader)

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
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kDitherShader)

    gatherer->writeHalf(ditherData.fRange);
}

} // anonymous namespace

void DitherShaderBlock::AddBlock(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 const DitherData& data) {
    add_dither_uniform_data(keyContext.dict(), data, gatherer);

    SkASSERT(data.fLUTProxy || !keyContext.recorder());
    gatherer->add(data.fLUTProxy, {SkFilterMode::kNearest, SkTileMode::kRepeat});

    builder->addBlock(BuiltInCodeSnippetID::kDitherShader);
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_perlin_noise_uniform_data(const ShaderCodeDictionary* dict,
                                   const PerlinNoiseShaderBlock::PerlinNoiseData& noiseData,
                                   PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kPerlinNoiseShader)

    gatherer->write(noiseData.fBaseFrequency);
    gatherer->write(noiseData.fStitchData);
    gatherer->write(static_cast<int>(noiseData.fType));
    gatherer->write(noiseData.fNumOctaves);
    gatherer->write(static_cast<int>(noiseData.stitching()));

    static const std::pair<SkTileMode, SkTileMode> kRepeatXTileModes =
            { SkTileMode::kRepeat, SkTileMode::kClamp };
    gatherer->add(noiseData.fPermutationsProxy, {SkFilterMode::kNearest, kRepeatXTileModes});
    gatherer->add(noiseData.fNoiseProxy, {SkFilterMode::kNearest, kRepeatXTileModes});
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

void BlendComposeBlock::BeginBlock(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* builder,
                                  PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kBlendCompose)

    builder->beginBlock(BuiltInCodeSnippetID::kBlendCompose);
}

//--------------------------------------------------------------------------------------------------

void PorterDuffBlenderBlock::AddBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder* builder,
                                      PipelineDataGatherer* gatherer,
                                      SkSpan<const float> coeffs) {
    BEGIN_WRITE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kPorterDuffBlender)
    SkASSERT(coeffs.size() == 4);
    gatherer->writeHalf(SkV4{coeffs[0], coeffs[1], coeffs[2], coeffs[3]});

    builder->addBlock(BuiltInCodeSnippetID::kPorterDuffBlender);
}

//--------------------------------------------------------------------------------------------------

void HSLCBlenderBlock::AddBlock(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* builder,
                                 PipelineDataGatherer* gatherer,
                                 SkSpan<const float> coeffs) {
    BEGIN_WRITE_UNIFORMS(gatherer, keyContext.dict(), BuiltInCodeSnippetID::kHSLCBlender)
    SkASSERT(coeffs.size() == 2);
    gatherer->writeHalf(SkV2{coeffs[0], coeffs[1]});

    builder->addBlock(BuiltInCodeSnippetID::kHSLCBlender);
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
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kMatrixColorFilter)
    gatherer->writeHalf(data.fMatrix);
    gatherer->writeHalf(data.fTranslate);
    if (data.fClamp) {
        gatherer->writeHalf(SkV2{0.f, 1.f});
    } else {
        // Alpha is always clamped to 1. RGB clamp to the max finite half value.
        static constexpr float kUnclamped = 65504.f; // SK_HalfMax converted back to float
        SkASSERT(SkHalfToFloat(SkFloatToHalf(kUnclamped)) == kUnclamped);
        SkASSERT(SkHalfToFloat(SkFloatToHalf(-kUnclamped)) == -kUnclamped);
        gatherer->writeHalf(SkV2{-kUnclamped, kUnclamped});
    }
}

void add_hsl_matrix_colorfilter_uniform_data(
        const ShaderCodeDictionary* dict,
        const MatrixColorFilterBlock::MatrixColorFilterData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kHSLMatrixColorFilter)
    gatherer->writeHalf(data.fMatrix);
    gatherer->writeHalf(data.fTranslate);
}

} // anonymous namespace

void MatrixColorFilterBlock::AddBlock(const KeyContext& keyContext,
                                      PaintParamsKeyBuilder* builder,
                                      PipelineDataGatherer* gatherer,
                                      const MatrixColorFilterData& matrixCFData) {
    if (matrixCFData.fInHSLA) {
        add_hsl_matrix_colorfilter_uniform_data(keyContext.dict(), matrixCFData, gatherer);

        builder->addBlock(BuiltInCodeSnippetID::kHSLMatrixColorFilter);
    } else {
        add_matrix_colorfilter_uniform_data(keyContext.dict(), matrixCFData, gatherer);

        builder->addBlock(BuiltInCodeSnippetID::kMatrixColorFilter);
    }
}

//--------------------------------------------------------------------------------------------------

namespace {

void add_table_colorfilter_uniform_data(const ShaderCodeDictionary* dict,
                                        const TableColorFilterBlock::TableColorFilterData& data,
                                        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kTableColorFilter)

    gatherer->add(data.fTextureProxy, {SkFilterMode::kNearest, SkTileMode::kClamp});
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
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformColorFilter)
    add_color_space_uniforms(data.fSteps, /*has_ootf=*/true, data.fReadSwizzle, gatherer);
}

void add_color_space_xform_premul_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformPremul)

    // If either of these asserts would fail, we can't correctly use this specialized shader for
    // the given transform.
    SkASSERT(data.fReadSwizzle == ReadSwizzle::kRGBA || data.fReadSwizzle == ReadSwizzle::kRGB1);
    // If these are both true, that implies there's a color space transfer or gamut transform.
    SkASSERT(!(data.fSteps.fFlags.unpremul && data.fSteps.fFlags.premul));

    // This shader can either do nothing, or perform one of three actions. These four possibilities
    // are encoded in a half2 argument as:
    // - identity: {0, 1}
    // - do unpremul: {-1, 1}
    // - do premul: {0, 0}
    // - make opaque: {1, 1}
    const bool opaque = data.fReadSwizzle == ReadSwizzle::kRGB1;
    const float x = data.fSteps.fFlags.unpremul ? -1.f :
                    opaque ? 1.f
                           : 0.f;
    const float y = data.fSteps.fFlags.premul ? 0.f : 1.f;
    gatherer->writeHalf(SkV2{x, y});
}

void add_color_space_xform_srgb_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformSRGB)
    add_color_space_uniforms(data.fSteps, /*has_ootf=*/false, data.fReadSwizzle, gatherer);
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
    const bool xformNeedsGamutOrXferFn = data.fSteps.fFlags.linearize || data.fSteps.fFlags.encode ||
                                         data.fSteps.fFlags.gamut_transform;
    const bool swizzleNeedsGamutTransform = !(data.fReadSwizzle == ReadSwizzle::kRGBA ||
                                              data.fReadSwizzle == ReadSwizzle::kRGB1);

    // Use a specialized shader if we don't need transfer function or gamut transforms.
    if (!(xformNeedsGamutOrXferFn || swizzleNeedsGamutTransform)) {
        // When enabled, the most specialized is to do nothing at all. To simplify calling code,
        // this adds a passthrough block vs. having callers know how to reconfigure their blocks.
        if (SkToBool(keyContext.flags() & KeyGenFlags::kEnableIdentityColorSpaceXform) &&
            data.fReadSwizzle == ReadSwizzle::kRGBA &&
            !data.fSteps.fFlags.premul && !data.fSteps.fFlags.unpremul) {
            builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
            return;
        }

        add_color_space_xform_premul_uniform_data(keyContext.dict(), data, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kColorSpaceXformPremul);
        return;
    }

    // Use a specialized shader if we're transferring to and from sRGB-ish color spaces.
    if (data.fSteps.fFlags.linearize && data.fSteps.fFlags.encode &&
        skcms_TransferFunction_isSRGBish(&data.fSteps.fSrcTF) &&
        skcms_TransferFunction_isSRGBish(&data.fSteps.fDstTFInv)) {
        add_color_space_xform_srgb_uniform_data(keyContext.dict(), data, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kColorSpaceXformSRGB);
        return;
    }

    // Use the most general color space transform shader if no specializations can be used.
    add_color_space_xform_uniform_data(keyContext.dict(), data, gatherer);
    builder->addBlock(BuiltInCodeSnippetID::kColorSpaceXformColorFilter);
}

//--------------------------------------------------------------------------------------------------
namespace {

void add_analytic_clip_data(
        const ShaderCodeDictionary* dict,
        const NonMSAAClipBlock::NonMSAAClipData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kAnalyticClip)
    gatherer->write(data.fRect);
    gatherer->write(data.fRadiusPlusHalf);
    gatherer->writeHalf(data.fEdgeSelect);
}

void add_analytic_and_atlas_clip_data(
        const ShaderCodeDictionary* dict,
        const NonMSAAClipBlock::NonMSAAClipData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kAnalyticAndAtlasClip)
    gatherer->write(data.fRect);
    gatherer->write(data.fRadiusPlusHalf);
    gatherer->writeHalf(data.fEdgeSelect);
    gatherer->write(data.fTexCoordOffset);
    gatherer->write(data.fMaskBounds);
    if (data.fAtlasTexture) {
        gatherer->write(SkSize::Make(1.f/data.fAtlasTexture->dimensions().width(),
                                     1.f/data.fAtlasTexture->dimensions().height()));
    } else {
        gatherer->write(SkSize::Make(0, 0));
    }
}

}  // anonymous namespace

void NonMSAAClipBlock::AddBlock(const KeyContext& keyContext,
                                PaintParamsKeyBuilder* builder,
                                PipelineDataGatherer* gatherer,
                                const NonMSAAClipData& data) {
    if (data.fAtlasTexture) {
        add_analytic_and_atlas_clip_data(keyContext.dict(), data, gatherer);
        builder->beginBlock(BuiltInCodeSnippetID::kAnalyticAndAtlasClip);

        const Caps* caps = keyContext.caps();
        ImmutableSamplerInfo info =
                caps->getImmutableSamplerInfo(data.fAtlasTexture->textureInfo());
        SamplerDesc samplerDesc {SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
                                 {SkTileMode::kClamp, SkTileMode::kClamp},
                                 info};
        gatherer->add(data.fAtlasTexture, samplerDesc);

        builder->endBlock();
    } else {
        add_analytic_clip_data(keyContext.dict(), data, gatherer);
        builder->addBlock(BuiltInCodeSnippetID::kAnalyticClip);
    }
}

//--------------------------------------------------------------------------------------------------

void AddPrimitiveColor(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkColorSpace* primitiveColorSpace) {
    ColorSpaceTransformBlock::ColorSpaceTransformData toDst(primitiveColorSpace,
                                                            kPremul_SkAlphaType,
                                                            keyContext.dstColorInfo().colorSpace(),
                                                            keyContext.dstColorInfo().alphaType());
    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                builder->addBlock(BuiltInCodeSnippetID::kPrimitiveColor);
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, toDst);
            });
}

//--------------------------------------------------------------------------------------------------

void AddBlendModeColorFilter(const KeyContext& keyContext,
                             PaintParamsKeyBuilder* builder,
                             PipelineDataGatherer* gatherer,
                             SkBlendMode bm,
                             const SkPMColor4f& srcColor) {
    Blend(keyContext, builder, gatherer,
          /* addBlendToKey= */ [&] () -> void {
              AddBlendMode(keyContext, builder, gatherer, bm);
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
}

// TODO(robertphillips): when BeginBlock fails we should mark the 'builder' as having failed
// and explicitly handle the failure in ShaderCodeDictionary::findOrCreate.
bool RuntimeEffectBlock::BeginBlock(const KeyContext& keyContext,
                                    PaintParamsKeyBuilder* builder,
                                    PipelineDataGatherer* gatherer,
                                    const ShaderData& shaderData) {
    ShaderCodeDictionary* dict = keyContext.dict();
    int codeSnippetID = dict->findOrCreateRuntimeEffectSnippet(shaderData.fEffect.get());

    if (codeSnippetID < 0) {
        return false;
    }

    if (SkKnownRuntimeEffects::IsUserDefinedRuntimeEffect(codeSnippetID)) {
        keyContext.rtEffectDict()->set(codeSnippetID, shaderData.fEffect);
    }

    const ShaderSnippet* entry = dict->getEntry(codeSnippetID);
    if (!entry) {
        return false;
    }

    gather_runtime_effect_uniforms(keyContext,
                                   shaderData.fEffect.get(),
                                   entry->fUniforms,
                                   shaderData.fUniforms.get(),
                                   gatherer);

    builder->beginBlock(codeSnippetID);
    return true;
}

// TODO(robertphillips): fuse this with the similar code in add_children_to_key and
// PrecompileRTEffect::addToKey.
void RuntimeEffectBlock::AddNoOpEffect(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* builder,
                                       PipelineDataGatherer* gatherer,
                                       SkRuntimeEffect* effect) {
    if (effect->allowShader()) {
        // A missing shader returns transparent black
        SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer,
                                        SK_PMColor4fTRANSPARENT);
    } else if (effect->allowColorFilter()) {
        // A "passthrough" color filter returns the input color as-is.
        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
    } else {
        SkASSERT(effect->allowBlender());
        // A "passthrough" blender performs `blend_src_over(src, dest)`.
        AddFixedBlendMode(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
    }
}

void RuntimeEffectBlock::HandleIntrinsics(const KeyContext& keyContext,
                                          PaintParamsKeyBuilder* builder,
                                          PipelineDataGatherer* gatherer,
                                          const SkRuntimeEffect* effect) {
    // Runtime effects that reference color transform intrinsics have two extra children that
    // are bound to the colorspace xform snippet with values to go to and from the linear srgb
    // to the current working/dst color space.
    if (SkRuntimeEffectPriv::UsesColorTransform(effect)) {
        SkColorSpace* dstCS = keyContext.dstColorInfo().colorSpace();
        if (!dstCS) {
            dstCS = sk_srgb_linear_singleton(); // turn colorspace conversion into a noop
        }

        // TODO(b/332565302): If the runtime shader only uses one of these transforms, we could
        // upload only one set of uniforms.

        // NOTE: This must be kept in sync with the logic used to generate the toLinearSrgb() and
        // fromLinearSrgb() expressions for each runtime effect. toLinearSrgb() is assumed to be
        // the second to last child, and fromLinearSrgb() is assumed to be the last.
        ColorSpaceTransformBlock::ColorSpaceTransformData dstToLinear(dstCS,
                                                                      kUnpremul_SkAlphaType,
                                                                      sk_srgb_linear_singleton(),
                                                                      kUnpremul_SkAlphaType);
        ColorSpaceTransformBlock::ColorSpaceTransformData linearToDst(sk_srgb_linear_singleton(),
                                                                      kUnpremul_SkAlphaType,
                                                                      dstCS,
                                                                      kUnpremul_SkAlphaType);

        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, dstToLinear);
        ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, linearToDst);
    }
}

// ==================================================================

namespace {

void add_to_key(const KeyContext& keyContext,
                PaintParamsKeyBuilder* builder,
                PipelineDataGatherer* gatherer,
                const SkBlendModeBlender* blender) {
    SkASSERT(blender);

    AddBlendMode(keyContext, builder, gatherer, blender->mode());
}

// Be sure to keep this function in sync w/ the code in PrecompileRTEffect::addToKey
void add_children_to_key(const KeyContext& keyContext,
                         PaintParamsKeyBuilder* builder,
                         PipelineDataGatherer* gatherer,
                         SkSpan<const SkRuntimeEffect::ChildPtr> children,
                         const SkRuntimeEffect* effect) {
    SkSpan<const SkRuntimeEffect::Child> childInfo = effect->children();
    SkASSERT(children.size() == childInfo.size());

    using ChildType = SkRuntimeEffect::ChildType;

    for (size_t index = 0; index < children.size(); ++index) {
        const SkRuntimeEffect::ChildPtr& child = children[index];
        KeyContextForRuntimeEffect childContext(keyContext, effect, index);

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
                    AddFixedBlendMode(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                    break;
            }
        }
    }

    RuntimeEffectBlock::HandleIntrinsics(keyContext, builder, gatherer, effect);
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

    if (!RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                        { effect, std::move(uniforms) })) {
        RuntimeEffectBlock::AddNoOpEffect(keyContext, builder, gatherer, effect.get());
        return;
    }

    add_children_to_key(keyContext, builder, gatherer,
                        blender->children(), effect.get());

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
        // Calling code assumes a block will be appended. Add a fixed block to preserve shader
        // and PaintParamsKey structure in release builds but assert since this should either not
        // happen or should be changing high-level logic within PaintParams::toKey().
        SkASSERT(false);
        AddFixedBlendMode(keyContext, builder, gatherer, SkBlendMode::kSrcOver);
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
    bool clamp = filter->clamp() == SkMatrixColorFilter::Clamp::kYes;
    MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(filter->matrix(), inHSLA, clamp);

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

    if (!RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                        { effect, std::move(uniforms) })) {
        RuntimeEffectBlock::AddNoOpEffect(keyContext, builder, gatherer, effect.get());
        return;
    }

    add_children_to_key(keyContext, builder, gatherer,
                        filter->children(), effect.get());

    builder->endBlock();
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkTableColorFilter* filter) {
    SkASSERT(filter);

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                                                filter->bitmap(),
                                                                "TableColorFilterTexture");
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
        // Calling code assumes a block will be appended. Add a fixed block to preserve shader
        // and PaintParamsKey structure in release builds but assert since this should either not
        // happen or should be changing high-level logic within PaintParams::toKey().
        SkASSERT(false);
        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
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
                AddBlendMode(keyContext, builder, gatherer, shader->mode());
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

static SkMatrix matrix_invert_or_identity(const SkMatrix& matrix) {
    SkMatrix inverseMatrix;
    if (!matrix.invert(&inverseMatrix)) {
        inverseMatrix.setIdentity();
    }

    return inverseMatrix;
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkCTMShader* shader) {
    // CTM shaders are always given device coordinates, so we don't have to modify the CTM itself
    // with keyContext's local transform.

    SkMatrix lmInverse = matrix_invert_or_identity(shader->ctm());
    LocalMatrixShaderBlock::LMShaderData lmShaderData(lmInverse);

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

    SkPMColor4f color = map_color(shader->color(), sk_srgb_singleton(),
                                  keyContext.dstColorInfo().colorSpace());

    SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, color);
}
static void notify_in_use(Recorder*, DrawContext*, const SkColorShader*) {
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

    KeyContextWithCoordClamp childContext(keyContext);
    CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, data);
    AddToKey(childContext, builder, gatherer, shader->shader().get());
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
            // For the hardcoded sampling no-swizzle case, we use this to set constant alpha
            imgData.fAlphaParam = 1;
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

    SkColorSpaceXformSteps steps;
    SkASSERT(steps.fFlags.mask() == 0);   // By default, the colorspace should have no effect

    // The actual output from the YUV image shader for non-opaque images is unpremul so
    // we need to correct for the fact that the Image_YUVA_Graphite's alpha type is premul.
    SkAlphaType srcAT = imageToDraw->alphaType() == kPremul_SkAlphaType
                                ? kUnpremul_SkAlphaType
                                : imageToDraw->alphaType();
    if (origShader->isRaw()) {
        // Because we've avoided the premul alpha step in the YUV shader, we need to make sure
        // it happens when drawing unpremul (i.e., non-opaque) images.
        steps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                       srcAT,
                                       imageToDraw->colorSpace(),
                                       imageToDraw->alphaType());
    } else {
        SkAlphaType dstAT = keyContext.dstColorInfo().alphaType();
        // Setting the dst alphaType up this way is necessary because otherwise the constructor
        // for SkColorSpaceXformSteps will set dstAT = srcAT when dstAT == kOpaque, and the
        // premul step needed for non-opaque images won't occur.
        if (dstAT == kOpaque_SkAlphaType && srcAT == kUnpremul_SkAlphaType) {
            dstAT = kPremul_SkAlphaType;
        }
        steps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                       srcAT,
                                       keyContext.dstColorInfo().colorSpace(),
                                       dstAT);
    }
    ColorSpaceTransformBlock::ColorSpaceTransformData data(steps);

    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, data);
            });
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
    if (!as_IB(shader->image())->isGraphiteBacked()) {
        // GetGraphiteBacked() created a new image (or fetched a cached image) from the client
        // image provider. This image was not available when NotifyInUse() visited the shader tree,
        // so call notify again. These images shouldn't really be producing new tasks since it's
        // unlikely that a client will be fulfilling with a dynamic image that wraps a long-lived
        // SkSurface. However, the images can be linked to a surface that rendered the initial
        // content and not calling notifyInUse() prevents unlinking the image from the Device.
        // If the client image provider then holds on to many of these images, the leaked Device and
        // DrawContext memory can be surprisingly high. b/338453542.
        // TODO (b/330864257): Once paint keys are extracted at draw time, AddToKey() will be
        // fully responsible for notifyInUse() calls and then we can simply always call this on
        // `imageToDraw`. The DrawContext that samples the image will also be available to AddToKey
        // so we won't have to pass in nullptr.
        SkASSERT(as_IB(imageToDraw)->isGraphiteBacked());
        static_cast<Image_Base*>(imageToDraw.get())->notifyInUse(keyContext.recorder(),
                                                                 /*drawContext=*/nullptr);
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
                                        shader->subset());

    // Here we detect pixel aligned blit-like image draws. Some devices have low precision filtering
    // and will produce degraded (blurry) images unexpectedly for sequential exact pixel blits when
    // not using nearest filtering. This is common for canvas scrolling implementations. Forcing
    // nearest filtering when possible can also be a minor perf/power optimization depending on the
    // hardware.
    bool samplingHasNoEffect = false;
    // Cubic sampling is will not filter the same as nearest even when pixel aligned.
    if (!(keyContext.flags() & KeyGenFlags::kDisableSamplingOptimization || newSampling.useCubic)) {
        SkMatrix totalM = keyContext.local2Dev().asM33();
        if (keyContext.localMatrix()) {
            totalM.preConcat(*keyContext.localMatrix());
        }
        totalM.normalizePerspective();
        // The matrix should be translation with only pixel aligned 2d translation.
        samplingHasNoEffect = totalM.isTranslate() && SkScalarIsInt(totalM.getTranslateX()) &&
                              SkScalarIsInt(totalM.getTranslateY());
    }

    imgData.fSampling = samplingHasNoEffect ? SkFilterMode::kNearest : newSampling;
    imgData.fTextureProxy = view.refProxy();
    skgpu::Swizzle readSwizzle = view.swizzle();
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (imageToDraw->isAlphaOnly()) {
        readSwizzle = skgpu::Swizzle::Concat(readSwizzle, skgpu::Swizzle("000a"));
    }
    ColorSpaceTransformBlock::ColorSpaceTransformData colorXformData(
            SwizzleClassToReadEnum(readSwizzle));

    if (!shader->isRaw()) {
        colorXformData.fSteps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                                       imageToDraw->alphaType(),
                                                       keyContext.dstColorInfo().colorSpace(),
                                                       keyContext.dstColorInfo().alphaType());

        if (imageToDraw->isAlphaOnly() &&
            !(keyContext.flags() & KeyGenFlags::kDisableAlphaOnlyImageColorization)) {
            // NOTE: Alpha is not affected by colorspace conversion to the dst, and the paint color
            // is already xformed to the dst, but the ColorSpaceTransformBlock is necessary to apply
            // any read swizzle, which is often necessary for alpha-only color types.
            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&] () -> void {
                      AddFixedBlendMode(keyContext, builder, gatherer, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      Compose(keyContext, builder, gatherer,
                              /* addInnerToKey= */ [&]() -> void {
                                  ImageShaderBlock::AddBlock(keyContext, builder, gatherer,
                                                             imgData);
                              },
                              /* addOuterToKey= */ [&]() -> void {
                                  ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer,
                                                                     colorXformData);
                              });
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
                  });
            return;
        }
    }

    Compose(keyContext, builder, gatherer,
            /* addInnerToKey= */ [&]() -> void {
                ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
            },
            /* addOuterToKey= */ [&]() -> void {
                ColorSpaceTransformBlock::AddBlock(keyContext, builder, gatherer, colorXformData);
            });
}
static void notify_in_use(Recorder* recorder,
                          DrawContext* drawContext,
                          const SkImageShader* shader) {
    auto image = as_IB(shader->image());
    if (!image->isGraphiteBacked()) {
        // If it's not graphite-backed, there's no pending graphite work.
        return;
    }

    static_cast<Image_Base*>(image)->notifyInUse(recorder, drawContext);
}

static void add_to_key(const KeyContext& keyContext,
                       PaintParamsKeyBuilder* builder,
                       PipelineDataGatherer* gatherer,
                       const SkLocalMatrixShader* shader) {
    SkASSERT(shader);
    auto wrappedShader = shader->wrappedShader().get();

    // Fold the texture's origin flip into the local matrix so that the image shader doesn't need
    // additional state.
    SkMatrix matrix;

    SkShaderBase* wrappedShaderBase = as_SB(wrappedShader);
    if (wrappedShaderBase->type() == SkShaderBase::ShaderType::kImage) {
        auto imgShader = static_cast<const SkImageShader*>(wrappedShader);
        // If the image is not graphite backed then we can assume the origin will be TopLeft as we
        // require that in the ImageProvider utility. Also Graphite YUV images are assumed to be
        // TopLeft origin.
        auto imgBase = as_IB(imgShader->image());
        if (imgBase->isGraphiteBacked()) {
            // The YUV formats can encode their own origin including reflection and rotation,
            // so we need to concat that to the local matrix transform.
            if (imgBase->isYUVA()) {
                auto imgYUVA = static_cast<const Image_YUVA*>(imgBase);
                SkASSERT(imgYUVA);
                matrix = matrix_invert_or_identity(imgYUVA->yuvaInfo().originMatrix());
            } else {
                auto imgGraphite = static_cast<Image*>(imgBase);
                SkASSERT(imgGraphite);
                const auto& view = imgGraphite->textureProxyView();
                if (view.origin() == Origin::kBottomLeft) {
                    matrix.setScaleY(-1);
                    matrix.setTranslateY(view.height());
                }
            }

        }
    } else if (wrappedShaderBase->type() == SkShaderBase::ShaderType::kGradientBase) {
        auto gradShader = static_cast<const SkGradientBaseShader*>(wrappedShader);
        matrix = gradShader->getGradientMatrix();

        // Override the conical gradient matrix since graphite uses a different algorithm
        // than the ganesh and raster backends.
        if (gradShader->asGradient() == SkShaderBase::GradientType::kConical) {
            auto conicalShader = static_cast<const SkConicalGradient*>(gradShader);

            SkMatrix conicalMatrix;
            if (conicalShader->getType() == SkConicalGradient::Type::kRadial) {
                SkPoint center = conicalShader->getStartCenter();
                conicalMatrix.postTranslate(-center.fX, -center.fY);

                float scale = sk_ieee_float_divide(1, conicalShader->getDiffRadius());
                conicalMatrix.postScale(scale, scale);
            } else {
                SkAssertResult(SkConicalGradient::MapToUnitX(conicalShader->getStartCenter(),
                                                             conicalShader->getEndCenter(),
                                                             &conicalMatrix));
            }
            matrix = conicalMatrix;
        }
    }

    SkMatrix lmInverse = matrix_invert_or_identity(shader->localMatrix());
    lmInverse.postConcat(matrix);

    LocalMatrixShaderBlock::LMShaderData lmShaderData(lmInverse);

    KeyContextWithLocalMatrix newContext(keyContext, shader->localMatrix());

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

    sk_sp<TextureProxy> perm =
            RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                            paintingData->getPermutationsBitmap(),
                                            "PerlinNoisePermTable");

    sk_sp<TextureProxy> noise =
            RecorderPriv::CreateCachedProxy(keyContext.recorder(), paintingData->getNoiseBitmap(),
                                            "PerlinNoiseNoiseTable");

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

    // NOTE: While this is intended to be a "scratch" surface, we don't use MakeScratch() because
    // the SkPicture could contain arbitrary operations that rely on the Recorder's atlases, which
    // means the Surface's device has to participate in flushing when the atlas fills up.
    // TODO: Can this be an approx-fit image that's generated?
    // TODO: right now we're explicitly not caching here. We could expand the ImageProvider
    // API to include already Graphite-backed images, add a Recorder-local cache or add
    // rendered-picture images to the global cache.
    sk_sp<Surface> surface = Surface::Make(recorder,
                                           info.imageInfo,
                                           "PictureShaderTexture",
                                           Budgeted::kYes,
                                           Mipmapped::kNo,
                                           SkBackingFit::kExact,
                                           &info.props);
    if (!surface) {
        SKGPU_LOG_W("Could not create surface to render PictureShader");
        builder->addBlock(BuiltInCodeSnippetID::kError);
        return;
    }

    // NOTE: Don't call CachedImageInfo::makeImage() since that uses the legacy makeImageSnapshot()
    // API, which results in an extra texture copy on a Graphite Surface.
    surface->getCanvas()->concat(info.matrixForDraw);
    surface->getCanvas()->drawPicture(shader->picture().get());
    sk_sp<SkImage> img = SkSurfaces::AsImage(std::move(surface));
    // TODO: 'img' did not exist when notify_in_use() was called, but ideally the DrawTask to render
    // into 'surface' would be a child of the current device. While we push all tasks to the root
    // list this works out okay, but will need to be addressed before we move off that system.
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

    if (!RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                        { effect, std::move(uniforms) })) {
        RuntimeEffectBlock::AddNoOpEffect(keyContext, builder, gatherer, effect.get());
        return;
    }

    add_children_to_key(keyContext, builder, gatherer,
                        shader->children(), effect.get());

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

    bool useStorageBuffer = keyContext.caps()->gradientBufferSupport();
    if (colorCount > GradientShaderBlocks::GradientData::kNumInternalStorageStops
            && !useStorageBuffer) {
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

        proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), shader->cachedBitmap(),
                                                "GradientTexture");
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
                                            shader,
                                            std::move(proxy),
                                            useStorageBuffer,
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
    SkScalar r0 = shader->getStartRadius();
    SkScalar r1 = shader->getEndRadius();

    if (shader->getType() != SkConicalGradient::Type::kRadial) {
        // Since we map the centers to be (0,0) and (1,0) in the gradient matrix,
        // there is a scale of 1/distance-between-centers that has to be applied to the radii.
        r0 /= shader->getCenterX1();
        r1 /= shader->getCenterX1();
    } else {
        r0 /= shader->getDiffRadius();
        r1 /= shader->getDiffRadius();
    }

    add_gradient_to_key(keyContext,
                        builder,
                        gatherer,
                        shader,
                        shader->getStartCenter(),
                        shader->getEndCenter(),
                        r0,
                        r1,
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
        // Calling code assumes a block will be appended. Add a fixed block to preserve shader
        // and PaintParamsKey structure in release builds but assert since this should either not
        // happen or should be changing high-level logic within PaintParams::toKey().
        SkASSERT(false);
        SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, SK_PMColor4fTRANSPARENT);
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
