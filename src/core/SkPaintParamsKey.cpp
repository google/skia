/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintParamsKey.h"

#include <cstring>
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fNumBytes == that.fNumBytes &&
           !memcmp(fData.data(), that.fData.data(), fNumBytes);
}

#ifdef SK_DEBUG
typedef void (*DumpMethod)(const SkPaintParamsKey&, int headerOffset);

namespace {

void dump_unknown_block(const SkPaintParamsKey& key, int headerOffset) {
    uint8_t id = key.byte(headerOffset);
    uint8_t blockSize = key.byte(headerOffset+1);
    SkASSERT(blockSize >= 2 && headerOffset+blockSize <= key.sizeInBytes());

    SkDebugf("Unknown block - id: %d size: %dB\n", id, blockSize);
}

DumpMethod get_dump_method(SkBuiltInCodeSnippetID id) {
    switch (id) {
        case SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw:  return DepthStencilOnlyBlock::Dump;

        // SkShader code snippets
        case SkBuiltInCodeSnippetID::kSolidColorShader:      return SolidColorShaderBlock::Dump;

        case SkBuiltInCodeSnippetID::kLinearGradientShader:  [[fallthrough]];
        case SkBuiltInCodeSnippetID::kRadialGradientShader:  [[fallthrough]];
        case SkBuiltInCodeSnippetID::kSweepGradientShader:   [[fallthrough]];
        case SkBuiltInCodeSnippetID::kConicalGradientShader: return GradientShaderBlocks::Dump;

        case SkBuiltInCodeSnippetID::kImageShader:           return ImageShaderBlock::Dump;
        case SkBuiltInCodeSnippetID::kBlendShader:           return BlendShaderBlock::Dump;

        // BlendMode code snippets
        case SkBuiltInCodeSnippetID::kSimpleBlendMode:       return BlendModeBlock::Dump;

        default:                                             return dump_unknown_block;
    }
}

} // anonymous namespace

int SkPaintParamsKey::DumpBlock(const SkPaintParamsKey& key, int headerOffset) {
    auto [codeSnippetID, blockSize] = key.readCodeSnippetID(headerOffset);

    get_dump_method(codeSnippetID)(key, headerOffset);

    return blockSize;
}

// This just iterates over the top-level blocks calling block-specific dump methods.
void SkPaintParamsKey::dump() const {
    SkDebugf("SkPaintParamsKey %dB:\n", this->sizeInBytes());

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        int blockSize = DumpBlock(*this, curHeaderOffset);
        curHeaderOffset += blockSize;
    }
}
#endif

int SkPaintParamsKey::AddBlockToShaderInfo(SkShaderCodeDictionary* dict,
                                           const SkPaintParamsKey& key,
                                           int headerOffset,
                                           SkShaderInfo* result) {
    auto [codeSnippetID, blockSize] = key.readCodeSnippetID(headerOffset);

    if (codeSnippetID != SkBuiltInCodeSnippetID::kSimpleBlendMode) {
        auto entry = dict->getEntry(codeSnippetID);

        result->add(*entry);

        if (codeSnippetID != SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw) {
            result->setWritesColor();
        }
    }

    return blockSize;
}

void SkPaintParamsKey::toShaderInfo(SkShaderCodeDictionary* dict, SkShaderInfo* result) const {

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        int blockSize = AddBlockToShaderInfo(dict, *this, curHeaderOffset, result);
        curHeaderOffset += blockSize;
    }
}
