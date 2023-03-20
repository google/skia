/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintParamsKey_DEFINED
#define skgpu_graphite_PaintParamsKey_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"

#include <array>
#include <limits>


namespace skgpu::graphite {

class PaintParamsKeyBuilder;
class ShaderCodeDictionary;
class ShaderInfo;
struct ShaderSnippet;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// Header, consisting of 2 bytes:
//   4 bytes: code-snippet ID
//   1 byte: size of the block, in bytes (header, plus all data payload bytes)
// The rest of the data in the block is dependent on the individual code snippet.
// If a given block has child blocks, they appear in the key right after their parent
// block's header.
class PaintParamsKey {
public:
    #pragma pack(push, 1)
    struct Header {
        int32_t codeSnippetID;
        uint8_t blockSize;
    };
    #pragma pack(pop)

    static const int kBlockSizeOffsetInBytes = offsetof(Header, blockSize);
    static const int kMaxBlockSize = std::numeric_limits<uint8_t>::max();

    enum class DataPayloadType {
        kByte,
        kInt,
        kFloat4,
    };

    // A given snippet's data payload is stored as an SkSpan of DataPayloadFields in the
    // ShaderCodeDictionary. That span just defines the structure of the data payload. The actual
    // data is stored in the paint params key.
    struct DataPayloadField {
        const char* fName;
        DataPayloadType fType;
        uint32_t fCount;
    };

    ~PaintParamsKey();

    class BlockReader {
    public:
        // Returns the combined size of the header, all children, and every data payload.
        uint8_t blockSize() const {
            SkASSERT(fBlock[kBlockSizeOffsetInBytes] == fBlock.size());
            return SkTo<uint8_t>(fBlock.size());
        }

        int numChildren() const;

        // Returns the code-snippet ID of this block.
        int32_t codeSnippetId() const;

        // Return the childIndex-th child's BlockReader
        BlockReader child(const ShaderCodeDictionary*, int childIndex) const;

        // Retrieve the fieldIndex-th field in the data payload as a span. The type being read
        // is checked against the data payload's structure.
        SkSpan<const uint8_t> bytes(int fieldIndex) const;
        SkSpan<const int32_t> ints(int fieldIndex) const;
        SkSpan<const SkColor4f> colors(int fieldIndex) const;

        const ShaderSnippet* entry() const { return fEntry; }

#ifdef SK_DEBUG
        int numDataPayloadFields() const;
        void dump(const ShaderCodeDictionary*, int indent) const;
#endif

    private:
        friend class PaintParamsKey; // for ctor

        BlockReader(const ShaderCodeDictionary*,
                    SkSpan<const uint8_t> parentSpan,
                    int offsetInParent);

        // The data payload appears after any children and occupies the remainder of the
        // block's space.
        SkSpan<const uint8_t> dataPayload() const;

        SkSpan<const uint8_t> fBlock;
        const ShaderSnippet* fEntry;
    };

    BlockReader reader(const ShaderCodeDictionary*, int headerOffset) const;

#ifdef SK_DEBUG
    uint8_t byte(int offset) const {
        SkASSERT(offset < this->sizeInBytes());
        return fData[offset];
    }
    void dump(const ShaderCodeDictionary*) const;
#endif
    void toShaderInfo(const ShaderCodeDictionary*, ShaderInfo*) const;

    SkSpan<const uint8_t> asSpan() const { return fData; }
    const uint8_t* data() const { return fData.data(); }
    int sizeInBytes() const { return SkTo<int>(fData.size()); }

    bool operator==(const PaintParamsKey& that) const;
    bool operator!=(const PaintParamsKey& that) const { return !(*this == that); }

#if GRAPHITE_TEST_UTILS
    bool isErrorKey() const;
#endif

private:
    friend class PaintParamsKeyBuilder;   // for the parented-data ctor
    friend class ShaderCodeDictionary;    // for the raw-data ctor

    // This ctor is to be used when paintparams keys are being consecutively generated
    // by a key builder. The memory backing this key's span is shared between the
    // builder and its keys.
    PaintParamsKey(SkSpan<const uint8_t> span, PaintParamsKeyBuilder* originatingBuilder);

    // This ctor is used when this key isn't being created by a builder (i.e., when the key
    // is in the dictionary). In this case the dictionary will own the memory backing the span.
    PaintParamsKey(SkSpan<const uint8_t> rawData);

    static void AddBlockToShaderInfo(const ShaderCodeDictionary*,
                                     const BlockReader&,
                                     ShaderInfo*);

    // The memory referenced in 'fData' is always owned by someone else.
    // If 'fOriginatingBuilder' is null, the dictionary's SkArena owns the 'fData' memory and no
    // explicit freeing is required.
    // If 'fOriginatingBuilder' is non-null then the 'fData' memory must be explicitly locked (in
    // the ctor) and unlocked (in the dtor) on the 'fOriginatingBuilder' object.
    SkSpan<const uint8_t> fData;

    // This class should only ever access the 'lock' and 'unlock' calls on 'fOriginatingBuilder'
    PaintParamsKeyBuilder* fOriginatingBuilder;
};

// The PaintParamsKeyBuilder and the PaintParamsKeys snapped from it share the same
// underlying block of memory. When an PaintParamsKey is snapped from the builder it 'locks'
// the memory and 'unlocks' it in its destructor. Because of this relationship, the builder
// can only have one extant key and that key must be destroyed before the builder can be reused
// to create another one.
//
// This arrangement is intended to improve performance in the expected case, where a builder is
// being used in a tight loop to generate keys which can be recycled once they've been used to
// find the dictionary's matching uniqueID. We don't expect the cost of copying the key's memory
// into the dictionary to be prohibitive since that should be infrequent.
class PaintParamsKeyBuilder {
public:
    PaintParamsKeyBuilder(const ShaderCodeDictionary*);
    ~PaintParamsKeyBuilder() {
        SkASSERT(!this->isLocked());
    }

    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }

    void beginBlock(int32_t codeSnippetID);
    void beginBlock(BuiltInCodeSnippetID id) { this->beginBlock(static_cast<int32_t>(id)); }
    void endBlock();

    void addBytes(uint32_t numBytes, const uint8_t* data);
    void addByte(uint8_t data) {
        this->addBytes(1, &data);
    }
    void addInts(uint32_t numInts, const int32_t* data);
    void addInt(int32_t data) {
        this->addInts(1, &data);
    }
    void add(int numColors, const SkColor4f* colors);
    void add(const SkColor4f& color) {
        this->add(/*numColors=*/1, &color);
    }

#ifdef SK_DEBUG
    // Check that the builder has been reset to its initial state prior to creating a new key.
    void checkReset();
    uint8_t byte(int offset) const { return fData[offset]; }
#endif

    PaintParamsKey lockAsKey();

    int sizeInBytes() const { return fData.size(); }

    bool isValid() const { return fIsValid; }

    void lock() {
        SkASSERT(!fLocked);
        SkDEBUGCODE(fLocked = true;)
    }

    void unlock() {
        SkASSERT(fLocked);
        fData.clear();
        fBlendInfo = {};

        SkDEBUGCODE(fLocked = false;)
        SkDEBUGCODE(this->checkReset();)
    }

    SkDEBUGCODE(bool isLocked() const { return fLocked; })

private:
    void addToKey(uint32_t count, const void* data, PaintParamsKey::DataPayloadType payloadType);
    void makeInvalid();

#ifdef SK_DEBUG
    void checkExpectations(PaintParamsKey::DataPayloadType actualType, uint32_t actualCount);
#endif

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fHeaderOffset;
#ifdef SK_DEBUG
        SkSpan<const PaintParamsKey::DataPayloadField> fDataPayloadExpectations;
        int fCurDataPayloadEntry = 0;
        int fNumExpectedChildren = 0;
        int fNumActualChildren = 0;
#endif
    };

    const ShaderCodeDictionary* fDict;

    bool fIsValid = true;
    SkDEBUGCODE(bool fLocked = false;)

    // Use SkTDArray so that we can guarantee that rewind() preserves the underlying storage and
    // repeated use of the builder will hit a high-water mark and avoid lots of allocations.
    SkTDArray<StackFrame> fStack;
    SkTDArray<uint8_t> fData;
    skgpu::BlendInfo fBlendInfo;
};

} // skgpu::graphite

#endif // skgpu_graphite_PaintParamsKey_DEFINED
