/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkVertices.h"

#include "include/core/SkData.h"
#include "include/private/SkTo.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkSafeRange.h"
#include "src/core/SkVerticesPriv.h"
#include "src/core/SkWriteBuffer.h"
#include <atomic>
#include <new>

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};

    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

SkVertices::Attribute::Attribute(Type t, Usage u, const char* markerName)
        : fType(t)
        , fUsage(u)
        , fMarkerName(markerName) {
    fMarkerID = fMarkerName ? SkOpts::hash_fn(fMarkerName, strlen(fMarkerName), 0) : 0;
    SkASSERT(!fMarkerName || fMarkerID != 0);
}

int SkVertices::Attribute::channelCount() const {
    SkASSERT(this->isValid());
    switch (fUsage) {
        case Usage::kRaw:          break;
        case Usage::kColor:        return 4;
        case Usage::kVector:       return 3;
        case Usage::kNormalVector: return 3;
        case Usage::kPosition:     return 3;
    }
    switch (fType) {
        case Type::kFloat:       return 1;
        case Type::kFloat2:      return 2;
        case Type::kFloat3:      return 3;
        case Type::kFloat4:      return 4;
        case Type::kByte4_unorm: return 4;
    }
    SkUNREACHABLE;
}

size_t SkVertices::Attribute::bytesPerVertex() const {
    switch (fType) {
        case Type::kFloat:       return 1 * sizeof(float);
        case Type::kFloat2:      return 2 * sizeof(float);
        case Type::kFloat3:      return 3 * sizeof(float);
        case Type::kFloat4:      return 4 * sizeof(float);
        case Type::kByte4_unorm: return 4 * sizeof(uint8_t);
    }
    SkUNREACHABLE;
}

bool SkVertices::Attribute::isValid() const {
    if (fMarkerName && !SkCanvasPriv::ValidateMarker(fMarkerName)) {
        return false;
    }
    switch (fUsage) {
        case Usage::kRaw:
            return fMarkerID == 0;
        case Usage::kColor:
            return fMarkerID == 0 && (fType == Type::kFloat3 || fType == Type::kFloat4 ||
                                      fType == Type::kByte4_unorm);
        case Usage::kVector:
        case Usage::kNormalVector:
        case Usage::kPosition:
            return fType == Type::kFloat2 || fType == Type::kFloat3;
    }
    SkUNREACHABLE;
}

static size_t custom_data_size(const SkVertices::Attribute* attrs, int attrCount) {
    size_t size = 0;
    for (int i = 0; i < attrCount; ++i) {
        size += attrs[i].bytesPerVertex();
    }
    return size;
}

struct SkVertices::Desc {
    VertexMode  fMode;
    int         fVertexCount,
                fIndexCount;
    bool        fHasTexs,
                fHasColors;

    const Attribute* fAttributes;
    int              fAttributeCount;

    void validate() const {
        SkASSERT(fAttributeCount == 0 || (!fHasTexs && !fHasColors));
    }
};

struct SkVertices::Sizes {
    Sizes(const Desc& desc) {
        desc.validate();

        SkSafeMath safe;

        fNameSize = 0;
        for (int i = 0; i < desc.fAttributeCount; ++i) {
            const Attribute& attr(desc.fAttributes[i]);
            if (!attr.isValid()) {
                return;
            }
            if (attr.fMarkerName) {
                fNameSize = safe.add(fNameSize, strlen(attr.fMarkerName) + 1 /*null terminator*/);
            }
        }
        fNameSize = SkAlign4(fNameSize);

        fAttrSize = safe.mul(desc.fAttributeCount, sizeof(Attribute));
        fVSize = safe.mul(desc.fVertexCount, sizeof(SkPoint));
        fDSize = safe.mul(custom_data_size(desc.fAttributes, desc.fAttributeCount),
                          desc.fVertexCount);
        fTSize = desc.fHasTexs ? safe.mul(desc.fVertexCount, sizeof(SkPoint)) : 0;
        fCSize = desc.fHasColors ? safe.mul(desc.fVertexCount, sizeof(SkColor)) : 0;

        fBuilderTriFanISize = 0;
        fISize = safe.mul(desc.fIndexCount, sizeof(uint16_t));
        if (kTriangleFan_VertexMode == desc.fMode) {
            int numFanTris = 0;
            if (desc.fIndexCount) {
                fBuilderTriFanISize = fISize;
                numFanTris = desc.fIndexCount - 2;
            } else {
                numFanTris = desc.fVertexCount - 2;
                // By forcing this to become indexed we are adding a constraint to the maximum
                // number of vertices.
                if (desc.fVertexCount > (SkTo<int>(UINT16_MAX) + 1)) {
                    sk_bzero(this, sizeof(*this));
                    return;
                }
            }
            if (numFanTris <= 0) {
                sk_bzero(this, sizeof(*this));
                return;
            }
            fISize = safe.mul(numFanTris, 3 * sizeof(uint16_t));
        }

        fTotal = safe.add(sizeof(SkVertices),
                 safe.add(fAttrSize,
                 safe.add(fNameSize,
                 safe.add(fVSize,
                 safe.add(fDSize,
                 safe.add(fTSize,
                 safe.add(fCSize,
                          fISize)))))));

        if (safe.ok()) {
            fArrays = fVSize + fDSize + fTSize + fCSize + fISize;  // just the sum of the arrays
        } else {
            sk_bzero(this, sizeof(*this));
        }
    }

    bool isValid() const { return fTotal != 0; }

    size_t fTotal = 0;  // size of entire SkVertices allocation (obj + arrays)
    size_t fAttrSize;  // size of attributes
    size_t fNameSize;  // size of attribute marker names
    size_t fArrays; // size of all the data arrays (V + D + T + C + I)
    size_t fVSize;
    size_t fDSize;  // size of all customData = [customDataSize * fVertexCount]
    size_t fTSize;
    size_t fCSize;
    size_t fISize;

    // For indexed tri-fans this is the number of amount of space fo indices needed in the builder
    // before conversion to indexed triangles (or zero if not indexed or not a triangle fan).
    size_t fBuilderTriFanISize;
};

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount,
                             uint32_t builderFlags) {
    bool hasTexs = SkToBool(builderFlags & SkVertices::kHasTexCoords_BuilderFlag);
    bool hasColors = SkToBool(builderFlags & SkVertices::kHasColors_BuilderFlag);
    this->init({mode, vertexCount, indexCount, hasTexs, hasColors, nullptr, 0});
}

SkVertices::Builder::Builder(VertexMode mode,
                             int vertexCount,
                             int indexCount,
                             const SkVertices::Attribute* attrs,
                             int attrCount) {
    if (attrCount <= 0 || attrCount > kMaxCustomAttributes || !attrs) {
        return;
    }
    this->init({mode, vertexCount, indexCount, false, false, attrs, attrCount});
}

SkVertices::Builder::Builder(const Desc& desc) {
    this->init(desc);
}

void SkVertices::Builder::init(const Desc& desc) {
    Sizes sizes(desc);
    if (!sizes.isValid()) {
        SkASSERT(!this->isValid());
        return;
    }

    void* storage = ::operator new (sizes.fTotal);
    if (sizes.fBuilderTriFanISize) {
        fIntermediateFanIndices.reset(new uint8_t[sizes.fBuilderTriFanISize]);
    }

    fVertices.reset(new (storage) SkVertices);

    // need to point past the object to store the arrays
    char* ptr = (char*)storage + sizeof(SkVertices);

    // return the original ptr (or null), but then advance it by size
    auto advance = [&ptr](size_t size) {
        char* new_ptr = size ? ptr : nullptr;
        ptr += size;
        return new_ptr;
    };

    fVertices->fAttributes = (Attribute*)advance(sizes.fAttrSize);
    char* markerNames      =             advance(sizes.fNameSize);

    // Copy the attributes into our block of memory (immediately after the SkVertices)
    sk_careful_memcpy(fVertices->fAttributes, desc.fAttributes,
                      desc.fAttributeCount * sizeof(Attribute));

    // Now copy the marker names, and fix up the pointers in our attributes
    for (int i = 0; i < desc.fAttributeCount; ++i) {
        Attribute& attr(fVertices->fAttributes[i]);
        if (attr.fMarkerName) {
            attr.fMarkerName = strcpy(markerNames, attr.fMarkerName);
            markerNames += (strlen(markerNames) + 1 /*null terminator*/);
        }
    }

    fVertices->fPositions      = (SkPoint*) advance(sizes.fVSize);
    fVertices->fCustomData     = (void*)    advance(sizes.fDSize);
    fVertices->fTexs           = (SkPoint*) advance(sizes.fTSize);
    fVertices->fColors         = (SkColor*) advance(sizes.fCSize);
    fVertices->fIndices        = (uint16_t*)advance(sizes.fISize);

    fVertices->fVertexCount    = desc.fVertexCount;
    fVertices->fIndexCount     = desc.fIndexCount;
    fVertices->fAttributeCount = desc.fAttributeCount;
    fVertices->fMode           = desc.fMode;

    // We defer assigning fBounds and fUniqueID until detach() is called
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds.setBounds(fVertices->fPositions, fVertices->fVertexCount);
        if (fVertices->fMode == kTriangleFan_VertexMode) {
            if (fIntermediateFanIndices) {
                SkASSERT(fVertices->fIndexCount);
                auto tempIndices = this->indices();
                for (int t = 0; t < fVertices->fIndexCount - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = tempIndices[0];
                    fVertices->fIndices[3 * t + 1] = tempIndices[t + 1];
                    fVertices->fIndices[3 * t + 2] = tempIndices[t + 2];
                }
                fVertices->fIndexCount = 3 * (fVertices->fIndexCount - 2);
            } else {
                SkASSERT(!fVertices->fIndexCount);
                for (int t = 0; t < fVertices->fVertexCount - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = 0;
                    fVertices->fIndices[3 * t + 1] = SkToU16(t + 1);
                    fVertices->fIndices[3 * t + 2] = SkToU16(t + 2);
                }
                fVertices->fIndexCount = 3 * (fVertices->fVertexCount - 2);
            }
            fVertices->fMode = kTriangles_VertexMode;
        }
        fVertices->fUniqueID = next_id();
        return std::move(fVertices);        // this will null fVertices after the return
    }
    return nullptr;
}

SkPoint* SkVertices::Builder::positions() {
    return fVertices ? const_cast<SkPoint*>(fVertices->fPositions) : nullptr;
}

void* SkVertices::Builder::customData() {
    return fVertices ? const_cast<void*>(fVertices->fCustomData) : nullptr;
}

SkPoint* SkVertices::Builder::texCoords() {
    return fVertices ? const_cast<SkPoint*>(fVertices->fTexs) : nullptr;
}

SkColor* SkVertices::Builder::colors() {
    return fVertices ? const_cast<SkColor*>(fVertices->fColors) : nullptr;
}

uint16_t* SkVertices::Builder::indices() {
    if (!fVertices) {
        return nullptr;
    }
    if (fIntermediateFanIndices) {
        return reinterpret_cast<uint16_t*>(fIntermediateFanIndices.get());
    }
    return const_cast<uint16_t*>(fVertices->fIndices);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkVertices::MakeCopy(VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkPoint texs[],
                                       const SkColor colors[],
                                       int indexCount, const uint16_t indices[]) {
    auto desc = Desc{mode, vertexCount, indexCount, !!texs, !!colors, nullptr, 0};
    Builder builder(desc);
    if (!builder.isValid()) {
        return nullptr;
    }

    Sizes sizes(desc);
    SkASSERT(sizes.isValid());
    sk_careful_memcpy(builder.positions(), pos, sizes.fVSize);
    sk_careful_memcpy(builder.texCoords(), texs, sizes.fTSize);
    sk_careful_memcpy(builder.colors(), colors, sizes.fCSize);
    size_t isize = (mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize : sizes.fISize;
    sk_careful_memcpy(builder.indices(), indices, isize);

    return builder.detach();
}

size_t SkVertices::approximateSize() const {
    return this->getSizes().fTotal;
}

SkVertices::Sizes SkVertices::getSizes() const {
    Sizes sizes(
            {fMode, fVertexCount, fIndexCount, !!fTexs, !!fColors, fAttributes, fAttributeCount});
    SkASSERT(sizes.isValid());
    return sizes;
}

size_t SkVerticesPriv::customDataSize() const {
    return custom_data_size(fVertices->fAttributes, fVertices->fAttributeCount);
}

bool SkVerticesPriv::hasUsage(SkVertices::Attribute::Usage u) const {
    for (int i = 0; i < fVertices->fAttributeCount; ++i) {
        if (fVertices->fAttributes[i].fUsage == u) {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// storage = packed | vertex_count | index_count | attr_count
//           | pos[] | custom[] | texs[] | colors[] | indices[]

#define kMode_Mask          0x0FF
#define kHasTexs_Mask       0x100
#define kHasColors_Mask     0x200

void SkVerticesPriv::encode(SkWriteBuffer& buffer) const {
    // packed has room for additional flags in the future
    uint32_t packed = static_cast<uint32_t>(fVertices->fMode);
    SkASSERT((packed & ~kMode_Mask) == 0);  // our mode fits in the mask bits
    if (fVertices->fTexs) {
        packed |= kHasTexs_Mask;
    }
    if (fVertices->fColors) {
        packed |= kHasColors_Mask;
    }

    SkVertices::Sizes sizes = fVertices->getSizes();
    SkASSERT(!sizes.fBuilderTriFanISize);

    // Header
    buffer.writeUInt(packed);
    buffer.writeInt(fVertices->fVertexCount);
    buffer.writeInt(fVertices->fIndexCount);
    buffer.writeInt(fVertices->fAttributeCount);

    // Attribute metadata
    for (int i = 0; i < fVertices->fAttributeCount; ++i) {
        buffer.writeInt(static_cast<int>(fVertices->fAttributes[i].fType));
        buffer.writeInt(static_cast<int>(fVertices->fAttributes[i].fUsage));
        buffer.writeString(fVertices->fAttributes[i].fMarkerName);
    }

    // Data arrays
    buffer.writeByteArray(fVertices->fPositions, sizes.fVSize);
    buffer.writeByteArray(fVertices->fCustomData, sizes.fDSize);
    buffer.writeByteArray(fVertices->fTexs, sizes.fTSize);
    buffer.writeByteArray(fVertices->fColors, sizes.fCSize);
    // if index-count is odd, we won't be 4-bytes aligned, so we call the pad version
    buffer.writeByteArray(fVertices->fIndices, sizes.fISize);
}

sk_sp<SkVertices> SkVerticesPriv::Decode(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kVerticesUseReadBuffer_Version)) {
        // Old versions used an embedded blob that was serialized with SkWriter32/SkReader32.
        // We don't support loading those, but skip over the vertices to keep the buffer valid.
        auto data = buffer.readByteArrayAsData();
        (void)data;
        return nullptr;
    }

    auto decode = [](SkReadBuffer& buffer) -> sk_sp<SkVertices> {
        SkSafeRange safe;

        const uint32_t packed = buffer.readUInt();
        const int vertexCount = safe.checkGE(buffer.readInt(), 0);
        const int indexCount = safe.checkGE(buffer.readInt(), 0);
        const int attrCount = safe.checkGE(buffer.readInt(), 0);
        const SkVertices::VertexMode mode = safe.checkLE<SkVertices::VertexMode>(
                packed & kMode_Mask, SkVertices::kLast_VertexMode);
        const bool hasTexs = SkToBool(packed & kHasTexs_Mask);
        const bool hasColors = SkToBool(packed & kHasColors_Mask);

        if (!safe                                           // Invalid header fields
            || attrCount > SkVertices::kMaxCustomAttributes // Too many custom attributes?
            || (attrCount > 0 && (hasTexs || hasColors))) { // Overspecified (incompatible features)
            return nullptr;
        }

        SkVertices::Attribute attrs[SkVertices::kMaxCustomAttributes];
        SkString attrNames[SkVertices::kMaxCustomAttributes];
        for (int i = 0; i < attrCount; ++i) {
            auto type = buffer.checkRange(SkVertices::Attribute::Type::kFloat,
                                          SkVertices::Attribute::Type::kByte4_unorm);
            auto usage = buffer.checkRange(SkVertices::Attribute::Usage::kRaw,
                                           SkVertices::Attribute::Usage::kPosition);
            buffer.readString(&attrNames[i]);
            const char* markerName = attrNames[i].isEmpty() ? nullptr : attrNames[i].c_str();
            if (markerName && !SkCanvasPriv::ValidateMarker(markerName)) {
                return nullptr;
            }
            attrs[i] = SkVertices::Attribute(type, usage, markerName);
        }

        // Ensure that all of the attribute metadata was valid before proceeding
        if (!buffer.isValid()) {
            return nullptr;
        }

        const SkVertices::Desc desc{mode, vertexCount, indexCount, hasTexs, hasColors,
                                    attrCount ? attrs : nullptr, attrCount};
        SkVertices::Sizes sizes(desc);
        if (!sizes.isValid()) {
            return nullptr;
        }

        SkVertices::Builder builder(desc);
        if (!builder.isValid()) {
            return nullptr;
        }

        buffer.readByteArray(builder.positions(), sizes.fVSize);
        buffer.readByteArray(builder.customData(), sizes.fDSize);
        buffer.readByteArray(builder.texCoords(), sizes.fTSize);
        buffer.readByteArray(builder.colors(), sizes.fCSize);
        size_t isize = (mode == SkVertices::kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize
                                                                     : sizes.fISize;
        buffer.readByteArray(builder.indices(), isize);

        if (!buffer.isValid()) {
            return nullptr;
        }

        if (indexCount > 0) {
            // validate that the indices are in range
            const uint16_t* indices = builder.indices();
            for (int i = 0; i < indexCount; ++i) {
                if (indices[i] >= (unsigned)vertexCount) {
                    return nullptr;
                }
            }
        }

        return builder.detach();
    };

    if (auto verts = decode(buffer)) {
        return verts;
    }
    buffer.validate(false);
    return nullptr;
}

void SkVertices::operator delete(void* p) {
    ::operator delete(p);
}
