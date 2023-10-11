/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UniformManager.h"

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkHalf.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Uniform.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);
static_assert(sizeof(int16_t) == 2);
static_assert(sizeof(SkHalf) == 2);

namespace skgpu::graphite {

//////////////////////////////////////////////////////////////////////////////
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
        SkASSERT(count >= 1 || count == graphite::Uniform::kNonArray);
        static_assert(RowsOrVecLength >= 1 && RowsOrVecLength <= 4);
        static_assert(Cols >= 1 && Cols <= 4);
        if (Cols != 1) {
            // This is a matrix or array of matrices. We return the stride between columns.
            SkASSERT(RowsOrVecLength > 1);
            uint32_t stride = Rules140<BaseType, RowsOrVecLength>::Stride(Uniform::kNonArray);

            // By Rule 4, the stride and alignment of the individual element must always match vec4.
            return SkAlignTo(stride, tight_vec_size<float>(4));
        }

        // Get alignment of a single non-array vector of BaseType by Rule 1, 2, or 3.
        int n = RowsOrVecLength == 3 ? 4 : RowsOrVecLength;
        if (count == Uniform::kNonArray) {
            return n * sizeof(BaseType);
        }

        // Rule 4.

        // Alignment of vec4 by Rule 2.
        constexpr size_t kVec4Alignment = tight_vec_size<float>(4);
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
            return Rules430<BaseType, RowsOrVecLength>::Stride(Uniform::kNonArray);
        }

        // Get alignment of a single non-array vector of BaseType by Rule 1, 2, or 3.
        int n = RowsOrVecLength == 3 ? 4 : RowsOrVecLength;
        if (count == Uniform::kNonArray) {
            return n * sizeof(BaseType);
        }

        // Rule 4 without the round up to a multiple of align-of vec4.
        return tight_vec_size<BaseType>(n);
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
            return RulesMetal<BaseType, RowsOrVecLength>::Stride(Uniform::kNonArray);
        }

        // Get alignment of a single non-array vector of BaseType by Rule 1, 2, or 3.
        int n = RowsOrVecLength == 3 ? 4 : RowsOrVecLength;
        if (count == 0) {
            return n * sizeof(BaseType);
        }

        return tight_vec_size<BaseType>(n);
    }
};

template<template<typename BaseType, int RowsOrVecLength, int Cols> class Rules>
class Writer {
private:
    template <typename MemType, typename UniformType>
    static void CopyUniforms(void* dst, const void* src, int numUniforms) {
        if constexpr (std::is_same<MemType, UniformType>::value) {
            // Matching types--use memcpy.
            std::memcpy(dst, src, numUniforms * sizeof(MemType));
            return;
        }

        if constexpr (std::is_same<MemType, float>::value &&
                      std::is_same<UniformType, SkHalf>::value) {
            // Convert floats to half.
            const float* floatBits = static_cast<const float*>(src);
            SkHalf* halfBits = static_cast<SkHalf*>(dst);
            while (numUniforms-- > 0) {
                *halfBits++ = SkFloatToHalf(*floatBits++);
            }
            return;
        }

        if constexpr (std::is_same<MemType, int32_t>::value &&
                      std::is_same<UniformType, int16_t>::value) {
            // Convert ints to short.
            const int32_t* intBits = static_cast<const int32_t*>(src);
            int16_t* shortBits = static_cast<int16_t*>(dst);
            while (numUniforms-- > 0) {
                *shortBits++ = int16_t(*intBits++);
            }
            return;
        }

        SK_ABORT("implement conversion from MemType to UniformType");
    }

    template <typename MemType, typename UniformType, int RowsOrVecLength = 1, int Cols = 1>
    static uint32_t Write(void *dst, int n, const MemType src[]) {
        size_t stride = Rules<UniformType, RowsOrVecLength, Cols>::Stride(n);
        n = (n == Uniform::kNonArray) ? 1 : n;
        n *= Cols;

        // A null value for `dst` means that this method was called to calculate the size of the
        // write without actually copying data.
        if (dst) {
            if (stride == RowsOrVecLength * sizeof(UniformType)) {
                CopyUniforms<MemType, UniformType>(dst, src, n * RowsOrVecLength);
            } else {
                for (int i = 0; i < n; ++i) {
                    CopyUniforms<MemType, UniformType>(dst, src, RowsOrVecLength);
                    src += RowsOrVecLength;
                    dst = SkTAddOffset<void>(dst, stride);
                }
            }
        }

        return n * stride;
    }

    template <typename UniformType>
    static uint32_t WriteSkMatrices(void *dst, int n, const SkMatrix m[]) {
        // Stride() will give us the stride of each column, so mul by 3 to get matrix stride.
        size_t stride = 3 * Rules<UniformType, 3, 3>::Stride(1);
        n = std::max(n, 1);

        // A null value for `dst` means that this method was called to calculate the size of the
        // write without actually copying data.
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
                Write<float, UniformType, 3, 3>(SkTAddOffset<void>(dst, offset), 1, mt);
                offset += stride;
            }
        }
        return n * stride;
    }

public:
    // If `dest` is a nullptr, then this method returns the size of the write without writing any
    // data.
    static uint32_t WriteUniform(SkSLType type,
                                 CType ctype,
                                 void *dest,
                                 int n,
                                 const void *src) {
        SkASSERT(n >= 1 || n == Uniform::kNonArray);
        switch (type) {
            case SkSLType::kShort:
                return Write<int32_t, int16_t>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kShort2:
                return Write<int32_t, int16_t, 2>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kShort3:
                return Write<int32_t, int16_t, 3>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kShort4:
                return Write<int32_t, int16_t, 4>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kInt:
                return Write<int32_t, int32_t>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kInt2:
                return Write<int32_t, int32_t, 2>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kInt3:
                return Write<int32_t, int32_t, 3>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kInt4:
                return Write<int32_t, int32_t, 4>(dest, n, static_cast<const int32_t *>(src));

            case SkSLType::kHalf:
                return Write<float, SkHalf>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat:
                return Write<float, float>(dest, n, static_cast<const float *>(src));

            case SkSLType::kHalf2:
                return Write<float, SkHalf, 2>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat2:
                return Write<float, float, 2>(dest, n, static_cast<const float *>(src));

            case SkSLType::kHalf3:
                return Write<float, SkHalf, 3>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat3:
                return Write<float, float, 3>(dest, n, static_cast<const float *>(src));

            case SkSLType::kHalf4:
                return Write<float, SkHalf, 4>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat4:
                return Write<float, float, 4>(dest, n, static_cast<const float *>(src));

            case SkSLType::kHalf2x2:
                return Write<float, SkHalf, 2, 2>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat2x2:
                return Write<float, float, 2, 2>(dest, n, static_cast<const float *>(src));

            case SkSLType::kHalf3x3:
                switch (ctype) {
                    case CType::kDefault:
                        return Write<float, SkHalf, 3, 3>(dest, n, static_cast<const float *>(src));
                    case CType::kSkMatrix:
                        return WriteSkMatrices<SkHalf>(dest, n, static_cast<const SkMatrix *>(src));
                }
                SkUNREACHABLE;

            case SkSLType::kFloat3x3:
                switch (ctype) {
                    case CType::kDefault:
                        return Write<float, float, 3, 3>(dest, n, static_cast<const float *>(src));
                    case CType::kSkMatrix:
                        return WriteSkMatrices<float>(dest, n, static_cast<const SkMatrix *>(src));
                }
                SkUNREACHABLE;

            case SkSLType::kHalf4x4:
                return Write<float, SkHalf, 4, 4>(dest, n, static_cast<const float *>(src));

            case SkSLType::kFloat4x4:
                return Write<float, float, 4, 4>(dest, n, static_cast<const float *>(src));

            default:
                SK_ABORT("Unexpected uniform type");
        }
    }
};

static bool is_matrix(SkSLType type) {
    switch (type) {
        case SkSLType::kHalf2x2:
        case SkSLType::kHalf3x3:
        case SkSLType::kHalf4x4:
        case SkSLType::kFloat2x2:
        case SkSLType::kFloat3x3:
        case SkSLType::kFloat4x4:
            return true;
        default:
            break;
    }
    return false;
}

// To determine whether a current offset is aligned, we can just 'and' the lowest bits with the
// alignment mask. A value of 0 means aligned, any other value is how many bytes past alignment we
// are. This works since all alignments are powers of 2. The mask is always (alignment - 1).
static uint32_t sksltype_to_alignment_mask(SkSLType type) {
    switch (type) {
        case SkSLType::kInt:
        case SkSLType::kUInt:
        case SkSLType::kFloat:
            return 0x3;
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
        case SkSLType::kFloat2:
            return 0x7;
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kFloat3:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
        case SkSLType::kFloat4:
            return 0xF;

        case SkSLType::kFloat2x2:
            return 0x7;
        case SkSLType::kFloat3x3:
            return 0xF;
        case SkSLType::kFloat4x4:
            return 0xF;

        case SkSLType::kShort:
        case SkSLType::kUShort:
        case SkSLType::kHalf:
            return 0x1;
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
        case SkSLType::kHalf2:
            return 0x3;
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kHalf3:
        case SkSLType::kHalf4:
            return 0x7;

        case SkSLType::kHalf2x2:
            return 0x3;
        case SkSLType::kHalf3x3:
            return 0x7;
        case SkSLType::kHalf4x4:
            return 0x7;

        // This query is only valid for certain types.
        case SkSLType::kVoid:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kInput:
            break;
    }
    SK_ABORT("Unexpected type");
}

// Given the current offset into the ubo, calculate the offset for the uniform we're trying to add
// taking into consideration all alignment requirements. Returns the aligned start offset for the
// new uniform.
static uint32_t get_ubo_aligned_offset(Layout layout,
                                       uint32_t currentOffset,
                                       SkSLType type,
                                       bool isArray) {
    uint32_t alignmentMask;
    if (layout == Layout::kStd140 && (isArray || is_matrix(type))) {
        // std140 array and matrix element alignment always equals the base alignment of a vec4.
        alignmentMask = sksltype_to_alignment_mask(SkSLType::kFloat4);
    } else {
        alignmentMask = sksltype_to_alignment_mask(type);
    }
    return (currentOffset + alignmentMask) & ~alignmentMask;
}

SkSLType UniformOffsetCalculator::getUniformTypeForLayout(SkSLType type) {
    if (fLayout != Layout::kMetal) {
        // GL/Vk expect uniforms in 32-bit precision. Convert lower-precision types to 32-bit.
        switch (type) {
            case SkSLType::kShort:      return SkSLType::kInt;
            case SkSLType::kUShort:     return SkSLType::kUInt;
            case SkSLType::kHalf:       return SkSLType::kFloat;

            case SkSLType::kShort2:     return SkSLType::kInt2;
            case SkSLType::kUShort2:    return SkSLType::kUInt2;
            case SkSLType::kHalf2:      return SkSLType::kFloat2;

            case SkSLType::kShort3:     return SkSLType::kInt3;
            case SkSLType::kUShort3:    return SkSLType::kUInt3;
            case SkSLType::kHalf3:      return SkSLType::kFloat3;

            case SkSLType::kShort4:     return SkSLType::kInt4;
            case SkSLType::kUShort4:    return SkSLType::kUInt4;
            case SkSLType::kHalf4:      return SkSLType::kFloat4;

            case SkSLType::kHalf2x2:    return SkSLType::kFloat2x2;
            case SkSLType::kHalf3x3:    return SkSLType::kFloat3x3;
            case SkSLType::kHalf4x4:    return SkSLType::kFloat4x4;

            default:                    break;
        }
    }

    return type;
}

void UniformOffsetCalculator::setLayout(Layout layout) {
    fLayout = layout;
    switch (layout) {
        case Layout::kStd140:
            fWriteUniform = Writer<Rules140>::WriteUniform;
            break;
        case Layout::kStd430:
            fWriteUniform = Writer<Rules430>::WriteUniform;
            break;
        case Layout::kMetal:
            fWriteUniform = Writer<RulesMetal>::WriteUniform;
            break;
        case Layout::kInvalid:
            SK_ABORT("Invalid layout type");
            break;
    }
}

UniformOffsetCalculator::UniformOffsetCalculator(Layout layout, uint32_t startingOffset)
        : fLayout(layout), fOffset(startingOffset) {
    this->setLayout(fLayout);
}

size_t UniformOffsetCalculator::advanceOffset(SkSLType type, unsigned int count) {
    SkSLType revisedType = this->getUniformTypeForLayout(type);

    // Insert padding as needed to get the correct uniform alignment.
    uint32_t alignedOffset = get_ubo_aligned_offset(fLayout,
                                                    fOffset,
                                                    revisedType,
                                                    /*isArray=*/count != Uniform::kNonArray);
    SkASSERT(alignedOffset >= fOffset);

    // Append the uniform size to our offset, then return the uniform start position.
    uint32_t uniformSize = fWriteUniform(revisedType, CType::kDefault,
                                         /*dest=*/nullptr, count, /*src=*/nullptr);
    fOffset = alignedOffset + uniformSize;
    return alignedOffset;
}

UniformDataBlock UniformManager::finishUniformDataBlock() {
    size_t size = SkAlignTo(fStorage.size(), fReqAlignment);
    size_t paddingSize = size - fStorage.size();
    char* padding = fStorage.append(paddingSize);
    memset(padding, 0, paddingSize);
    return UniformDataBlock(SkSpan(fStorage.begin(), size));
}

void UniformManager::resetWithNewLayout(Layout layout) {
    if (layout != fLayout) {
        this->setLayout(layout);
    }
    this->reset();
}

void UniformManager::reset() {
    fOffset = 0;
    fReqAlignment = 0;
    fStorage.clear();
    fWrotePaintColor = false;
}

void UniformManager::checkReset() const {
    SkASSERT(fOffset == 0);
    SkASSERT(fStorage.empty());
}

void UniformManager::setExpectedUniforms(SkSpan<const Uniform> expectedUniforms) {
    SkDEBUGCODE(fExpectedUniforms = expectedUniforms;)
    SkDEBUGCODE(fExpectedUniformIndex = 0;)
}

void UniformManager::checkExpected(SkSLType type, unsigned int count) {
    SkASSERT(fExpectedUniforms.size());
    SkASSERT(fExpectedUniformIndex >= 0 && fExpectedUniformIndex < (int)fExpectedUniforms.size());

    SkASSERT(fExpectedUniforms[fExpectedUniformIndex].type() == type);
    SkASSERT((fExpectedUniforms[fExpectedUniformIndex].count() == 0 && count == 1) ||
             fExpectedUniforms[fExpectedUniformIndex].count() == count);
    SkDEBUGCODE(fExpectedUniformIndex++;)
}

void UniformManager::doneWithExpectedUniforms() {
    SkASSERT(fExpectedUniformIndex == static_cast<int>(fExpectedUniforms.size()));
    SkDEBUGCODE(fExpectedUniforms = {};)
}

void UniformManager::writeInternal(SkSLType type, unsigned int count, const void* src) {
    SkSLType revisedType = this->getUniformTypeForLayout(type);

    const uint32_t startOffset = fOffset;
    const uint32_t alignedStartOffset = this->advanceOffset(revisedType, count);
    SkASSERT(fOffset > alignedStartOffset);  // `fOffset` now equals the total bytes to be written
    const uint32_t bytesNeeded = fOffset - alignedStartOffset;

    // Insert padding if needed.
    if (alignedStartOffset > startOffset) {
        fStorage.append(alignedStartOffset - startOffset);
    }
    char* dst = fStorage.append(bytesNeeded);
    [[maybe_unused]] uint32_t bytesWritten =
            fWriteUniform(revisedType, CType::kDefault, dst, count, src);
    SkASSERT(bytesNeeded == bytesWritten);

    fReqAlignment = std::max(fReqAlignment, sksltype_to_alignment_mask(revisedType) + 1);
}

void UniformManager::write(SkSLType type, const void* src) {
    this->checkExpected(type, 1);
    this->writeInternal(type, Uniform::kNonArray, src);
}

void UniformManager::writeArray(SkSLType type, const void* src, unsigned int count) {
    // Don't write any elements if count is 0. Since Uniform::kNonArray == 0, passing count
    // directly would cause a one-element non-array write.
    if (count > 0) {
        this->checkExpected(type, count);
        this->writeInternal(type, count, src);
    }
}

void UniformManager::write(const Uniform& u, const uint8_t* src) {
    this->checkExpected(u.type(), (u.count() == Uniform::kNonArray) ? 1 : u.count());
    this->writeInternal(u.type(), u.count(), src);
}

void UniformManager::write(const SkM44& mat) {
    static constexpr SkSLType kType = SkSLType::kFloat4x4;
    this->write(kType, &mat);
}

void UniformManager::write(const SkPMColor4f& color) {
    static constexpr SkSLType kType = SkSLType::kFloat4;
    this->write(kType, &color);
}

// This is a specialized uniform writing entry point intended to deduplicate the paint
// color. If a more general system is required, the deduping logic can be added to the
// other write methods (and this specialized method would be removed).
void UniformManager::writePaintColor(const SkPMColor4f& color) {
    static constexpr SkSLType kType = SkSLType::kFloat4;

    SkASSERT(fExpectedUniforms[fExpectedUniformIndex].isPaintColor());
    if (fWrotePaintColor) {
        this->checkExpected(kType, 1);
        return;
    }

    fWrotePaintColor = true;
    this->write(kType, &color);
}

void UniformManager::write(const SkRect& rect) {
    static constexpr SkSLType kType = SkSLType::kFloat4;
    this->write(kType, &rect);
}

void UniformManager::write(const SkPoint& point) {
    static constexpr SkSLType kType = SkSLType::kFloat2;
    this->write(kType, &point);
}

void UniformManager::write(const SkSize& size) {
    static constexpr SkSLType kType = SkSLType::kFloat2;
    this->write(kType, &size);
}

void UniformManager::write(const SkPoint3& point3) {
    static constexpr SkSLType kType = SkSLType::kFloat3;
    this->write(kType, &point3);
}

void UniformManager::write(float f) {
    static constexpr SkSLType kType = SkSLType::kFloat;
    this->write(kType, &f);
}

void UniformManager::write(int i) {
    static constexpr SkSLType kType = SkSLType::kInt;
    this->write(kType, &i);
}

void UniformManager::write(const SkV2& v) {
    static constexpr SkSLType kType = SkSLType::kFloat2;
    this->write(kType, &v);
}

void UniformManager::write(const SkV4& v) {
    static constexpr SkSLType kType = SkSLType::kFloat4;
    this->write(kType, &v);
}

void UniformManager::writeArray(SkSpan<const SkColor4f> arr) {
    static constexpr SkSLType kType = SkSLType::kFloat4;
    this->writeArray(kType, arr.data(), arr.size());
}

void UniformManager::writeArray(SkSpan<const SkPMColor4f> arr) {
    static constexpr SkSLType kType = SkSLType::kFloat4;
    this->writeArray(kType, arr.data(), arr.size());
}

void UniformManager::writeArray(SkSpan<const float> arr) {
    static constexpr SkSLType kType = SkSLType::kFloat;
    this->writeArray(kType, arr.data(), arr.size());
}

void UniformManager::writeHalf(float f) {
    static constexpr SkSLType kType = SkSLType::kHalf;
    this->write(kType, &f);
}

void UniformManager::writeHalf(const SkMatrix& mat) {
    static constexpr SkSLType kType = SkSLType::kHalf3x3;
    this->write(kType, &mat);
}

void UniformManager::writeHalf(const SkM44& mat) {
    static constexpr SkSLType kType = SkSLType::kHalf4x4;
    this->write(kType, &mat);
}

void UniformManager::writeHalf(const SkColor4f& unpremulColor) {
    static constexpr SkSLType kType = SkSLType::kHalf4;
    this->write(kType, &unpremulColor);
}

void UniformManager::writeHalfArray(SkSpan<const float> arr) {
    static constexpr SkSLType kType = SkSLType::kHalf;
    this->writeArray(kType, arr.data(), arr.size());
}

} // namespace skgpu::graphite
