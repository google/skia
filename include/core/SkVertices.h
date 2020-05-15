/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVertices_DEFINED
#define SkVertices_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

class SkData;
struct SkPoint;
class SkVerticesPriv;

/**
 * An immutable set of vertex data that can be used with SkCanvas::drawVertices.
 */
class SK_API SkVertices : public SkNVRefCnt<SkVertices> {
    struct Desc;
    struct Sizes;
public:
    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode,

        kLast_VertexMode = kTriangleFan_VertexMode,
    };

    /**
     *  Create a vertices by copying the specified arrays. texs, colors may be nullptr,
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
        return MakeCopy(mode,
                        vertexCount,
                        positions,
                        texs,
                        colors,
                        0,
                        nullptr);
    }

    static constexpr int kMaxCustomAttributes = 8;

    /**
     *  EXPERIMENTAL - An SkVertices object can be constructed with a custom collection of vertex
     *  attributes. Each attribute is described by a single Attribute struct. Type defines the CPU
     *  type of the data. Usage determines what transformation (if any) is applied to that data in
     *  the vertex shader. For positions or vectors, markerName identifies what matrix is used in
     *  the vertex shader to transform the data. Those names should match a named transform on the
     *  CTM stack, created by calling SkCanvas::markCTM().
     *
     *  For attributes with a usage of kVector, kNormalVector, or kPosition, a null markerName
     *  will transform the attribute by the canvas CTM matrix.
     */
    struct Attribute {
        enum class Type : uint8_t {
            kFloat,
            kFloat2,
            kFloat3,
            kFloat4,
            kByte4_unorm,
        };

        enum class Usage : uint8_t {
            // Raw values passed directly to effect
            kRaw,

            // sRGB unpremul colors, transformed to destination color space (3 or 4 channels)
            // Colors are always assumed to be in RGBA order, and are automatically premultiplied.
            kColor,

            // Local vector, transformed via marker (2 or 3 channels)
            kVector,

            // Normal vector (or any other bivector), transformed via marker (2 or 3 channels)
            kNormalVector,

            // Local position, transformed via marker (2 or 3 channels)
            kPosition,
        };

        /**
         *  markerName is not copied by the Attribute, so it must outlive this struct.
         *  It is copied when this Attribute is passed to the Builder constructor.
         */
        Attribute(Type t = Type::kFloat, Usage u = Usage::kRaw, const char* markerName = nullptr);

        bool operator==(const Attribute& that) const {
            return fType == that.fType && fUsage == that.fUsage && fMarkerID == that.fMarkerID;
        }
        bool operator!=(const Attribute& that) const { return !(*this == that); }

        // Number of channels that will be produced for the SkRuntimeEffect to consume.
        // May not match the number of channels in fType. For example, kVector Attributes always
        // produce three channels, even if the input is kFloat2.
        int channelCount() const;
        size_t bytesPerVertex() const;
        bool isValid() const;

        Type        fType;
        Usage       fUsage;
        uint32_t    fMarkerID;
        const char* fMarkerName;  // Preserved for serialization and debugging
    };

    enum BuilderFlags {
        kHasTexCoords_BuilderFlag   = 1 << 0,
        kHasColors_BuilderFlag      = 1 << 1,
    };
    class Builder {
    public:
        Builder(VertexMode mode, int vertexCount, int indexCount, uint32_t flags);

        // EXPERIMENTAL -- do not call if you care what happens
        Builder(VertexMode mode,
                int vertexCount,
                int indexCount,
                const Attribute* attrs,
                int attrCount);

        bool isValid() const { return fVertices != nullptr; }

        SkPoint* positions();
        uint16_t* indices();        // returns null if there are no indices

        // if we have texCoords or colors, this will always be null
        void* customData();         // returns null if there are no custom attributes

        // If we have custom attributes, these will always be null
        SkPoint* texCoords();       // returns null if there are no texCoords
        SkColor* colors();          // returns null if there are no colors

        // Detach the built vertices object. After the first call, this will always return null.
        sk_sp<SkVertices> detach();

    private:
        Builder(const Desc&);

        void init(const Desc&);

        // holds a partially complete object. only completed in detach()
        sk_sp<SkVertices> fVertices;
        // Extra storage for intermediate vertices in the case where the client specifies indexed
        // triangle fans. These get converted to indexed triangles when the Builder is finalized.
        std::unique_ptr<uint8_t[]> fIntermediateFanIndices;

        friend class SkVertices;
        friend class SkVerticesPriv;
    };

    uint32_t uniqueID() const { return fUniqueID; }
    const SkRect& bounds() const { return fBounds; }

    // returns approximate byte size of the vertices object
    size_t approximateSize() const;

    // Provides access to functions that aren't part of the public API.
    SkVerticesPriv priv();
    const SkVerticesPriv priv() const;

private:
    SkVertices() {}

    friend class SkVerticesPriv;

    // these are needed since we've manually sized our allocation (see Builder::init)
    friend class SkNVRefCnt<SkVertices>;
    void operator delete(void* p);

    Sizes getSizes() const;

    // we store this first, to pair with the refcnt in our base-class, so we don't have an
    // unnecessary pad between it and the (possibly 8-byte aligned) ptrs.
    uint32_t fUniqueID;

    // these point inside our allocation, so none of these can be "freed"
    Attribute*   fAttributes;       // [attributeCount] or null
    SkPoint*     fPositions;        // [vertexCount]
    uint16_t*    fIndices;          // [indexCount] or null
    void*        fCustomData;       // [customDataSize * vertexCount] or null
    SkPoint*     fTexs;             // [vertexCount] or null
    SkColor*     fColors;           // [vertexCount] or null

    SkRect  fBounds;    // computed to be the union of the fPositions[]
    int     fVertexCount;
    int     fIndexCount;
    int     fAttributeCount;

    VertexMode fMode;
    // below here is where the actual array data is stored.
};

#endif
