/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintParamsKey_DEFINED
#define SkPaintParamsKey_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkBuiltInCodeSnippetID.h"

#include <array>
#include <limits>
#include <vector>

enum class SkBackend : uint8_t {
    kGanesh,
    kGraphite,
    kSkVM
};
struct SkDataPayloadInfo;
class SkPaintParamsKeyBuilder;
class SkShaderCodeDictionary;
class SkShaderInfo;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// header that consists of 2-bytes:
//   a 1-byte code-snippet ID
//   a 1-byte number-of-bytes-in-the-block field (incl. the space for the header)
// The rest of the data in the block is dependent on the individual code snippet.
// If a given block has child blocks, they appear in the key right after their
// parent block's header.
class SkPaintParamsKey {
public:
    static const int kBlockHeaderSizeInBytes = 2;
    static const int kBlockSizeOffsetInBytes = 1; // offset to the block size w/in the header
    static const int kMaxBlockSize = std::numeric_limits<uint8_t>::max();

    enum class DataPayloadType {
        kByte,
    };

    // A given snippet's data payload is stored as an SkSpan of DataPayloadFields in the
    // SkShaderCodeDictionary. That span just defines the structure of the data payload. The actual
    // data is stored in the paint params key.
    struct DataPayloadField {
        const char* fName;
        DataPayloadType fType;
        uint32_t fCount;
    };

    ~SkPaintParamsKey();

    std::pair<SkBuiltInCodeSnippetID, uint8_t> readCodeSnippetID(int headerOffset) const {
        SkASSERT(headerOffset <= this->sizeInBytes() - kBlockHeaderSizeInBytes);

        SkBuiltInCodeSnippetID id = static_cast<SkBuiltInCodeSnippetID>(fData[headerOffset]);
        uint8_t blockSize = fData[headerOffset+1];
        SkASSERT(headerOffset + blockSize <= this->sizeInBytes());

        return { id, blockSize };
    }

#ifdef SK_DEBUG
    uint8_t byte(int offset) const {
        SkASSERT(offset < this->sizeInBytes());
        return fData[offset];
    }
    void dump(const SkShaderCodeDictionary*) const;
#endif
    void toShaderInfo(SkShaderCodeDictionary*, SkShaderInfo*) const;

    SkSpan<const uint8_t> asSpan() const { return fData; }
    const uint8_t* data() const { return fData.data(); }
    int sizeInBytes() const { return SkTo<int>(fData.size()); }

    bool operator==(const SkPaintParamsKey& that) const;
    bool operator!=(const SkPaintParamsKey& that) const { return !(*this == that); }

#if GR_TEST_UTILS
    bool isErrorKey() const;
#endif

private:
    friend class SkPaintParamsKeyBuilder;   // for the parented-data ctor
    friend class SkShaderCodeDictionary;    // for the raw-data ctor

    // This ctor is to be used when paintparams keys are being consecutively generated
    // by a key builder. The memory backing this key's span is shared between the
    // builder and its keys.
    SkPaintParamsKey(SkSpan<const uint8_t> span, SkPaintParamsKeyBuilder* originatingBuilder);

    // This ctor is used when this key isn't being created by a builder (i.e., when the key
    // is in the dictionary). In this case the dictionary will own the memory backing the span.
    SkPaintParamsKey(SkSpan<const uint8_t> rawData);

    static int AddBlockToShaderInfo(SkShaderCodeDictionary*,
                                    const SkPaintParamsKey&,
                                    int headerOffset,
                                    SkShaderInfo*);

    // The memory referenced in 'fData' is always owned by someone else.
    // If 'fOriginatingBuilder' is null, the dictionary's SkArena owns the memory and no explicit
    // freeing is required.
    // If 'fOriginatingBuilder' is non-null then the memory must be explicitly locked (in the ctor)
    // and unlocked (in the dtor) on the 'fOriginatingBuilder' object.
    SkSpan<const uint8_t> fData;
    // This class should only ever access the 'lock' and 'unlock' calls on 'fOriginatingBuilder'
    SkPaintParamsKeyBuilder* fOriginatingBuilder;
};

// The SkPaintParamsKeyBuilder and the SkPaintParamsKeys snapped from it share the same
// underlying block of memory. When an SkPaintParamsKey is snapped from the builder it 'locks'
// the memory and 'unlocks' it in its destructor. Because of this relationship, the builder
// can only have one extant key and that key must be destroyed before the builder can be reused
// to create another one.
//
// This arrangement is intended to improve performance in the expected case, where a builder is
// being used in a tight loop to generate keys which can be recycled once they've been used to
// find the dictionary's matching uniqueID. We don't expect the cost of copying the key's memory
// into the dictionary to be prohibitive since that should be infrequent.
// TODO: Make the builder carry the 'backend' rather than passing it everywhere separately
class SkPaintParamsKeyBuilder {
public:
    SkPaintParamsKeyBuilder(const SkShaderCodeDictionary*);
    ~SkPaintParamsKeyBuilder() {
        SkASSERT(!this->isLocked());
    }

    void beginBlock(int codeSnippetID);
    void beginBlock(SkBuiltInCodeSnippetID id) { this->beginBlock(static_cast<int>(id)); }
    void endBlock();

    void addBytes(uint32_t numBytes, const uint8_t* data);
    void addByte(uint8_t data) {
        this->addBytes(1, &data);
    }

#ifdef SK_DEBUG
    // Check that the builder has been reset to its initial state prior to creating a new key.
    void checkReset();
    uint8_t byte(int offset) const { return fData[offset]; }
#endif

    SkPaintParamsKey lockAsKey();

    int sizeInBytes() const { return fData.count(); }

    bool isValid() const { return fIsValid; }

    void lock() {
        SkASSERT(!fLocked);
        SkDEBUGCODE(fLocked = true;)
    }

    void unlock() {
        SkASSERT(fLocked);
        fData.reset();
        SkDEBUGCODE(fLocked = false;)
        SkDEBUGCODE(this->checkReset();)
    }

    SkDEBUGCODE(bool isLocked() const { return fLocked; })

private:
    void makeInvalid();

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fHeaderOffset;
#ifdef SK_DEBUG
        SkSpan<const SkPaintParamsKey::DataPayloadField> fDataPayloadExpectations;
        int fCurDataPayloadEntry = 0;
#endif
    };

    bool fIsValid = true;
    const SkShaderCodeDictionary* fDict;
    std::vector<StackFrame> fStack;

    // TODO: It is probably overkill but we could encode the SkBackend in the first byte of
    // the key.
    SkDEBUGCODE(bool fLocked = false;)
    SkTArray<uint8_t, true> fData;
};

#endif // SkPaintParamsKey_DEFINED
