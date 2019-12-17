/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrQuadBuffer_DEFINED
#define GrQuadBuffer_DEFINED

#include "include/private/SkTFitsIn.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/geometry/GrQuad.h"

template<typename T>
class GrQuadBuffer {
private:
    static_assert(alignof(T) == 4, "Buffer metadata T must be 4 byte aligned");

    struct Header;
    struct Block;

public:
    GrQuadBuffer()
            : fHead(nullptr)
            , fTail(nullptr)
            , fDeviceType(static_cast<uint64_t>(GrQuad::Type::kAxisAligned))
            , fLocalType(static_cast<uint64_t>(GrQuad::Type::kAxisAligned))
            , fHasLocals(static_cast<uint64_t>(false))
            , fCount(0)
            , fFib0(0)
            , fFib1(1) {}

    GrQuadBuffer(SkArenaAlloc* arena, int expectedEntryCount, GrQuad::Type expectedDeviceType,
                 GrQuad::Type expectedLocalType, bool expectedHasLocals)
            : fDeviceType(static_cast<uint64_t>(GrQuad::Type::kAxisAligned))
            , fLocalType(static_cast<uint64_t>(GrQuad::Type::kAxisAligned))
            , fHasLocals(static_cast<uint64_t>(false))
            , fCount(0)
            , fFib0(0)
            , fFib1(1) {
        // blockSize is just a hint here, so allow the minimum allocation to be for a reduced
        // number of entries if it makes for more efficient arena usage.
        int minEntryCount = SkTMax(expectedEntryCount / 3, 1);
        size_t expectedEntrySize = EntrySize(expectedDeviceType, expectedLocalType,
                                             expectedHasLocals);

        fHead = this->addBlock(arena, minEntryCount * expectedEntrySize,
                               expectedEntryCount * expectedEntrySize);
        fTail = fHead;
    }

    // The number of device-space quads (and metadata, and optional local quads) that are in the
    // the buffer.
    int count() const {
        SkASSERT(SkTFitsIn<int32_t>(fCount));
        return fCount;
    }

    // The most general type for the device-space quads in this buffer
    GrQuad::Type deviceQuadType() const { return static_cast<GrQuad::Type>(fDeviceType); }

    // The most general type for the local quads; if no local quads are ever added, this will still
    // return kAxisAligned.
    GrQuad::Type localQuadType() const { return static_cast<GrQuad::Type>(fLocalType); }

    // Append the given 'deviceQuad' to this buffer, with its associated 'metadata'. If 'localQuad'
    // is not null, the local coordinates will also be attached to the entry. When an entry
    // has local coordinates, during iteration, the Iter::hasLocals() will return true and its
    // Iter::localQuad() will be equivalent to the provided local coordinates. If 'localQuad' is
    // null then Iter::hasLocals() will report false for the added entry.
    void append(SkArenaAlloc* arena, const GrQuad& deviceQuad, T&& metadata,
                const GrQuad* localQuad = nullptr);

    // Concatenates all entries from 'that' to this buffer. This may copy entries or steal internal
    //  nodes from 'that'. 'that' should not be used after this returns.
    void concat(SkArenaAlloc* arena, GrQuadBuffer<T>* that);

    // Provides a read-only iterator over a quad buffer, giving access to the device quad, metadata
    // and optional local quad.
    class Iter {
    public:
        Iter(const GrQuadBuffer<T>* buffer)
                : fCurrentBlock(buffer->fHead)
                , fCurrentIndex(-1)
                , fNextIndex(0) {
            SkDEBUGCODE(fBuffer = buffer);
            SkDEBUGCODE(fExpectedCount = buffer->count();)
        }

        bool next();

        const T& metadata() const {
            SkDEBUGCODE(this->validate();)
            return *(fCurrentBlock->metadata(fCurrentIndex));
        }

        // The returned pointer is mutable so that the object can be used for scratch calculations
        // during op preparation. However, any changes are not persisted in the GrQuadBuffer and
        // subsequent calls to next() will overwrite the state of the GrQuad.
        GrQuad* deviceQuad() {
            SkDEBUGCODE(this->validate();)
            return &fDeviceQuad;
        }

        // If isLocalValid() returns false, this returns nullptr. Otherwise, the returned pointer
        // is mutable in the same manner as deviceQuad().
        GrQuad* localQuad() {
            SkDEBUGCODE(this->validate();)
            return this->isLocalValid() ? &fLocalQuad : nullptr;
        }

        bool isLocalValid() const {
            SkDEBUGCODE(this->validate();)
            return fCurrentBlock->header(fCurrentIndex)->fHasLocals;
        }

    private:
        // Quads are stored locally so that calling code doesn't need to re-declare their own quads
        GrQuad fDeviceQuad;
        GrQuad fLocalQuad;

        const Block* fCurrentBlock;
        // The index into the current entry to read metadata/header properties
        int          fCurrentIndex;
        // The index to the next entry, cached since it is calculated automatically while unpacking
        // the quad data from fCurrentIndex. If fNextIndex >= fCurrentBlock->used() then the
        // iterator must advance to the next block instead.
        int          fNextIndex;

        SkDEBUGCODE(const GrQuadBuffer<T>* fBuffer;)
        SkDEBUGCODE(uint32_t fExpectedCount;)

#ifdef SK_DEBUG
        void validate() const {
            fBuffer->validate(fCurrentBlock, fCurrentIndex, fExpectedCount);
        }
#endif
    };

    Iter iterator() const { return Iter(this); }

    // Provides a *mutable* iterator over just the metadata stored in the quad buffer. This skips
    // unpacking the device and local quads into GrQuads and is intended for use during op
    // finalization, which may require rewriting state such as color.
    class MetadataIter {
    public:
        MetadataIter(GrQuadBuffer<T>* buffer)
                : fCurrentBlock(buffer->fHead)
                , fCurrentIndex(-1) {
            SkDEBUGCODE(fBuffer = buffer;)
            SkDEBUGCODE(fExpectedCount = buffer->count();)
        }

        bool next();

        T& operator*() {
            SkDEBUGCODE(this->validate();)
            return *(fCurrentBlock->metadata(fCurrentIndex));
        }

        T* operator->() {
            SkDEBUGCODE(this->validate();)
            return fCurrentBlock->metadata(fCurrentIndex);
        }

    private:
        Block* fCurrentBlock;
        int    fCurrentIndex;

        SkDEBUGCODE(GrQuadBuffer<T>* fBuffer;)
        SkDEBUGCODE(uint32_t fExpectedCount;)

#ifdef SK_DEBUG
        void validate() const {
            fBuffer->validate(fCurrentBlock, fCurrentIndex, fExpectedCount);
        }
#endif
    };

    MetadataIter metadata() { return MetadataIter(this); }

private:
    // Header per entry in a block. This is 4 byte aligned to match all of the floats that the
    // block will be storing, even though it currently only holds 5 bits of data.
    struct Header {
        uint32_t fDeviceType : 2;
        uint32_t fLocalType  : 2; // Ignore if fHasLocals is false
        uint32_t fHasLocals  : 1;
        // Known value to detect if iteration doesn't properly advance through the buffer
        SkDEBUGCODE(uint32_t fSentinel : 27;)
    };
    static_assert(sizeof(Header) == sizeof(uint32_t), "Header should be 4 bytes");

    // Each logical entry in the Block's data is a variable length tuple storing device coordinates,
    // optional local coordinates, and metadata. An entry always has a header that defines the
    // quad types of device and local coordinates, and always has metadata of type T. The device
    // and local quads' data follows as a variable length array of floats:
    //  [ header    ] = 4 bytes
    //  [ metadata  ] = sizeof(T), assert alignof(T) == 4 so that pointer casts are valid
    //  [ device xs ] = 4 floats = 16 bytes
    //  [ device ys ] = 4 floats
    //  [ device ws ] = 4 floats or 0 floats depending on fDeviceType in header
    //  [ local xs  ] = 4 floats or 0 floats depending on fHasLocals in header
    //  [ local ys  ] = 4 floats or 0 floats depending on fHasLocals in header
    //  [ local ws  ] = 4 floats or 0 floats depending on fHasLocals and fLocalType in header
    struct Block {
        // fDataSize and fIndex are packed into 16 bits, so make sure not to allocate more
        // than can be indexed.
        static constexpr size_t kMaxDataSize = UINT16_MAX;

        // The next block in the linked list of blocks. A block should not have new quads added to
        // its data if it already has a next block.
        Block* fNext;
        // Size of fData, which is dynamic and allocated on the arena. Since this is 16 bits, it
        // limits each Block to 64k (roughly 400 quads, so not bad, and helps prevent excessively
        // large allocations).
        uint32_t fDataSize : 16;
        // Index of the next quad entry to be appended to the block.
        uint32_t fIndex    : 16;
        // fData stores contiguous Headers, T's, and float coordinates (all 4 byte aligned).
        // Must be at the end because it fills the allocation from the arena. fDataSize and fIndex
        // are bit fields of uint32_t to avoid padding between fIndex and fData.
        char fData[4];

        // For use with in-place new, where the bytesAllocated represents the total allocation
        // including the Blocks fields, its first fData value, and all remaining uint32_ts.
        Block(size_t bytesAllocated)
                : fNext(nullptr)
                , fDataSize(bytesAllocated - kBlockSize)
                , fIndex(0) {
            SkASSERT(SkTFitsIn<uint16_t>(bytesAllocated - kBlockSize));
        }

        inline int remaining() const { return fDataSize - fIndex; }

        inline int used() const { return fIndex; }

        // Helpers to access typed sections of fData, given the index that represents the start of
        // an entry (header, metadata, device and local coords).
        inline Header* header(int index) {
            SkASSERT(index >= 0 && index < fIndex);
            return static_cast<Header*>(static_cast<void*>(fData + index));
        }
        inline const Header* header(int index) const {
            SkASSERT(index >= 0 && index < fIndex);
            return static_cast<const Header*>(static_cast<const void*>(fData + index));
        }

        inline T* metadata(int index) {
            return static_cast<T*>(static_cast<void*>(fData + index + kHeaderSize));
        }
        inline const T* metadata(int index) const {
            return static_cast<const T*>(static_cast<const void*>(fData + index + kHeaderSize));
        }

        inline float* coords(int index) {
            return static_cast<float*>(static_cast<void*>(fData + index + kMetaSize));
        }
        inline const float* coords(int index) const {
            return static_cast<const float*>(static_cast<const void*>(fData + index + kMetaSize));
        }
    };
    // fNext == 8, fDataSize + fIndex == 4, fData == 4 -> 16 total
    static_assert(sizeof(Block) == 16, "Block should be 16 bytes");

    static constexpr uint32_t kSentinel = 0xbaffe;
    static constexpr size_t kHeaderSize = sizeof(Header);
    static constexpr size_t kMetaSize = sizeof(Header) + sizeof(T);
    // Does not include Block's fData field, so this represents the size of the Block struct
    // that isn't storing the entry data.
    static constexpr size_t kBlockSize = sizeof(Block) - 4;
    static constexpr size_t k2DQuadSize = 8 * sizeof(float);
    static constexpr size_t k3DQuadSize = 12 * sizeof(float);

    // Each Fibonacci term is packed into 13 bits, so saturate the series at this value
    static constexpr size_t kMaxFib = (1 << 13) - 1;

    // These point to block allocated in an SkArenaAlloc, preferably the record time allocator of
    // the GrRecordingContext for the ops that store geometry in the buffer.
    Block*   fHead;
    Block*   fTail;

    // GrTextureOp's performance is highly sensitive to op size, so make this as small as possible.
    // Packing everything into 64 bits doesn't really limit us since the Block* force us to be 8
    // byte aligned anyways, and 2^32 well exceeds the quad index buffer limits of GrResourceManager
    uint64_t fDeviceType : 2; // GrQuad::Type
    uint64_t fLocalType  : 2; // ""
    uint64_t fHasLocals  : 1; // This is tracked to help estimate future quad entry sizes
    static_assert(GrQuad::kTypeCount <= 4);

    // The number of entries in the buffer (e.g. # of (Headers + coords) across all linked blocks)
    uint64_t fCount      : 32;
    // The fibonacci sequence doesn't need that many bits, since (fFib0 + fFib1) is not restricted
    // to these 13 bits until after a block allocation. Eventually, the allocation sequence will
    // converge to a constant 2 * kMaxFib * entrySize; but this is further restricted to 16-bit max
    // block size and with entrySize generally about 100 bytes, this aligns pretty well.
    uint64_t fFib0       : 13;
    uint64_t fFib1       : 13;

    Block* addBlock(SkArenaAlloc* arena, size_t minSize, size_t maxSize) {
        size_t allocated;
        void* blockPtr = arena->makeAtLeastBytesAlignedTo(
                SkTMin(minSize, Block::kMaxDataSize) + kBlockSize,
                SkTMin(maxSize, Block::kMaxDataSize) + kBlockSize,
                alignof(uint32_t), &allocated);
        return new (blockPtr) Block(allocated);
    }

    Block* addBlock(SkArenaAlloc* arena, size_t entrySize) {
        // Determine max based on Fibonacci growth, min will always be the single entry
        size_t n = fFib0 + fFib1;
        fFib0 = fFib1;
        fFib1 = SkTMin(n, kMaxFib);
        return this->addBlock(arena, entrySize, n * entrySize);
    }

    void growBlock(SkArenaAlloc* arena, Block* block) {
        size_t n = fFib0 + fFib1;
        size_t expectedEntrySize = EntrySize(this->deviceQuadType(),
                                             this->localQuadType(),
                                             fHasLocals);
        // Add the expected size to the current data size so that arena->resize() will always
        // increase the block's allocation. resize() is allowed to shrink an allocation, so we
        // don't want to inadvertently calculate an expected new size that is less than the current
        size_t newSize = kBlockSize + SkTMin(n * expectedEntrySize + block->fDataSize,
                                             Block::kMaxDataSize);
        if (arena->resize(block, block->fDataSize + kBlockSize, &newSize)) {
            block->fDataSize = newSize - kBlockSize;
            // Only update size sequence if growth was successful
            fFib0 = fFib1;
            fFib1 = SkTMin(n, kMaxFib);
        }
    }

    static inline int EntrySize(GrQuad::Type deviceType, GrQuad::Type localType, bool hasLocals) {
        int size = kMetaSize;
        size += (deviceType == GrQuad::Type::kPerspective ? k3DQuadSize
                                                          : k2DQuadSize);
        if (hasLocals) {
            size += (localType == GrQuad::Type::kPerspective ? k3DQuadSize
                                                             : k2DQuadSize);
        }
        return size;
    }
    static inline int EntrySize(const Header* header) {
        return EntrySize(static_cast<GrQuad::Type>(header->fDeviceType),
                         static_cast<GrQuad::Type>(header->fLocalType),
                         header->fHasLocals);
    }

    // Helpers to convert from coordinates to GrQuad and vice versa, returning pointer to the
    // next packed quad coordinates.
    static float* PackQuad(const GrQuad& quad, float* coords) {
        // Copies all 12 (or 8) floats at once, so requires the 3 arrays to be contiguous
        SkASSERT(quad.xs() + 4 == quad.ys() && quad.xs() + 8 == quad.ws());
        if (quad.hasPerspective()) {
            memcpy(coords, quad.xs(), k3DQuadSize);
            return coords + 12;
        } else {
            memcpy(coords, quad.xs(), k2DQuadSize);
            return coords + 8;
        }
    }
    static const float* UnpackQuad(GrQuad::Type type, const float* coords, GrQuad* quad) {
        SkASSERT(quad->xs() + 4 == quad->ys() && quad->xs() + 8 == quad->ws());
        if (type == GrQuad::Type::kPerspective) {
            // Fill in X, Y, and W in one go
            memcpy(quad->xs(), coords, k3DQuadSize);
            coords = coords + 12;
        } else {
            // Fill in X and Y of the quad, the setQuadType() below will set Ws to 1 if needed
            memcpy(quad->xs(), coords, k2DQuadSize);
            coords = coords + 8;
        }

        quad->setQuadType(type);
        return coords;
    }

#ifdef SK_DEBUG
    void validate(const Block* currentBlock, int currentIndex, int expectedCount) const {
        // Triggers if accessing before next() is called on an iterator
        SkASSERT(currentIndex >= 0);
        // Triggers if accessing after next() returns false
        SkASSERT(currentBlock);
        // Triggers if reading past the end of a block
        SkASSERT(currentIndex < currentBlock->fIndex);
        // Triggers if elements have been added to the buffer while iterating entries
        SkASSERT(expectedCount == this->count());
        // Make sure the start of the entry looks like a header
        SkASSERT(currentBlock->header(currentIndex)->fSentinel == kSentinel);
    }
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
void GrQuadBuffer<T>::append(SkArenaAlloc* arena,
                             const GrQuad& deviceQuad,
                             T&& metadata,
                             const GrQuad* localQuad) {
    GrQuad::Type localType;
    int entrySize;
    if (localQuad) {
        localType = localQuad->quadType();
        entrySize = EntrySize(deviceQuad.quadType(), localType, true);
    } else {
        localType = GrQuad::Type::kAxisAligned;
        entrySize = EntrySize(deviceQuad.quadType(), localType, false);
    }

    if (!fHead || fTail->remaining() < entrySize) {
        // Need a new Block from the arena.
        Block* nextBlock = this->addBlock(arena, entrySize);
        SkASSERT(nextBlock->remaining() >= entrySize);

        if (!fHead) {
            fHead = nextBlock;
        } else {
            SkASSERT(fTail);
            fTail->fNext = nextBlock;
        }
        fTail = nextBlock;
    }

    // Fill in the entry, as described in fData's declaration
    SkASSERT(entrySize <= fTail->remaining());
    int index = fTail->fIndex;
    fTail->fIndex += entrySize;

    // First the header
    Header* h = fTail->header(index);
    h->fDeviceType = static_cast<uint32_t>(deviceQuad.quadType());
    h->fHasLocals = static_cast<uint32_t>(localQuad != nullptr);
    h->fLocalType = static_cast<uint32_t>(localType);
    SkDEBUGCODE(h->fSentinel = static_cast<uint32_t>(kSentinel);)

    // Second, the fixed-size metadata
    *(fTail->metadata(index)) = std::move(metadata);

    // Finally the variable blocks of x, y, and w float coordinates
    float* coords = fTail->coords(index);
    coords = PackQuad(deviceQuad, coords);
    if (localQuad) {
        coords = PackQuad(*localQuad, coords);
    }
    SkASSERT((char*)coords - (fTail->fData + index) == entrySize);

    // Entry complete, update buffer-level state
    fCount++;

    if (h->fDeviceType > fDeviceType) {
        fDeviceType = h->fDeviceType;
    }
    if (h->fLocalType > fLocalType) {
        fLocalType = h->fLocalType;
    }
    fHasLocals |= h->fHasLocals;
}

template<typename T>
void GrQuadBuffer<T>::concat(SkArenaAlloc* arena, GrQuadBuffer<T>* that) {
    if (!fHead) {
        // Steal that's head and tail since this buffer is empty
        fHead = that->fHead;
        fTail = that->fTail;
    } else {
        // Pack as much as possible into the remainder of fTail with memcpy.
        // NOTE: This currently adds a non-trivial amount of memory moving because a new rect op
        // data is appended to the end of the arena when it's created, and then that is copied back
        // into any mergeable GrQuadBuffer. Once a draw can be added in-place on an existing op,
        // without allocating the GrOp up front, many of these moves can go away because they will
        // get written directly to the packed blocks.
        SkASSERT(fTail);
        Block* thatHead = that->fHead;
        while(thatHead && thatHead->used() <= fTail->remaining()) {
            memcpy(fTail->fData + fTail->fIndex, thatHead->fData, thatHead->used());
            fTail->fIndex += thatHead->used();
            // Release thatHead (this will really only do anything if thatHead->fNext is null and
            // thatHead happened to be at the end of the arena).
            Block* next = thatHead->fNext;
            arena->release(thatHead, thatHead->fDataSize + kBlockSize);
            thatHead = next;
            SkASSERT(!success || !next);
        }

        // If we still have a 'thatHead', we ran out of room to copy full blocks
        // into this buffer's current tail. Simply link the remaining nodes.
        if (thatHead) {
            fTail->fNext = thatHead;
            fTail = that->fTail;
            // Conservatively update the growth rate as well (this is slower than summing them)
            fFib0 = SkTMax(fFib0, that->fFib0);
            fFib1 = SkTMax(fFib1, that->fFib1);

            // At this point, we know the GrQuadBuffer is used by an op that captures multiple draws
            // Attempt to grow the new tail to allow subsequent ops to be densely packed into it.
            this->growBlock(arena, fTail);
        }
    }

    fCount += that->fCount;
    fDeviceType = SkTMax(fDeviceType, that->fDeviceType);
    fLocalType = SkTMax(fLocalType, that->fLocalType);
    fHasLocals |= that->fHasLocals;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Iterator implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool GrQuadBuffer<T>::Iter::next() {
    if (!fCurrentBlock) {
        // No more blocks
        return false;
    }

    if (fCurrentBlock->used() <= fNextIndex) {
        // Used up current block, go to the next
        fCurrentBlock = fCurrentBlock->fNext;
        fCurrentIndex = 0;
        if (!fCurrentBlock) {
            // No more blocks
            return false;
        }
    } else {
        // Iteration within the current block
        fCurrentIndex = fNextIndex;
    }

    SkASSERT(fCurrentBlock && fCurrentIndex < fCurrentBlock->used());


    // Unpack the device and optional local coordinates into fDeviceQuad and fLocalQuad
    const Header* h = fCurrentBlock->header(fCurrentIndex);
    const float* coords = fCurrentBlock->coords(fCurrentIndex);
    coords = UnpackQuad(static_cast<GrQuad::Type>(h->fDeviceType), coords, &fDeviceQuad);
    if (h->fHasLocals) {
        coords = UnpackQuad(static_cast<GrQuad::Type>(h->fLocalType), coords, &fLocalQuad);
    } // else localQuad() will return a nullptr so no need to reset fLocalQuad

    // At this point, coords points to the start of the next entry
    fNextIndex = fCurrentIndex + EntrySize(h);
    SkASSERT((fCurrentBlock->fData + fNextIndex) == (char*) coords));
    return true;
}

template<typename T>
bool GrQuadBuffer<T>::MetadataIter::next() {
    if (!fCurrentBlock) {
        return false;
    }

    if (fCurrentIndex >= 0) {
        // Advance pointer by entry size
        int entrySize = EntrySize(fCurrentBlock->header(fCurrentIndex));
        fCurrentIndex += entrySize;
    } else {
        // First call to next
        fCurrentIndex = 0;
    }

    if (fCurrentIndex >= fCurrentBlock->used()) {
        // Advanced past the end of the current block
        fCurrentBlock = fCurrentBlock->fNext;
        fCurrentIndex = 0;
    }
    // Nothing else is needed to do but report whether or not the updated block+index is valid
    return fCurrentBlock && fCurrentIndex < fCurrentBlock->used();
}
#endif  // GrQuadBuffer_DEFINED
