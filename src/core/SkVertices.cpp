/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"

SkVertices::Builder::Builder(SkCanvas::VertexMode mode, int vertexCount, uint32_t flags,
                             const SkRect* bounds) {
    fVertexCnt = 0;
    fIndexCnt = 0;
    fPositions = nullptr;   // signal that we have nothing to cleanup
    fColors = nullptr;
    fTexs = nullptr;
    fIndices = nullptr;

    if (vertexCount <= 2) {
        return;
    }

    int indexCount = 0;
    if (flags & kHasIndices_Flag) {
        switch (mode) {
            case SkCanvas::kTriangles_VertexMode:
                indexCount = vertexCount * 3;
                break;
            case SkCanvas::kTriangleStrip_VertexMode:
            case SkCanvas::kTriangleFan_VertexMode:
                indexCount = vertexCount + 1;
                break;
        }
    }
    if (indexCount < 0) {
        return;
    }

    uint64_t size = vertexCount * sizeof(SkPoint);
    if (flags & kHasColors_Flag) {
        size += vertexCount * sizeof(SkColor);
    }
    if (flags & kHasTexs_Flag) {
        size += vertexCount * sizeof(SkPoint);
    }
    size += indexCount * sizeof(uint16_t);
    if (!sk_64_isS32(size)) {
        return;
    }

    char* ptr = (char*)sk_malloc_throw(sk_64_asS32(size));

    fMode = mode;
    fVertexCnt = vertexCount;
    fIndexCnt = indexCount;
    fPositions = (SkPoint*)ptr;  // owner
    ptr += vertexCount * sizeof(SkPoint);

    if (flags & kHasColors_Flag) {
        fColors = (SkColor*)ptr;
        ptr += vertexCount * sizeof(SkColor);
    }
    if (flags & kHasTexs_Flag) {
        fTexs = (SkPoint*)ptr;
        ptr += vertexCount * sizeof(SkPoint);
    }
    if (flags & kHasIndices_Flag) {
        fIndices = (uint16_t*)ptr;
    }

    if (bounds) {
        fBounds = *bounds;
    } else {
        fBounds.setEmpty(); // sentinel that we need to compute later
    }
}

SkVertices::Builder::~Builder() {
    if (fPositions) {
        sk_free(fPositions);
    }
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (!fPositions) {
        return nullptr;
    }

    if (fBounds.isEmpty()) {
        fBounds.set(fPositions, fVertexCnt);
    }

    SkVertices* obj = new SkVertices;
    obj->fMode = fMode;
    obj->fVertexCnt = fVertexCnt;
    obj->fIndexCnt = fIndexCnt;
    obj->fPositions = fPositions;  // owner of storage, use sk_free
    obj->fColors = fColors;
    obj->fTexs = fTexs;
    obj->fIndices = fIndices;
    obj->fBounds = fBounds;

    fPositions = nullptr;   // so we don't free the memory, now that obj owns it

    return sk_sp<SkVertices>(obj);
}

sk_sp<SkVertices> SkVertices::MakeCopy(SkCanvas::VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkColor colors[],
                                       const SkPoint texs[], const uint16_t indices[],
                                       const SkRect* bounds) {
    uint32_t flags = 0;
    if (colors) {
        flags |= kHasColors_Flag;
    }
    if (texs) {
        flags |= kHasTexs_Flag;
    }
    if (indices) {
        flags |= kHasIndices_Flag;
    }
    Builder builder(mode, vertexCount, flags, bounds);
    if (!builder.positions()) {
        return nullptr;
    }
    SkASSERT(builder.vertexCount() == vertexCount);

    memcpy(builder.positions(), pos, vertexCount * sizeof(SkPoint));
    if (colors) {
        memcpy(builder.colors(), colors, vertexCount * sizeof(SkColor));
    }
    if (texs) {
        memcpy(builder.texCoords(), texs, vertexCount * sizeof(SkPoint));
    }
    if (indices) {
        memcpy(builder.indices(), indices, builder.indexCount() * sizeof(uint16_t));
    }
    return builder.detach();
}
