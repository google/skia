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

enum class InlineFirstQuad : bool {
    kNo = false,
    kYes = true
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Header, Block, and InlineBlock are internal structs to the GrQuadBuffer template.
// However, they do not themselves depend on the template parameters so are defined outside
// of their scope to avoid duplication (and allow )

template<typename T, InlineFirstQuad kInline>
class GrQuadBuffer {
public:
    GrQuadBuffer()
            : fCount(0)
            , fDeviceType(static_cast<uint32_t>(GrQuad::Type::kAxisAligned))
            , fLocalType(static_cast<uint32_t>(GrQuad::Type::kAxisAligned)) {
        // A fullsize block will be allocated on the first append if fHead is null.
        fHead = kInline == InlineFirstQuad::kYes ? &fFirstQuad.fBlock : nullptr;
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
    void append(const GrQuad& deviceQuad, T&& metadata, const GrQuad* localQuad = nullptr);

    // Copies all entries from 'that' to this buffer
    void concat(const GrQuadBuffer<T>& that);

    // Provides a read-only iterator over a quad buffer, giving access to the device quad, metadata
    // and optional local quad.
    class Iter {
    public:
        Iter(const GrQuadBuffer<T>* buffer)
                : fDeviceQuad(SkRect::MakeEmpty())
                , fLocalQuad(SkRect::MakeEmpty())
                , fBuffer(buffer)
                , fCurrentEntry(nullptr)
                , fNextEntry(buffer->fData.begin()) {
            SkDEBUGCODE(fExpectedCount = buffer->count();)
        }

        bool next();

        const T& metadata() const { this->validate(); return *(fBuffer->metadata(fCurrentEntry)); }

        // The returned pointer is mutable so that the object can be used for scratch calculations
        // during op preparation. However, any changes are not persisted in the GrQuadBuffer and
        // subsequent calls to next() will overwrite the state of the GrQuad.
        GrQuad* deviceQuad() { this->validate(); return &fDeviceQuad; }

        // If isLocalValid() returns false, this returns nullptr. Otherwise, the returned pointer
        // is mutable in the same manner as deviceQuad().
        GrQuad* localQuad() {
            this->validate();
            return this->isLocalValid() ? &fLocalQuad : nullptr;
        }

        bool isLocalValid() const {
            this->validate();
            return fBuffer->header(fCurrentEntry)->fHasLocals;
        }

    private:
        // Quads are stored locally so that calling code doesn't need to re-declare their own quads
        GrQuad fDeviceQuad;
        GrQuad fLocalQuad;

        const GrQuadBuffer<T>* fBuffer;
        // FIXME this iteration will get a little trickier
        // The pointer to the current entry to read metadata/header details from
        const char* fCurrentEntry;
        // The pointer to replace fCurrentEntry when next() is called, cached since it is calculated
        // automatically while unpacking the quad data.
        const char* fNextEntry;

        SkDEBUGCODE(int fExpectedCount;)

        void validate() const {
            SkDEBUGCODE(fBuffer->validate(fCurrentEntry, fExpectedCount);)
        }
    };

    Iter iterator() const { return Iter(this); }

    // Provides a *mutable* iterator over just the metadata stored in the quad buffer. This skips
    // unpacking the device and local quads into GrQuads and is intended for use during op
    // finalization, which may require rewriting state such as color.
    class MetadataIter {
    public:
        MetadataIter(GrQuadBuffer<T>* list)
                : fBuffer(list)
                , fCurrentEntry(nullptr) {
            SkDEBUGCODE(fExpectedCount = list->count();)
        }

        bool next();

        T& operator*() { this->validate(); return *(fBuffer->metadata(fCurrentEntry)); }

        T* operator->() { this->validate(); return fBuffer->metadata(fCurrentEntry); }

    private:
        GrQuadBuffer<T>* fBuffer;
        // FIXME this iteration also gets trickier (may not need to hold on to the GrQuadBuffer pointer
        // though since we can just iterate over the linked blocks at this point).
        char* fCurrentEntry;

        SkDEBUGCODE(int fExpectedCount;)

        void validate() const {
            SkDEBUGCODE(fBuffer->validate(fCurrentEntry, fExpectedCount);)
        }
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

    struct Block {
        // The next block in the linked list of blocks. A block should not have new quads added to
        // its data if it already has a next block.
        Block* fNext;
        // Size of fData, which is dynamic and allocated on the arena.
        uint32_t fBlockSize : 16;
        // Index of the next quad entry to be appended to the block.
        uint32_t fIndex     : 16;
        // Must be at the end because it fills the allocation from the arena. fBlockSize and fIndex
        // are bit fields of int32_t to avoid padding between fIndex and fData.
        // fData stores contiguous Headers, T's, and float coordinates (all 4 byte aligned).
        uint32_t fData[1];

        int remaining() const { return fBlockSize - fIndex; }
    };
    static_assert(sizeof(Block) == 16);

    template<int kBlockSize>
    struct InlineBlock {
        InlineBlock() {
            fBlock.fNext = nullptr;
            fBlock.fBlockSize = kBlockSize;
            fBlock.fIndex = 0;
        }

        Block fBlock;
        // The rest of fData (subtract one since Block holds the first uint32_t of fData already).
        uint32_t fRemaining[kBlockSize - 1];
    };

    // If the GrQuadBuffer is not inlining a block, it shouldn't increase the size of the type.
    template struct InlineBlock<0> {};
    static_assert(sizeof(InlineBlock<0>) == 0);

    static constexpr unsigned kSentinel = 0xbaffe;
    static constexpr int kMetaSize = sizeof(Header) + sizeof(T);
    static constexpr int k2DQuadFloats = 8;
    static constexpr int k3DQuadFloats = 12;

    // If inlining, hold onto enough bytes to store a 3D quad with 2D local coords and the metadata
    static constexpr int kInlineBlockSize = kInline == InlineFirstQuad::kYes
            ? (kMetaSize + sizeof(float) * (k2DQuadFloats + k3DQuadFloats))
            : 0;

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
    // FIXME (michaelludwig) - Since this is intended only for ops, can we use the arena to
    //      allocate storage for the quad buffer? Since this is forward-iteration only, could also
    //      explore a linked-list structure for concatenating quads when batching ops
    // SkSTArray<kMetaSize + sizeof(float) * 2 * k2DQuadFloats, char, true> fData;

    InlineBlock<kInlineBlockSize> fFirstQuad;
    // These either point to fFirstQuad's fBlock, or to a block allocated in an SkArenaAlloc.
    Block*                        fHead;
    Block*                        fTail;

    // GrTextureOp's performance is highly sensitive to op size, so make this as small as possible.
    // Packing everything into 32 bits doesn't really limit us since the quad index buffers are
    // much smaller than the 28 bits for this 'count'.
    uint32_t fDeviceType : 2; // GrQuad::Type
    uint32_t fLocalType  : 2; // ""
    uint32_t fCount      : 28;

    static_assert(GrQuad::TypeCount <= 4);

    inline int entrySize(GrQuad::Type deviceType, const GrQuad::Type* localType) const {
        int size = kMetaSize;
        size += (deviceType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                          : k2DQuadFloats) * sizeof(float);
        if (localType) {
            size += (*localType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                              : k2DQuadFloats) * sizeof(float);
        }
        return size;
    }
    inline int entrySize(const Header* header) const {
        if (header->fHasLocals) {
            GrQuad::Type localType = static_cast<GrQuad::Type>(header->fLocalType);
            return this->entrySize(static_cast<GrQuad::Type>(header->fDeviceType), &localType);
        } else {
            return this->entrySize(static_cast<GrQuad::Type>(header->fDeviceType), nullptr);
        }
    }

    // Helpers to access typed sections of the buffer, given the start of an entry
    inline Header* header(char* entry) {
        return static_cast<Header*>(static_cast<void*>(entry));
    }
    inline const Header* header(const char* entry) const {
        return static_cast<const Header*>(static_cast<const void*>(entry));
    }

    inline T* metadata(char* entry) {
        return static_cast<T*>(static_cast<void*>(entry + sizeof(Header)));
    }
    inline const T* metadata(const char* entry) const {
        return static_cast<const T*>(static_cast<const void*>(entry + sizeof(Header)));
    }

    inline float* coords(char* entry) {
        return static_cast<float*>(static_cast<void*>(entry + kMetaSize));
    }
    inline const float* coords(const char* entry) const {
        return static_cast<const float*>(static_cast<const void*>(entry + kMetaSize));
    }

    // Helpers to convert from coordinates to GrQuad and vice versa, returning pointer to the
    // next packed quad coordinates.
    float* packQuad(const GrQuad& quad, float* coords);
    const float* unpackQuad(GrQuad::Type type, const float* coords, GrQuad* quad) const;

#ifdef SK_DEBUG
    void validate(const char* entry, int expectedCount) const;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
float* GrQuadBuffer<T>::packQuad(const GrQuad& quad, float* coords) {
    // Copies all 12 (or 8) floats at once, so requires the 3 arrays to be contiguous
    // FIXME(michaelludwig) - If this turns out not to be the case, just do 4 copies
    SkASSERT(quad.xs() + 4 == quad.ys() && quad.xs() + 8 == quad.ws());
    if (quad.hasPerspective()) {
        memcpy(coords, quad.xs(), k3DQuadFloats * sizeof(float));
        return coords + k3DQuadFloats;
    } else {
        memcpy(coords, quad.xs(), k2DQuadFloats * sizeof(float));
        return coords + k2DQuadFloats;
    }
}

template<typename T>
const float* GrQuadBuffer<T>::unpackQuad(GrQuad::Type type, const float* coords, GrQuad* quad) const {
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

template<typename T>
void GrQuadBuffer<T>::append(const GrQuad& deviceQuad, T&& metadata, const GrQuad* localQuad) {
    GrQuad::Type localType = localQuad ? localQuad->quadType() : GrQuad::Type::kAxisAligned;
    int entrySize = this->entrySize(deviceQuad.quadType(), localQuad ? &localType : nullptr);

    // If we don't have a head, or entrySize doesn't fit in remaining on tail, we allocate
    // a new block of some size(?) --> fixed size, or fibonacci like the arena? definitely some
    // strat in being pessimestic at first and then growing more when we realize we have a large
    // amount of ops that are batchable.
    //
    // So we then allocate, which is standard set head or set tail's next to the new block, tail
    // becomes the new block.
    //
    // Then we're left with our current tail that definitely has enough room to fit 'entrySize'
    // bytes. At this point, we ought to be able to write into the block's fData basically the
    // same as the old char* fData.

    //
    // Hmm, remaining() ought to be in bytes, since we will likely be using makeBytesAlignedTo()
    // to get the new Block and then placement new it. (if we actually placement new it, we need
    // to give it a useful constructor. Which will also make InlineBlock easier).

    // Since we have some flexibility in how we expand to our next block, it would be nice if
    // we could ask the SkArenaAllocator for how much is left on one of its pages. If the page
    // size is less than we'd have requested, but not unreasonable, might as well request that
    // so we don't waste space. If we continue to batch, the next append will get the big page.

    // Fill in the entry, as described in fData's declaration
    char* entry = fData.push_back_n(entrySize);
    // First the header
    Header* h = this->header(entry);
    h->fDeviceType = static_cast<uint32_t>(deviceQuad.quadType());
    h->fHasLocals = static_cast<uint32_t>(localQuad != nullptr);
    h->fLocalType = static_cast<uint32_t>(localQuad ? localQuad->quadType()
                                                    : GrQuad::Type::kAxisAligned);
    SkDEBUGCODE(h->fSentinel = static_cast<uint32_t>(kSentinel);)

    // Second, the fixed-size metadata
    static_assert(alignof(T) == 4, "Metadata must be 4 byte aligned");
    *(this->metadata(entry)) = std::move(metadata);

    // Then the variable blocks of x, y, and w float coordinates
    float* coords = this->coords(entry);
    coords = this->packQuad(deviceQuad, coords);
    if (localQuad) {
        coords = this->packQuad(*localQuad, coords);
    }
    SkASSERT((char*)coords - entry == entrySize);

    // Entry complete, update buffer-level state
    fCount++;
    if (deviceQuad.quadType() > fDeviceType) {
        fDeviceType = deviceQuad.quadType();
    }
    if (localQuad && localQuad->quadType() > fLocalType) {
        fLocalType = localQuad->quadType();
    }
}

template<typename T>
void GrQuadBuffer<T>::concat(const GrQuadBuffer<T>& that) {

    // FIXME when concatenating two quad buffers, what do we do??
    // Regardless of first quad policy, we should consume blocks from 'that'
    // and memcpy them into the tail block until there's no more room.
    // But we only memcpy if the whole block were to fit, so once done with
    // the copy phase, we're left with 0 or more complete blocks. If we have
    // > 0, the first one becomes the next block of this's tail block and
    // this's tail block is updated to the tail block of 'that'.

    char* begin = fData.push_back_n(that.fData.count());
    memcpy(begin, that.fData.begin(), that.fData.count() * sizeof(char));
    fCount += that.fCount;
    if (that.fDeviceType > fDeviceType) {
        fDeviceType = that.fDeviceType;
    }
    if (that.fLocalType > fLocalType) {
        fLocalType = that.fLocalType;
    }
}

#ifdef SK_DEBUG
template<typename T>
void GrQuadBuffer<T>::validate(const char* entry, int expectedCount) const {
    // Triggers if accessing before next() is called on an iterator
    SkASSERT(entry);
    // Triggers if accessing after next() returns false
    SkASSERT(entry < fData.end());
    // Triggers if elements have been added to the buffer while iterating entries
    SkASSERT(expectedCount == fCount);
    // Make sure the start of the entry looks like a header
    SkASSERT(this->header(entry)->fSentinel == kSentinel);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Iterator implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool GrQuadBuffer<T>::Iter::next() {
    SkASSERT(fNextEntry);
    if (fNextEntry >= fBuffer->fData.end()) {
        return false;
    }
    // There is at least one more entry, so store the current start for metadata access
    fCurrentEntry = fNextEntry;

    // And then unpack the device and optional local coordinates into fDeviceQuad and fLocalQuad
    const Header* h = fBuffer->header(fCurrentEntry);
    const float* coords = fBuffer->coords(fCurrentEntry);
    coords = fBuffer->unpackQuad(static_cast<GrQuad::Type>(h->fDeviceType), coords, &fDeviceQuad);
    if (h->fHasLocals) {
        coords = fBuffer->unpackQuad(static_cast<GrQuad::Type>(h->fLocalType), coords, &fLocalQuad);
    } // else localQuad() will return a nullptr so no need to reset fLocalQuad

    // At this point, coords points to the start of the next entry
    fNextEntry = static_cast<const char*>(static_cast<const void*>(coords));
    SkASSERT((fNextEntry - fCurrentEntry) == fBuffer->entrySize(h));
    return true;
}

template<typename T>
bool GrQuadBuffer<T>::MetadataIter::next() {
    if (fCurrentEntry) {
        // Advance pointer by entry size
        if (fCurrentEntry < fBuffer->fData.end()) {
            const Header* h = fBuffer->header(fCurrentEntry);
            fCurrentEntry += fBuffer->entrySize(h);
        }
    } else {
        // First call to next
        fCurrentEntry = fBuffer->fData.begin();
    }
    // Nothing else is needed to do but report whether or not the updated pointer is valid
    return fCurrentEntry < fBuffer->fData.end();
}
#endif  // GrQuadBuffer_DEFINED
