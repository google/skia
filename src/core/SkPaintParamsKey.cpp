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
SkPaintParamsKeyBuilder::SkPaintParamsKeyBuilder(const SkShaderCodeDictionary* dict,
                                                 SkBackend backend)
        : fDict(dict)
        , fBackend(backend) {
}

#ifdef SK_DEBUG
void SkPaintParamsKeyBuilder::checkReset() {
    SkASSERT(!this->isLocked());
    SkASSERT(this->sizeInBytes() == 0);
    SkASSERT(fIsValid);
    SkASSERT(fStack.empty());
}
#endif

// Block headers have the following structure:
//  1st byte: codeSnippetID
//  2nd byte: total blockSize in bytes
// This call stores the header's offset in the key on the stack to be used in 'endBlock'
void SkPaintParamsKeyBuilder::beginBlock(int codeSnippetID) {
    if (!this->isValid()) {
        return;
    }

    if (codeSnippetID < 0 || codeSnippetID > fDict->maxCodeSnippetID()) {
        // SKGPU_LOG_W("Unknown code snippet ID.");
        this->makeInvalid();
        return;
    }

#ifdef SK_DEBUG
    if (!fStack.empty()) {
        // The children of a block should appear before any of the parent's data
        SkASSERT(fStack.back().fCurDataPayloadEntry == 0);
    }

    static const SkPaintParamsKey::DataPayloadField kHeader[2] = {
            {"snippetID", SkPaintParamsKey::DataPayloadType::kByte, 1},
            {"blockSize", SkPaintParamsKey::DataPayloadType::kByte, 1},
    };

    static const SkSpan<const SkPaintParamsKey::DataPayloadField> kHeaderExpectations(kHeader, 2);
#endif

    SkASSERT(!this->isLocked());

    fData.reserve_back(SkPaintParamsKey::kBlockHeaderSizeInBytes);
    fStack.push_back({ codeSnippetID, this->sizeInBytes(),
                       SkDEBUGCODE(kHeaderExpectations, 0) });

    this->addByte(SkTo<uint8_t>(codeSnippetID));
    this->addByte(0);  // this needs to be patched up with a call to endBlock

#ifdef SK_DEBUG
    fStack.back().fDataPayloadExpectations = fDict->dataPayloadExpectations(codeSnippetID);
    fStack.back().fCurDataPayloadEntry = 0;
#endif
}

// Update the size byte of a block header
void SkPaintParamsKeyBuilder::endBlock() {
    if (!this->isValid()) {
        return;
    }

    if (fStack.empty()) {
        // SKGPU_LOG_W("Mismatched beginBlock/endBlocks.");
        this->makeInvalid();
        return;
    }

    // All the expected fields should be filled in at this point
    SkASSERT(fStack.back().fCurDataPayloadEntry ==
             SkTo<int>(fStack.back().fDataPayloadExpectations.size()));
    SkASSERT(!this->isLocked());

    int headerOffset = fStack.back().fHeaderOffset;

    SkASSERT(fData[headerOffset] == fStack.back().fCodeSnippetID);
    SkASSERT(fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] == 0);

    int blockSize = this->sizeInBytes() - headerOffset;
    if (blockSize > SkPaintParamsKey::kMaxBlockSize) {
        // SKGPU_LOG_W("Key's data payload is too large.");
        this->makeInvalid();
        return;
    }

    fData[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes] = blockSize;

    fStack.pop_back();

#ifdef SK_DEBUG
    if (!fStack.empty()) {
        // The children of a block should appear before any of the parent's data
        SkASSERT(fStack.back().fCurDataPayloadEntry == 0);
    }
#endif
}

void SkPaintParamsKeyBuilder::addBytes(uint32_t numBytes, const uint8_t* data) {
    if (!this->isValid()) {
        return;
    }

    if (fStack.empty()) {
        // SKGPU_LOG_W("Missing call to 'beginBlock'.");
        this->makeInvalid();
        return;
    }

#ifdef SK_DEBUG
    const StackFrame& frame = fStack.back();
    const auto& expectations = frame.fDataPayloadExpectations;

    // TODO: right now we reject writing 'n' bytes one at a time. We could allow it by tracking
    // the number of bytes written in the stack frame.
    SkASSERT(frame.fCurDataPayloadEntry < SkTo<int>(expectations.size()) &&
             expectations.data() &&
             expectations[frame.fCurDataPayloadEntry].fType ==
                                                        SkPaintParamsKey::DataPayloadType::kByte &&
             expectations[frame.fCurDataPayloadEntry].fCount == numBytes);

    fStack.back().fCurDataPayloadEntry++;
#endif

    SkASSERT(!this->isLocked());

    fData.push_back_n(numBytes, data);
}

SkPaintParamsKey SkPaintParamsKeyBuilder::lockAsKey() {
    if (!fStack.empty()) {
        // SKGPU_LOG_W("Mismatched beginBlock/endBlocks.");
        this->makeInvalid();  // fall through
    }

    SkASSERT(!this->isLocked());

    // Partially reset for reuse. Note that the key resulting from this call will be holding a lock
    // on this builder and must be deleted before this builder is fully reset.
    fIsValid = true;
    fStack.clear();

    return SkPaintParamsKey(SkMakeSpan(fData.data(), fData.count()), this);
}

void SkPaintParamsKeyBuilder::makeInvalid() {
    SkASSERT(fIsValid);
    SkASSERT(!this->isLocked());

    fStack.clear();
    fData.reset();
    this->beginBlock(SkBuiltInCodeSnippetID::kError);
    this->endBlock();

    SkASSERT(fIsValid);
    fIsValid = false;
}

//--------------------------------------------------------------------------------------------------
SkPaintParamsKey::SkPaintParamsKey(SkSpan<const uint8_t> span,
                                   SkPaintParamsKeyBuilder* originatingBuilder)
        : fData(span)
        , fOriginatingBuilder(originatingBuilder) {
    fOriginatingBuilder->lock();
}

SkPaintParamsKey::SkPaintParamsKey(SkSpan<const uint8_t> rawData)
        : fData(rawData)
        , fOriginatingBuilder(nullptr) {
}

SkPaintParamsKey::~SkPaintParamsKey() {
    if (fOriginatingBuilder) {
        fOriginatingBuilder->unlock();
    }
}

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fData.size() == that.fData.size() &&
           !memcmp(fData.data(), that.fData.data(), fData.size());
}

SkPaintParamsKey::BlockReader SkPaintParamsKey::reader(const SkShaderCodeDictionary* dict,
                                                       int headerOffset) const {
    return BlockReader(dict, fData, headerOffset);
}

#ifdef SK_DEBUG

// This just iterates over the top-level blocks calling block-specific dump methods.
void SkPaintParamsKey::dump(const SkShaderCodeDictionary* dict) const {
    SkDebugf("--------------------------------------\n");
    SkDebugf("SkPaintParamsKey (%dB):\n", this->sizeInBytes());

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        BlockReader reader = this->reader(dict, curHeaderOffset);
        reader.dump(dict, /* indent */ 0);
        curHeaderOffset += reader.blockSize();
    }
}
#endif // SK_DEBUG

void SkPaintParamsKey::AddBlockToShaderInfo(SkShaderCodeDictionary* dict,
                                            const SkPaintParamsKey::BlockReader& reader,
                                            SkShaderInfo* result) {

    result->add(reader);

    // The child blocks appear right after the parent block's header in the key and go
    // right after the parent's SnippetEntry in the shader info
    for (int i = 0; i < reader.numChildren(); ++i) {
        SkPaintParamsKey::BlockReader childReader = reader.child(dict, i);

        AddBlockToShaderInfo(dict, childReader, result);
    }
}

void SkPaintParamsKey::toShaderInfo(SkShaderCodeDictionary* dict, SkShaderInfo* result) const {

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        SkPaintParamsKey::BlockReader reader = this->reader(dict, curHeaderOffset);
        AddBlockToShaderInfo(dict, reader, result);
        curHeaderOffset += reader.blockSize();
    }
}

#if GR_TEST_UTILS
bool SkPaintParamsKey::isErrorKey() const {
    return this->sizeInBytes() == SkPaintParamsKey::kBlockHeaderSizeInBytes &&
           fData[0] == static_cast<int>(SkBuiltInCodeSnippetID::kError) &&
           fData[1] == SkPaintParamsKey::kBlockHeaderSizeInBytes;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

#ifdef SK_DEBUG
void output_indent(int indent) {
    for (int i = 0; i < indent; ++i) {
        SkDebugf("    ");
    }
}
#endif

std::pair<SkBuiltInCodeSnippetID, uint8_t> read_header(SkSpan<const uint8_t> parentSpan,
                                                       int headerOffset) {
    SkASSERT(headerOffset + SkPaintParamsKey::kBlockHeaderSizeInBytes <=
             SkTo<int>(parentSpan.size()));

    SkBuiltInCodeSnippetID id = static_cast<SkBuiltInCodeSnippetID>(parentSpan[headerOffset]);
    uint8_t blockSize = parentSpan[headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes];
    SkASSERT(blockSize >= SkPaintParamsKey::kBlockHeaderSizeInBytes);
    SkASSERT(headerOffset + blockSize <= static_cast<int>(parentSpan.size()));

    return { id, blockSize };
}

} // anonymous namespace

SkPaintParamsKey::BlockReader::BlockReader(const SkShaderCodeDictionary* dict,
                                           SkSpan<const uint8_t> parentSpan,
                                           int offsetInParent) {
    auto [codeSnippetID, blockSize] = read_header(parentSpan, offsetInParent);

    fBlock = parentSpan.subspan(offsetInParent, blockSize);
    fEntry = dict->getEntry(codeSnippetID);
    SkASSERT(fEntry);
}

int SkPaintParamsKey::BlockReader::numChildren() const { return fEntry->fNumChildren; }

SkPaintParamsKey::BlockReader SkPaintParamsKey::BlockReader::child(
        const SkShaderCodeDictionary* dict,
        int childIndex) const {
    SkASSERT(childIndex < fEntry->fNumChildren);

    int childOffset = kBlockHeaderSizeInBytes;
    for (int i = 0; i < childIndex; ++i) {
        auto [_, childBlockSize] = read_header(fBlock, childOffset);
        childOffset += childBlockSize;
    }

    return BlockReader(dict, fBlock, childOffset);
}

SkSpan<const uint8_t> SkPaintParamsKey::BlockReader::dataPayload() const {
    int payloadOffset = kBlockHeaderSizeInBytes;
    for (int i = 0; i < fEntry->fNumChildren; ++i) {
        auto [_, childBlockSize] = read_header(fBlock, payloadOffset);
        payloadOffset += childBlockSize;
    }

    int payloadSize = this->blockSize() - payloadOffset;
    return fBlock.subspan(payloadOffset, payloadSize);
}

SkSpan<const uint8_t> SkPaintParamsKey::BlockReader::bytes(int fieldIndex) const {
    SkASSERT(fEntry->fDataPayloadExpectations[fieldIndex].fType == DataPayloadType::kByte);

    int byteOffsetInPayload = 0;
    for (int i = 0; i < fieldIndex; ++i) {
        SkASSERT(fEntry->fDataPayloadExpectations[i].fType == DataPayloadType::kByte);
        byteOffsetInPayload += fEntry->fDataPayloadExpectations[i].fCount;
    }

    SkSpan<const uint8_t> dataPayload = this->dataPayload();
    return dataPayload.subspan(byteOffsetInPayload,
                               fEntry->fDataPayloadExpectations[fieldIndex].fCount);
}

#ifdef SK_DEBUG

int SkPaintParamsKey::BlockReader::numDataPayloadFields() const {
    return fEntry->fDataPayloadExpectations.size();
}

void SkPaintParamsKey::BlockReader::dump(const SkShaderCodeDictionary* dict, int indent) const {
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

        SkPaintParamsKey::BlockReader childReader = this->child(dict, i);
        childReader.dump(dict, indent+1);
    }

    for (int i = 0; i < (int) fEntry->fDataPayloadExpectations.size(); ++i) {
        output_indent(indent);
        SkDebugf("%s[%d]: ",
                 fEntry->fDataPayloadExpectations[i].fName,
                 fEntry->fDataPayloadExpectations[i].fCount);
        SkSpan<const uint8_t> bytes = this->bytes(i);
        for (uint8_t b : bytes) {
            SkDebugf("%d,", b);
        }

        SkDebugf("\n");
    }
}

#endif // SK_DEBUG
