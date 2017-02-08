/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVertices_DEFINED
#define SkVertices_DEFINED

#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"

/**
 * An immutable set of vertex data that can be used with SkCanvas::drawVertices. Clients are
 * encouraged to provide a bounds on the vertex positions if they can compute one more cheaply than
 * looping over the positions.
 */
class SkVertices : public SkNVRefCnt<SkVertices> {
public:
    static sk_sp<SkVertices> Make(SkCanvas::VertexMode mode,
                                  std::unique_ptr<const SkPoint[]> positions,
                                  std::unique_ptr<const SkColor[]> colors, /* optional */
                                  std::unique_ptr<const SkPoint[]> texs,   /* optional */
                                  int vertexCnt) {
        if (!positions || vertexCnt <= 0) {
            return nullptr;
        }
        return sk_sp<SkVertices>(new SkVertices(mode, std::move(positions), std::move(colors),
                                                std::move(texs), vertexCnt, nullptr, 0, nullptr));
    }

    static sk_sp<SkVertices> Make(SkCanvas::VertexMode mode,
                                  std::unique_ptr<const SkPoint[]> positions,
                                  std::unique_ptr<const SkColor[]> colors, /* optional */
                                  std::unique_ptr<const SkPoint[]> texs,   /* optional */
                                  int vertexCnt,
                                  const SkRect& bounds) {
        if (!positions || vertexCnt <= 0) {
            return nullptr;
        }
        return sk_sp<SkVertices>(new SkVertices(mode, std::move(positions), std::move(colors),
                                                std::move(texs), vertexCnt, nullptr, 0, &bounds));
    }

    static sk_sp<SkVertices> MakeIndexed(SkCanvas::VertexMode mode,
                                         std::unique_ptr<const SkPoint[]> positions,
                                         std::unique_ptr<const SkColor[]> colors, /* optional */
                                         std::unique_ptr<const SkPoint[]> texs,   /* optional */
                                         int vertexCnt,
                                         std::unique_ptr<const uint16_t[]> indices,
                                         int indexCnt) {
        if (!positions || !indices || vertexCnt <= 0 || indexCnt <= 0) {
            return nullptr;
        }
        return sk_sp<SkVertices>(new SkVertices(mode, std::move(positions), std::move(colors),
                                                std::move(texs), vertexCnt, std::move(indices),
                                                indexCnt, nullptr));
    }

    static sk_sp<SkVertices> MakeIndexed(SkCanvas::VertexMode mode,
                                         std::unique_ptr<const SkPoint[]> positions,
                                         std::unique_ptr<const SkColor[]> colors, /* optional */
                                         std::unique_ptr<const SkPoint[]> texs,   /* optional */
                                         int vertexCnt,
                                         std::unique_ptr<const uint16_t[]> indices,
                                         int indexCnt,
                                         const SkRect& bounds) {
        if (!positions || !indices || vertexCnt <= 0 || indexCnt <= 0) {
            return nullptr;
        }
        return sk_sp<SkVertices>(new SkVertices(mode, std::move(positions), std::move(colors),
                                                std::move(texs), vertexCnt, std::move(indices),
                                                indexCnt, &bounds));
    }

    SkCanvas::VertexMode mode() const { return fMode; }

    int vertexCount() const { return fVertexCnt; }
    bool hasColors() const { return SkToBool(fColors); }
    bool hasTexCoords() const { return SkToBool(fTexs); }
    const SkPoint* positions() const { return fPositions.get(); }
    const SkPoint* texCoords() const { return fTexs.get(); }
    const SkColor* colors() const { return fColors.get(); }

    bool isIndexed() const { return SkToBool(fIndexCnt); }
    int indexCount() const { return fIndexCnt; }
    const uint16_t* indices() const { return fIndices.get(); }

    size_t size() const {
        return fVertexCnt * (sizeof(SkPoint) * (this->hasTexCoords() ? 2 : 1) + sizeof(SkColor)) +
               fIndexCnt * sizeof(uint16_t);
    }

    const SkRect& bounds() const { return fBounds; }

private:
    SkVertices(SkCanvas::VertexMode mode, std::unique_ptr<const SkPoint[]> positions,
               std::unique_ptr<const SkColor[]> colors, std::unique_ptr<const SkPoint[]> texs,
               int vertexCnt, std::unique_ptr<const uint16_t[]> indices, int indexCnt,
               const SkRect* bounds)
            : fMode(mode)
            , fVertexCnt(vertexCnt)
            , fIndexCnt(indexCnt)
            , fPositions(std::move(positions))
            , fColors(std::move(colors))
            , fTexs(std::move(texs))
            , fIndices(std::move(indices)) {
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

    SkCanvas::VertexMode fMode;
    int fVertexCnt;
    int fIndexCnt;
    std::unique_ptr<const SkPoint[]> fPositions;
    std::unique_ptr<const SkColor[]> fColors;
    std::unique_ptr<const SkPoint[]> fTexs;
    std::unique_ptr<const uint16_t[]> fIndices;
    SkRect fBounds;
};

#endif
