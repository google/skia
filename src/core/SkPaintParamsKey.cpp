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

//--------------------------------------------------------------------------------------------------
SkPaintParamsKeyBuilder::SkPaintParamsKeyBuilder(const SkShaderCodeDictionary* dict)
        : fDict(dict) {
}

// Block headers have the following structure:
//  1st byte: codeSnippetID
//  2nd byte: total blockSize in bytes
// This call stores the header's offset in the key on the stack to be used in 'endBlock'
void SkPaintParamsKeyBuilder::beginBlock(int codeSnippetID) {
    if (!this->isValid()) {
        return;
    }

    if (codeSnippetID < 0 || codeSnippetID > fDict->maxCodeSnippetID()) {
        this->makeInvalid();
        return;
    }

    fData.reserve_back(SkPaintParamsKey::kBlockHeaderSizeInBytes);
    fStack.push_back({ codeSnippetID, this->sizeInBytes() });

    this->addByte(SkTo<uint8_t>(codeSnippetID));
    this->addByte(0);  // this needs to be patched up with a call to endBlock
}

// Update the size byte of a block header
void SkPaintParamsKeyBuilder::endBlock() {
    if (!this->isValid()) {
        return;
    }

    if (fStack.empty()) {
        SkASSERT(0);
        this->makeInvalid();
        return;
    }

    int headerOffset = fStack.back().fHeaderOffset;

    SkASSERT(fData[headerOffset] == fStack.back().fCodeSnippetID);
    SkASSERT(fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] == 0);

    int blockSize = this->sizeInBytes() - headerOffset;
    if (blockSize > SkPaintParamsKey::kMaxBlockSize) {
        this->makeInvalid();
        return;
    }

    fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] = blockSize;

    fStack.pop_back();
}

void SkPaintParamsKeyBuilder::addBytes(uint32_t numBytes, const uint8_t* data) {
    if (!this->isValid()) {
        return;
    }

    fData.push_back_n(numBytes, data);
}

std::unique_ptr<SkPaintParamsKey> SkPaintParamsKeyBuilder::snap() {
    if (!fStack.empty()) {
        this->makeInvalid();  // fall through
    }

    auto key = std::unique_ptr<SkPaintParamsKey>(new SkPaintParamsKey(std::move(fData)));

    // Reset for reuse
    fIsValid = true;
    fStack.clear();

    return key;
}

void SkPaintParamsKeyBuilder::makeInvalid() {
    SkASSERT(fIsValid);

    fStack.clear();
    fData.reset();
    this->beginBlock(SkBuiltInCodeSnippetID::kError);
    this->endBlock();

    SkASSERT(fIsValid);
    fIsValid = false;
}

//--------------------------------------------------------------------------------------------------
SkPaintParamsKey::SkPaintParamsKey(SkTArray<uint8_t, true>&& data) : fData(std::move(data)) {}

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fData == that.fData;
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

        // The child blocks appear right after the parent block's header in the key and go
        // right after the parent's SnippetEntry in the shader info
        int childOffset = headerOffset + kBlockHeaderSizeInBytes;
        for (int i = 0; i < entry->fNumChildren; ++i) {
            SkASSERT(childOffset < headerOffset + blockSize);

            int childBlockSize = AddBlockToShaderInfo(dict, key, childOffset, result);
            childOffset += childBlockSize;
        }

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
