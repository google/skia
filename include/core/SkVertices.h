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
    ~SkVertices() { sk_free((void*)fPositions); }

    static sk_sp<SkVertices> MakeCopy(SkCanvas::VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkColor colors[],   // optional
                                      const SkPoint texs[],     // optional
                                      const uint16_t indices[], // optional
                                      const SkRect* bounds);    // computed if nullptr

    enum Flags {
        kHasColors_Flag  = 1 << 0,
        kHasTexs_Flag    = 1 << 1,
        kHasIndices_Flag = 1 << 2,
    };
    class Builder {
    public:
        Builder(SkCanvas::VertexMode mode, int vertexCount, uint32_t flags, const SkRect* bounds);
        ~Builder();

        int vertexCount() const { return fVertexCnt; }
        int indexCount() const { return fIndexCnt; }
        SkPoint* positions() { return fPositions; }
        SkPoint* texCoords() { return fTexs; }
        SkColor* colors() { return fColors; }
        uint16_t* indices() { return fIndices; }

        sk_sp<SkVertices> detach();

    private:
        SkCanvas::VertexMode fMode;
        int fVertexCnt;
        int fIndexCnt;
        SkPoint* fPositions;  // owner of storage, use sk_free
        SkColor* fColors;
        SkPoint* fTexs;
        uint16_t* fIndices;
        SkRect fBounds;
    };

    SkCanvas::VertexMode mode() const { return fMode; }

    int vertexCount() const { return fVertexCnt; }
    bool hasColors() const { return SkToBool(fColors); }
    bool hasTexCoords() const { return SkToBool(fTexs); }
    const SkPoint* positions() const { return fPositions; }
    const SkPoint* texCoords() const { return fTexs; }
    const SkColor* colors() const { return fColors; }

    bool isIndexed() const { return SkToBool(fIndexCnt); }
    int indexCount() const { return fIndexCnt; }
    const uint16_t* indices() const { return fIndices; }

    size_t size() const {
        return fVertexCnt * (sizeof(SkPoint) * (this->hasTexCoords() ? 2 : 1) + sizeof(SkColor)) +
               fIndexCnt * sizeof(uint16_t);
    }

    const SkRect& bounds() const { return fBounds; }

private:
    SkVertices() {}

    SkCanvas::VertexMode fMode;
    int fVertexCnt;
    int fIndexCnt;
    const SkPoint* fPositions;  // owner of storage, use sk_free
    const SkColor* fColors;
    const SkPoint* fTexs;
    const uint16_t* fIndices;
    SkRect fBounds;
};

#endif
