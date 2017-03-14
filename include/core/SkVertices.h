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
#include "SkData.h"
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

    /**
     *  Create a vertices by copying the specified arrays. texs and colors may be nullptr,
     *  and indices is ignored if indexCount == 0.
     */
    static sk_sp<SkVertices> MakeCopy(SkCanvas::VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      int indexCount,
                                      const uint16_t indices[]);

    static sk_sp<SkVertices> MakeCopy(SkCanvas::VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[]) {
        return MakeCopy(mode, vertexCount, positions, texs, colors, 0, nullptr);
    }

    enum Flags {
        kHasTexs_Flag    = 1 << 0,
        kHasColors_Flag  = 1 << 1,
    };
    class Builder {
    public:
        Builder(SkCanvas::VertexMode mode, int vertexCount, int indexCount, uint32_t flags);
        ~Builder();

        bool isValid() const { return fPositions != nullptr; }

        int vertexCount() const { return fVertexCnt; }
        int indexCount() const { return fIndexCnt; }
        SkPoint* positions() { return fPositions; }
        SkPoint* texCoords() { return fTexs; }
        SkColor* colors() { return fColors; }
        uint16_t* indices() { return fIndices; }

        sk_sp<SkVertices> detach();

    private:
        SkPoint* fPositions;  // owner of storage, use sk_free
        SkPoint* fTexs;
        SkColor* fColors;
        uint16_t* fIndices;
        int fVertexCnt;
        int fIndexCnt;
        SkCanvas::VertexMode fMode;
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


    static sk_sp<SkVertices> Decode(const void*, size_t);
    sk_sp<SkData> encode() const;

private:
    SkVertices() {}

    const SkPoint* fPositions;  // owner of storage, use sk_free
    const SkPoint* fTexs;
    const SkColor* fColors;
    const uint16_t* fIndices;
    SkRect fBounds;
    int fVertexCnt;
    int fIndexCnt;
    SkCanvas::VertexMode fMode;
};

#endif
