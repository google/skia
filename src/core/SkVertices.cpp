/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"
#include "SkAtomics.h"

static int32_t gNextID = 1;
static int32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return id;
}

SkVertices::SkVertices(SkCanvas::VertexMode mode, std::unique_ptr<const SkPoint[]> positions,
           std::unique_ptr<const SkColor[]> colors, std::unique_ptr<const SkPoint[]> texs,
           int vertexCnt, std::unique_ptr<const uint16_t[]> indices, int indexCnt,
           const SkRect* bounds)
    : fUniqueID(next_id())
    , fMode(mode)
    , fVertexCnt(vertexCnt)
    , fIndexCnt(indexCnt)
    , fPositions(std::move(positions))
    , fColors(std::move(colors))
    , fTexs(std::move(texs))
    , fIndices(std::move(indices))
{
    SkASSERT(SkToBool(fPositions) && SkToBool(fVertexCnt));
    SkASSERT(SkToBool(fIndices) == SkToBool(fIndexCnt));
    if (bounds) {
#ifdef SK_DEBUG
        fBounds.setBounds(fPositions.get(), fVertexCnt);
        SkASSERT(bounds->fLeft <= fBounds.fLeft && bounds->fRight >= fBounds.fRight &&
                 bounds->fTop <= fBounds.fTop && bounds->fBottom >= fBounds.fBottom);
#endif
        fBounds = *bounds;
    } else {
        fBounds.setBounds(fPositions.get(), fVertexCnt);
    }
#ifdef SK_DEBUG
    for (int i = 0; i < fIndexCnt; ++i) {
        SkASSERT(fIndices[i] < fVertexCnt);
    }
#endif
}

