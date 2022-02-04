/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkUniform.h"
#include "src/core/SkUniformData.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/src/UniformManager.h"
#endif

namespace skgpu {
SkSpan<const SkUniform> GetUniforms(CodeSnippetID snippetID);
}

namespace {

#if defined(SK_DEBUG) && defined(SK_GRAPHITE_ENABLED)
CodeSnippetID read_code_snippet_id(const SkPaintParamsKey& key, int headerOffset) {
    uint8_t byte = key.byte(headerOffset);

    SkASSERT(byte <= static_cast<int>(CodeSnippetID::kLast));

    return static_cast<CodeSnippetID>(byte);
}
#endif

// This can be used to catch errors in blocks that have a fixed, known block data size
void validate_block_header(const SkPaintParamsKey& key, int headerOffset,
                           CodeSnippetID codeSnippetID, int blockDataSize) {
    SkASSERT(key.byte(headerOffset) == static_cast<int>(codeSnippetID));
    SkASSERT(key.byte(headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes) ==
             SkPaintParamsKey::kBlockHeaderSizeInBytes + blockDataSize);
}

#ifdef SK_GRAPHITE_ENABLED
void add_blendmode_to_key(SkPaintParamsKey* key, SkBlendMode bm) {
    SkASSERT(static_cast<int>(bm) <= std::numeric_limits<uint8_t>::max());
    key->addByte(static_cast<uint8_t>(bm));
}

#ifdef SK_DEBUG
SkBlendMode to_blendmode(uint8_t data) {
    SkASSERT(data <= static_cast<int>(SkBlendMode::kLastMode));
    return static_cast<SkBlendMode>(data);
}

SkTileMode to_tilemode(uint8_t data) {
    SkASSERT(data <= static_cast<int>(SkTileMode::kLastTileMode));
    return static_cast<SkTileMode>(data);
}
#endif // SK_DEBUG

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
namespace DepthStencilOnlyBlock {

static const int kBlockDataSize = 0;

void AddToKey(SkBackend /* backend */,
              SkPaintParamsKey* key,
              SkUniformBlock* /* uniformBlock */) {
    int headerOffset = key->beginBlock(CodeSnippetID::kDepthStencilOnlyDraw);
    key->endBlock(headerOffset, CodeSnippetID::kDepthStencilOnlyDraw);

    validate_block_header(*key, headerOffset,
                          CodeSnippetID::kDepthStencilOnlyDraw, kBlockDataSize);
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {
    validate_block_header(key, headerOffset,
                          CodeSnippetID::kDepthStencilOnlyDraw, kBlockDataSize);

    SkDebugf("kDepthStencilOnlyDraw\n");
}
#endif

} // namespace DepthStencilOnlyBlock

//--------------------------------------------------------------------------------------------------
namespace SolidColorShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 0;

sk_sp<SkUniformData> make_solid_uniform_data(SkColor4f color) {
    static constexpr size_t kExpectedNumUniforms = 1;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kSolidColorShader);
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

void AddToKey(SkBackend backend,
              SkPaintParamsKey* key,
              SkUniformBlock* uniformBlock,
              const SkColor4f& color) {

#ifdef SK_GRAPHITE_ENABLED
    if (backend == SkBackend::kGraphite) {
        int headerOffset = key->beginBlock(CodeSnippetID::kSolidColorShader);
        key->endBlock(headerOffset, CodeSnippetID::kSolidColorShader);

        validate_block_header(*key, headerOffset,
                              CodeSnippetID::kSolidColorShader, kBlockDataSize);

        if (uniformBlock) {
            uniformBlock->add(make_solid_uniform_data(color));
        }
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (backend == SkBackend::kSkVM || backend == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
    }

}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {

#ifdef SK_GRAPHITE_ENABLED
    validate_block_header(key, headerOffset,
                          CodeSnippetID::kSolidColorShader, kBlockDataSize);

    SkDebugf("kSolidColorShader\n");
#endif

}
#endif

} // namespace SolidColorShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 1;
static const int kExpectedNumGradientUniforms = 6;

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

sk_sp<SkUniformData> make_linear_gradient_uniform_data(const GradientData& gradData) {

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kLinearGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1],
        &gradData.fRadii[0], // unused
        &gradData.fRadii[1], // unused
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_radial_gradient_uniform_data(const GradientData& gradData) {

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kRadialGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1], // unused
        &gradData.fRadii[0],
        &gradData.fRadii[1],  // unused
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_sweep_gradient_uniform_data(const GradientData& gradData) {

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kSweepGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1], // unused
        &gradData.fRadii[0],  // unused
        &gradData.fRadii[1],  // unused
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_conical_gradient_uniform_data(const GradientData& gradData) {

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kConicalGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumGradientUniforms);

    const void* srcs[kExpectedNumGradientUniforms] = {
        gradData.fColor4fs,
        gradData.fOffsets,
        &gradData.fPoints[0],
        &gradData.fPoints[1],
        &gradData.fRadii[0],
        &gradData.fRadii[1],
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

void AddToKey(SkBackend backend,
              SkPaintParamsKey *key,
              SkUniformBlock* uniformBlock,
              const GradientData& gradData) {

#ifdef SK_GRAPHITE_ENABLED
    if (backend == SkBackend::kGraphite) {
        CodeSnippetID codeSnippetID = CodeSnippetID::kSolidColorShader;
        switch (gradData.fType) {
            case SkShader::kLinear_GradientType:
                codeSnippetID = CodeSnippetID::kLinearGradientShader;
                if (uniformBlock) {
                    uniformBlock->add(make_linear_gradient_uniform_data(gradData));
                }
                break;
            case SkShader::kRadial_GradientType:
                codeSnippetID = CodeSnippetID::kRadialGradientShader;
                if (uniformBlock) {
                    uniformBlock->add(make_radial_gradient_uniform_data(gradData));
                }
                break;
            case SkShader::kSweep_GradientType:
                codeSnippetID = CodeSnippetID::kSweepGradientShader;
                if (uniformBlock) {
                    uniformBlock->add(make_sweep_gradient_uniform_data(gradData));
                }
                break;
            case SkShader::GradientType::kConical_GradientType:
                codeSnippetID = CodeSnippetID::kConicalGradientShader;
                if (uniformBlock) {
                    uniformBlock->add(make_conical_gradient_uniform_data(gradData));
                }
                break;
            case SkShader::GradientType::kColor_GradientType:
            case SkShader::GradientType::kNone_GradientType:
            default:
                SkASSERT(0);
                break;
        }

        int headerOffset = key->beginBlock(codeSnippetID);

        SkASSERT(static_cast<int>(gradData.fTM) <= std::numeric_limits<uint8_t>::max());
        key->addByte(static_cast<uint8_t>(gradData.fTM));

        key->endBlock(headerOffset, codeSnippetID);

        validate_block_header(*key, headerOffset, codeSnippetID, kBlockDataSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (backend == SkBackend::kSkVM || backend == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
        SolidColorShaderBlock::AddToKey(backend, key, uniformBlock, SkColors::kRed);
    }
}

#ifdef SK_DEBUG

#ifdef SK_GRAPHITE_ENABLED

std::pair<CodeSnippetID, SkTileMode> ExtractFromKey(const SkPaintParamsKey& key,
                                                    uint32_t headerOffset) {
    CodeSnippetID id = read_code_snippet_id(key, headerOffset);

    SkASSERT(id == CodeSnippetID::kLinearGradientShader ||
             id == CodeSnippetID::kRadialGradientShader ||
             id == CodeSnippetID::kSweepGradientShader ||
             id == CodeSnippetID::kConicalGradientShader);
    SkASSERT(key.byte(headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes) ==
             SkPaintParamsKey::kBlockHeaderSizeInBytes+kBlockDataSize);

    uint8_t data = key.byte(headerOffset + SkPaintParamsKey::kBlockHeaderSizeInBytes);
    SkTileMode tm = to_tilemode(data);

    return { id, tm };
}

#endif // SK_GRAPHITE_ENABLED

void Dump(const SkPaintParamsKey& key, int headerOffset) {

#ifdef SK_GRAPHITE_ENABLED
    auto [id, tm] =  ExtractFromKey(key, headerOffset);

    switch (id) {
        case CodeSnippetID::kLinearGradientShader:
            SkDebugf("kLinearGradientShader: %s\n", SkTileModeToStr(tm));
            break;
        case CodeSnippetID::kRadialGradientShader:
            SkDebugf("kRadialGradientShader: %s\n", SkTileModeToStr(tm));
            break;
        case CodeSnippetID::kSweepGradientShader:
            SkDebugf("kSweepGradientShader: %s\n", SkTileModeToStr(tm));
            break;
        case CodeSnippetID::kConicalGradientShader:
            SkDebugf("kConicalGradientShader: %s\n", SkTileModeToStr(tm));
            break;
        default:
            SkDebugf("Unknown!!\n");
            break;
    }
#endif // SK_GRAPHITE_ENABLED

}
#endif

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
namespace ImageShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

inline static constexpr int kTileModeBits = 2;

static const int kXTileModeShift = 0;
static const int kYTileModeShift = kTileModeBits;

#ifdef SK_DEBUG
static const int kBlockDataSize = 1;

inline static constexpr int kTileModeMask = 0x3;

ImageData ExtractFromKey(const SkPaintParamsKey& key, uint32_t headerOffset) {
    validate_block_header(key, headerOffset,
                          CodeSnippetID::kImageShader, kBlockDataSize);

    uint8_t data = key.byte(headerOffset+SkPaintParamsKey::kBlockHeaderSizeInBytes);

    SkTileMode tmX = to_tilemode(((data) >> kXTileModeShift) & kTileModeMask);
    SkTileMode tmY = to_tilemode(((data) >> kYTileModeShift) & kTileModeMask);

    return { tmX, tmY };
}
#endif // SK_DEBUG

sk_sp<SkUniformData> make_image_uniform_data(const ImageData& imgData) {
    SkDEBUGCODE(static constexpr size_t kExpectedNumUniforms = 0;)

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kImageShader);
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

void AddToKey(SkBackend backend,
              SkPaintParamsKey* key,
              SkUniformBlock* uniformBlock,
              const ImageData& imgData) {

#ifdef SK_GRAPHITE_ENABLED
    if (backend == SkBackend::kGraphite) {

        uint8_t data = (static_cast<uint8_t>(imgData.fTileModes[0]) << kXTileModeShift) |
                       (static_cast<uint8_t>(imgData.fTileModes[1]) << kYTileModeShift);

        int headerOffset = key->beginBlock(CodeSnippetID::kImageShader);

        key->addByte(data);

        key->endBlock(headerOffset, CodeSnippetID::kImageShader);

        SkASSERT(imgData == ExtractFromKey(*key, headerOffset));

        if (uniformBlock) {
            uniformBlock->add(make_image_uniform_data(imgData));
        }
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (backend == SkBackend::kSkVM || backend == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(backend, key, uniformBlock, SkColors::kRed);
    }
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {

#ifdef SK_GRAPHITE_ENABLED
    ImageData imgData = ExtractFromKey(key, headerOffset);

    SkDebugf("kImageShader: tileModes(%s, %s) ",
             SkTileModeToStr(imgData.fTileModes[0]),
             SkTileModeToStr(imgData.fTileModes[1]));
#endif // SK_GRAPHITE_ENABLED

}
#endif // SK_DEBUG

} // namespace ImageShaderBlock

//--------------------------------------------------------------------------------------------------
namespace BlendShaderBlock {

void AddToKey(SkBackend backend,
              SkPaintParamsKey *key,
              SkUniformBlock* uniformBlock,
              const BlendData& blendData) {

#ifdef SK_GRAPHITE_ENABLED
    if (backend == SkBackend::kGraphite) {
        int headerOffset = key->beginBlock(CodeSnippetID::kBlendShader);

        add_blendmode_to_key(key, blendData.fBM);
        int start = key->sizeInBytes();
        as_SB(blendData.fDst)->addToKey(nullptr, backend, key, uniformBlock);
        int firstShaderSize = key->sizeInBytes() - start;

        start = key->sizeInBytes();
        as_SB(blendData.fSrc)->addToKey(nullptr, backend, key, uniformBlock);
        int secondShaderSize = key->sizeInBytes() - start;

        key->endBlock(headerOffset, CodeSnippetID::kBlendShader);

        int expectedBlockSize = 1 + firstShaderSize + secondShaderSize;
        validate_block_header(*key, headerOffset, CodeSnippetID::kBlendShader, expectedBlockSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (backend == SkBackend::kSkVM || backend == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(backend, key, uniformBlock, SkColors::kRed);
    }
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {
#ifdef SK_GRAPHITE_ENABLED
    auto [id, storedBlockSize] = key.readCodeSnippetID(headerOffset);
    SkASSERT(id == CodeSnippetID::kBlendShader);

    int runningOffset = headerOffset + SkPaintParamsKey::kBlockHeaderSizeInBytes;

    uint8_t data = key.byte(runningOffset);
    SkBlendMode bm = to_blendmode(data);

    SkDebugf("BlendMode: %s\n", SkBlendMode_Name(bm));
    runningOffset += 1; // 1 byte for blendmode

    SkDebugf("\nDst:  ");
    int firstBlockSize = SkPaintParamsKey::DumpBlock(key, runningOffset);
    runningOffset += firstBlockSize;

    SkDebugf("Src: ");
    int secondBlockSize = SkPaintParamsKey::DumpBlock(key, runningOffset);

    int calculatedBlockSize = SkPaintParamsKey::kBlockHeaderSizeInBytes +
                              firstBlockSize + secondBlockSize + 1;
    SkASSERT(calculatedBlockSize == storedBlockSize);
#endif// SK_GRAPHITE_ENABLED
}
#endif

} // namespace BlendShaderBlock

//--------------------------------------------------------------------------------------------------
namespace BlendModeBlock {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 1;
#endif

void AddToKey(SkBackend backend,
              SkPaintParamsKey *key,
              SkUniformBlock* uniformBlock,
              SkBlendMode bm) {

#ifdef SK_GRAPHITE_ENABLED
    if (backend == SkBackend::kGraphite) {
        int headerOffset = key->beginBlock(CodeSnippetID::kSimpleBlendMode);
        add_blendmode_to_key(key, bm);
        key->endBlock(headerOffset, CodeSnippetID::kSimpleBlendMode);

        validate_block_header(*key, headerOffset,
                              CodeSnippetID::kSimpleBlendMode, kBlockDataSize);
        return;
    }
#endif// SK_GRAPHITE_ENABLED

    if (backend == SkBackend::kSkVM || backend == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(backend, key, uniformBlock, SkColors::kRed);
    }
}

#ifdef SK_DEBUG

#ifdef SK_GRAPHITE_ENABLED
SkBlendMode ExtractFromKey(const SkPaintParamsKey& key, uint32_t headerOffset) {
    validate_block_header(key, headerOffset, CodeSnippetID::kSimpleBlendMode, kBlockDataSize);

    uint8_t data = key.byte(headerOffset + SkPaintParamsKey::kBlockHeaderSizeInBytes);
    return to_blendmode(data);
}
#endif // SK_GRAPHITE_ENABLED

void Dump(const SkPaintParamsKey& key, int headerOffset) {

#ifdef SK_GRAPHITE_ENABLED
    SkBlendMode bm = ExtractFromKey(key, headerOffset);

    SkDebugf("kSimpleBlendMode: %s\n", SkBlendMode_Name(bm));
#endif

}
#endif

} // namespace BlendModeBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkPaintParamsKey CreateKey(SkBackend backend,
                           skgpu::ShaderCombo::ShaderType s,
                           SkTileMode tm,
                           SkBlendMode bm) {
    SkPaintParamsKey key;

    switch (s) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            DepthStencilOnlyBlock::AddToKey(backend, &key, nullptr);
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(backend, &key, nullptr, SkColors::kRed);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(backend, &key, nullptr,
                                           { SkShader::kLinear_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(backend, &key, nullptr,
                                           { SkShader::kRadial_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(backend, &key, nullptr,
                                           { SkShader::kSweep_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(backend, &key, nullptr,
                                           { SkShader::kConical_GradientType, tm, 0 });
            break;
    }

    BlendModeBlock::AddToKey(backend, &key, nullptr, bm);
    return key;
}
#endif
