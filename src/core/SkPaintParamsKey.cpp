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

using DataPayloadType  = SkPaintParamsKey::DataPayloadType;
using DataPayloadField = SkPaintParamsKey::DataPayloadField;

//--------------------------------------------------------------------------------------------------
static SkPaintParamsKey::Header read_header(SkSpan<const uint8_t> parentSpan, int headerOffset) {
    SkASSERT(headerOffset + sizeof(SkPaintParamsKey::Header) <= parentSpan.size());

    SkPaintParamsKey::Header header;
    memcpy(&header, &parentSpan[headerOffset], sizeof(SkPaintParamsKey::Header));
    SkASSERT(header.blockSize >= sizeof(SkPaintParamsKey::Header));
    SkASSERT(headerOffset + header.blockSize <= static_cast<int>(parentSpan.size()));

    return header;
}

//--------------------------------------------------------------------------------------------------
SkPaintParamsKeyBuilder::SkPaintParamsKeyBuilder(const SkShaderCodeDictionary* dict)
        : fDict(dict) {}

#ifdef SK_DEBUG
void SkPaintParamsKeyBuilder::checkReset() {
    SkASSERT(!this->isLocked());
    SkASSERT(this->sizeInBytes() == 0);
    SkASSERT(fIsValid);
    SkASSERT(fStack.empty());
#ifdef SK_GRAPHITE_ENABLED
    SkASSERT(fBlendInfo == skgpu::BlendInfo());
#endif
}
#endif

// Block headers have the following structure:
//   4 bytes: code-snippet ID
//   1 byte: size of the block, in bytes (header, plus all data payload bytes)
// This call stores the header's offset in the key on the stack to be used in 'endBlock'
void SkPaintParamsKeyBuilder::beginBlock(int32_t codeSnippetID) {
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
        SkASSERT(fStack.back().fCurDataPayloadEntry == 0);
        fStack.back().fNumActualChildren++;
    }

    static constexpr DataPayloadField kHeader[2] = {
            {"snippetID", DataPayloadType::kInt, 1},
            {"blockSize", DataPayloadType::kByte, 1},
    };

    static const SkSpan<const DataPayloadField> kHeaderExpectations(kHeader);
#endif

    SkASSERT(!this->isLocked());

    fStack.push_back({ codeSnippetID, this->sizeInBytes(),
                       SkDEBUGCODE(kHeaderExpectations, 0) });

    this->addInt(codeSnippetID);
    this->addByte(0);  // this will be filled in when endBlock is called

#ifdef SK_DEBUG
    const SkShaderSnippet* snippet = fDict->getEntry(codeSnippetID);

    fStack.back().fDataPayloadExpectations = snippet->fDataPayloadExpectations;
    fStack.back().fCurDataPayloadEntry = 0;
    fStack.back().fNumExpectedChildren = snippet->fNumChildren;
    fStack.back().fNumActualChildren = 0;
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
    SkASSERT(fStack.back().fNumActualChildren == fStack.back().fNumExpectedChildren);
    SkASSERT(!this->isLocked());

    int headerOffset = fStack.back().fHeaderOffset;

#ifdef SK_DEBUG
    // We don't use `read_header` here because the header block size isn't valid yet.
    SkPaintParamsKey::Header header;
    memcpy(&header, &fData[headerOffset], sizeof(SkPaintParamsKey::Header));
    SkASSERT(header.codeSnippetID == fStack.back().fCodeSnippetID);
    SkASSERT(header.blockSize == 0);
#endif

    int blockSize = this->sizeInBytes() - headerOffset;
    if (blockSize > SkPaintParamsKey::kMaxBlockSize) {
        // SKGPU_LOG_W("Key's data payload is too large.");
        this->makeInvalid();
        return;
    }

    fData[headerOffset + SkPaintParamsKey::kBlockSizeOffsetInBytes] = blockSize;

    fStack.pop_back();

#ifdef SK_DEBUG
    if (!fStack.empty()) {
        // The children of a block should appear before any of the parent's data
        SkASSERT(fStack.back().fCurDataPayloadEntry == 0);
    }
#endif
}

#ifdef SK_DEBUG
void SkPaintParamsKeyBuilder::checkExpectations(DataPayloadType actualType, uint32_t actualCount) {
    StackFrame& frame = fStack.back();
    const auto& expectations = frame.fDataPayloadExpectations;

    // TODO: right now we reject writing 'n' bytes one at a time. We could allow it by tracking
    // the number of bytes written in the stack frame.
    SkASSERT(expectations[frame.fCurDataPayloadEntry].fType == actualType);
    SkASSERT(expectations[frame.fCurDataPayloadEntry].fCount == actualCount);

    frame.fCurDataPayloadEntry++;
}
#endif // SK_DEBUG

static int field_size(DataPayloadType type) {
    switch (type) {
        case DataPayloadType::kByte:   return 1;
        case DataPayloadType::kInt:    return 4;
        case DataPayloadType::kFloat4: return 16;
    }
    SkUNREACHABLE;
}

void SkPaintParamsKeyBuilder::addToKey(uint32_t count,
                                       const void* data,
                                       DataPayloadType payloadType) {
    if (!this->isValid()) {
        return;
    }

    if (fStack.empty()) {
        // SKGPU_LOG_W("Missing call to 'beginBlock'.");
        this->makeInvalid();
        return;
    }

    SkDEBUGCODE(this->checkExpectations(payloadType, count);)
    SkASSERT(!this->isLocked());

    fData.append(field_size(payloadType) * count, reinterpret_cast<const uint8_t*>(data));
}

void SkPaintParamsKeyBuilder::addBytes(uint32_t numBytes, const uint8_t* data) {
    this->addToKey(numBytes, data, DataPayloadType::kByte);
}

void SkPaintParamsKeyBuilder::addInts(uint32_t numInts, const int32_t* data) {
    this->addToKey(numInts, data, DataPayloadType::kInt);
}

void SkPaintParamsKeyBuilder::add(int numColors, const SkColor4f* colors) {
    this->addToKey(numColors, colors, DataPayloadType::kFloat4);
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

    return SkPaintParamsKey(SkSpan(fData.begin(), fData.count()), this);
}

void SkPaintParamsKeyBuilder::makeInvalid() {
    SkASSERT(fIsValid);
    SkASSERT(!this->isLocked());

    fStack.clear();
    fData.clear();
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
        reader.dump(dict, /*indent=*/0);
        curHeaderOffset += reader.blockSize();
    }
}
#endif // SK_DEBUG

void SkPaintParamsKey::AddBlockToShaderInfo(const SkShaderCodeDictionary* dict,
                                            const SkPaintParamsKey::BlockReader& reader,
                                            SkShaderInfo* result) {

    result->add(reader);
#ifdef SK_GRAPHITE_ENABLED
    result->addFlags(dict->getEntry(reader.codeSnippetId())->fSnippetRequirementFlags);
#endif

    // The child blocks appear right after the parent block's header in the key and go
    // right after the parent's SnippetEntry in the shader info
    for (int i = 0; i < reader.numChildren(); ++i) {
        SkPaintParamsKey::BlockReader childReader = reader.child(dict, i);

        AddBlockToShaderInfo(dict, childReader, result);
    }
}

void SkPaintParamsKey::toShaderInfo(const SkShaderCodeDictionary* dict,
                                    SkShaderInfo* result) const {

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        SkPaintParamsKey::BlockReader reader = this->reader(dict, curHeaderOffset);
        AddBlockToShaderInfo(dict, reader, result);
        curHeaderOffset += reader.blockSize();
    }
}

#if GR_TEST_UTILS
bool SkPaintParamsKey::isErrorKey() const {
    if (this->sizeInBytes() != sizeof(Header)) {
        return false;
    }
    Header header = read_header(this->asSpan(), /*headerOffset=*/0);
    return header.codeSnippetID == (int32_t)SkBuiltInCodeSnippetID::kError &&
           header.blockSize == sizeof(Header);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

SkPaintParamsKey::BlockReader::BlockReader(const SkShaderCodeDictionary* dict,
                                           SkSpan<const uint8_t> parentSpan,
                                           int offsetInParent) {
    Header header = read_header(parentSpan, offsetInParent);

    fBlock = parentSpan.subspan(offsetInParent, header.blockSize);
    fEntry = dict->getEntry(header.codeSnippetID);
    SkASSERT(fEntry);
}

int SkPaintParamsKey::BlockReader::numChildren() const { return fEntry->fNumChildren; }

SkPaintParamsKey::BlockReader SkPaintParamsKey::BlockReader::child(
        const SkShaderCodeDictionary* dict,
        int childIndex) const {
    SkASSERT(childIndex < fEntry->fNumChildren);

    int childOffset = sizeof(Header);
    for (int i = 0; i < childIndex; ++i) {
        Header header = read_header(fBlock, childOffset);
        childOffset += header.blockSize;
    }

    return BlockReader(dict, fBlock, childOffset);
}

int32_t SkPaintParamsKey::BlockReader::codeSnippetId() const {
    Header header = read_header(fBlock, 0);
    return header.codeSnippetID;
}

SkSpan<const uint8_t> SkPaintParamsKey::BlockReader::dataPayload() const {
    int payloadOffset = sizeof(Header);
    for (int i = 0; i < fEntry->fNumChildren; ++i) {
        Header header = read_header(fBlock, payloadOffset);
        payloadOffset += header.blockSize;
    }

    int payloadSize = this->blockSize() - payloadOffset;
    return fBlock.subspan(payloadOffset, payloadSize);
}

static int field_offset(SkSpan<const DataPayloadField> fields, int fieldIndex) {
    int byteOffset = 0;
    for (int i = 0; i < fieldIndex; ++i) {
        byteOffset += field_size(fields[i].fType) * fields[i].fCount;
    }
    return byteOffset;
}

template <typename T>
static SkSpan<const T> payload_subspan_for_field(SkSpan<const uint8_t> dataPayload,
                                                 SkSpan<const DataPayloadField> fields,
                                                 int fieldIndex) {
    int offset = field_offset(fields, fieldIndex);
    return {reinterpret_cast<const T*>(&dataPayload[offset]), fields[fieldIndex].fCount};
}

SkSpan<const uint8_t> SkPaintParamsKey::BlockReader::bytes(int fieldIndex) const {
    SkASSERT(fEntry->fDataPayloadExpectations[fieldIndex].fType == DataPayloadType::kByte);
    return payload_subspan_for_field<uint8_t>(this->dataPayload(),
                                              fEntry->fDataPayloadExpectations,
                                              fieldIndex);
}

SkSpan<const int32_t> SkPaintParamsKey::BlockReader::ints(int fieldIndex) const {
    SkASSERT(fEntry->fDataPayloadExpectations[fieldIndex].fType == DataPayloadType::kInt);
    return payload_subspan_for_field<int32_t>(this->dataPayload(),
                                              fEntry->fDataPayloadExpectations,
                                              fieldIndex);
}

SkSpan<const SkColor4f> SkPaintParamsKey::BlockReader::colors(int fieldIndex) const {
    SkASSERT(fEntry->fDataPayloadExpectations[fieldIndex].fType == DataPayloadType::kFloat4);
    return payload_subspan_for_field<SkColor4f>(this->dataPayload(),
                                                fEntry->fDataPayloadExpectations,
                                                fieldIndex);
}

#ifdef SK_DEBUG

int SkPaintParamsKey::BlockReader::numDataPayloadFields() const {
    return fEntry->fDataPayloadExpectations.size();
}

static void output_indent(int indent) {
    SkDebugf("%*c", 4 * indent, ' ');
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
