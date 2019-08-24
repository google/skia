/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkVertices.h"

#include "include/core/SkData.h"
#include "include/private/SkTo.h"
#include "src/core/SkReader32.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkSafeRange.h"
#include "src/core/SkWriter32.h"
#include <atomic>
#include <new>

static int32_t next_id() {
    static std::atomic<int32_t> nextID{1};

    int32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidGenID);
    return id;
}

struct SkVertices::Sizes {
    Sizes(SkVertices::VertexMode mode, int vertexCount, int indexCount, bool hasTexs,
          bool hasColors, bool hasBones) {
        SkSafeMath safe;

        fVSize = safe.mul(vertexCount, sizeof(SkPoint));
        fTSize = hasTexs ? safe.mul(vertexCount, sizeof(SkPoint)) : 0;
        fCSize = hasColors ? safe.mul(vertexCount, sizeof(SkColor)) : 0;
        fBISize = hasBones ? safe.mul(vertexCount, sizeof(BoneIndices)) : 0;
        fBWSize = hasBones ? safe.mul(vertexCount, sizeof(BoneWeights)) : 0;

        fBuilderTriFanISize = 0;
        fISize = safe.mul(indexCount, sizeof(uint16_t));
        if (kTriangleFan_VertexMode == mode) {
            int numFanTris = 0;
            if (indexCount) {
                fBuilderTriFanISize = fISize;
                numFanTris = indexCount - 2;
            } else {
                numFanTris = vertexCount - 2;
                // By forcing this to become indexed we are adding a constraint to the maximum
                // number of vertices.
                if (vertexCount > (SkTo<int>(UINT16_MAX) + 1)) {
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
                 safe.add(fBISize,
                 safe.add(fBWSize,
                          fISize))))));

        if (safe.ok()) {
            fArrays = fTotal - sizeof(SkVertices);  // just the sum of the arrays
        } else {
            sk_bzero(this, sizeof(*this));
        }
    }

    bool isValid() const { return fTotal != 0; }

    size_t fTotal;  // size of entire SkVertices allocation (obj + arrays)
    size_t fArrays; // size of all the arrays (V + T + C + BI + BW + I)
    size_t fVSize;
    size_t fTSize;
    size_t fCSize;
    size_t fBISize;
    size_t fBWSize;
    size_t fISize;

    // For indexed tri-fans this is the number of amount of space fo indices needed in the builder
    // before conversion to indexed triangles (or zero if not indexed or not a triangle fan).
    size_t fBuilderTriFanISize;
};

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount,
                             uint32_t builderFlags) {
    bool hasTexs = SkToBool(builderFlags & SkVertices::kHasTexCoords_BuilderFlag);
    bool hasColors = SkToBool(builderFlags & SkVertices::kHasColors_BuilderFlag);
    bool hasBones = SkToBool(builderFlags & SkVertices::kHasBones_BuilderFlag);
    bool isVolatile = !SkToBool(builderFlags & SkVertices::kIsNonVolatile_BuilderFlag);
    this->init(mode, vertexCount, indexCount, isVolatile,
               SkVertices::Sizes(mode, vertexCount, indexCount, hasTexs, hasColors, hasBones));
}

SkVertices::Builder::Builder(VertexMode mode, int vertexCount, int indexCount, bool isVolatile,
                             const SkVertices::Sizes& sizes) {
    this->init(mode, vertexCount, indexCount, isVolatile, sizes);
}

void SkVertices::Builder::init(VertexMode mode, int vertexCount, int indexCount, bool isVolatile,
                               const SkVertices::Sizes& sizes) {
    if (!sizes.isValid()) {
        return; // fVertices will already be null
    }

    void* storage = ::operator new (sizes.fTotal);
    if (sizes.fBuilderTriFanISize) {
        fIntermediateFanIndices.reset(new uint8_t[sizes.fBuilderTriFanISize]);
    }

    fVertices.reset(new (storage) SkVertices);

    // need to point past the object to store the arrays
    char* ptr = (char*)storage + sizeof(SkVertices);

    fVertices->fPositions = (SkPoint*)ptr;                                  ptr += sizes.fVSize;
    fVertices->fTexs = sizes.fTSize ? (SkPoint*)ptr : nullptr;              ptr += sizes.fTSize;
    fVertices->fColors = sizes.fCSize ? (SkColor*)ptr : nullptr;            ptr += sizes.fCSize;
    fVertices->fBoneIndices = sizes.fBISize ? (BoneIndices*) ptr : nullptr; ptr += sizes.fBISize;
    fVertices->fBoneWeights = sizes.fBWSize ? (BoneWeights*) ptr : nullptr; ptr += sizes.fBWSize;
    fVertices->fIndices = sizes.fISize ? (uint16_t*)ptr : nullptr;
    fVertices->fVertexCnt = vertexCount;
    fVertices->fIndexCnt = indexCount;
    fVertices->fIsVolatile = isVolatile;
    fVertices->fMode = mode;

    // We defer assigning fBounds and fUniqueID until detach() is called
}

sk_sp<SkVertices> SkVertices::Builder::detach() {
    if (fVertices) {
        fVertices->fBounds.setBounds(fVertices->fPositions, fVertices->fVertexCnt);
        if (fVertices->fMode == kTriangleFan_VertexMode) {
            if (fIntermediateFanIndices.get()) {
                SkASSERT(fVertices->fIndexCnt);
                auto tempIndices = this->indices();
                for (int t = 0; t < fVertices->fIndexCnt - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = tempIndices[0];
                    fVertices->fIndices[3 * t + 1] = tempIndices[t + 1];
                    fVertices->fIndices[3 * t + 2] = tempIndices[t + 2];
                }
                fVertices->fIndexCnt = 3 * (fVertices->fIndexCnt - 2);
            } else {
                SkASSERT(!fVertices->fIndexCnt);
                for (int t = 0; t < fVertices->fVertexCnt - 2; ++t) {
                    fVertices->fIndices[3 * t + 0] = 0;
                    fVertices->fIndices[3 * t + 1] = SkToU16(t + 1);
                    fVertices->fIndices[3 * t + 2] = SkToU16(t + 2);
                }
                fVertices->fIndexCnt = 3 * (fVertices->fVertexCnt - 2);
            }
            fVertices->fMode = kTriangles_VertexMode;
        }
        fVertices->fUniqueID = next_id();
        return std::move(fVertices);        // this will null fVertices after the return
    }
    return nullptr;
}

int SkVertices::Builder::vertexCount() const {
    return fVertices ? fVertices->vertexCount() : 0;
}

int SkVertices::Builder::indexCount() const {
    return fVertices ? fVertices->indexCount() : 0;
}

bool SkVertices::Builder::isVolatile() const {
    return fVertices ? fVertices->isVolatile() : true;
}

SkPoint* SkVertices::Builder::positions() {
    return fVertices ? const_cast<SkPoint*>(fVertices->positions()) : nullptr;
}

SkPoint* SkVertices::Builder::texCoords() {
    return fVertices ? const_cast<SkPoint*>(fVertices->texCoords()) : nullptr;
}

SkColor* SkVertices::Builder::colors() {
    return fVertices ? const_cast<SkColor*>(fVertices->colors()) : nullptr;
}

SkVertices::BoneIndices* SkVertices::Builder::boneIndices() {
    return fVertices ? const_cast<BoneIndices*>(fVertices->boneIndices()) : nullptr;
}

SkVertices::BoneWeights* SkVertices::Builder::boneWeights() {
    return fVertices ? const_cast<BoneWeights*>(fVertices->boneWeights()) : nullptr;
}

uint16_t* SkVertices::Builder::indices() {
    if (!fVertices) {
        return nullptr;
    }
    if (fIntermediateFanIndices) {
        return reinterpret_cast<uint16_t*>(fIntermediateFanIndices.get());
    }
    return const_cast<uint16_t*>(fVertices->indices());
}

/** Makes a copy of the SkVertices and applies a set of bones, then returns the deformed
    vertices.

    @param bones      The bones to apply.
    @param boneCount  The number of bones.
    @return           The transformed SkVertices.
*/
sk_sp<SkVertices> SkVertices::applyBones(const SkVertices::Bone bones[], int boneCount) const {
    // If there aren't any bones, then nothing changes.
    // We don't check if the SkVertices object has bone indices/weights because there is the case
    // where the object can have no indices/weights but still have a world transform applied.
    if (!bones || !boneCount) {
        return sk_ref_sp(this);
    }
    SkASSERT(boneCount >= 1);

    // Copy the SkVertices.
    sk_sp<SkVertices> copy = SkVertices::MakeCopy(this->mode(),
                                                  this->vertexCount(),
                                                  this->positions(),
                                                  this->texCoords(),
                                                  this->colors(),
                                                  nullptr,
                                                  nullptr,
                                                  this->indexCount(),
                                                  this->indices());

    // Transform the positions.
    for (int i = 0; i < this->vertexCount(); i++) {
        SkPoint& position = copy->fPositions[i];

        // Apply the world transform.
        position = bones[0].mapPoint(position);

        // Apply the bone deformations.
        if (boneCount > 1) {
            SkASSERT(this->boneIndices());
            SkASSERT(this->boneWeights());

            SkPoint result = SkPoint::Make(0.0f, 0.0f);
            const SkVertices::BoneIndices& indices = this->boneIndices()[i];
            const SkVertices::BoneWeights& weights = this->boneWeights()[i];
            for (int j = 0; j < 4; j++) {
                int index = indices[j];
                float weight = weights[j];
                if (index == 0 || weight == 0.0f) {
                    continue;
                }
                SkASSERT(index < boneCount);

                // result += M * v * w.
                result += bones[index].mapPoint(position) * weight;
            }
            position = result;
        }
    }

    // Recalculate the bounds.
    copy->fBounds.setBounds(copy->fPositions, copy->fVertexCnt);

    return copy;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkVertices::MakeCopy(VertexMode mode, int vertexCount,
                                       const SkPoint pos[], const SkPoint texs[],
                                       const SkColor colors[],
                                       const BoneIndices boneIndices[],
                                       const BoneWeights boneWeights[],
                                       int indexCount, const uint16_t indices[],
                                       bool isVolatile) {
    SkASSERT((!boneIndices && !boneWeights) || (boneIndices && boneWeights));
    Sizes sizes(mode,
                vertexCount,
                indexCount,
                texs != nullptr,
                colors != nullptr,
                boneIndices != nullptr);
    if (!sizes.isValid()) {
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, isVolatile, sizes);
    SkASSERT(builder.isValid());

    sk_careful_memcpy(builder.positions(), pos, sizes.fVSize);
    sk_careful_memcpy(builder.texCoords(), texs, sizes.fTSize);
    sk_careful_memcpy(builder.colors(), colors, sizes.fCSize);
    sk_careful_memcpy(builder.boneIndices(), boneIndices, sizes.fBISize);
    sk_careful_memcpy(builder.boneWeights(), boneWeights, sizes.fBWSize);
    size_t isize = (mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize : sizes.fISize;
    sk_careful_memcpy(builder.indices(), indices, isize);

    return builder.detach();
}

size_t SkVertices::approximateSize() const {
    Sizes sizes(fMode,
                fVertexCnt,
                fIndexCnt,
                this->hasTexCoords(),
                this->hasColors(),
                this->hasBones());
    SkASSERT(sizes.isValid());
    return sizeof(SkVertices) + sizes.fArrays;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// storage = packed | vertex_count | index_count | pos[] | texs[] | colors[] | boneIndices[] |
//           boneWeights[] | indices[]
//         = header + arrays

#define kMode_Mask          0x0FF
#define kHasTexs_Mask       0x100
#define kHasColors_Mask     0x200
#define kHasBones_Mask      0x400
#define kIsNonVolatile_Mask 0x800
#define kHeaderSize         (3 * sizeof(uint32_t))

sk_sp<SkData> SkVertices::encode() const {
    // packed has room for addtional flags in the future (e.g. versioning)
    uint32_t packed = static_cast<uint32_t>(fMode);
    SkASSERT((packed & ~kMode_Mask) == 0);  // our mode fits in the mask bits
    if (this->hasTexCoords()) {
        packed |= kHasTexs_Mask;
    }
    if (this->hasColors()) {
        packed |= kHasColors_Mask;
    }
    if (this->hasBones()) {
        packed |= kHasBones_Mask;
    }
    if (!this->isVolatile()) {
        packed |= kIsNonVolatile_Mask;
    }

    Sizes sizes(fMode,
                fVertexCnt,
                fIndexCnt,
                this->hasTexCoords(),
                this->hasColors(),
                this->hasBones());
    SkASSERT(sizes.isValid());
    SkASSERT(!sizes.fBuilderTriFanISize);
    // need to force alignment to 4 for SkWriter32 -- will pad w/ 0s as needed
    const size_t size = SkAlign4(kHeaderSize + sizes.fArrays);

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    SkWriter32 writer(data->writable_data(), data->size());

    writer.write32(packed);
    writer.write32(fVertexCnt);
    writer.write32(fIndexCnt);
    writer.write(fPositions, sizes.fVSize);
    writer.write(fTexs, sizes.fTSize);
    writer.write(fColors, sizes.fCSize);
    writer.write(fBoneIndices, sizes.fBISize);
    writer.write(fBoneWeights, sizes.fBWSize);
    // if index-count is odd, we won't be 4-bytes aligned, so we call the pad version
    writer.writePad(fIndices, sizes.fISize);

    return data;
}

sk_sp<SkVertices> SkVertices::Decode(const void* data, size_t length) {
    if (length < kHeaderSize) {
        return nullptr;
    }

    SkReader32 reader(data, length);
    SkSafeRange safe;

    const uint32_t packed = reader.readInt();
    const int vertexCount = safe.checkGE(reader.readInt(), 0);
    const int indexCount = safe.checkGE(reader.readInt(), 0);
    const VertexMode mode = safe.checkLE<VertexMode>(packed & kMode_Mask,
                                                     SkVertices::kLast_VertexMode);
    if (!safe) {
        return nullptr;
    }
    const bool hasTexs = SkToBool(packed & kHasTexs_Mask);
    const bool hasColors = SkToBool(packed & kHasColors_Mask);
    const bool hasBones = SkToBool(packed & kHasBones_Mask);
    const bool isVolatile = !SkToBool(packed & kIsNonVolatile_Mask);
    Sizes sizes(mode, vertexCount, indexCount, hasTexs, hasColors, hasBones);
    if (!sizes.isValid()) {
        return nullptr;
    }
    // logically we can be only 2-byte aligned, but our buffer is always 4-byte aligned
    if (SkAlign4(kHeaderSize + sizes.fArrays) != length) {
        return nullptr;
    }

    Builder builder(mode, vertexCount, indexCount, isVolatile, sizes);

    reader.read(builder.positions(), sizes.fVSize);
    reader.read(builder.texCoords(), sizes.fTSize);
    reader.read(builder.colors(), sizes.fCSize);
    reader.read(builder.boneIndices(), sizes.fBISize);
    reader.read(builder.boneWeights(), sizes.fBWSize);
    size_t isize = (mode == kTriangleFan_VertexMode) ? sizes.fBuilderTriFanISize : sizes.fISize;
    reader.read(builder.indices(), isize);
    if (indexCount > 0) {
        // validate that the indicies are in range
        SkASSERT(indexCount == builder.indexCount());
        const uint16_t* indices = builder.indices();
        for (int i = 0; i < indexCount; ++i) {
            if (indices[i] >= (unsigned)vertexCount) {
                return nullptr;
            }
        }
    }
    return builder.detach();
}

void SkVertices::operator delete(void* p)
{
    ::operator delete(p);
}
