/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrQuadBuffer_DEFINED
#define GrQuadBuffer_DEFINED

#include "src/gpu/GrTAllocator.h"
#include "src/gpu/geometry/GrQuad.h"

template<typename T>
class GrQuadBuffer {
public:
    GrQuadBuffer() : fAggregate{static_cast<uint8_t>(GrQuad::Type::kAxisAligned),
                                static_cast<uint8_t>(GrQuad::Type::kAxisAligned),
                                false} {}

    // The number of device-space quads (and metadata, and optional local quads) that are in the
    // the buffer.
    int count() const { return fQuads.count(); }

    // The most general type for the device-space quads in this buffer
    GrQuad::Type deviceQuadType() const {
        return static_cast<GrQuad::Type>(fAggregate.fDeviceType);
    }

    // The most general type for the local quads; if no local quads are ever added, this will still
    // return kAxisAligned.
    GrQuad::Type localQuadType() const {
        return static_cast<GrQuad::Type>(fAggregate.fLocalType);
    }

    // Append the given 'deviceQuad' to this buffer, with its associated 'metadata'. If 'localQuad'
    // is not null, the local coordinates will also be attached to the entry. When an entry
    // has local coordinates, during iteration, the Iter::hasLocals() will return true and its
    // Iter::localQuad() will be equivalent to the provided local coordinates. If 'localQuad' is
    // null then Iter::hasLocals() will report false for the added entry.
    void append(const GrQuad& deviceQuad, T&& metadata, const GrQuad* localQuad = nullptr);

    // Moves all entries from 'that' to this buffer; 'that' becomes empty.
    void concat(GrQuadBuffer<T>& that);

    void reserve(int count, GrQuad::Type expectedDeviceType, GrQuad::Type expectedLocalType,
                 bool expectedHasLocals);

    /**
     * Provides a read-only iterator over a quad buffer, giving access to the device quad, metadata
     * and optional local quad, as follows:
     *
     *    auto iter = buffer.iterator();
     *    while(iter.next()) {
     *       // iter.metadata() -> const T&
     *       // iter.deviceQuad() -> GrQuad* (can be modified, but is not persisted)
     *       // iter.localQuad() -> GrQuad* (can be modified, can be null)
     *       // iter.isLocalValid() -> bool (true if localQuad() will not be null)
     *    }
     */
    class Iter;
    Iter iterator() const;

    /**
     * Provides a *mutable* iterator over just the metadata stored in the quad buffer. This skips
     * unpacking the device and local quads into GrQuads and is intended for using during op
     * finalization, which may require rewriting state such as color. Use as follows:
     *
     *    auto iter = buffer.metadata();
     *    while(iter.next()) {
     *        // *iter -> T&
     *        // iter->T::foo()
     *    }
     */
    class MetadataIter;
    MetadataIter metadata();

private:
    struct Header {
        uint8_t fDeviceType : 2;
        uint8_t fLocalType  : 2;
        uint8_t fHasLocals  : 1;
        static_assert(GrQuad::kTypeCount <= 4);
    };
    struct Metadata {
        T      fData;
        // Determines how to interpret the variable number of floats in an item
        Header fHeader;
    };

    static constexpr int k2DQuadFloats = 8;
    static constexpr int k3DQuadFloats = 12;

    // Each logical entry in the buffer is a variable length tuple storing device coordinates,
    // optional local coordinates, and metadata. An entry always has a header that defines the
    // quad types of device and local coordinates, and always has metadata of type T. The device
    // and local quads' data follows as a variable length array of floats:
    //  [ metadata  ] = sizeof(T) + 1 byte for our internal header
    //  [ device xs ] = 4 floats = 16 bytes
    //  [ device ys ] = 4 floats
    //  [ device ws ] = 4 floats or 0 floats depending on fDeviceType in header
    //  [ local xs  ] = 4 floats or 0 floats depending on fHasLocals in header
    //  [ local ys  ] = 4 floats or 0 floats depending on fHasLocals in header
    //  [ local ws  ] = 4 floats or 0 floats depending on fHasLocals and fLocalType in header
    using QuadList = GrTVAllocator<Metadata, float, 1, k2DQuadFloats + k3DQuadFloats>;
    QuadList fQuads;
    Header fAggregate;

    // Helpers to convert from coordinates to GrQuad and vice versa, returning pointer to the
    // next packed quad coordinates.
    static float* PackQuad(const GrQuad& quad, float* coords);
    static const float* UnpackQuad(GrQuad::Type type, const float* coords, GrQuad* quad);

#ifdef SK_DEBUG
    void validate(typename QuadList::CItem item, int expectedCount) const;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
float* GrQuadBuffer<T>::PackQuad(const GrQuad& quad, float* coords) {
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

template<typename T>
const float* GrQuadBuffer<T>::UnpackQuad(GrQuad::Type type, const float* coords,
                                         GrQuad* quad) {
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
    int numFloats = deviceQuad.hasPerspective() ? k3DQuadFloats : k2DQuadFloats;
    if (localQuad) {
        numFloats += localQuad->hasPerspective() ? k3DQuadFloats : k2DQuadFloats;
    }

    auto [m, coords, n] = fQuads.push_back(numFloats);
    SkASSERT(n == numFloats);

    // First the fixed data (the public metadata and internal quad type tracking)
    m.fData = std::move(metadata);
    m.fHeader.fDeviceType = static_cast<uint8_t>(deviceQuad.quadType());
    if (localQuad) {
        m.fHeader.fLocalType = static_cast<uint8_t>(localQuad->quadType());
        m.fHeader.fHasLocals = true;
    } else {
        m.fHeader.fLocalType = static_cast<uint8_t>(GrQuad::Type::kAxisAligned);
        m.fHeader.fHasLocals = false;
    }

    // Then the variable blocks of x, y, and w float coordinates
    SkDEBUGCODE(float* coordStart = coords;)
    coords = PackQuad(deviceQuad, coords);
    if (localQuad) {
        coords = PackQuad(*localQuad, coords);
    }
    SkASSERT((coords - coordStart) == n);

    // Entry complete, update aggregate state
    fAggregate.fDeviceType = std::max(fAggregate.fDeviceType, m.fHeader.fDeviceType);
    fAggregate.fLocalType = std::max(fAggregate.fLocalType, m.fHeader.fLocalType);
}

template<typename T>
void GrQuadBuffer<T>::reserve(int count, GrQuad::Type expectedDeviceType,
                              GrQuad::Type expectedLocalType, bool expectedHasLocals) {
    int numFloats = expectedDeviceType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                                     : k2DQuadFloats;
    if (expectedHasLocals) {
        numFloats += expectedLocalType == GrQuad::Type::kPerspective ? k3DQuadFloats
                                                                     : k2DQuadFloats;
    }
    fQuads.reserve(count, numFloats);
}

template<typename T>
void GrQuadBuffer<T>::concat(GrQuadBuffer<T>& that) {
    fQuads.concat(&that.fQuads);
    fAggregate.fDeviceType = std::max(fAggregate.fDeviceType, that.fAggregate.fDeviceType);
    fAggregate.fLocalType = std::max(fAggregate.fLocalType, that.fAggregate.fLocalType);
}

#ifdef SK_DEBUG
template<typename T>
void GrQuadBuffer<T>::validate(typename GrQuadBuffer<T>::QuadList::CItem item,
                               int expectedCount) const {
    SkASSERT(this->count() == expectedCount);

    auto header = std::get<0>(item).fHeader;
    int numFloats = std::get<2>(item);
    int expected =  static_cast<GrQuad::Type>(header.fDeviceType) == GrQuad::Type::kPerspective
                        ? k3DQuadFloats : k2DQuadFloats;
    if (header.fHasLocals) {
        expected += static_cast<GrQuad::Type>(header.fLocalType) == GrQuad::Type::kPerspective
                        ? k3DQuadFloats : k2DQuadFloats;
    }
    SkASSERT(numFloats == expected);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Iterator implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
class GrQuadBuffer<T>::Iter {
public:
    Iter(const GrQuadBuffer<T>* buffer, typename GrQuadBuffer<T>::QuadList::CIter iter)
            : fMetadata(nullptr)
            , fIter(iter.begin())
            , fEnd(iter.end()) {
        SkDEBUGCODE(fBuffer = buffer;)
        SkDEBUGCODE(fExpectedCount = buffer->count();)
    }

    bool next() {
        if (fIter != fEnd) {
            auto [t, coords, n] = *fIter;
            SkDEBUGCODE(fBuffer->validate(*fIter, fExpectedCount);)
            fMetadata = &t;

            SkDEBUGCODE(const float* coordStart = coords;)
            coords = UnpackQuad(static_cast<GrQuad::Type>(t.fHeader.fDeviceType),
                                coords, &fDeviceQuad);
            if (t.fHeader.fHasLocals) {
                coords = UnpackQuad(static_cast<GrQuad::Type>(t.fHeader.fLocalType),
                                    coords, &fLocalQuad);
            }
            SkASSERT((coords - coordStart) == n);

            fIter = ++fIter;
            return true;
        } else {
            return false;
        }
    }

    const T& metadata() const {
        SkASSERT(fMetadata);
        return fMetadata->fData;
    }

    // The returned pointer is mutable so that the object can be used for scratch calculations
    // during op preparation. However, any changes are not persisted in the GrQuadBuffer and
    // subsequent calls to next() will overwrite the state of the GrQuad.
    GrQuad* deviceQuad() {
        SkASSERT(fMetadata);
        return &fDeviceQuad;
    }

    // If isLocalValid() returns false, this returns nullptr. Otherwise, the returned pointer
    // is mutable in the same manner as deviceQuad().
    GrQuad* localQuad() {
        SkASSERT(fMetadata);
        return this->isLocalValid() ? &fLocalQuad : nullptr;
    }

    bool isLocalValid() const {
        SkASSERT(fMetadata);
        return fMetadata->fHeader.fHasLocals;
    }

private:
    // Quads are stored locally so that calling code doesn't need to re-declare their own quads
    GrQuad          fDeviceQuad;
    GrQuad          fLocalQuad;
    const Metadata* fMetadata;

    typename QuadList::CIter::Item fIter;
    typename QuadList::CIter::Item fEnd;

#ifdef SK_DEBUG
    const GrQuadBuffer<T>* fBuffer;
    int fExpectedCount;
#endif
};

template<typename T>
typename GrQuadBuffer<T>::Iter GrQuadBuffer<T>::iterator() const {
    return Iter(this, fQuads.items());
}

template<typename T>
class GrQuadBuffer<T>::MetadataIter {
public:
    MetadataIter(GrQuadBuffer<T>* buffer, typename GrQuadBuffer<T>::QuadList::Iter iter)
            : fMetadata(nullptr)
            , fIter(iter.begin())
            , fEnd(iter.end()) {
        SkDEBUGCODE(fBuffer = buffer;)
        SkDEBUGCODE(fExpectedCount = buffer->count();)
    }

    bool next() {
        if (fIter != fEnd) {
            SkDEBUGCODE(fBuffer->validate(*fIter, fExpectedCount);)
            fMetadata = &std::get<0>(*fIter).fData;
            fIter = ++fIter;
            return true;
        } else {
            return false;
        }
    }

    T& operator*() {
        SkASSERT(fMetadata);
        return *fMetadata;
    }

    T* operator->() {
        SkASSERT(fMetadata);
        return fMetadata;
    }

private:
    T* fMetadata;

    typename QuadList::Iter::Item fIter;
    typename QuadList::Iter::Item fEnd;

#ifdef SK_DEBUG
    const GrQuadBuffer<T>* fBuffer;
    int fExpectedCount;
#endif
};

template<typename T>
typename GrQuadBuffer<T>::MetadataIter GrQuadBuffer<T>::metadata() {
    return MetadataIter(this, fQuads.items());
}

#endif  // GrQuadBuffer_DEFINED
