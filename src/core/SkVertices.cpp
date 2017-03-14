/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"
#include "SkData.h"
#include "SkReader32.h"
#include "SkWriter32.h"

static size_t compute_arrays_size(int vertexCount, int indexCount, uint32_t builderFlags) {
    if (vertexCount < 0 || indexCount < 0) {
        return 0;   // signal error
    }

    uint64_t size = vertexCount * sizeof(SkPoint);
    if (builderFlags & SkVertices::kHasTexs_Flag) {
        size += vertexCount * sizeof(SkPoint);
    }
    if (builderFlags & SkVertices::kHasColors_Flag) {
        size += vertexCount * sizeof(SkColor);
    }
    size += indexCount * sizeof(uint16_t);
    if (!sk_64_isS32(size)) {
        return 0;   // signal error
    }
    return (size_t)size;
}

SkVertices::Builder::Builder(SkCanvas::VertexMode mode, int vertexCount, int indexCount,
                             uint32_t flags) {
    fPositions = nullptr;   // signal that we have nothing to cleanup
    fColors = nullptr;
    fTexs = nullptr;
    fIndices = nullptr;
    fVertexCnt = 0;
    fIndexCnt = 0;

    size_t size = compute_arrays_size(vertexCount, indexCount, flags);
    if (0 == size) {
        return;
    }

    char* ptr = (char*)sk_malloc_throw(sk_64_asS32(size));

    fMode = mode;
    fVertexCnt = vertexCount;
    fIndexCnt = indexCount;
    fPositions = (SkPoint*)ptr;  // owner
    ptr += vertexCount * sizeof(SkPoint);

    if (flags & kHasTexs_Flag) {
        fTexs = (SkPoint*)ptr;
        ptr += vertexCount * sizeof(SkPoint);
    }
    if (flags & kHasColors_Flag) {
        fColors = (SkColor*)ptr;
        ptr += vertexCount * sizeof(SkColor);
    }
    if (indexCount) {
        fIndices = (uint16_t*)ptr;
    }
}

SkVertices::Builder::~Builder() {
    sk_free(fPositions);
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (!fPositions) {
        return nullptr;
    }

    SkVertices* obj = new SkVertices;
    obj->fPositions = fPositions;  // owner of storage, use sk_free
    obj->fTexs = fTexs;
    obj->fColors = fColors;
    obj->fIndices = fIndices;
    obj->fBounds.set(fPositions, fVertexCnt);
    obj->fVertexCnt = fVertexCnt;
    obj->fIndexCnt = fIndexCnt;
    obj->fMode = fMode;

    fPositions = nullptr;   // so we don't free the memory, now that obj owns it

    return sk_sp<SkVertices>(obj);
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
    uint32_t builderFlags = 0;
    if (storageFlags & kHasTexs_Mask) {
        builderFlags |= SkVertices::kHasTexs_Flag;
    }
    if (storageFlags & kHasColors_Mask) {
        builderFlags |= SkVertices::kHasColors_Flag;
    }

    size_t size = compute_arrays_size(vertexCount, indexCount, builderFlags);
    if (0 == size) {
        return nullptr;
    }

    length -= 3 * sizeof(uint32_t); // already read the header
    if (length < size) {    // buffer too small
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, builderFlags);
    if (!builder.isValid()) {
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
