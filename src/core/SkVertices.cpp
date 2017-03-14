/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"

SkVertices::Builder::Builder(SkCanvas::VertexMode mode, int vertexCount, int indexCount,
                             uint32_t flags) {
    fPositions = nullptr;   // signal that we have nothing to cleanup
    fColors = nullptr;
    fTexs = nullptr;
    fIndices = nullptr;
    fVertexCnt = 0;
    fIndexCnt = 0;

    // If we public merge drawPoints and drawVertices to share this object, we can perform
    // meaningful checks on counts based on mode.
#if 0
    if (vertexCount <= 2) {
        return;
    }
    if (indexCount && indexCount <= 2) {
        return;
    }
#endif

    uint64_t size = vertexCount * sizeof(SkPoint);
    if (flags & kHasTexs_Flag) {
        size += vertexCount * sizeof(SkPoint);
    }
    if (flags & kHasColors_Flag) {
        size += vertexCount * sizeof(SkColor);
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
