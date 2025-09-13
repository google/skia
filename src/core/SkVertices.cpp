/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkVertices.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkSafeMath.h"
#include "src/base/SkVx.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeRange.h"
#include "src/core/SkVerticesPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <new>
#include <utility>

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};

    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}

struct SkVertices::Desc {
    VertexMode  fMode;
    int         fVertexCount,
                fIndexCount;
    bool        fHasTexs,
                fHasColors;
};

struct SkVertices::Sizes {
    Sizes(const Desc& desc) {
        SkSafeMath safe;

        fVSize = safe.mul(desc.fVertexCount, sizeof(SkPoint));
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
                 safe.add(fVSize,
                 safe.add(fTSize,
                 safe.add(fCSize,
                          fISize))));

        if (safe.ok()) {
            fArrays = fVSize + fTSize + fCSize + fISize;  // just the sum of the arrays
        } else {
            sk_bzero(this, sizeof(*this));
        }
    }

    bool isValid() const {
        SkASSERT(fTotal >= fVSize);
        return fVSize > 0;
    }

    size_t fTotal = 0;  // size of entire SkVertices allocation (obj + arrays)
    size_t fArrays; // size of all the data arrays (V + D + T + C + I)
    size_t fVSize;
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
    this->init({mode, vertexCount, indexCount, hasTexs, hasColors});
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

    fVertices->fPositions      = (SkPoint*) advance(sizes.fVSize);
    fVertices->fTexs           = (SkPoint*) advance(sizes.fTSize);
    fVertices->fColors         = (SkColor*) advance(sizes.fCSize);
    fVertices->fIndices        = (uint16_t*)advance(sizes.fISize);

    fVertices->fVertexCount    = desc.fVertexCount;
    fVertices->fIndexCount     = desc.fIndexCount;
    fVertices->fMode           = desc.fMode;

    // We defer assigning fBounds and fUniqueID until detach() is called
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds = SkRect::BoundsOrEmpty({fVertices->fPositions,
                                                    fVertices->fVertexCount});
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
    auto desc = Desc{mode, vertexCount, indexCount, !!texs, !!colors};
    Builder builder(desc);
    if (!builder.isValid()) {
        return nullptr;
    }

    Sizes sizes(desc);
    SkASSERT(sizes.isValid());
    sk_careful_memcpy(builder.positions(), pos, sizes.fVSize);
    sk_careful_memcpy(builder.texCoords(), texs, sizes.fTSize);
    sk_careful_memcpy(builder.colors(), colors, sizes.fCSize);

    // The builder can update the number of indices.
    const size_t isize = ((mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize
                                                            : sizes.fISize),
                icount = isize / sizeof(uint16_t);

    // Ensure that indices are valid for the given vertex count.
    SkASSERT(vertexCount > 0);
    const uint16_t max_index = SkToU16(vertexCount - 1);

    size_t i = 0;
    for (; i + 8 <= icount; i += 8) {
        const skvx::ushort8 ind8 = skvx::ushort8::Load(indices + i),
                    clamped_ind8 = skvx::min(ind8, max_index);
        clamped_ind8.store(builder.indices() + i);
    }
    if (i + 4 <= icount) {
        const skvx::ushort4 ind4 = skvx::ushort4::Load(indices + i),
                    clamped_ind4 = skvx::min(ind4, max_index);
        clamped_ind4.store(builder.indices() + i);

        i += 4;
    }
    if (i + 2 <= icount) {
        const skvx::ushort2 ind2 = skvx::ushort2::Load(indices + i),
                    clamped_ind2 = skvx::min(ind2, max_index);
        clamped_ind2.store(builder.indices() + i);

        i += 2;
    }
    if (i < icount) {
        builder.indices()[i] = std::min(indices[i], max_index);
        SkDEBUGCODE(i += 1);
    }
    SkASSERT(i == icount);

    return builder.detach();
}

size_t SkVertices::approximateSize() const {
    return this->getSizes().fTotal;
}

SkVertices::Sizes SkVertices::getSizes() const {
    Sizes sizes({fMode, fVertexCount, fIndexCount, !!fTexs, !!fColors});
    SkASSERT(sizes.isValid());
    return sizes;
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

    // Data arrays
    buffer.writeByteArray(fVertices->fPositions, sizes.fVSize);
    buffer.writeByteArray(fVertices->fTexs, sizes.fTSize);
    buffer.writeByteArray(fVertices->fColors, sizes.fCSize);
    // if index-count is odd, we won't be 4-bytes aligned, so we call the pad version
    buffer.writeByteArray(fVertices->fIndices, sizes.fISize);
}

sk_sp<SkVertices> SkVerticesPriv::Decode(SkReadBuffer& buffer) {
    auto decode = [](SkReadBuffer& buffer) -> sk_sp<SkVertices> {
        SkSafeRange safe;
        bool hasCustomData = buffer.isVersionLT(SkPicturePriv::kVerticesRemoveCustomData_Version);

        const uint32_t packed = buffer.readUInt();
        const int vertexCount = safe.checkGE(buffer.readInt(), 0);
        const int indexCount = safe.checkGE(buffer.readInt(), 0);
        const int attrCount = hasCustomData ? safe.checkGE(buffer.readInt(), 0) : 0;
        const SkVertices::VertexMode mode = safe.checkLE<SkVertices::VertexMode>(
                packed & kMode_Mask, SkVertices::kLast_VertexMode);
        const bool hasTexs = SkToBool(packed & kHasTexs_Mask);
        const bool hasColors = SkToBool(packed & kHasColors_Mask);

        // Check that the header fields and buffer are valid. If this is data with the experimental
        // custom attributes feature - we don't support that any more.
        // We also don't support serialized triangle-fan data. We stopped writing that long ago,
        // so it should never appear in valid encoded data.
        if (!safe || !buffer.isValid() || attrCount ||
            mode == SkVertices::kTriangleFan_VertexMode) {
            return nullptr;
        }

        const SkVertices::Desc desc{mode, vertexCount, indexCount, hasTexs, hasColors};
        SkVertices::Sizes sizes(desc);
        if (!sizes.isValid() || sizes.fArrays > buffer.available()) {
            return nullptr;
        }

        SkVertices::Builder builder(desc);
        if (!builder.isValid()) {
            return nullptr;
        }

        buffer.readByteArray(builder.positions(), sizes.fVSize);
        if (hasCustomData) {
            size_t customDataSize = 0;
            buffer.skipByteArray(&customDataSize);
            if (customDataSize != 0) {
                return nullptr;
            }
        }
        buffer.readByteArray(builder.texCoords(), sizes.fTSize);
        buffer.readByteArray(builder.colors(), sizes.fCSize);
        buffer.readByteArray(builder.indices(), sizes.fISize);

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
