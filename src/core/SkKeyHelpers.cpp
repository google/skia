/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "src/core/SkDebugUtils.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/src/UniformManager.h"
#endif

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

void AddToKey(SkShaderCodeDictionary* /* dict */,
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

sk_sp<SkUniformData> make_solid_uniform_data(SkShaderCodeDictionary* dict, SkColor4f color) {
    static constexpr size_t kExpectedNumUniforms = 1;

    SkSpan<const SkUniform> uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kSolidColorShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    const void* srcs[kExpectedNumUniforms] = { &color };

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}
#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(SkShaderCodeDictionary* dict,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineData* pipelineData,
              const SkColor4f& color) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        builder->beginBlock(SkBuiltInCodeSnippetID::kSolidColorShader);
        builder->endBlock();

        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kSolidColorShader,
                              kBlockDataSize);

        if (pipelineData) {
            pipelineData->add(make_solid_uniform_data(dict, color));
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

void AddToKey(SkShaderCodeDictionary* dict,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              const GradientData& gradData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
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
        SolidColorShaderBlock::AddToKey(dict, builder, pipelineData, SkColors::kRed);
    }
}

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
namespace ImageShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

sk_sp<SkUniformData> make_image_uniform_data(SkShaderCodeDictionary* dict,
                                             const ImageData& imgData) {
    SkDEBUGCODE(static constexpr size_t kExpectedNumUniforms = 0;)

    SkSpan<const SkUniform> uniforms = dict->getUniforms(SkBuiltInCodeSnippetID::kImageShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    skgpu::UniformManager mgr(skgpu::Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    // TODO: add the required data to ImageData and assemble the uniforms here

    mgr.writeUniforms(result->uniforms(), nullptr, result->offsets(), result->data());
    return result;
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(SkShaderCodeDictionary* dict,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineData* pipelineData,
              const ImageData& imgData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        builder->beginBlock(SkBuiltInCodeSnippetID::kImageShader);

        // TODO: bytes are overkill for just tilemodes. We could add smaller/bit-width
        // types.
        static_assert(SkTFitsIn<uint8_t>(SkTileMode::kLastTileMode));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[0]));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[1]));

        builder->endBlock();

        if (pipelineData) {
            pipelineData->add(make_image_uniform_data(dict, imgData));
        }
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(dict, builder, pipelineData, SkColors::kRed);
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

void AddToKey(SkShaderCodeDictionary* dict,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              const BlendData& blendData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
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
        as_SB(blendData.fDst)->addToKey(dict, builder, pipelineData);
        int firstShaderSize = builder->sizeInBytes() - start;

        start = builder->sizeInBytes();
        as_SB(blendData.fSrc)->addToKey(dict, builder, pipelineData);
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
        SolidColorShaderBlock::AddToKey(dict, builder, pipelineData, SkColors::kRed);
    }
}

} // namespace BlendShaderBlock

//--------------------------------------------------------------------------------------------------
namespace BlendModeBlock {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 1;
#endif

void AddToKey(SkShaderCodeDictionary* dict,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineData* pipelineData,
              SkBlendMode bm) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        builder->beginBlock(SkBuiltInCodeSnippetID::kShaderBasedBlender);
        add_blendmode_to_key(builder, bm);
        builder->endBlock();

        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kShaderBasedBlender,
                              kBlockDataSize);
        return;
    }
#endif// SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(dict, builder, pipelineData, SkColors::kRed);
    }
}

} // namespace BlendModeBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkUniquePaintParamsID CreateKey(SkShaderCodeDictionary* dict,
                                SkPaintParamsKeyBuilder* builder,
                                skgpu::ShaderCombo::ShaderType s,
                                SkTileMode tm,
                                SkBlendMode bm) {
    SkDEBUGCODE(builder->checkReset());

    switch (s) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            DepthStencilOnlyBlock::AddToKey(dict, builder, nullptr);
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(dict, builder, nullptr, SkColors::kRed);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(dict, builder, nullptr,
                                           { SkShader::kLinear_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(dict, builder, nullptr,
                                           { SkShader::kRadial_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(dict, builder, nullptr,
                                           { SkShader::kSweep_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(dict, builder, nullptr,
                                           { SkShader::kConical_GradientType, tm, 0 });
            break;
    }

    BlendModeBlock::AddToKey(dict, builder, nullptr, bm);
    SkPaintParamsKey key = builder->lockAsKey();

    const SkShaderCodeDictionary::Entry* entry = dict->findOrCreate(key);

    return  entry->uniqueID();
}
#endif
