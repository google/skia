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

sk_sp<SkVertices> SkVertices::Alloc(int vCount, int iCount, uint32_t builderFlags,
                                    size_t* arraySize) {
    int64_t vSize = (int64_t)vCount * sizeof(SkPoint);
    int64_t tSize = (builderFlags & kHasTexs_Flag) ? (int64_t)vCount * sizeof(SkPoint) : 0;
    int64_t cSize = (builderFlags & kHasColors_Flag) ? (int64_t)vCount * sizeof(SkColor) : 0;
    int64_t iSize = (int64_t)iCount * sizeof(uint16_t);

    int64_t total = sizeof(SkVertices) + vSize + tSize + cSize + iSize;
    if (!sk_64_isS32(total)) {
        return nullptr;
    }
    size_t size = (size_t)total;
    *arraySize = size - sizeof(SkVertices);

    void* storage = ::operator new (size);
    sk_sp<SkVertices> verts(new (storage) SkVertices);

    char* ptr = (char*)storage + sizeof(SkVertices);

    verts->fPositions = (SkPoint*)ptr;  ptr += vSize;
    verts->fTexs = (SkPoint*)ptr;       ptr += tSize;
    verts->fColors = (SkColor*)ptr;     ptr += cSize;
    verts->fIndices = (uint16_t*)ptr;
    verts->fVertexCnt = vCount;
    verts->fIndexCnt = iCount;
    // does not set fBounds, fMode, fUniqueID

    return verts;
}

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

SkVertices::Builder::Builder(SkCanvas::VertexMode mode, int vertexCount, int indexCount,
                             uint32_t flags) {
    fVertices = SkVertices::Alloc(vertexCount, indexCount, flags, &fArraySize);
    if (fVertices) {
        fVertices->fMode = mode;
    }
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds.set(fVertices->fPositions, fVertices->fVertexCnt);
        fVertices->fUniqueID = next_id();
        return std::move(fVertices);
    }
    return nullptr;
}

sk_sp<SkVertices> SkVertices::MakeCopy(SkCanvas::VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkPoint texs[],
                                       const SkColor colors[], int indexCount,
                                       const uint16_t indices[]) {
    uint32_t flags = 0;
    if (texs) {
        flags |= kHasTexs_Flag;
    }
    if (colors) {
        flags |= kHasColors_Flag;
    }
    Builder builder(mode, vertexCount, indexCount, flags);
    if (!builder.isValid()) {
        return nullptr;
    }

    memcpy(builder.positions(), pos, vertexCount * sizeof(SkPoint));
    if (texs) {
        memcpy(builder.texCoords(), texs, vertexCount * sizeof(SkPoint));
    }
    if (colors) {
        memcpy(builder.colors(), colors, vertexCount * sizeof(SkColor));
    }
    if (indices) {
        memcpy(builder.indices(), indices, indexCount * sizeof(uint16_t));
    }
    return builder.detach();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// storage = flags | vertex_count | index_count | pos[] | texs[] | colors[] | indices[]

#define kMode_Mask          0x0FF
#define kHasTexs_Mask       0x100
#define kHasColors_Mask     0x200

sk_sp<SkData> SkVertices::encode() const {
    uint32_t flags = static_cast<uint32_t>(fMode);
    SkASSERT((flags & ~kMode_Mask) == 0);
    if (fTexs) {
        flags |= kHasTexs_Mask;
    }
    if (fColors) {
        flags |= kHasColors_Mask;
    }

    size_t size = sizeof(uint32_t) * 3; // flags | verts_count | indices_count
    size += fVertexCnt * sizeof(SkPoint);
    if (fTexs) {
        size += fVertexCnt * sizeof(SkPoint);
    }
    if (fColors) {
        size += fVertexCnt * sizeof(SkColor);
    }
    size += fIndexCnt * sizeof(uint16_t);

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    SkWriter32 writer(data->writable_data(), data->size());

    writer.write32(flags);
    writer.write32(fVertexCnt);
    writer.write32(fIndexCnt);
    writer.write(fPositions, fVertexCnt * sizeof(SkPoint));
    if (fTexs) {
        writer.write(fTexs, fVertexCnt * sizeof(SkPoint));
    }
    if (fColors) {
        writer.write(fColors, fVertexCnt * sizeof(SkColor));
    }
    writer.write(fIndices, fIndexCnt * sizeof(uint16_t));

    return data;
}

sk_sp<SkVertices> SkVertices::Decode(const void* data, size_t length) {
    if (length < 3 * sizeof(uint32_t)) {
        return nullptr; // buffer too small
    }

    SkReader32 reader(data, length);

    uint32_t storageFlags = reader.readInt();
    SkCanvas::VertexMode mode = static_cast<SkCanvas::VertexMode>(storageFlags & kMode_Mask);
    int vertexCount = reader.readInt();
    int indexCount = reader.readInt();
    length -= 3 * sizeof(uint32_t); // already read the header

    uint32_t builderFlags = 0;
    if (storageFlags & kHasTexs_Mask) {
        builderFlags |= SkVertices::kHasTexs_Flag;
    }
    if (storageFlags & kHasColors_Mask) {
        builderFlags |= SkVertices::kHasColors_Flag;
    }

    Builder builder(mode, vertexCount, indexCount, builderFlags);
    if (!builder.isValid()) {
        return nullptr;
    }

    if (length < builder.fArraySize) {    // data too small to read all the arrays
        return nullptr;
    }

    reader.read(builder.positions(), vertexCount * sizeof(SkPoint));
    if (builderFlags & SkVertices::kHasTexs_Flag) {
        reader.read(builder.texCoords(), vertexCount * sizeof(SkPoint));
    }
    if (builderFlags & SkVertices::kHasColors_Flag) {
        reader.read(builder.colors(), vertexCount * sizeof(SkColor));
    }
    reader.read(builder.indices(), indexCount * sizeof(uint16_t));
    
    return builder.detach();
}
