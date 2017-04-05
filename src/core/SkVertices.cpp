/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkVertices.h"
#include "SkData.h"
#include "SkReader32.h"
#include "SkWriter32.h"

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

struct SkVertices::Sizes {
    Sizes(int vertexCount, int indexCount, bool hasTexs, bool hasColors) {
        int64_t vSize = (int64_t)vertexCount * sizeof(SkPoint);
        int64_t tSize = hasTexs ? (int64_t)vertexCount * sizeof(SkPoint) : 0;
        int64_t cSize = hasColors ? (int64_t)vertexCount * sizeof(SkColor) : 0;
        int64_t iSize = (int64_t)indexCount * sizeof(uint16_t);

        int64_t total = sizeof(SkVertices) + vSize + tSize + cSize + iSize;
        if (!sk_64_isS32(total)) {
            sk_bzero(this, sizeof(*this));
        } else {
            fTotal = SkToSizeT(total);
            fVSize = SkToSizeT(vSize);
            fTSize = SkToSizeT(tSize);
            fCSize = SkToSizeT(cSize);
            fISize = SkToSizeT(iSize);
            fArrays = fTotal - sizeof(SkVertices);  // just the sum of the arrays
        }
    }

    bool isValid() const { return fTotal != 0; }

    size_t fTotal;  // size of entire SkVertices allocation (obj + arrays)
    size_t fArrays; // size of all the arrays (V + T + C + I)
    size_t fVSize;
    size_t fTSize;
    size_t fCSize;
    size_t fISize;
};

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount,
                             uint32_t builderFlags) {
    bool hasTexs = SkToBool(builderFlags & SkVertices::kHasTexCoords_BuilderFlag);
    bool hasColors = SkToBool(builderFlags & SkVertices::kHasColors_BuilderFlag);
    this->init(mode, vertexCount, indexCount,
               SkVertices::Sizes(vertexCount, indexCount, hasTexs, hasColors));
}

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount,
                             const SkVertices::Sizes& sizes) {
    this->init(mode, vertexCount, indexCount, sizes);
}

void SkVertices::Builder::init(VertexMode mode, int vertexCount, int indexCount,
                               const SkVertices::Sizes& sizes) {
    if (!sizes.isValid()) {
        return; // fVertices will already be null
    }

    void* storage = ::operator new (sizes.fTotal);
    fVertices.reset(new (storage) SkVertices);

    // need to point past the object to store the arrays
    char* ptr = (char*)storage + sizeof(SkVertices);

    fVertices->fPositions = (SkPoint*)ptr;                          ptr += sizes.fVSize;
    fVertices->fTexs = sizes.fTSize ? (SkPoint*)ptr : nullptr;      ptr += sizes.fTSize;
    fVertices->fColors = sizes.fCSize ? (SkColor*)ptr : nullptr;    ptr += sizes.fCSize;
    fVertices->fIndices = sizes.fISize ? (uint16_t*)ptr : nullptr;
    fVertices->fVertexCnt = vertexCount;
    fVertices->fIndexCnt = indexCount;
    fVertices->fMode = mode;
    // We defer assigning fBounds and fUniqueID until detach() is called
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds.set(fVertices->fPositions, fVertices->fVertexCnt);
        fVertices->fUniqueID = next_id();
        return std::move(fVertices);        // this will null fVertices after the return
    }
    return nullptr;
}

int SkVertices::Builder::vertexCount() const {
    return fVertices ? fVertices->vertexCount() : 0;
}

int SkVertices::Builder::indexCount() const {
    return fVertices ? fVertices->indexCount() : 0;
}

SkPoint* SkVertices::Builder::positions() {
    return fVertices ? const_cast<SkPoint*>(fVertices->positions()) : nullptr;
}

SkPoint* SkVertices::Builder::texCoords() {
    return fVertices ? const_cast<SkPoint*>(fVertices->texCoords()) : nullptr;
}

SkColor* SkVertices::Builder::colors() {
    return fVertices ? const_cast<SkColor*>(fVertices->colors()) : nullptr;
}

uint16_t* SkVertices::Builder::indices() {
    return fVertices ? const_cast<uint16_t*>(fVertices->indices()) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkVertices::MakeCopy(VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkPoint texs[],
                                       const SkColor colors[], int indexCount,
                                       const uint16_t indices[]) {
    Sizes sizes(vertexCount, indexCount, texs != nullptr, colors != nullptr);
    if (!sizes.isValid()) {
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, sizes);
    SkASSERT(builder.isValid());

    sk_careful_memcpy(builder.positions(), pos, sizes.fVSize);
    sk_careful_memcpy(builder.texCoords(), texs, sizes.fTSize);
    sk_careful_memcpy(builder.colors(), colors, sizes.fCSize);
    sk_careful_memcpy(builder.indices(), indices, sizes.fISize);

    return builder.detach();
}

size_t SkVertices::approximateSize() const {
    Sizes sizes(fVertexCnt, fIndexCnt, this->hasTexCoords(), this->hasColors());
    SkASSERT(sizes.isValid());
    return sizeof(SkVertices) + sizes.fArrays;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// storage = packed | vertex_count | index_count | pos[] | texs[] | colors[] | indices[]
//         = header + arrays

#define kMode_Mask          0x0FF
#define kHasTexs_Mask       0x100
#define kHasColors_Mask     0x200
#define kHeaderSize         (3 * sizeof(uint32_t))

sk_sp<SkData> SkVertices::encode() const {
    // packed has room for addtional flags in the future (e.g. versioning)
    uint32_t packed = static_cast<uint32_t>(fMode);
    SkASSERT((packed & ~kMode_Mask) == 0);  // our mode fits in the mask bits
    if (this->hasTexCoords()) {
        packed |= kHasTexs_Mask;
    }
    if (this->hasColors()) {
        packed |= kHasColors_Mask;
    }

    Sizes sizes(fVertexCnt, fIndexCnt, this->hasTexCoords(), this->hasColors());
    SkASSERT(sizes.isValid());
    // need to force alignment to 4 for SkWriter32 -- will pad w/ 0s as needed
    const size_t size = SkAlign4(kHeaderSize + sizes.fArrays);

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    SkWriter32 writer(data->writable_data(), data->size());

    writer.write32(packed);
    writer.write32(fVertexCnt);
    writer.write32(fIndexCnt);
    writer.write(fPositions, sizes.fVSize);
    writer.write(fTexs, sizes.fTSize);
    writer.write(fColors, sizes.fCSize);
    // if index-count is odd, we won't be 4-bytes aligned, so we call the pad version
    writer.writePad(fIndices, sizes.fISize);

    return data;
}

sk_sp<SkVertices> SkVertices::Decode(const void* data, size_t length) {
    if (length < kHeaderSize) {
        return nullptr;
    }

    SkReader32 reader(data, length);

    const uint32_t packed = reader.readInt();
    const int vertexCount = reader.readInt();
    const int indexCount = reader.readInt();

    const VertexMode mode = static_cast<VertexMode>(packed & kMode_Mask);
    const bool hasTexs = SkToBool(packed & kHasTexs_Mask);
    const bool hasColors = SkToBool(packed & kHasColors_Mask);
    Sizes sizes(vertexCount, indexCount, hasTexs, hasColors);
    if (!sizes.isValid()) {
        return nullptr;
    }
    // logically we can be only 2-byte aligned, but our buffer is always 4-byte aligned
    if (SkAlign4(kHeaderSize + sizes.fArrays) != length) {
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, sizes);

    reader.read(builder.positions(), sizes.fVSize);
    reader.read(builder.texCoords(), sizes.fTSize);
    reader.read(builder.colors(), sizes.fCSize);
    reader.read(builder.indices(), sizes.fISize);
    
    return builder.detach();
}
