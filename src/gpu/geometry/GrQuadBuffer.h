/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrQuadBuffer_DEFINED
#define GrQuadBuffer_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/geometry/GrQuad.h"

// FIXME I could instead take a char* pointer to local parent storage for the first block
// and then inlining doesn't become part of the template?
template<typename T>
class GrQuadBuffer {
private:
    struct Header;
    struct Block;

public:
    // FIXME bulk ops should be able to allocate one big block for their entries
    GrQuadBuffer() //uint32_t* inlineBlockData, size_t dataSize)
            : fDeviceType(static_cast<uint32_t>(GrQuad::Type::kAxisAligned))
            , fLocalType(static_cast<uint32_t>(GrQuad::Type::kAxisAligned))
            , fCount(0) {
        // A fullsize block will be allocated on the first append if fHead is null.
        // if (inlineBlockData) {
            // SkASSERT(dataSize % size(uint32_t) == 0 && dataSize > sizeof(Block));
            // FIXME this is the same as what happens in append(), maybe we have an init() block
            // that takes a uint32_t* or void* and inits the block + returns cast ptr?
            // fHead = static_cast<Block*>(inlineBlockData);
            // fHead->fNext = nullptr;
            // fHead->fBlockSize = dataSize - sizeof(Block) / sizeof(uint32_t) + 1;
            // fHead->fIndex = 0;
        // } else {
            // FIXME or do we take an arena and do a first block alloc of size 1 or batch cnt?
            fHead = nullptr;
        // }

        fTail = fHead;
    }

    // The number of device-space quads (and metadata, and optional local quads) that are in the
    // the buffer.
    int count() const { return fCount; }

    // The most general type for the device-space quads in this buffer
    GrQuad::Type deviceQuadType() const { return static_cast<GrQuad::Type>(fDeviceType); }

    // The most general type for the local quads; if no local quads are ever added, this will
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
    void concat(GrQuadBuffer<T>* that);

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
        SkDEBUGCODE(int fExpectedCount;)

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
        SkDEBUGCODE(int fExpectedCount;)

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
    struct alignas(uint32_t) Header {
        uint32_t fDeviceType : 2;
        uint32_t fLocalType  : 2; // Ignore if fHasLocals is false
        uint32_t fHasLocals  : 1;
        // Known value to detect if iteration doesn't properly advance through the buffer
        SkDEBUGCODE(uint32_t fSentinel : 27;)
    };
    static_assert(sizeof(Header) == sizeof(uint32_t), "Header should be 4 bytes");


    // Each logical entry in the buffer is a variable length tuple storing device coordinates,
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
        // The next block in the linked list of blocks. A block should not have new quads added to
        // its data if it already has a next block.
        Block* fNext;
        // Size of fData, which is dynamic and allocated on the arena.
        uint32_t fBlockSize : 16;
        // Index of the next quad entry to be appended to the block.
        uint32_t fIndex     : 16;
        // fData stores contiguous Headers, T's, and float coordinates (all 4 byte aligned).
        // Must be at the end because it fills the allocation from the arena. fBlockSize and fIndex
        // are bit fields of uint32_t to avoid padding between fIndex and fData.
        uint32_t fData[1];

        inline int remaining() const { return fBlockSize - fIndex; }

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
    static_assert(sizeof(Block) == 16);

    static constexpr uint32_t kSentinel = 0xbaffe;
    // FIXME are things easier or harder if we operate in sizeof(uint32_t) or in bytes?
    static constexpr int kHeaderSize = sizeof(Header) / sizeof(uint32_t);
    static constexpr int kMetaSize = (sizeof(Header) + sizeof(T)) / sizeof(uint32_t);
    static constexpr int k2DQuadFloats = 8;
    static constexpr int k3DQuadFloats = 12;

    // If inlining, hold onto enough bytes to store a 3D quad with 2D local coords and the metadata
    // static constexpr int kInlineBlockSize = kInline == InlineFirstQuad::kYes
    //         ? (kMetaSize + k2DQuadFloats + k3DQuadFloats)
    //         : 0;

    // These either point to an inline array from owner, a block allocated in an SkArenaAlloc.
    Block*                        fHead;
    Block*                        fTail;

    // GrTextureOp's performance is highly sensitive to op size, so make this as small as possible.
    // Packing everything into 32 bits doesn't really limit us since the quad index buffers are
    // much smaller than the 28 bits for this 'count'.
    uint32_t fDeviceType : 2; // GrQuad::Type
    uint32_t fLocalType  : 2; // ""
    uint32_t fCount      : 28;

    static_assert(GrQuad::kTypeCount <= 4);

    static inline int EntrySize(GrQuad::Type deviceType, const GrQuad::Type* localType) {
        int size = kMetaSize;
        size += (deviceType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                          : k2DQuadFloats);
        if (localType) {
            size += (*localType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                              : k2DQuadFloats);
        }
        return size;
    }
    static inline int EntrySize(const Header* header) {
        if (header->fHasLocals) {
            GrQuad::Type localType = static_cast<GrQuad::Type>(header->fLocalType);
            return EntrySize(static_cast<GrQuad::Type>(header->fDeviceType), &localType);
        } else {
            return EntrySize(static_cast<GrQuad::Type>(header->fDeviceType), nullptr);
        }
    }

    // Helpers to convert from coordinates to GrQuad and vice versa, returning pointer to the
    // next packed quad coordinates.
    static float* PackQuad(const GrQuad& quad, float* coords) {
        // Copies all 12 (or 8) floats at once, so requires the 3 arrays to be contiguous
        SkASSERT(quad.xs() + 4 == quad.ys() && quad.xs() + 8 == quad.ws());
        if (quad.hasPerspective()) {
            memcpy(coords, quad.xs(), k3DQuadFloats * sizeof(float));
            return coords + k3DQuadFloats;
        } else {
            memcpy(coords, quad.xs(), k2DQuadFloats * sizeof(float));
            return coords + k2DQuadFloats;
        }
    }
    static const float* UnpackQuad(GrQuad::Type type, const float* coords, GrQuad* quad) {
        SkASSERT(quad->xs() + 4 == quad->ys() && quad->xs() + 8 == quad->ws());
        if (type == GrQuad::Type::kPerspective) {
            // Fill in X, Y, and W in one go
            memcpy(quad->xs(), coords, k3DQuadFloats * sizeof(float));
            coords = coords + k3DQuadFloats;
        } else {
            // Fill in X and Y of the quad, the setQuadType() below will set Ws to 1 if needed
            memcpy(quad->xs(), coords, k2DQuadFloats * sizeof(float));
            coords = coords + k2DQuadFloats;
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
        // Triggers if elements have been added to the buffer while iterating entries
        SkASSERT(expectedCount == fCount);
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
        entrySize = EntrySize(deviceQuad.quadType(), &localType);
    } else {
        localType = GrQuad::Type::kAxisAligned;
        entrySize = EntrySize(deviceQuad.quadType(), nullptr);
    }

    if (!fHead || fTail->remaining() < entrySize) {
        // Need a new Block from the arena.
        int minSize = sizeof(Block) + (entrySize - 1) * sizeof(uint32_t);
        int maxSize = sizeof(Block) + (20 * entrySize - 1) * sizeof(uint32_t); // FIXME better heuristic
        size_t allocated;
        void* blockPtr = arena->makeAtleastBytesAlignedTo(minSize, maxSize, alignof(uint32_t), &allocated);

        Block* nextBlock = static_cast<Block*>(blockPtr);
        nextBlock->fNext = nullptr;
        nextBlock->fBlockSize = (allocated - sizeof(Block)) / sizeof(uint32_t) + 1;
        nextBlock->fIndex = 0;

        if (!fHead) {
            fHead = nextBlock;
        }
        if (fTail) {
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
    h->fLocalType = static_cast<uint32_t>(localQuad ? localQuad->quadType()
                                                    : GrQuad::Type::kAxisAligned);
    SkDEBUGCODE(h->fSentinel = static_cast<uint32_t>(kSentinel);)

    // Second, the fixed-size metadata
    *(fTail->metadata(index)) = std::move(metadata);

    // Then the variable blocks of x, y, and w float coordinates
    float* coords = fTail->coords(index);
    coords = PackQuad(deviceQuad, coords);
    if (localQuad) {
        coords = PackQuad(*localQuad, coords);
    }
    SkASSERT((uint32_t*)coords - (fTail->fData + index) == entrySize);

    // Entry complete, update buffer-level state
    fCount++;

    fDeviceType = SkTMax(fDeviceType, h->fDeviceType);
    fLocalType = SkTMax(fLocalType, h->fLocalType);
}

template<typename T>
void GrQuadBuffer<T>::concat(GrQuadBuffer<T>* that) {
    if (!fHead) {
        // Steal that's head and tail since this buffer is empty
        fHead = that->fHead;
        fTail = that->fTail;
    } else {
        // Pack as much as possible into the remainder of fTail with memcpy
        Block* thatHead = that->fHead;
        while(thatHead && thatHead->used() <= fTail->remaining()) {
            memcpy(fTail->fData + fTail->fIndex, thatHead->fData, thatHead->used() * sizeof(uint32_t));
            fTail->fIndex += thatHead->used();
            thatHead = thatHead->fNext;
        }
        // If we still have a 'thatHead', we ran out of room to copy full blocks
        // into this buffer's current tail. Simply link the remaining nodes.
        if (thatHead) {
            fTail->fNext = thatHead;
            fTail = that->fTail;
        }
    }

    fCount += that->fCount;
    fDeviceType = SkTMax(fDeviceType, that->fDeviceType);
    fLocalType = SkTMax(fLocalType, that->fLocalType);
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
    SkASSERT((fCurrentBlock->fData + fNextIndex) == static_cast<const uint32_t*>(static_cast<const void*>(coords)));
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
