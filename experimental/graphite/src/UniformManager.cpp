/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformManager.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkTemplates.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

namespace skgpu {

//////////////////////////////////////////////////////////////////////////////

UniformManager::UniformManager(Layout layout) : fLayout(layout) {}

template<typename BaseType>
static constexpr size_t tight_vec_size(int vecLength) {
    return sizeof(BaseType) * vecLength;
}

/**
 * From Section 7.6.2.2 "Standard Uniform Block Layout":
 *  1. If the member is a scalar consuming N basic machine units, the base alignment is N.
 *  2. If the member is a two- or four-component vector with components consuming N basic machine
 *     units, the base alignment is 2N or 4N, respectively.
 *  3. If the member is a three-component vector with components consuming N
 *     basic machine units, the base alignment is 4N.
 *  4. If the member is an array of scalars or vectors, the base alignment and array
 *     stride are set to match the base alignment of a single array element, according
 *     to rules (1), (2), and (3), and rounded up to the base alignment of a vec4. The
 *     array may have padding at the end; the base offset of the member following
 *     the array is rounded up to the next multiple of the base alignment.
 *  5. If the member is a column-major matrix with C columns and R rows, the
 *     matrix is stored identically to an array of C column vectors with R components each,
 *     according to rule (4).
 *  6. If the member is an array of S column-major matrices with C columns and
 *     R rows, the matrix is stored identically to a row of S × C column vectors
 *     with R components each, according to rule (4).
 *  7. If the member is a row-major matrix with C columns and R rows, the matrix
 *     is stored identically to an array of R row vectors with C components each,
 *     according to rule (4).
 *  8. If the member is an array of S row-major matrices with C columns and R
 *     rows, the matrix is stored identically to a row of S × R row vectors with C
 *    components each, according to rule (4).
 *  9. If the member is a structure, the base alignment of the structure is N, where
 *     N is the largest base alignment value of any of its members, and rounded
 *     up to the base alignment of a vec4. The individual members of this substructure are then
 *     assigned offsets by applying this set of rules recursively,
 *     where the base offset of the first member of the sub-structure is equal to the
 *     aligned offset of the structure. The structure may have padding at the end;
 *     the base offset of the member following the sub-structure is rounded up to
 *     the next multiple of the base alignment of the structure.
 * 10. If the member is an array of S structures, the S elements of the array are laid
 *     out in order, according to rule (9).
 */
template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct Rules140 {
    /**
     * For an array of scalars or vectors this returns the stride between array elements. For
     * matrices or arrays of matrices this returns the stride between columns of the matrix. Note
     * that for single (non-array) scalars or vectors we don't require a stride.
     */
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == Uniform::kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);
        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return Rules140<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return RowsOrVecLength * sizeof(BaseType);
        }

        // Rule 4.

        // Alignment of vec4 by Rule 2.
        constexpr size_t kVec4Alignment = tight_vec_size<float>(4);
        // Get alignment of a single vector of BaseType by Rule 1, 2, or 3
        int n = RowsOrVecLength == 3 ? 4 : RowsOrVecLength;
        size_t kElementAlignment = tight_vec_size<BaseType>(n);
        // Round kElementAlignment up to multiple of kVec4Alignment.
        size_t m = (kElementAlignment + kVec4Alignment - 1) / kVec4Alignment;
        return m * kVec4Alignment;
    }
};

/**
 * When using the std430 storage layout, shader storage blocks will be laid out in buffer storage
 * identically to uniform and shader storage blocks using the std140 layout, except that the base
 * alignment and stride of arrays of scalars and vectors in rule 4 and of structures in rule 9 are
 * not rounded up a multiple of the base alignment of a vec4.
 */
template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct Rules430 {
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == Uniform::kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);

        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return Rules430<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return RowsOrVecLength * sizeof(BaseType);
        }
        // Rule 4 without the round up to a multiple of align-of vec4.
        return tight_vec_size<BaseType>(RowsOrVecLength == 3 ? 4 : RowsOrVecLength);
    }
};

// The strides used here were derived from the rules we've imposed on ourselves in
// GrMtlPipelineStateDataManger. Everything is tight except 3-component which have the stride of
// their 4-component equivalents.
template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct RulesMetal {
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == Uniform::kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);
        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return RulesMetal<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return RowsOrVecLength * sizeof(BaseType);
        }
        return tight_vec_size<BaseType>(RowsOrVecLength == 3 ? 4 : RowsOrVecLength);
    }
};

template<template<typename BaseType, int RowsOrVecLength, int Cols> class Rules>
class Writer {
private:
    template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
    static uint32_t Write(void *dst, int n, const BaseType src[]) {
        size_t stride = Rules<BaseType, RowsOrVecLength, Cols>::Stride(n);
        n = (n == Uniform::kNonArray) ? 1 : n;
        n *= Cols;

        if (dst) {
            if (stride == RowsOrVecLength * sizeof(BaseType)) {
                std::memcpy(dst, src, n * stride);
            } else {
                for (int i = 0; i < n; ++i) {
                    std::memcpy(dst, src, RowsOrVecLength * sizeof(BaseType));
                    src += RowsOrVecLength;
                    dst = SkTAddOffset<void>(dst, stride);
                }
            }
        }

        return n * stride;
    }

    static uint32_t WriteSkMatrices(void *dst, int n, const SkMatrix m[]) {
        // Stride() will give us the stride of each column, so mul by 3 to get matrix stride.
        size_t stride = 3 * Rules<float, 3, 3>::Stride(1);
        n = std::max(n, 1);

        if (dst) {
            size_t offset = 0;
            for (int i = 0; i < n; ++i) {
                float mt[] = {
                        m[i].get(SkMatrix::kMScaleX),
                        m[i].get(SkMatrix::kMSkewY),
                        m[i].get(SkMatrix::kMPersp0),
                        m[i].get(SkMatrix::kMSkewX),
                        m[i].get(SkMatrix::kMScaleY),
                        m[i].get(SkMatrix::kMPersp1),
                        m[i].get(SkMatrix::kMTransX),
                        m[i].get(SkMatrix::kMTransY),
                        m[i].get(SkMatrix::kMPersp2),
                };
                Write<float, 3, 3>(SkTAddOffset<void>(dst, offset), 1, mt);
                offset += stride;
            }
        }
        return n * stride;
    }

public:
    static uint32_t WriteUniform(SLType type,
                                 CType ctype,
                                 void *dest,
                                 int n,
                                 const void *src) {
        SkASSERT(n >= 1 || n == Uniform::kNonArray);
        switch (type) {
            case SLType::kInt:
                return Write<int32_t>(dest, n, static_cast<const int32_t *>(src));

            case SLType::kInt2:
                return Write<int32_t, 2>(dest, n, static_cast<const int32_t *>(src));

            case SLType::kInt3:
                return Write<int32_t, 3>(dest, n, static_cast<const int32_t *>(src));

            case SLType::kInt4:
                return Write<int32_t, 4>(dest, n, static_cast<const int32_t *>(src));

            case SLType::kHalf:
            case SLType::kFloat:
                return Write<float>(dest, n, static_cast<const float *>(src));

            case SLType::kHalf2:
            case SLType::kFloat2:
                return Write<float, 2>(dest, n, static_cast<const float *>(src));

            case SLType::kHalf3:
            case SLType::kFloat3:
                return Write<float, 3>(dest, n, static_cast<const float *>(src));

            case SLType::kHalf4:
            case SLType::kFloat4:
                return Write<float, 4>(dest, n, static_cast<const float *>(src));

            case SLType::kHalf2x2:
            case SLType::kFloat2x2:
                return Write<float, 2, 2>(dest, n, static_cast<const float *>(src));

            case SLType::kHalf3x3:
            case SLType::kFloat3x3: {
                switch (ctype) {
                    case CType::kDefault:
                        return Write<float, 3, 3>(dest, n, static_cast<const float *>(src));
                    case CType::kSkMatrix:
                        return WriteSkMatrices(dest, n, static_cast<const SkMatrix *>(src));
                }
                SkUNREACHABLE;
            }

            case SLType::kHalf4x4:
            case SLType::kFloat4x4:
                return Write<float, 4, 4>(dest, n, static_cast<const float *>(src));

            default:
                SK_ABORT("Unexpected uniform type");
        }
    }
};

#ifdef SK_DEBUG
// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
static uint32_t sltype_to_alignment_mask(SLType type) {
    switch (type) {
        case SLType::kInt:
        case SLType::kUint:
        case SLType::kFloat:
            return 0x3;
        case SLType::kInt2:
        case SLType::kUint2:
        case SLType::kFloat2:
            return 0x7;
        case SLType::kInt3:
        case SLType::kUint3:
        case SLType::kFloat3:
        case SLType::kInt4:
        case SLType::kUint4:
        case SLType::kFloat4:
            return 0xF;

        case SLType::kFloat2x2:
            return 0x7;
        case SLType::kFloat3x3:
            return 0xF;
        case SLType::kFloat4x4:
            return 0xF;

/*
        // TODO(skia:12339): Enable these once MetalCodeGenerator supports half-precision types.
        case SLType::kShort:
        case SLType::kUShort:
        case SLType::kHalf:
            return 0x1;
        case SLType::kShort2:
        case SLType::kUShort2:
        case SLType::kHalf2:
            return 0x3;
        case SLType::kShort3:
        case SLType::kShort4:
        case SLType::kUShort3:
        case SLType::kUShort4:
        case SLType::kHalf3:
        case SLType::kHalf4:
            return 0x7;

        case SLType::kHalf2x2:
            return 0x3;
        case SLType::kHalf3x3:
            return 0x7;
        case SLType::kHalf4x4:
            return 0x7;
*/
        // TODO(skia:12339): Remove these once MetalCodeGenerator supports half-precision types.
        case SLType::kShort:
        case SLType::kUShort:
        case SLType::kHalf:
            return 0x3;
        case SLType::kShort2:
        case SLType::kUShort2:
        case SLType::kHalf2:
            return 0x7;
        case SLType::kShort3:
        case SLType::kShort4:
        case SLType::kUShort3:
        case SLType::kUShort4:
        case SLType::kHalf3:
        case SLType::kHalf4:
            return 0xF;

        case SLType::kHalf2x2:
            return 0x7;
        case SLType::kHalf3x3:
            return 0xF;
        case SLType::kHalf4x4:
            return 0xF;

        // This query is only valid for certain types.
        case SLType::kVoid:
        case SLType::kBool:
        case SLType::kBool2:
        case SLType::kBool3:
        case SLType::kBool4:
        case SLType::kTexture2DSampler:
        case SLType::kTextureExternalSampler:
        case SLType::kTexture2DRectSampler:
        case SLType::kSampler:
        case SLType::kTexture2D:
        case SLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

/** Returns the size in bytes taken up in Metal buffers for GrSLTypes. */
inline uint32_t sltype_to_mtl_size(SLType type) {
    switch (type) {
        case SLType::kInt:
        case SLType::kUint:
        case SLType::kFloat:
            return 4;
        case SLType::kInt2:
        case SLType::kUint2:
        case SLType::kFloat2:
            return 8;
        case SLType::kInt3:
        case SLType::kUint3:
        case SLType::kFloat3:
        case SLType::kInt4:
        case SLType::kUint4:
        case SLType::kFloat4:
            return 16;

        case SLType::kFloat2x2:
            return 16;
        case SLType::kFloat3x3:
            return 48;
        case SLType::kFloat4x4:
            return 64;

/*
        // TODO(skia:12339): Enable these once MetalCodeGenerator supports half-precision types.
        case SLType::kShort:
        case SLType::kUShort:
        case SLType::kHalf:
            return 2;
        case SLType::kShort2:
        case SLType::kUShort2:
        case SLType::kHalf2:
            return 4;
        case SLType::kShort3:
        case SLType::kShort4:
        case SLType::kUShort3:
        case SLType::kUShort4:
        case SLType::kHalf3:
        case SLType::kHalf4:
            return 8;

        case SLType::kHalf2x2:
            return 8;
        case SLType::kHalf3x3:
            return 24;
        case SLType::kHalf4x4:
            return 32;
*/
        // TODO(skia:12339): Remove these once MetalCodeGenerator supports half-precision types.
        case SLType::kShort:
        case SLType::kUShort:
        case SLType::kHalf:
            return 4;
        case SLType::kShort2:
        case SLType::kUShort2:
        case SLType::kHalf2:
            return 8;
        case SLType::kShort3:
        case SLType::kShort4:
        case SLType::kUShort3:
        case SLType::kUShort4:
        case SLType::kHalf3:
        case SLType::kHalf4:
            return 16;

        case SLType::kHalf2x2:
            return 16;
        case SLType::kHalf3x3:
            return 48;
        case SLType::kHalf4x4:
            return 64;

        // This query is only valid for certain types.
        case SLType::kVoid:
        case SLType::kBool:
        case SLType::kBool2:
        case SLType::kBool3:
        case SLType::kBool4:
        case SLType::kTexture2DSampler:
        case SLType::kTextureExternalSampler:
        case SLType::kTexture2DRectSampler:
        case SLType::kSampler:
        case SLType::kTexture2D:
        case SLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. The uniformOffset is set to the offset for
// the new uniform, and currentOffset is updated to be the offset to the end of the new uniform.
static uint32_t get_ubo_aligned_offset(uint32_t* currentOffset,
                                       uint32_t* maxAlignment,
                                       SLType type,
                                       int arrayCount) {
    uint32_t alignmentMask = sltype_to_alignment_mask(type);
    if (alignmentMask > *maxAlignment) {
        *maxAlignment = alignmentMask;
    }
    uint32_t offsetDiff = *currentOffset & alignmentMask;
    if (offsetDiff != 0) {
        offsetDiff = alignmentMask - offsetDiff + 1;
    }
    uint32_t uniformOffset = *currentOffset + offsetDiff;
    SkASSERT(sizeof(float) == 4);
    if (arrayCount) {
        *currentOffset = uniformOffset + sltype_to_mtl_size(type) * arrayCount;
    } else {
        *currentOffset = uniformOffset + sltype_to_mtl_size(type);
    }
    return uniformOffset;
}
#endif // SK_DEBUG

uint32_t UniformManager::writeUniforms(SkSpan<const Uniform> uniforms,
                                       void** srcs,
                                       uint32_t* offsets,
                                       void *dst) {
    decltype(&Writer<Rules140>::WriteUniform) write;
    switch (fLayout) {
        case Layout::kStd140:
            write = Writer<Rules140>::WriteUniform;
            break;
        case Layout::kStd430:
            write = Writer<Rules430>::WriteUniform;
            break;
        case Layout::kMetal:
            write = Writer<RulesMetal>::WriteUniform;
            break;
    }

#ifdef SK_DEBUG
    uint32_t curUBOOffset = 0;
    uint32_t curUBOMaxAlignment = 0;
#endif // SK_DEBUG

    uint32_t offset = 0;

    for (int i = 0; i < (int) uniforms.size(); ++i) {
        const Uniform& u = uniforms[i];

#ifdef SK_DEBUG
        uint32_t debugOffset = get_ubo_aligned_offset(&curUBOOffset,
                                                      &curUBOMaxAlignment,
                                                      u.type(),
                                                      u.count());
#endif // SK_DEBUG

        uint32_t bytesWritten = write(u.type(),
                                      CType::kDefault,
                                      dst,
                                      u.count(),
                                      srcs ? srcs[i] : nullptr);
        SkASSERT(debugOffset == offset);

        if (offsets) {
            offsets[i] = offset;
        }
        offset += bytesWritten;
    }

    return offset;
}

} // namespace skgpu
