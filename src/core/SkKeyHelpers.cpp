/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkDebugUtils.h"

namespace {

#ifdef SK_DEBUG
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
#endif

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
namespace DepthStencilOnlyBlock {

static const int kBlockDataSize = 0;

void AddToKey(SkBackend /* backend */, SkPaintParamsKey* key) {
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

static const int kBlockDataSize = 0;

void AddToKey(SkBackend /* backend */, SkPaintParamsKey* key) {
    int headerOffset = key->beginBlock(CodeSnippetID::kSolidColorShader);
    key->endBlock(headerOffset, CodeSnippetID::kSolidColorShader);

    validate_block_header(*key, headerOffset,
                          CodeSnippetID::kSolidColorShader, kBlockDataSize);
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {
    validate_block_header(key, headerOffset,
                          CodeSnippetID::kSolidColorShader, kBlockDataSize);

    SkDebugf("kSolidColorShader\n");
}
#endif

} // namespace SolidColorShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

static const int kBlockDataSize = 1;

void AddToKey(SkBackend backend,
              SkPaintParamsKey *key,
              const GradientData& gradData) {

    if (backend == SkBackend::kGraphite) {
        CodeSnippetID codeSnippetID = CodeSnippetID::kSolidColorShader;
        switch (gradData.fType) {
            case SkShader::kLinear_GradientType:
                codeSnippetID = CodeSnippetID::kLinearGradientShader;
                break;
            case SkShader::kRadial_GradientType:
                codeSnippetID = CodeSnippetID::kRadialGradientShader;
                break;
            case SkShader::kSweep_GradientType:
                codeSnippetID = CodeSnippetID::kSweepGradientShader;
                break;
            case SkShader::GradientType::kConical_GradientType:
                codeSnippetID = CodeSnippetID::kConicalGradientShader;
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
    } else {
        // TODO: add implementation of other backends
        SolidColorShaderBlock::AddToKey(backend, key);
    }
}

#ifdef SK_DEBUG
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

void Dump(const SkPaintParamsKey& key, int headerOffset) {
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
}
#endif

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
namespace BlendModeBlock {

static const int kBlockDataSize = 1;

void AddToKey(SkBackend /* backend */, SkPaintParamsKey *key, SkBlendMode bm) {

    int headerOffset = key->beginBlock(CodeSnippetID::kSimpleBlendMode);
    add_blendmode_to_key(key, bm);
    key->endBlock(headerOffset, CodeSnippetID::kSimpleBlendMode);

    validate_block_header(*key, headerOffset,
                          CodeSnippetID::kSimpleBlendMode, kBlockDataSize);
}

#ifdef SK_DEBUG
SkBlendMode ExtractFromKey(const SkPaintParamsKey& key, uint32_t headerOffset) {
    validate_block_header(key, headerOffset,
                          CodeSnippetID::kSimpleBlendMode, kBlockDataSize);

    uint8_t data = key.byte(headerOffset + SkPaintParamsKey::kBlockHeaderSizeInBytes);
    return to_blendmode(data);
}

void Dump(const SkPaintParamsKey& key, int headerOffset) {
    SkBlendMode bm = ExtractFromKey(key, headerOffset);

    SkDebugf("kSimpleBlendMode: %s\n", SkBlendMode_Name(bm));
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
            DepthStencilOnlyBlock::AddToKey(backend, &key);
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(backend, &key);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(backend, &key,
                                           { SkShader::kLinear_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(backend, &key,
                                           { SkShader::kRadial_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(backend, &key,
                                           { SkShader::kSweep_GradientType, tm, 0 });
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(backend, &key,
                                           { SkShader::kConical_GradientType, tm, 0 });
            break;
    }

    BlendModeBlock::AddToKey(backend, &key, bm);
    return key;
}
#endif
