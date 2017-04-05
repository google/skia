/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVertices_DEFINED
#define SkVertices_DEFINED

#include "SkColor.h"
#include "SkData.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"

/**
 * An immutable set of vertex data that can be used with SkCanvas::drawVertices.
 */
class SkVertices : public SkNVRefCnt<SkVertices> {
public:
    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode,
    };

    /**
     *  Create a vertices by copying the specified arrays. texs and colors may be nullptr,
     *  and indices is ignored if indexCount == 0.
     */
    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      int indexCount,
                                      const uint16_t indices[]);

    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[]) {
        return MakeCopy(mode, vertexCount, positions, texs, colors, 0, nullptr);
    }

    struct Sizes;

    enum BuilderFlags {
        kHasTexCoords_BuilderFlag   = 1 << 0,
        kHasColors_BuilderFlag      = 1 << 1,
    };
    class Builder {
    public:
        Builder(VertexMode mode, int vertexCount, int indexCount, uint32_t flags);

        bool isValid() const { return fVertices != nullptr; }

        // if the builder is invalid, these will return 0
        int vertexCount() const;
        int indexCount() const;
        SkPoint* positions();
        SkPoint* texCoords();   // returns null if there are no texCoords
        SkColor* colors();      // returns null if there are no colors
        uint16_t* indices();    // returns null if there are no indices

        // Detach the built vertices object. After the first call, this will always return null.
        sk_sp<SkVertices> detach();

    private:
        Builder(VertexMode mode, int vertexCount, int indexCount, const Sizes&);

        void init(VertexMode mode, int vertexCount, int indexCount, const Sizes&);

        // holds a partially complete object. only completed in detach()
        sk_sp<SkVertices> fVertices;

        friend class SkVertices;
    };

    uint32_t uniqueID() const { return fUniqueID; }
    VertexMode mode() const { return fMode; }
    const SkRect& bounds() const { return fBounds; }

    bool hasColors() const { return SkToBool(this->colors()); }
    bool hasTexCoords() const { return SkToBool(this->texCoords()); }
    bool hasIndices() const { return SkToBool(this->indices()); }

    int vertexCount() const { return fVertexCnt; }
    const SkPoint* positions() const { return fPositions; }
    const SkPoint* texCoords() const { return fTexs; }
    const SkColor* colors() const { return fColors; }

    int indexCount() const { return fIndexCnt; }
    const uint16_t* indices() const { return fIndices; }

    // returns approximate byte size of the vertices object
    size_t approximateSize() const;

    /**
     *  Recreate a vertices from a buffer previously created by calling encode().
     *  Returns null if the data is corrupt or the length is incorrect for the contents.
     */
    static sk_sp<SkVertices> Decode(const void* buffer, size_t length);

    /**
     *  Pack the vertices object into a byte buffer. This can be used to recreate the vertices
     *  by calling Decode() with the buffer.
     */
    sk_sp<SkData> encode() const;

private:
    SkVertices() {}

    // these are needed since we've manually sized our allocation (see Builder::init)
    friend class SkNVRefCnt<SkVertices>;
    void operator delete(void* p) { ::operator delete(p); }

    static sk_sp<SkVertices> Alloc(int vCount, int iCount, uint32_t builderFlags,
                                   size_t* arraySize);

    // we store this first, to pair with the refcnt in our base-class, so we don't have an
    // unnecessary pad between it and the (possibly 8-byte aligned) ptrs.
    uint32_t fUniqueID;

    // these point inside our allocation, so none of these can be "freed"
    SkPoint*    fPositions;
    SkPoint*    fTexs;
    SkColor*    fColors;
    uint16_t*   fIndices;

    SkRect  fBounds;    // computed to be the union of the fPositions[]
    int     fVertexCnt;
    int     fIndexCnt;

    VertexMode fMode;
    // below here is where the actual array data is stored.
};

#endif
