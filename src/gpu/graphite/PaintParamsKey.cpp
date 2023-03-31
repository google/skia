/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParamsKey.h"

#include <cstring>
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
static PaintParamsKey::Header read_header(SkSpan<const uint8_t> parentSpan, int headerOffset) {
    SkASSERT(headerOffset + sizeof(PaintParamsKey::Header) <= parentSpan.size());

    PaintParamsKey::Header header;
    memcpy(&header, &parentSpan[headerOffset], sizeof(PaintParamsKey::Header));
    SkASSERT(header.blockSize >= sizeof(PaintParamsKey::Header));
    SkASSERT(headerOffset + header.blockSize <= static_cast<int>(parentSpan.size()));

    return header;
}

//--------------------------------------------------------------------------------------------------
PaintParamsKeyBuilder::PaintParamsKeyBuilder(const ShaderCodeDictionary* dict)
        : fDict(dict) {}

#ifdef SK_DEBUG
void PaintParamsKeyBuilder::checkReset() {
    SkASSERT(!this->isLocked());
    SkASSERT(this->sizeInBytes() == 0);
    SkASSERT(fIsValid);
    SkASSERT(fStack.empty());
}
#endif

// Block headers have the following structure:
//   4 bytes: code-snippet ID
//   1 byte: size of the block, in bytes (header, plus all data payload bytes)
// This call stores the header's offset in the key on the stack to be used in 'endBlock'
void PaintParamsKeyBuilder::beginBlock(int32_t codeSnippetID) {
    if (!this->isValid()) {
        return;
    }

    if (!fDict->isValidID(codeSnippetID)) {
        // SKGPU_LOG_W("Unknown code snippet ID.");
        this->makeInvalid();
        return;
    }

#ifdef SK_DEBUG
    if (!fStack.empty()) {
        // The children of a block should appear before any of the parent's data
        fStack.back().fNumActualChildren++;
    }
#endif

    SkASSERT(!this->isLocked());

    fStack.push_back({ codeSnippetID, this->sizeInBytes() });

    memcpy(fData.append(sizeof(int32_t)), &codeSnippetID, sizeof(int32_t));
    fData.push_back(0); // block size, this will be filled in when endBlock is called

#ifdef SK_DEBUG
    const ShaderSnippet* snippet = fDict->getEntry(codeSnippetID);

    fStack.back().fNumExpectedChildren = snippet->fNumChildren;
    fStack.back().fNumActualChildren = 0;
#endif
}

// Update the size byte of a block header
void PaintParamsKeyBuilder::endBlock() {
    if (!this->isValid()) {
        return;
    }

    if (fStack.empty()) {
        // SKGPU_LOG_W("Mismatched beginBlock/endBlocks.");
        this->makeInvalid();
        return;
    }

    // All the expected fields should be filled in at this point
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    SkASSERT(!this->isLocked());

    int headerOffset = fStack.back().fHeaderOffset;

#ifdef SK_DEBUG
    // We don't use `read_header` here because the header block size isn't valid yet.
    PaintParamsKey::Header header;
    memcpy(&header, &fData[headerOffset], sizeof(PaintParamsKey::Header));
    SkASSERT(header.codeSnippetID == fStack.back().fCodeSnippetID);
    SkASSERT(header.blockSize == 0);
#endif

    int blockSize = this->sizeInBytes() - headerOffset;
    if (blockSize > PaintParamsKey::kMaxBlockSize) {
        // SKGPU_LOG_W("Key's data payload is too large.");
        this->makeInvalid();
        return;
    }

    fData[headerOffset + PaintParamsKey::kBlockSizeOffsetInBytes] = blockSize;

    fStack.pop_back();
}

PaintParamsKey PaintParamsKeyBuilder::lockAsKey() {
    if (!fStack.empty()) {
        // SKGPU_LOG_W("Mismatched beginBlock/endBlocks.");
        this->makeInvalid();  // fall through
    }

    SkASSERT(!this->isLocked());

    // Partially reset for reuse. Note that the key resulting from this call will be holding a lock
    // on this builder and must be deleted before this builder is fully reset.
    fIsValid = true;
    fStack.clear();

    return PaintParamsKey(SkSpan(fData.begin(), fData.size()), this);
}

void PaintParamsKeyBuilder::makeInvalid() {
    SkASSERT(fIsValid);
    SkASSERT(!this->isLocked());

    fStack.clear();
    fData.clear();
    this->beginBlock(BuiltInCodeSnippetID::kError);
    this->endBlock();

    SkASSERT(fIsValid);
    fIsValid = false;
}

//--------------------------------------------------------------------------------------------------
PaintParamsKey::PaintParamsKey(SkSpan<const uint8_t> span,
                               PaintParamsKeyBuilder* originatingBuilder)
        : fData(span)
        , fOriginatingBuilder(originatingBuilder) {
    fOriginatingBuilder->lock();
}

PaintParamsKey::PaintParamsKey(SkSpan<const uint8_t> rawData)
        : fData(rawData)
        , fOriginatingBuilder(nullptr) {
}

PaintParamsKey::~PaintParamsKey() {
    if (fOriginatingBuilder) {
        fOriginatingBuilder->unlock();
    }
}

bool PaintParamsKey::operator==(const PaintParamsKey& that) const {
    return fData.size() == that.fData.size() &&
           !memcmp(fData.data(), that.fData.data(), fData.size());
}

PaintParamsKey::BlockReader PaintParamsKey::reader(const ShaderCodeDictionary* dict,
                                                   int headerOffset) const {
    return BlockReader(dict, fData, headerOffset);
}

#ifdef SK_DEBUG

// This just iterates over the top-level blocks calling block-specific dump methods.
void PaintParamsKey::dump(const ShaderCodeDictionary* dict) const {
    SkDebugf("--------------------------------------\n");
    SkDebugf("PaintParamsKey (%dB):\n", this->sizeInBytes());

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        BlockReader reader = this->reader(dict, curHeaderOffset);
        reader.dump(dict, /*indent=*/0);
        curHeaderOffset += reader.blockSize();
    }
}
#endif // SK_DEBUG

constexpr skgpu::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                 skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
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

static void add_block_to_shader_info(const ShaderCodeDictionary* dict,
                                     const PaintParamsKey::BlockReader& reader,
                                     ShaderInfo* result) {
    result->add(reader);
    result->addFlags(dict->getEntry(reader.codeSnippetId())->fSnippetRequirementFlags);

    if (reader.codeSnippetId() < kBuiltInCodeSnippetIDCount &&
        reader.codeSnippetId() >= kFixedFunctionBlendModeIDOffset) {
        SkASSERT(reader.numChildren() == 0);

        // Set SkBlendInfo on the ShaderInfo if the block represents a fixed function blend. The
        // key and paint are defined in such a way that this can occur at most once.
        int coeffBlendMode = reader.codeSnippetId() - kFixedFunctionBlendModeIDOffset;
        SkASSERT(coeffBlendMode >= 0 &&
                 static_cast<SkBlendMode>(coeffBlendMode) <= SkBlendMode::kLastCoeffMode);
        result->setBlendInfo(gBlendTable[coeffBlendMode]);
    } else {
        // The child blocks appear right after the parent block's header in the key and go
        // right after the parent's SnippetEntry in the shader info
        for (int i = 0; i < reader.numChildren(); ++i) {
            add_block_to_shader_info(dict, reader.child(dict, i), result);
        }
    }
}

void PaintParamsKey::toShaderInfo(const ShaderCodeDictionary* dict,
                                  ShaderInfo* result) const {
    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        BlockReader reader = this->reader(dict, curHeaderOffset);
        add_block_to_shader_info(dict, reader, result);
        curHeaderOffset += reader.blockSize();
    }
}

#if GRAPHITE_TEST_UTILS
bool PaintParamsKey::isErrorKey() const {
    if (this->sizeInBytes() != sizeof(Header)) {
        return false;
    }
    Header header = read_header(this->asSpan(), /*headerOffset=*/0);
    return header.codeSnippetID == (int32_t) BuiltInCodeSnippetID::kError &&
           header.blockSize == sizeof(Header);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

PaintParamsKey::BlockReader::BlockReader(const ShaderCodeDictionary* dict,
                                         SkSpan<const uint8_t> parentSpan,
                                         int offsetInParent) {
    Header header = read_header(parentSpan, offsetInParent);

    fBlock = parentSpan.subspan(offsetInParent, header.blockSize);
    fEntry = dict->getEntry(header.codeSnippetID);
    SkASSERT(fEntry);
}

int PaintParamsKey::BlockReader::numChildren() const { return fEntry->fNumChildren; }

PaintParamsKey::BlockReader PaintParamsKey::BlockReader::child(
        const ShaderCodeDictionary* dict,
        int childIndex) const {
    SkASSERT(childIndex < fEntry->fNumChildren);

    int childOffset = sizeof(Header);
    for (int i = 0; i < childIndex; ++i) {
        Header header = read_header(fBlock, childOffset);
        childOffset += header.blockSize;
    }

    return BlockReader(dict, fBlock, childOffset);
}

int32_t PaintParamsKey::BlockReader::codeSnippetId() const {
    Header header = read_header(fBlock, 0);
    return header.codeSnippetID;
}

#ifdef SK_DEBUG

static void output_indent(int indent) {
    SkDebugf("%*c", 4 * indent, ' ');
}

void PaintParamsKey::BlockReader::dump(const ShaderCodeDictionary* dict, int indent) const {
    uint8_t id = static_cast<uint8_t>(this->codeSnippetId());
    uint8_t blockSize = this->blockSize();

    auto entry = dict->getEntry(id);
    if (!entry) {
        output_indent(indent);
        SkDebugf("unknown block! (%dB)\n", blockSize);
    }

    output_indent(indent);
    SkDebugf("%s block (%dB)\n", entry->fStaticFunctionName, blockSize);

    for (int i = 0; i < this->numChildren(); ++i) {
        output_indent(indent);
        // TODO: it would be nice if the names of the children were also stored (i.e., "src"/"dst")
        SkDebugf("child %d:\n", i);

        PaintParamsKey::BlockReader childReader = this->child(dict, i);
        childReader.dump(dict, indent+1);
    }
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
