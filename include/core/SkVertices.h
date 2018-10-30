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
class SK_API SkVertices : public SkNVRefCnt<SkVertices> {
public:
    // BoneIndices indicates which (of a maximum of 4 bones) a given vertex will interpolate
    // between. To indicate that a slot is not used, the convention is to assign the bone index
    // to 0.
    struct BoneIndices {
        uint32_t indices[4];

        uint32_t& operator[] (int i) {
            SkASSERT(i >= 0);
            SkASSERT(i < 4);
            return indices[i];
        }

        const uint32_t& operator[] (int i) const {
            SkASSERT(i >= 0);
            SkASSERT(i < 4);
            return indices[i];
        }
    };

    // BoneWeights stores the interpolation weight for each of the (maximum of 4) bones a given
    // vertex interpolates between. To indicate that a slot is not used, the weight for that
    // slot should be 0.
    struct BoneWeights {
        float weights[4];

        float& operator[] (int i) {
            SkASSERT(i >= 0);
            SkASSERT(i < 4);
            return weights[i];
        }

        const float& operator[] (int i) const {
            SkASSERT(i >= 0);
            SkASSERT(i < 4);
            return weights[i];
        }
    };

    // Bone stores a 3x2 transformation matrix in column major order:
    // | scaleX   skewX transX |
    // |  skewY  scaleY transY |
    // SkRSXform is insufficient because bones can have non uniform scale.
    struct Bone {
        float values[6];

        float& operator[] (int i) {
            SkASSERT(i >= 0);
            SkASSERT(i < 6);
            return values[i];
        }

        const float& operator[] (int i) const {
            SkASSERT(i >= 0);
            SkASSERT(i < 6);
            return values[i];
        }

        SkPoint mapPoint(const SkPoint& point) const {
            float x = values[0] * point.x() + values[2] * point.y() + values[4];
            float y = values[1] * point.x() + values[3] * point.y() + values[5];
            return SkPoint::Make(x, y);
        }

        SkRect mapRect(const SkRect& rect) const {
            SkRect dst = SkRect::MakeEmpty();
            SkPoint quad[4];
            rect.toQuad(quad);
            for (int i = 0; i < 4; i ++) {
                quad[i] = mapPoint(quad[i]);
            }
            dst.setBoundsNoCheck(quad, 4);
            return dst;
        }
    };

    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode,

        kLast_VertexMode = kTriangleFan_VertexMode,
    };

    /**
     *  Create a vertices by copying the specified arrays. texs, colors, boneIndices, and
     *  boneWeights may be nullptr, and indices is ignored if indexCount == 0.
     *
     *  boneIndices and boneWeights must either both be nullptr or both point to valid data.
     *  If specified, they must both contain 'vertexCount' entries.
     */
    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      const BoneIndices boneIndices[],
                                      const BoneWeights boneWeights[],
                                      int indexCount,
                                      const uint16_t indices[],
                                      bool isVolatile = true);

    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      const BoneIndices boneIndices[],
                                      const BoneWeights boneWeights[],
                                      bool isVolatile = true) {
        return MakeCopy(mode,
                        vertexCount,
                        positions,
                        texs,
                        colors,
                        boneIndices,
                        boneWeights,
                        0,
                        nullptr,
                        isVolatile);
    }

    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      int indexCount,
                                      const uint16_t indices[],
                                      bool isVolatile = true) {
        return MakeCopy(mode,
                        vertexCount,
                        positions,
                        texs,
                        colors,
                        nullptr,
                        nullptr,
                        indexCount,
                        indices,
                        isVolatile);
    }

    static sk_sp<SkVertices> MakeCopy(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      bool isVolatile = true) {
        return MakeCopy(mode, vertexCount, positions, texs, colors, nullptr, nullptr, isVolatile);
    }

    struct Sizes;

    enum BuilderFlags {
        kHasTexCoords_BuilderFlag   = 1 << 0,
        kHasColors_BuilderFlag      = 1 << 1,
        kHasBones_BuilderFlag       = 1 << 2,
        kIsNonVolatile_BuilderFlag  = 1 << 3,
    };
    class Builder {
    public:
        Builder(VertexMode mode, int vertexCount, int indexCount, uint32_t flags);

        bool isValid() const { return fVertices != nullptr; }

        // if the builder is invalid, these will return 0
        int vertexCount() const;
        int indexCount() const;
        bool isVolatile() const;
        SkPoint* positions();
        SkPoint* texCoords();       // returns null if there are no texCoords
        SkColor* colors();          // returns null if there are no colors
        BoneIndices* boneIndices(); // returns null if there are no bone indices
        BoneWeights* boneWeights(); // returns null if there are no bone weights
        uint16_t* indices();        // returns null if there are no indices

        // Detach the built vertices object. After the first call, this will always return null.
        sk_sp<SkVertices> detach();

    private:
        Builder(VertexMode mode, int vertexCount, int indexCount, bool isVolatile, const Sizes&);

        void init(VertexMode mode, int vertexCount, int indexCount, bool isVolatile, const Sizes&);

        // holds a partially complete object. only completed in detach()
        sk_sp<SkVertices> fVertices;
        // Extra storage for intermediate vertices in the case where the client specifies indexed
        // triangle fans. These get converted to indexed triangles when the Builder is finalized.
        std::unique_ptr<uint8_t[]> fIntermediateFanIndices;

        friend class SkVertices;
    };

    uint32_t uniqueID() const { return fUniqueID; }
    VertexMode mode() const { return fMode; }
    const SkRect& bounds() const { return fBounds; }

    bool hasColors() const { return SkToBool(this->colors()); }
    bool hasTexCoords() const { return SkToBool(this->texCoords()); }
    bool hasBones() const { return SkToBool(this->boneIndices()); }
    bool hasIndices() const { return SkToBool(this->indices()); }

    int vertexCount() const { return fVertexCnt; }
    const SkPoint* positions() const { return fPositions; }
    const SkPoint* texCoords() const { return fTexs; }
    const SkColor* colors() const { return fColors; }

    const BoneIndices* boneIndices() const { return fBoneIndices; }
    const BoneWeights* boneWeights() const { return fBoneWeights; }

    int indexCount() const { return fIndexCnt; }
    const uint16_t* indices() const { return fIndices; }

    bool isVolatile() const { return fIsVolatile; }

    sk_sp<SkVertices> applyBones(const Bone bones[], int boneCount) const;

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
    void operator delete(void* p);

    static sk_sp<SkVertices> Alloc(int vCount, int iCount, uint32_t builderFlags,
                                   size_t* arraySize);

    // we store this first, to pair with the refcnt in our base-class, so we don't have an
    // unnecessary pad between it and the (possibly 8-byte aligned) ptrs.
    uint32_t fUniqueID;

    // these point inside our allocation, so none of these can be "freed"
    SkPoint*     fPositions;
    SkPoint*     fTexs;
    SkColor*     fColors;
    BoneIndices* fBoneIndices;
    BoneWeights* fBoneWeights;
    uint16_t*    fIndices;

    SkRect  fBounds;    // computed to be the union of the fPositions[]
    int     fVertexCnt;
    int     fIndexCnt;

    bool fIsVolatile;

    VertexMode fMode;
    // below here is where the actual array data is stored.
};

#endif
