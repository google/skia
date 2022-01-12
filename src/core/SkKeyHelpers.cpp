/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "include/private/SkPaintParamsKey.h"
#include "src/core/SkDebugUtils.h"

//--------------------------------------------------------------------------------------------------
namespace DepthStencilOnlyBlock {

void AddToKey(SkPaintParamsKey* key) {
    int blockIndex = key->beginBlock(CodeSnippetID::kDepthStencilOnlyDraw);
    key->endBlock(blockIndex, CodeSnippetID::kDepthStencilOnlyDraw);
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kDepthStencilOnlyDraw);
    SkASSERT(key.byte(headerOffset+1) == 2);

    SkDebugf("kDepthStencilOnlyDraw\n");
}
#endif

} // namespace DepthStencilOnlyBlock

//--------------------------------------------------------------------------------------------------
namespace SolidColorShaderBlock {

void AddToKey(SkPaintParamsKey* key) {
    int blockIndex = key->beginBlock(CodeSnippetID::kSolidColorShader);
    key->endBlock(blockIndex, CodeSnippetID::kSolidColorShader);
}

#ifdef SK_DEBUG
void Dump(const SkPaintParamsKey& key, int headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kSolidColorShader);
    SkASSERT(key.byte(headerOffset+1) == 2);

    SkDebugf("kSolidColorShader\n");
}
#endif

} // namespace SolidColorShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

void AddToKey(SkPaintParamsKey *key, SkShader::GradientType type, SkTileMode tm) {

    CodeSnippetID id = CodeSnippetID::kSolidColorShader;
    switch (type) {
        case SkShader::kLinear_GradientType:
            id = CodeSnippetID::kLinearGradientShader;
            break;
        case SkShader::kRadial_GradientType:
            id = CodeSnippetID::kRadialGradientShader;
            break;
        case SkShader::kSweep_GradientType:
            id = CodeSnippetID::kSweepGradientShader;
            break;
        case SkShader::GradientType::kConical_GradientType:
            id = CodeSnippetID::kConicalGradientShader;
            break;
        case SkShader::GradientType::kColor_GradientType:
        case SkShader::GradientType::kNone_GradientType:
        default:
            SkASSERT(0);
            break;
    }

    int blockIndex = key->beginBlock(id);

    key->addByte(static_cast<uint8_t>(tm));

    key->endBlock(blockIndex, id);
}

#ifdef SK_DEBUG
std::pair<CodeSnippetID, SkTileMode> ExtractFromKey(const SkPaintParamsKey& key,
                                                    uint32_t headerOffset) {
    CodeSnippetID id = static_cast<CodeSnippetID>(key.byte(headerOffset));

    SkASSERT(id == CodeSnippetID::kLinearGradientShader ||
             id == CodeSnippetID::kRadialGradientShader ||
             id == CodeSnippetID::kSweepGradientShader ||
             id == CodeSnippetID::kConicalGradientShader);
    SkASSERT(key.byte(headerOffset+1) == 3);

    SkTileMode tm = static_cast<SkTileMode>(key.byte(headerOffset+2));

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

void AddToKey(SkPaintParamsKey *key, SkBlendMode bm) {
    int blockIndex = key->beginBlock(CodeSnippetID::kSimpleBlendMode);

    key->addByte(static_cast<uint8_t>(bm));

    key->endBlock(blockIndex, CodeSnippetID::kSimpleBlendMode);
}

#ifdef SK_DEBUG
SkBlendMode ExtractFromKey(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kSimpleBlendMode);
    SkASSERT(key.byte(headerOffset+1) == 3);
    return static_cast<SkBlendMode>(key.byte(headerOffset+2));
}

void Dump(const SkPaintParamsKey& key, int headerOffset) {
    SkBlendMode bm = ExtractFromKey(key, headerOffset);

    SkDebugf("kSimpleBlendMode: %s\n", SkBlendMode_Name(bm));
}
#endif

} // namespace BlendModeBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkPaintParamsKey CreateKey(skgpu::ShaderCombo::ShaderType s, SkTileMode tm, SkBlendMode bm) {
    SkPaintParamsKey key;

    switch (s) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            DepthStencilOnlyBlock::AddToKey(&key);
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(&key);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(&key, SkShader::kLinear_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(&key, SkShader::kRadial_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(&key, SkShader::kSweep_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(&key, SkShader::kConical_GradientType, tm);
            break;
    }

    BlendModeBlock::AddToKey(&key, bm);
    return key;
}
#endif
