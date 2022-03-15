/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "src/core/SkDebugUtils.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/UniformManager.h"
#include "src/gpu/Blend.h"
#endif

constexpr SkPMColor4f kErrorColor = { 1, 0, 0, 1 };

namespace {

// This can be used to catch errors in blocks that have a fixed, known block data size
void validate_block_header(const SkPaintParamsKeyBuilder* builder,
                           SkBuiltInCodeSnippetID codeSnippetID,
                           int blockDataSize) {
    SkDEBUGCODE(int fullBlockSize = SkPaintParamsKey::kBlockHeaderSizeInBytes + blockDataSize;)
    SkDEBUGCODE(int headerOffset = builder->sizeInBytes() - fullBlockSize;)
    SkASSERT(builder->byte(headerOffset) == static_cast<int>(codeSnippetID));
    SkASSERT(builder->byte(headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes) ==
             fullBlockSize);
}

#ifdef SK_GRAPHITE_ENABLED
void add_blendmode_to_key(SkPaintParamsKeyBuilder* builder, SkBlendMode bm) {
    static_assert(SkTFitsIn<uint8_t>(static_cast<int>(SkBlendMode::kLastMode)));
    builder->addByte(static_cast<uint8_t>(bm));
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
namespace DepthStencilOnlyBlock {

static const int kBlockDataSize = 0;

void AddToKey(const SkKeyContext& /* keyContext */,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineData* /* pipelineData */) {
    builder->beginBlock(SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw);
    builder->endBlock();

    validate_block_header(builder,
                          SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw,
                          kBlockDataSize);
}

} // namespace DepthStencilOnlyBlock

//--------------------------------------------------------------------------------------------------
namespace SolidColorShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 0;

sk_sp<SkUniformData> make_solid_uniform_data(SkShaderCodeDictionary* dict,
                                             const SkPMColor4f& premulColor) {
    static constexpr size_t kExpectedNumUniforms = 1;

    SkSpan<const SkUniform> uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kSolidColorShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    const void* srcs[kExpectedNumUniforms] = { &premulColor };

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}
#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineData* pipelineData,
              const SkPMColor4f& premulColor) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();

        builder->beginBlock(SkBuiltInCodeSnippetID::kSolidColorShader);
        builder->endBlock();

        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kSolidColorShader,
                              kBlockDataSize);

        if (pipelineData) {
            pipelineData->add(make_solid_uniform_data(dict, premulColor));
        }
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
    }

}

} // namespace SolidColorShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 1;
static const int kExpectedNumGradientUniforms = 7;

sk_sp<SkUniformData> make_gradient_uniform_data_common(
        SkSpan<const SkUniform> uniforms,
        const void* srcs[kExpectedNumGradientUniforms]) {
    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    // TODO: Given that, for the sprint, we always know the uniforms we could cache 'dataSize'
    // for each layout and skip the first call.
    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}

sk_sp<SkUniformData> make_linear_gradient_uniform_data(SkShaderCodeDictionary* dict,
                                                       const GradientData& gradData) {

    auto uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kLinearGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    SkPoint padding{0, 0};
    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1],
        &gradData.fRadii[0], // unused
        &gradData.fRadii[1], // unused
        &padding
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_radial_gradient_uniform_data(SkShaderCodeDictionary* dict,
                                                       const GradientData& gradData) {

    auto uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kRadialGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    SkPoint padding{0, 0};
    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1], // unused
        &gradData.fRadii[0],
        &gradData.fRadii[1],  // unused
        &padding
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_sweep_gradient_uniform_data(SkShaderCodeDictionary* dict,
                                                      const GradientData& gradData) {

    auto uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kSweepGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    SkPoint padding{0, 0};
    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1], // unused
        &gradData.fRadii[0],  // unused
        &gradData.fRadii[1],  // unused
        &padding
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_conical_gradient_uniform_data(SkShaderCodeDictionary* dict,
                                                        const GradientData& gradData) {

    auto uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kConicalGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    SkPoint padding{0, 0};
    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1],
        &gradData.fRadii[0],
        &gradData.fRadii[1],
        &padding,
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

GradientData::GradientData(SkShader::GradientType type,
                           SkTileMode tm,
                           int numStops)
        : fType(type)
        , fPoints{{0.0f, 0.0f}, {0.0f, 0.0f}}
        , fRadii{0.0f, 0.0f}
        , fTM(tm)
        , fNumStops(numStops) {
    sk_bzero(fColor4fs, sizeof(fColor4fs));
    sk_bzero(fOffsets, sizeof(fOffsets));
}

GradientData::GradientData(SkShader::GradientType type,
                           SkPoint point0, SkPoint point1,
                           float radius0, float radius1,
                           SkTileMode tm,
                           int numStops,
                           SkColor4f* color4fs,
                           float* offsets)
        : fType(type)
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

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              const GradientData& gradData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();
        SkBuiltInCodeSnippetID codeSnippetID = SkBuiltInCodeSnippetID::kSolidColorShader;
        switch (gradData.fType) {
            case SkShader::kLinear_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kLinearGradientShader;
                if (pipelineData) {
                    pipelineData->add(make_linear_gradient_uniform_data(dict, gradData));
                }
                break;
            case SkShader::kRadial_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kRadialGradientShader;
                if (pipelineData) {
                    pipelineData->add(make_radial_gradient_uniform_data(dict, gradData));
                }
                break;
            case SkShader::kSweep_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kSweepGradientShader;
                if (pipelineData) {
                    pipelineData->add(make_sweep_gradient_uniform_data(dict, gradData));
                }
                break;
            case SkShader::GradientType::kConical_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kConicalGradientShader;
                if (pipelineData) {
                    pipelineData->add(make_conical_gradient_uniform_data(dict, gradData));
                }
                break;
            case SkShader::GradientType::kColor_GradientType:
            case SkShader::GradientType::kNone_GradientType:
            default:
                SkASSERT(0);
                break;
        }

        builder->beginBlock(codeSnippetID);

        SkASSERT(static_cast<int>(gradData.fTM) <= std::numeric_limits<uint8_t>::max());
        builder->addByte(static_cast<uint8_t>(gradData.fTM));

        builder->endBlock();

        validate_block_header(builder, codeSnippetID, kBlockDataSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, pipelineData, kErrorColor);
    }
}

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
namespace ImageShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

sk_sp<SkUniformData> make_image_uniform_data(SkShaderCodeDictionary* dict,
                                             const ImageData& imgData) {
    static constexpr size_t kExpectedNumUniforms = 1;

    SkSpan<const SkUniform> uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kImageShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    // TODO: add the required data to ImageData and assemble the uniforms here
    const void* srcs[kExpectedNumUniforms] = { &imgData.fSubset };

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

ImageData::ImageData(const SkSamplingOptions& sampling,
                     SkTileMode tileModeX,
                     SkTileMode tileModeY,
                     SkRect subset)
    : fSampling(sampling)
    , fTileModes{tileModeX, tileModeY}
    , fSubset(subset) {
}

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineData* pipelineData,
              const ImageData& imgData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        if (pipelineData && !imgData.fTextureProxy) {
            // We're dropping the ImageShader here. This could be an instance of trying to draw
            // a raster-backed image w/ a Graphite-backed canvas.
            // TODO: At some point the pre-compile path should also be creating a texture
            // proxy (i.e., we can remove the 'pipelineData' in the above test).
            SolidColorShaderBlock::AddToKey(keyContext, builder, pipelineData, kErrorColor);
            return;
        }

        auto dict = keyContext.dict();
        builder->beginBlock(SkBuiltInCodeSnippetID::kImageShader);

        // TODO: bytes are overkill for just tilemodes. We could add smaller/bit-width
        // types.
        static_assert(SkTFitsIn<uint8_t>(SkTileMode::kLastTileMode));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[0]));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[1]));

        builder->endBlock();

        if (pipelineData) {
            pipelineData->addImage(imgData.fSampling,
                                   imgData.fTileModes,
                                   std::move(imgData.fTextureProxy));

            pipelineData->add(make_image_uniform_data(dict, imgData));
        }

        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, pipelineData, kErrorColor);
    }
}

} // namespace ImageShaderBlock

//--------------------------------------------------------------------------------------------------
namespace BlendShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

sk_sp<SkUniformData> make_blendshader_uniform_data(SkShaderCodeDictionary* dict, SkBlendMode bm) {
    static constexpr size_t kExpectedNumUniforms = 4; // actual blend uniform + 3 padding int

    SkSpan<const SkUniform> uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kBlendShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    int tmp = SkTo<int>(bm);
    const void* srcs[kExpectedNumUniforms] = { &tmp, &tmp, &tmp, &tmp };

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              const BlendData& blendData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();
        // When extracted into SkShaderInfo::SnippetEntries the children will appear after their
        // parent. Thus, the parent's uniform data must appear in the uniform block before the
        // uniform data of the children.
        if (pipelineData) {
            pipelineData->add(make_blendshader_uniform_data(dict, blendData.fBM));
        }

        builder->beginBlock(SkBuiltInCodeSnippetID::kBlendShader);

        // Child blocks always go right after the parent block's header
        // TODO: add startChild/endChild entry points to SkPaintParamsKeyBuilder. They could be
        // used to compute and store the number of children w/in a block's header.
        int start = builder->sizeInBytes();
        as_SB(blendData.fDst)->addToKey(keyContext, builder, pipelineData);
        int firstShaderSize = builder->sizeInBytes() - start;

        start = builder->sizeInBytes();
        as_SB(blendData.fSrc)->addToKey(keyContext, builder, pipelineData);
        int secondShaderSize = builder->sizeInBytes() - start;

        builder->endBlock();

        int expectedBlockSize = firstShaderSize + secondShaderSize;
        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kBlendShader,
                              expectedBlockSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, pipelineData, kErrorColor);
    }
}

} // namespace BlendShaderBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
namespace {

constexpr SkPipelineData::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                          skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

/*>> No coverage, input color unknown <<*/
static constexpr SkPipelineData::BlendInfo gBlendTable[(int)SkBlendMode::kLastCoeffMode + 1] = {
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

const SkPipelineData::BlendInfo& get_blend_info(SkBlendMode bm) {
    if (bm <= SkBlendMode::kLastCoeffMode) {
        return gBlendTable[(int) bm];
    }

    return gBlendTable[(int) SkBlendMode::kSrc];
}

} // anonymous namespace
#endif // SK_GRAPHITE_ENABLED

namespace BlendModeBlock {

#ifdef SK_GRAPHITE_ENABLED
static const int kFixedFunctionBlockDataSize = 1;
static const int kShaderBasedBlockDataSize = 1;
#endif

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              SkBlendMode bm) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        if (bm <= SkBlendMode::kLastCoeffMode) {
            builder->beginBlock(SkBuiltInCodeSnippetID::kFixedFunctionBlender);
            add_blendmode_to_key(builder, bm);
            builder->endBlock();

            validate_block_header(builder,
                                  SkBuiltInCodeSnippetID::kFixedFunctionBlender,
                                  kFixedFunctionBlockDataSize);

            if (pipelineData) {
                pipelineData->setBlendInfo(get_blend_info(bm));
            }
        } else {
            builder->beginBlock(SkBuiltInCodeSnippetID::kShaderBasedBlender);
            add_blendmode_to_key(builder, bm);
            builder->endBlock();

            validate_block_header(builder,
                                  SkBuiltInCodeSnippetID::kShaderBasedBlender,
                                  kShaderBasedBlockDataSize);

            if (pipelineData) {
                // TODO: set up the correct blend info
                pipelineData->setBlendInfo(SkPipelineData::BlendInfo());
            }
        }
        return;
    }
#endif// SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, pipelineData, kErrorColor);
    }
}

} // namespace BlendModeBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkUniquePaintParamsID CreateKey(const SkKeyContext& keyContext,
                                SkPaintParamsKeyBuilder* builder,
                                skgpu::ShaderCombo::ShaderType s,
                                SkTileMode tm,
                                SkBlendMode bm) {
    SkDEBUGCODE(builder->checkReset());

    switch (s) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            DepthStencilOnlyBlock::AddToKey(keyContext, builder, nullptr);
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(keyContext, builder, nullptr, kErrorColor);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kLinear_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kRadial_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kSweep_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kConical_GradientType, tm, 0 });
            break;
    }

    // TODO: the blendInfo should be filled in by BlendModeBlock::AddToKey
    SkPipelineData::BlendInfo blendInfo = get_blend_info(bm);
    BlendModeBlock::AddToKey(keyContext, builder, /* pipelineData*/ nullptr, bm);
    SkPaintParamsKey key = builder->lockAsKey();

    auto dict = keyContext.dict();

    auto entry = dict->findOrCreate(key, blendInfo);

    return  entry->uniqueID();
}
#endif
