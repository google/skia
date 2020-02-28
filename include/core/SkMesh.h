/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMesh_DEFINED
#define SkMesh_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

class SK_API SkMesh : public SkNVRefCnt<SkMesh> {
public:
    enum class VertexMode {
        kTriangles,
        kTriangleStrip,
        kTriangleFan,

        kLastMode = kTriangleFan,
    };

    static sk_sp<SkMesh> MakeCopy(VertexMode,
                                  int positionCount, const SkPoint positions[],
                                  int vertexDataCount, const float vertextData[],
                                  int indexCount, const uint16_t indices[],
                                  bool isVolatile = true);

    struct Sizes;

    class Builder {
    public:
        Builder(VertexMode mode, int positionCount, int vertexDataCount, int indexCount,
                bool isVolatile);

        bool isValid() const { return fVertices != nullptr; }

        // if the builder is invalid, these will return 0
        int positionCount() const;
        int vertexDataCount() const;
        int indexCount() const;
        bool isVolatile() const;

        SkPoint*  positions();
        float*    vertexData(); // returns null if there is no vertexData
        uint16_t* indices();    // returns null if there are no indices

        // Detach the built vertices object. After the first call, this will always return null.
        sk_sp<SkMesh> detach();

    private:
        Builder(VertexMode mode, int vertexCount, int indexCount, bool isVolatile, const Sizes&);

        void init(VertexMode mode, int vertexCount, int indexCount, bool isVolatile, const Sizes&);

        // holds a partially complete object. only completed in detach()
        sk_sp<SkMesh> fMesh;
        // Extra storage for intermediate vertices in the case where the client specifies indexed
        // triangle fans. These get converted to indexed triangles when the Builder is finalized.
        std::unique_ptr<uint8_t[]> fIntermediateFanIndices;

        friend class SkMesh;
    };

    uint32_t uniqueID() const { return fUniqueID; }
    VertexMode mode() const { return fMode; }
    SkRect bounds() const { return fBounds; }

    int positionCount() const { return fVertexCnt; }
    int vertexDataCount() const { return fVertexCnt; }
    int indexCount() const { return fIndexCnt; }

    const SkPoint* positions() const { return fPositions; }
    const float* vertexData() const { return fVertexData; }
    const uint16_t* indices() const { return fIndices; }

    bool isVolatile() const { return fIsVolatile; }

    // returns approximate byte size of the vertices object
    size_t approximateSize() const;

    /**
     *  Recreate a vertices from a buffer previously created by calling encode().
     *  Returns null if the data is corrupt or the length is incorrect for the contents.
     */
    static sk_sp<SkMesh> Decode(const void* buffer, size_t length);

    /**
     *  Pack the vertices object into a byte buffer. This can be used to recreate the vertices
     *  by calling Decode() with the buffer.
     */
    sk_sp<SkData> encode() const;

private:
    SkMesh() {}

    // these are needed since we've manually sized our allocation (see Builder::init)
    friend class SkNVRefCnt<SkMesh>;
    void operator delete(void* p);

    static sk_sp<SkMesh> Alloc(int pCount, int vdCount, int iCount, size_t* arraySize);

    // we store this first, to pair with the refcnt in our base-class, so we don't have an
    // unnecessary pad between it and the (possibly 8-byte aligned) ptrs.
    uint32_t fUniqueID;

    // these point inside our allocation, so none of these can be "freed"
    SkPoint*    fPositions;
    float*      fVertexData; // [positionCount * vertexDataCount]
    uint16_t*   fIndices;

    SkRect  fBounds;    // computed to be the union of the fPositions[]
    int     fPositionCount;
    int     fVetexDataCount;
    int     fIndexCount;

    bool fIsVolatile;

    VertexMode fMode;
    // below here is where the actual array data is stored.
};

#endif
