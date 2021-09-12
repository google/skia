/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrShaderVar.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);

//////////////////////////////////////////////////////////////////////////////

GrUniformDataManager::UniformManager::UniformManager(ProgramUniforms uniforms, Layout layout)
        : fUniforms(std::move(uniforms)), fLayout(layout) {}

template <typename BaseType> static constexpr size_t tight_vec_size(int vecLength) {
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
template <typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct Rules140 {
    /**
     * For an array of scalars or vectors this returns the stride between array elements. For
     * matrices or arrays of matrices this returns the stride between columns of the matrix. Note
     * that for single (non-array) scalars or vectors we don't require a stride.
     */
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == GrShaderVar::kNonArray);
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
        size_t m = (kElementAlignment + kVec4Alignment - 1)/kVec4Alignment;
        return m*kVec4Alignment;
    }
};

/**
 * When using the std430 storage layout, shader storage blocks will be laid out in buffer storage
 * identically to uniform and shader storage blocks using the std140 layout, except that the base
 * alignment and stride of arrays of scalars and vectors in rule 4 and of structures in rule 9 are
 * not rounded up a multiple of the base alignment of a vec4.
 */
template <typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct Rules430 {
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == GrShaderVar::kNonArray);
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
template <typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
struct RulesMetal {
    static constexpr size_t Stride(int count) {
        SkASSERT(count >= 1 || count == GrShaderVar::kNonArray);
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

template <template <typename BaseType, int RowsOrVecLength, int Cols> class Rules>
class Writer {
private:
    using CType = GrProcessor::Uniform::CType;

    template<typename BaseType, int RowsOrVecLength = 1, int Cols = 1>
    static void Write(void* dst, int n, const BaseType v[]) {
        if (dst) {
            size_t stride = Rules<BaseType, RowsOrVecLength, Cols>::Stride(n);
            n = (n == GrShaderVar::kNonArray) ? 1 : n;
            n *= Cols;
            if (stride == RowsOrVecLength*sizeof(BaseType)) {
                std::memcpy(dst, v, n*stride);
            } else {
                for (int i = 0; i < n; ++i) {
                    std::memcpy(dst, v, RowsOrVecLength*sizeof(BaseType));
                    v += RowsOrVecLength;
                    dst = SkTAddOffset<void>(dst, stride);
                }
            }
        }
    }

    static void WriteSkMatrices(void* d, int n, const SkMatrix m[]) {
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
            offset += 3*Rules<float, 3, 3>::Stride(1);
        }
    }

public:
    static void WriteUniform(GrSLType type, CType ctype, void* d, int n, const void* v) {
        SkASSERT(d);
        SkASSERT(n >= 1 || n == GrShaderVar::kNonArray);
        switch (type) {
            case kInt_GrSLType:
                return Write<int32_t>(d, n, static_cast<const int32_t*>(v));

            case kInt2_GrSLType:
                return Write<int32_t, 2>(d, n, static_cast<const int32_t*>(v));

            case kInt3_GrSLType:
                return Write<int32_t, 3>(d, n, static_cast<const int32_t*>(v));

            case kInt4_GrSLType:
                return Write<int32_t, 4>(d, n, static_cast<const int32_t*>(v));

            case kHalf_GrSLType:
            case kFloat_GrSLType:
                return Write<float>(d, n, static_cast<const float*>(v));

            case kHalf2_GrSLType:
            case kFloat2_GrSLType:
                return Write<float, 2>(d, n, static_cast<const float*>(v));

            case kHalf3_GrSLType:
            case kFloat3_GrSLType:
                return Write<float, 3>(d, n, static_cast<const float*>(v));

            case kHalf4_GrSLType:
            case kFloat4_GrSLType:
                return Write<float, 4>(d, n, static_cast<const float*>(v));

            case kHalf2x2_GrSLType:
            case kFloat2x2_GrSLType:
                return Write<float, 2, 2>(d, n, static_cast<const float*>(v));

            case kHalf3x3_GrSLType:
            case kFloat3x3_GrSLType: {
                switch (ctype) {
                    case CType::kDefault:
                        return Write<float, 3, 3>(d, n, static_cast<const float*>(v));
                    case CType::kSkMatrix:
                        return WriteSkMatrices(d, n, static_cast<const SkMatrix*>(v));
                }
                SkUNREACHABLE;
            }

            case kHalf4x4_GrSLType:
            case kFloat4x4_GrSLType:
                return Write<float, 4, 4>(d, n, static_cast<const float*>(v));

            default:
                SK_ABORT("Unexpect uniform type");
        }
    }
};

bool GrUniformDataManager::UniformManager::writeUniforms(const GrProgramInfo& info, void* buffer) {
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
    auto set = [&, processorIndex = 0](const GrProcessor& p) mutable {
        SkASSERT(buffer);
        const ProcessorUniforms& uniforms = fUniforms[processorIndex];
        for (const NewUniform& u : uniforms) {
            if (u.type != kVoid_GrSLType) {
                SkASSERT(u.count >= 0);
                static_assert(GrShaderVar::kNonArray == 0);
                void* d = SkTAddOffset<void>(buffer, u.offset);
                size_t index = u.indexInProcessor;
                const void* v = p.uniformData(index);
                write(u.type, p.uniforms()[index].ctype(), d, u.count, v);
                wrote = true;
            }
        }
        ++processorIndex;
    };

    info.visitProcessors(set);
    return wrote;
}

//////////////////////////////////////////////////////////////////////////////

GrUniformDataManager::GrUniformDataManager(ProgramUniforms uniforms,
                                           Layout layout,
                                           uint32_t uniformCount,
                                           uint32_t uniformSize)
        : fUniformSize(uniformSize)
        , fUniformsDirty(false)
        , fUniformManager(std::move(uniforms), layout) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniformCount);
    // subclasses fill in the legacy uniforms in their constructor
}

void GrUniformDataManager::setUniforms(const GrProgramInfo& info) {
    if (fUniformManager.writeUniforms(info, fUniformData.get())) {
        this->markDirty();
    }
}

void* GrUniformDataManager::getBufferPtrAndMarkDirty(const Uniform& uni) const {
    fUniformsDirty = true;
    return static_cast<char*>(fUniformData.get())+uni.fOffset;
}

void GrUniformDataManager::set1i(UniformHandle u, int32_t i) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &i, sizeof(int32_t));
}

void GrUniformDataManager::set1iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt_GrSLType || uni.fType == kShort_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set1f(UniformHandle u, float v0) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, &v0, sizeof(float));
}

void GrUniformDataManager::set1fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat_GrSLType || uni.fType == kHalf_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[i];
        memcpy(buffer, curVec, sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set2i(UniformHandle u, int32_t i0, int32_t i1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[2] = { i0, i1 };
    memcpy(buffer, v, 2 * sizeof(int32_t));
}

void GrUniformDataManager::set2iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt2_GrSLType || uni.fType == kShort2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set2f(UniformHandle u, float v0, float v1) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[2] = { v0, v1 };
    memcpy(buffer, v, 2 * sizeof(float));
}

void GrUniformDataManager::set2fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2_GrSLType || uni.fType == kHalf2_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[2 * i];
        memcpy(buffer, curVec, 2 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set3i(UniformHandle u,
                                         int32_t i0,
                                         int32_t i1,
                                         int32_t i2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[3] = { i0, i1, i2 };
    memcpy(buffer, v, 3 * sizeof(int32_t));
}

void GrUniformDataManager::set3iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt3_GrSLType || uni.fType == kShort3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const int32_t* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(int32_t));
        buffer = static_cast<char*>(buffer) + 4*sizeof(int32_t);
    }
}

void GrUniformDataManager::set3f(UniformHandle u, float v0, float v1, float v2) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[3] = { v0, v1, v2 };
    memcpy(buffer, v, 3 * sizeof(float));
}

void GrUniformDataManager::set3fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat3_GrSLType || uni.fType == kHalf3_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    for (int i = 0; i < arrayCount; ++i) {
        const float* curVec = &v[3 * i];
        memcpy(buffer, curVec, 3 * sizeof(float));
        buffer = static_cast<char*>(buffer) + 4*sizeof(float);
    }
}

void GrUniformDataManager::set4i(UniformHandle u,
                                         int32_t i0,
                                         int32_t i1,
                                         int32_t i2,
                                         int32_t i3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    int32_t v[4] = { i0, i1, i2, i3 };
    memcpy(buffer, v, 4 * sizeof(int32_t));
}

void GrUniformDataManager::set4iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kInt4_GrSLType || uni.fType == kShort4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(int32_t));
}

void GrUniformDataManager::set4f(UniformHandle u,
                                         float v0,
                                         float v1,
                                         float v2,
                                         float v3) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(GrShaderVar::kNonArray == uni.fArrayCount);
    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    float v[4] = { v0, v1, v2, v3 };
    memcpy(buffer, v, 4 * sizeof(float));
}

void GrUniformDataManager::set4fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat4_GrSLType || uni.fType == kHalf4_GrSLType);
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = this->getBufferPtrAndMarkDirty(uni);
    memcpy(buffer, v, arrayCount * 4 * sizeof(float));
}

void GrUniformDataManager::setMatrix2f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<2>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix2fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<2>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix3f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<3>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix3fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<3>(u, arrayCount, m);
}

void GrUniformDataManager::setMatrix4f(UniformHandle u, const float matrix[]) const {
    this->setMatrices<4>(u, 1, matrix);
}

void GrUniformDataManager::setMatrix4fv(UniformHandle u, int arrayCount, const float m[]) const {
    this->setMatrices<4>(u, arrayCount, m);
}

template<int N> struct set_uniform_matrix;

template<int N> inline void GrUniformDataManager::setMatrices(UniformHandle u,
                                                              int arrayCount,
                                                              const float matrices[]) const {
    const Uniform& uni = fUniforms[u.toIndex()];
    SkASSERT(uni.fType == kFloat2x2_GrSLType + (N - 2) ||
             uni.fType == kHalf2x2_GrSLType + (N - 2));
    SkASSERT(arrayCount > 0);
    SkASSERT(arrayCount <= uni.fArrayCount ||
             (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

    void* buffer = fUniformData.get();
    fUniformsDirty = true;

    set_uniform_matrix<N>::set(buffer, uni.fOffset, arrayCount, matrices);
}

template<int N> struct set_uniform_matrix {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        buffer = static_cast<char*>(buffer) + uniformOffset;
        for (int i = 0; i < count; ++i) {
            const float* matrix = &matrices[N * N * i];
            for (int j = 0; j < N; ++j) {
                memcpy(buffer, &matrix[j * N], N * sizeof(float));
                buffer = static_cast<char*>(buffer) + 4 * sizeof(float);
            }
        }
    }
};

template<> struct set_uniform_matrix<4> {
    inline static void set(void* buffer, int uniformOffset, int count, const float matrices[]) {
        buffer = static_cast<char*>(buffer) + uniformOffset;
        memcpy(buffer, matrices, count * 16 * sizeof(float));
    }
};

