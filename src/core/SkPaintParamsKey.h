/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintParamsKey_DEFINED
#define SkPaintParamsKey_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkBuiltInCodeSnippetID.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/Blend.h"
#endif

#include <array>
#include <limits>

enum class SkBackend : uint8_t {
    kGanesh,
    kGraphite,
    kSkVM
};
class SkPaintParamsKeyBuilder;
class SkShaderCodeDictionary;
class SkShaderInfo;
struct SkShaderSnippet;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of blocks where each block has a
// Header, consisting of 2 bytes:
//   1 byte: code-snippet ID
//   1 byte: size of the block, in bytes (header, plus all data payload bytes)
// The rest of the data and pointers in the block are dependent on the individual code snippet.
// If a given block has child blocks, they appear in the key right after their parent
// block's header.
class SkPaintParamsKey {
public:
    SK_BEGIN_REQUIRE_DENSE
    struct Header {
        uint8_t codeSnippetID;
        uint8_t blockSize;
    };
    SK_END_REQUIRE_DENSE

    static const int kBlockSizeOffsetInBytes = offsetof(Header, blockSize);
    static const int kMaxBlockSize = std::numeric_limits<uint8_t>::max();

    enum class DataPayloadType {
        kByte,
        kFloat4,
        // Represents a position inside the fPointerData span.
        kPointerIndex,
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

    class BlockReader {
    public:
        // Returns the combined size of the header, all children, and every data payload.
        uint8_t blockSize() const {
            SkASSERT(fBlock[kBlockSizeOffsetInBytes] == fBlock.size());
            return SkTo<uint8_t>(fBlock.size());
        }

        int numChildren() const;

        // Return the childIndex-th child's BlockReader
        BlockReader child(const SkShaderCodeDictionary*, int childIndex) const;

        // Retrieve the fieldIndex-th field in the data payload as a span of bytes. The type
        // being read (bytes in this case) is checked against the data payload's structure.
        SkSpan<const uint8_t> bytes(int fieldIndex) const;
        // TODO: add more types (as needed) and their corresponding access methods

        const SkShaderSnippet* entry() const { return fEntry; }

#ifdef SK_DEBUG
        int numDataPayloadFields() const;
        void dump(const SkShaderCodeDictionary*, int indent) const;
#endif

    private:
        friend class SkPaintParamsKey; // for ctor

        BlockReader(const SkShaderCodeDictionary*,
                    SkSpan<const uint8_t> parentSpan,
                    int offsetInParent);

        SkBuiltInCodeSnippetID codeSnippetId() const {
            return static_cast<SkBuiltInCodeSnippetID>(fBlock[0]);
        }

        // The data payload appears after any children and occupies the remainder of the
        // block's space.
        SkSpan<const uint8_t> dataPayload() const;

        SkSpan<const uint8_t> fBlock;
        const SkShaderSnippet* fEntry;
    };

    BlockReader reader(const SkShaderCodeDictionary*, int headerOffset) const;

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

    SkSpan<const void*> pointerSpan() const { return fPointerData; }
    const void** pointerData() const { return fPointerData.data(); }
    int numPointers() const { return SkTo<int>(fPointerData.size()); }

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
    SkPaintParamsKey(SkSpan<const uint8_t> span,
                     SkSpan<const void*> pointerSpan,
                     SkPaintParamsKeyBuilder* originatingBuilder);

    // This ctor is used when this key isn't being created by a builder (i.e., when the key
    // is in the dictionary). In this case the dictionary will own the memory backing the span.
    SkPaintParamsKey(SkSpan<const uint8_t> rawData);

    static void AddBlockToShaderInfo(SkShaderCodeDictionary*,
                                     const SkPaintParamsKey::BlockReader&,
                                     SkShaderInfo*);

    // The memory referenced in 'fData' and 'fPointerData' is always owned by someone else.
    // If 'fOriginatingBuilder' is null, the dictionary's SkArena owns the 'fData' memory and no
    // explicit freeing is required.
    // If 'fOriginatingBuilder' is non-null then the 'fData' memory must be explicitly locked (in
    // the ctor) and unlocked (in the dtor) on the 'fOriginatingBuilder' object.
    // The 'fPointerData' memory is always managed external to this class.
    SkSpan<const uint8_t> fData;
    SkSpan<const void*> fPointerData;

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
class SkPaintParamsKeyBuilder {
public:
    SkPaintParamsKeyBuilder(const SkShaderCodeDictionary*, SkBackend);
    ~SkPaintParamsKeyBuilder() {
        SkASSERT(!this->isLocked());
    }

    SkBackend backend() const { return fBackend; }

#ifdef SK_GRAPHITE_ENABLED
    void setBlendInfo(const skgpu::BlendInfo& blendInfo) {
        fBlendInfo = blendInfo;
    }
    const skgpu::BlendInfo& blendInfo() const { return fBlendInfo; }
#endif

    void beginBlock(int codeSnippetID);
    void beginBlock(SkBuiltInCodeSnippetID id) { this->beginBlock(static_cast<int>(id)); }
    void endBlock();

    void addBytes(uint32_t numBytes, const uint8_t* data);
    void addByte(uint8_t data) {
        this->addBytes(1, &data);
    }
    void add(const SkColor4f& color);

    // `addPointer` is optional sidecar data. The pointer data in a PaintParamsKey is not checked at
    // all when checking the equality of two keys; cached PaintParamsKey objects will not hold
    // pointer data. However, pointer data will be required for actually painting pixels on the
    // screen.
    void addPointer(const void* ptr);

#ifdef SK_DEBUG
    // Check that the builder has been reset to its initial state prior to creating a new key.
    void checkReset();
    uint8_t byte(int offset) const { return fData[offset]; }
#endif

    SkPaintParamsKey lockAsKey();

    int sizeInBytes() const { return fData.count(); }
    int numPointers() const { return fPointerData.count(); }

    bool isValid() const { return fIsValid; }

    void lock() {
        SkASSERT(!fLocked);
        SkDEBUGCODE(fLocked = true;)
    }

    void unlock() {
        SkASSERT(fLocked);
        fData.rewind();
        fPointerData.rewind();
#ifdef SK_GRAPHITE_ENABLED
        fBlendInfo = {};
#endif
        SkDEBUGCODE(fLocked = false;)
        SkDEBUGCODE(this->checkReset();)
    }

    SkDEBUGCODE(bool isLocked() const { return fLocked; })

private:
    void makeInvalid();

#ifdef SK_DEBUG
    void checkExpectations(SkPaintParamsKey::DataPayloadType actualType, uint32_t actualCount);
#endif

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fHeaderOffset;
#ifdef SK_DEBUG
        SkSpan<const SkPaintParamsKey::DataPayloadField> fDataPayloadExpectations;
        int fCurDataPayloadEntry = 0;
        int fNumExpectedChildren = 0;
        int fNumActualChildren = 0;
        int fNumExpectedPointers = 0;
        int fNumActualPointers = 0;
#endif
    };

    const SkShaderCodeDictionary* fDict;
    // TODO: It is probably overkill but we could encode the SkBackend in the first byte of
    // the key.
    const SkBackend fBackend;

    bool fIsValid = true;
    SkDEBUGCODE(bool fLocked = false;)

    // Use SkTDArray so that we can guarantee that rewind() preserves the underlying storage and
    // repeated use of the builder will hit a high-water mark and avoid lots of allocations.
    SkTDArray<StackFrame> fStack;
    SkTDArray<uint8_t> fData;

    // The pointer data is used by some paint types, when the key data is not sufficient to
    // reconstruct all the information needed to draw. (For instance, the key for a runtime effect
    // contains a hash of the shader text, but to draw, we need entire compiled shader program.)
    // Cached paint-param keys will discard the pointer data. When comparing paint-param keys,
    // pointer data (if any) will be ignored.
    SkTDArray<const void*> fPointerData;

#ifdef SK_GRAPHITE_ENABLED
    skgpu::BlendInfo fBlendInfo;
#endif
};

#endif // SkPaintParamsKey_DEFINED
