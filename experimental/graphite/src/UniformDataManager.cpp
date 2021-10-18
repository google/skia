/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformDataManager.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkTemplates.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

static constexpr int kNonArray = 0;

namespace skgpu {

//////////////////////////////////////////////////////////////////////////////

UniformDataManager::UniformManager::UniformManager(Layout layout) : fLayout(layout) {}

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
        SkASSERT(count >= 1 || count == kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);
        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return Rules140<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return 0;
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
        SkASSERT(count >= 1 || count == kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);

        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return Rules430<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return 0;
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
        SkASSERT(count >= 1 || count == kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);
        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            return RulesMetal<BaseType, RowsOrVecLength>::Stride(1);
        }
        if (count == 0) {
            // Stride doesn't matter for a non-array.
            return 0;
        }
        return tight_vec_size<BaseType>(RowsOrVecLength == 3 ? 4 : RowsOrVecLength);
    }
};

template<template<typename BaseType, int RowsOrVecLength, int Cols> class Rules>
class Writer {
private:
    template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
    static void Write(void *dst, int n, const BaseType v[]) {
        if (dst) {
            size_t stride = Rules<BaseType, RowsOrVecLength, Cols>::Stride(n);
            n = (n == kNonArray) ? 1 : n;
            n *= Cols;
            if (stride == RowsOrVecLength * sizeof(BaseType)) {
                std::memcpy(dst, v, n * stride);
            } else {
                for (int i = 0; i < n; ++i) {
                    std::memcpy(dst, v, RowsOrVecLength * sizeof(BaseType));
                    v += RowsOrVecLength;
                    dst = SkTAddOffset<void>(dst, stride);
                }
            }
        }
    }

    static void WriteSkMatrices(void *d, int n, const SkMatrix m[]) {
        size_t offset = 0;
        for (int i = 0; i < std::max(n, 1); ++i) {
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
            Write<float, 3, 3>(SkTAddOffset<void>(d, offset), 1, mt);
            // Stride() will give us the stride of each column, so mul by 3 to get matrix stride.
            offset += 3 * Rules<float, 3, 3>::Stride(1);
        }
    }

public:
    static void WriteUniform(SLType type, CType ctype, void *d, int n, const void *v) {
        SkASSERT(d);
        SkASSERT(n >= 1 || n == kNonArray);
        switch (type) {
            case SLType::kInt:
                return Write<int32_t>(d, n, static_cast<const int32_t *>(v));

            case SLType::kInt2:
                return Write<int32_t, 2>(d, n, static_cast<const int32_t *>(v));

            case SLType::kInt3:
                return Write<int32_t, 3>(d, n, static_cast<const int32_t *>(v));

            case SLType::kInt4:
                return Write<int32_t, 4>(d, n, static_cast<const int32_t *>(v));

            case SLType::kHalf:
            case SLType::kFloat:
                return Write<float>(d, n, static_cast<const float *>(v));

            case SLType::kHalf2:
            case SLType::kFloat2:
                return Write<float, 2>(d, n, static_cast<const float *>(v));

            case SLType::kHalf3:
            case SLType::kFloat3:
                return Write<float, 3>(d, n, static_cast<const float *>(v));

            case SLType::kHalf4:
            case SLType::kFloat4:
                return Write<float, 4>(d, n, static_cast<const float *>(v));

            case SLType::kHalf2x2:
            case SLType::kFloat2x2:
                return Write<float, 2, 2>(d, n, static_cast<const float *>(v));

            case SLType::kHalf3x3:
            case SLType::kFloat3x3: {
                switch (ctype) {
                    case CType::kDefault:
                        return Write<float, 3, 3>(d, n, static_cast<const float *>(v));
                    case CType::kSkMatrix:
                        return WriteSkMatrices(d, n, static_cast<const SkMatrix *>(v));
                }
                SkUNREACHABLE;
            }

            case SLType::kHalf4x4:
            case SLType::kFloat4x4:
                return Write<float, 4, 4>(d, n, static_cast<const float *>(v));

            default:
                SK_ABORT("Unexpected uniform type");
        }
    }
};

bool UniformDataManager::UniformManager::writeUniforms(/*const GrProgramInfo &info,*/
                                                       void *buffer) {
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

    bool wrote = false;

    {
        // TODO: loop over the GrProgramInfo replacement, writing uniforms.
        int src = 22;
        int dst[2];
        write(SLType::kInt, CType::kDefault, &dst, kNonArray, &src);
        wrote = true;
    }

    return wrote;
}

//////////////////////////////////////////////////////////////////////////////

UniformDataManager::UniformDataManager(Layout layout,
                                       uint32_t uniformCount,
                                       uint32_t uniformSize)
        : fUniformSize(uniformSize)
        , fUniformsDirty(false)
        , fUniformManager(layout) {
    fUniformData.reset(uniformSize);
    fUniforms.reserve(uniformCount);
}

void UniformDataManager::setUniforms(/*const GrProgramInfo &info*/) {
    if (fUniformManager.writeUniforms(/*info,*/ fUniformData.get())) {
        this->markDirty();
    }
}

} // namespace skgpu
