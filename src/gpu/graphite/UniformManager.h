/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkHalf.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkColorData.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Uniform.h"

#include <algorithm>
#include <memory>

namespace skgpu::graphite {

class UniformDataBlock;

/**
 * Layout::kStd140
 * ===============
 *
 * From OpenGL Specification Section 7.6.2.2 "Standard Uniform Block Layout"
 * [https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159]:
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
 *
 * Layout::kStd430
 * ===============
 *
 * When using the std430 storage layout, shader storage blocks will be laid out in buffer storage
 * identically to uniform and shader storage blocks using the std140 layout, except that the base
 * alignment and stride of arrays of scalars and vectors in rule 4 and of structures in rule 9 are
 * not rounded up a multiple of the base alignment of a vec4.
 *
 * NOTE: While not explicitly stated, the layout rules for WebGPU and WGSL are identical to std430
 * for SSBOs and nearly identical to std140 for UBOs. The default mat2x2 type is treated as two
 * float2's (not an array), so its size is 16 and alignment is 8 (vs. a size of 32 and alignment of
 * 16 in std140). When emitting WGSL from SkSL, prepareUniformPolyfillsForInterfaceBlock() defined
 * in WGSLCodeGenerator, will modify the type declaration to match std140 exactly. This allows the
 * UniformManager and UniformOffsetCalculator to avoid having WebGPU-specific layout rules
 * (whereas SkSL::MemoryLayout has more complete rules).
 *
 * Layout::kMetal
 * ===============
 *
 * SkSL converts its types to the non-packed SIMD vector types in MSL. The size and alignment rules
 * are equivalent to std430 with the exception of half3 and float3. In std430, the size consumed
 * by non-array uniforms of these types is 3N while Metal consumes 4N (which is equal to the
 * alignment of a vec3 in both Layouts).
 *
 * Half vs. Float Uniforms
 * =======================
 *
 * Regardless of the precision when the shader is executed, std140 and std430 layouts consume
 * "half"-based uniforms in full 32-bit precision. Metal consumes "half"-based uniforms expecting
 * them to have already been converted to f16. WebGPU has an extension to support f16 types, which
 * behave like this, but we do not currently utilize it.
 *
 * The rules for std430 can be easily extended to f16 by applying N = 2 instead of N = 4 for the
 * base primitive alignment.
 *
 * NOTE: This could also apply to the int vs. short or uint vs. ushort types, but these smaller
 * integer types are not supported on all platforms as uniforms. We disallow short integer uniforms
 * entirely, and if the data savings are required, packing should be implemented manually.
 * Short integer vertex attributes are supported when the vector type lets it pack into 32 bits
 * (e.g. int16x2 or int8x4).
 *
 *
 * Generalized Layout Rules
 * ========================
 *
 * From the Layout descriptions above, the following simpler rules are sufficient:
 *
 * 1. If the base primitive type is "half" and the Layout expects half floats, N = 2; else, N = 4.
 *
 * 2. For arrays of scalars or vectors (with # of components, M = 1,2,3,4):
 *    a. If arrays must be aligned on vec4 boundaries OR M=3, then align and stride = 4*N.
 *    b. Otherwise, the align and stride = M*N.
 *
 *    In both cases, the total size required for the uniform is "array size"*stride.
 *
 * 3. For single scalars or vectors (M = 1,2,3,4), the align is SkNextPow2(M)*N (e.g. N,2N,4N,4N).
 *    a. If M = 3 and the Layout aligns the size with the alignment, the size is 4*N and N
 *       padding bytes must be zero'ed out afterwards.
 *    b. Otherwise, the align and size = M*N
 *
 * 4. The starting offset to write data is the current offset aligned to the calculated align value.
 *    The current offset is then incremented by the total size of the uniform.
 *
 *    For arrays and padded vec3's, the padding is included in the stride and total size, meeting
 *    the requirements of the original rule 4 in std140. When a single float3 that is not padded
 *    is written, the next offset only advances 12 bytes allowing a smaller type to pack tightly
 *    next to the Z coordinate.
 *
 * When N = 4, the CPU and GPU primitives are compatible, regardless of being float, int, or uint.
 * Contiguous ranges between any padding (for alignment or for array stride) can be memcpy'ed.
 * When N = 2, the CPU data is float and the GPU data f16, so values must be converted one primitive
 * at a time using SkFloatToHalf or skvx::to_half.
 *
 * The UniformManager will zero out any padding bytes (either prepended for starting alignment,
 * or appended for stride alignment). This is so that the final byte array can be hashed for uniform
 * value de-duplication before uploading to the GPU.
 *
 * While SkSL supports non-square matrices, the SkSLType enum and Graphite only expose support for
 * square matrices. Graphite assumes all matrix uniforms are in column-major order. This matches the
 * data layout of SkM44 already and UniformManager automatically transposes SkMatrix (which is in
 * row-major data) to be column-major. Thus, for layout purposes, a matrix or an array of matrices
 * can be laid out equivalently to an array of the column type with an array count multiplied by the
 * number of columns.
 *
 * Graphite does not embed structs within structs for its UBO or SSBO declarations for paint or
 * RenderSteps. However, when the "uniforms" are defined for use with SSBO random access, the
 * ordered set of uniforms is actually defining a struct instead of just a top-level interface.
 * As such, once all uniforms are recorded, the size must be rounded up to the maximum alignment
 * encountered for its members to satisfy alignment rules for all Layouts.
 *
 * If Graphite starts to define sub-structs, UniformOffsetCalculator can be used recursively.
 */
namespace LayoutRules {
    // The three diverging behaviors across the different Layouts:
    static constexpr bool PadVec3Size(Layout layout) { return layout == Layout::kMetal; }
    static constexpr bool AlignArraysAsVec4(Layout layout) { return layout == Layout::kStd140; }
    static constexpr bool UseFullPrecision(Layout layout) { return layout != Layout::kMetal; }
}

class UniformOffsetCalculator {
public:
    UniformOffsetCalculator() = default;

    static UniformOffsetCalculator ForTopLevel(Layout layout, int offset = 0) {
        return UniformOffsetCalculator(layout, offset, /*reqAlignment=*/1);
    }

    static UniformOffsetCalculator ForStruct(Layout layout) {
        const int reqAlignment = LayoutRules::AlignArraysAsVec4(layout) ? 16 : 1;
        return UniformOffsetCalculator(layout, /*offset=*/0, reqAlignment);
    }

    Layout layout() const { return fLayout; }

    // NOTE: The returned size represents the last consumed byte (if the recorded
    // uniforms are embedded within a struct, this will need to be rounded up to a multiple of
    // requiredAlignment()).
    int size() const { return fOffset; }
    int requiredAlignment() const { return fReqAlignment; }

    // Returns the correctly aligned offset to accommodate `count` instances of `type` and
    // advances the internal offset.
    //
    // After a call to this method, `size()` will return the offset to the end of `count` instances
    // of `type` (while the return value equals the aligned start offset). Subsequent calls will
    // calculate the new start offset starting at `size()`.
    int advanceOffset(SkSLType type, int count = Uniform::kNonArray);

    // Returns the correctly aligned offset to accommodate `count` instances of a custom struct
    // type that has had its own fields passed into the `substruct` offset calculator.
    //
    // After a call to this method, `size()` will return the offset to the end of `count` instances
    // of the struct types (while the return value equals the aligned start offset). This includes
    // any required padding of the struct size per rule #9.
    int advanceStruct(const UniformOffsetCalculator& substruct, int count = Uniform::kNonArray);

private:
    UniformOffsetCalculator(Layout layout, int offset, int reqAlignment)
            : fLayout(layout), fOffset(offset), fReqAlignment((reqAlignment)) {}

    Layout fLayout    = Layout::kInvalid;
    int fOffset       = 0;
    int fReqAlignment = 1;
};

class UniformManager {
public:
    UniformManager(Layout layout) { this->resetWithNewLayout(layout); }

    SkSpan<const char> finish() {
        this->alignTo(fReqAlignment);
        return SkSpan(fStorage);
    }

    size_t size() const { return fStorage.size(); }

    void resetWithNewLayout(Layout layout);
    void reset() { this->resetWithNewLayout(fLayout); }

    // scalars
    void write(float f)     { this->write<SkSLType::kFloat>(&f); }
    void write(int32_t i)   { this->write<SkSLType::kInt  >(&i); }
    void writeHalf(float f) { this->write<SkSLType::kHalf >(&f); }

    // [i|h]vec4 and arrays thereof (just add overloads as needed)
    void write(const SkPMColor4f& c) { this->write<SkSLType::kFloat4>(c.vec()); }
    void write(const SkRect& r)      { this->write<SkSLType::kFloat4>(r.asScalars()); }
    void write(const SkV4& v)        { this->write<SkSLType::kFloat4>(v.ptr()); }

    void write(const SkIRect& r)     { this->write<SkSLType::kInt4>(&r); }

    void writeHalf(const SkPMColor4f& c) { this->write<SkSLType::kHalf4>(c.vec()); }
    void writeHalf(const SkRect& r)      { this->write<SkSLType::kHalf4>(r.asScalars()); }
    void writeHalf(const SkV4& v)        { this->write<SkSLType::kHalf4>(v.ptr()); }

    void writeArray(SkSpan<const SkV4> v) {
        this->writeArray<SkSLType::kFloat4>(v.data(), v.size());
    }
    void writeArray(SkSpan<const SkPMColor4f> c) {
        this->writeArray<SkSLType::kFloat4>(c.data(), c.size());
    }
    void writeHalfArray(SkSpan<const SkPMColor4f> c) {
        this->writeArray<SkSLType::kHalf4>(c.data(), c.size());
    }

    // [i|h]vec3
    void write(const SkV3& v)     { this->write<SkSLType::kFloat3>(v.ptr()); }
    void write(const SkPoint3& p) { this->write<SkSLType::kFloat3>(&p); }

    void writeHalf(const SkV3& v)     { this->write<SkSLType::kHalf3>(v.ptr()); }
    void writeHalf(const SkPoint3& p) { this->write<SkSLType::kHalf3>(&p); }

    // NOTE: 3-element vectors never pack efficiently in arrays, so avoid using them

    // [i|h]vec2
    void write(const SkV2& v)    { this->write<SkSLType::kFloat2>(v.ptr()); }
    void write(const SkSize& s)  { this->write<SkSLType::kFloat2>(&s); }
    void write(const SkPoint& p) { this->write<SkSLType::kFloat2>(&p); }

    void write(const SkISize& s) { this->write<SkSLType::kInt2>(&s); }

    void writeHalf(const SkV2& v)    { this->write<SkSLType::kHalf2>(v.ptr()); }
    void writeHalf(const SkSize& s)  { this->write<SkSLType::kHalf2>(&s); }
    void writeHalf(const SkPoint& p) { this->write<SkSLType::kHalf2>(&p); }

    // NOTE: 2-element vectors don't pack efficiently in std140, so avoid using them

    // matrices
    void write(const SkM44& m) {
        // All Layouts treat a 4x4 column-major matrix as an array of vec4's, which is exactly how
        // SkM44 already stores its data.
        this->writeArray<SkSLType::kFloat4>(SkMatrixPriv::M44ColMajor(m), 4);
    }

    void writeHalf(const SkM44& m) {
        this->writeArray<SkSLType::kHalf4>(SkMatrixPriv::M44ColMajor(m), 4);
    }

    void write(const SkMatrix& m) {
        // SkMatrix is row-major, so rewrite to column major. All Layouts treat a 3x3 column
        // major matrix as an array of vec3's.
        float colMajor[9] = {m[0], m[3], m[6],
                             m[1], m[4], m[7],
                             m[2], m[5], m[8]};
        this->writeArray<SkSLType::kFloat3>(colMajor, 3);
    }
    void writeHalf(const SkMatrix& m) {
        float colMajor[9] = {m[0], m[3], m[6],
                             m[1], m[4], m[7],
                             m[2], m[5], m[8]};
        this->writeArray<SkSLType::kHalf3>(colMajor, 3);
    }

    // NOTE: 2x2 matrices can be manually packed the same or better as a vec4, so prefer that

    // This is a specialized uniform writing entry point intended to deduplicate the paint
    // color. If a more general system is required, the deduping logic can be added to the
    // other write methods (and this specialized method would be removed).
    void writePaintColor(const SkPMColor4f& color) {
        if (fWrotePaintColor) {
            // Validate expected uniforms, but don't write a second copy since the paint color
            // uniform can only ever be declared once in the final SkSL program.
            SkASSERT(this->checkExpected(/*dst=*/nullptr, SkSLType::kFloat4, Uniform::kNonArray));
        } else {
            this->write<SkSLType::kFloat4>(&color);
            fWrotePaintColor = true;
        }
    }

    // Copy from `src` using Uniform array-count semantics.
    void write(const Uniform&, const void* src);

    // UniformManager has basic support for writing substructs with the caveats:
    // 1. The base alignment of the substruct must be known a priori so the first member can be
    //    written immediately.
    // 2. Nested substructs are not supported (but could be if the padded-struct size was also
    //    provided to endStruct()).
    //
    // Call beginStruct(baseAlignment) before writing the first field. Then call the regular
    // write functions for each of the substruct's fields in order. Lastly, call endStruct() to
    // go back to writing fields in the top-level interface block.
    void beginStruct(int baseAlignment) {
        SkASSERT(this->checkBeginStruct(baseAlignment)); // verifies baseAlignment matches layout

        this->alignTo(baseAlignment);
        fStructBaseAlignment = baseAlignment;
        fReqAlignment = std::max(fReqAlignment, baseAlignment);
    }
    void endStruct() {
        SkASSERT(fStructBaseAlignment >= 1); // Must have started a struct
        this->alignTo(fStructBaseAlignment);
        SkASSERT(this->checkEndStruct()); // validate after padding out to struct's alignment
        fStructBaseAlignment = 0;
    }

    // Debug-only functions to control uniform expectations.
#ifdef SK_DEBUG
    bool isReset() const;
    void setExpectedUniforms(SkSpan<const Uniform> expected, bool isSubstruct);
    void doneWithExpectedUniforms();
#endif // SK_DEBUG

private:
    // All public write() functions in UniformManager already match scalar/vector SkSLTypes or have
    // explicitly converted matrix SkSLTypes to a writeArray<column type> so this does not need to
    // check anything beyond half[2,3,4].
    static constexpr bool IsHalfVector(SkSLType type) {
        return type >= SkSLType::kHalf && type <= SkSLType::kHalf4;
    }

    // Other than validation, actual layout doesn't care about 'type' and the logic can be
    // based on vector length and whether or not it's half or full precision.
    template <int N, bool Half> void write(const void* src, SkSLType type);
    template <int N, bool Half> void writeArray(const void* src, int count, SkSLType type);

    // Helpers to select dimensionality and convert to full precision if required by the Layout.
    template <SkSLType Type> void write(const void* src) {
        static constexpr int N = SkSLTypeVecLength(Type);
        if (IsHalfVector(Type) && !LayoutRules::UseFullPrecision(fLayout)) {
            this->write<N, /*Half=*/true>(src, Type);
        } else {
            this->write<N, /*Half=*/false>(src, Type);
        }
    }
    template <SkSLType Type> void writeArray(const void* src, int count) {
        static constexpr int N = SkSLTypeVecLength(Type);
        if (IsHalfVector(Type) && !LayoutRules::UseFullPrecision(fLayout)) {
            this->writeArray<N, /*Half=*/true>(src, count, Type);
        } else {
            this->writeArray<N, /*Half=*/false>(src, count, Type);
        }
    }

    // This is marked 'inline' so that it can be defined below with write() and writeArray() and
    // still link correctly.
    inline char* append(int alignment, int size);
    inline void alignTo(int alignment);

    SkTDArray<char> fStorage;

    Layout fLayout;
    int fReqAlignment = 0;
    int fStructBaseAlignment = 0;
    // The paint color is treated special and we only add its uniform once.
    bool fWrotePaintColor = false;

    // Debug-only verification that UniformOffsetCalculator is consistent and that write() calls
    // match the expected uniform declaration order.
#ifdef SK_DEBUG
    UniformOffsetCalculator fOffsetCalculator; // should match implicit offsets from append()
    UniformOffsetCalculator fSubstructCalculator; // 0-based, used when inside a substruct
    int fSubstructStartingOffset = -1; // offset within fOffsetCalculator of first field

    SkSpan<const Uniform> fExpectedUniforms;
    int fExpectedUniformIndex = 0;

    bool checkExpected(const void* dst, SkSLType, int count);
    bool checkBeginStruct(int baseAlignment);
    bool checkEndStruct();
#endif // SK_DEBUG
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions

// Shared helper for both write() and writeArray()
template <int N, bool Half>
struct LayoutTraits {
    static_assert(1 <= N && N <= 4);

    static constexpr int kElemSize = Half ? sizeof(SkHalf) : sizeof(float);
    static constexpr int kSize     = N * kElemSize;
    static constexpr int kAlign    = SkNextPow2_portable(N) * kElemSize;

    // Reads kSize bytes from 'src' and copies or converts (float->half) the N values
    // into 'dst'. Does not add any other padding that may depend on usage and Layout.
    static void Copy(const void* src, void* dst) {
        if constexpr (Half) {
            using VecF = skvx::Vec<SkNextPow2_portable(N), float>;
            VecF srcData;
            if constexpr (N == 3) {
                // Load the 3 values into a float4 to take advantage of vectorized conversion.
                // The 4th value will not be copied to dst.
                const float* srcF = static_cast<const float*>(src);
                srcData = VecF{srcF[0], srcF[1], srcF[2], 0.f};
            } else {
                srcData = VecF::Load(src);
            }

            auto dstData = to_half(srcData);
            // NOTE: this is identical to Vec::store() for N=1,2,4 and correctly drops the 4th
            // lane when N=3.
            memcpy(dst, &dstData, kSize);
        } else {
            memcpy(dst, src, kSize);
        }
    }

#ifdef SK_DEBUG
    static void Validate(const void* src, SkSLType type, Layout layout) {
        // Src validation
        SkASSERT(src);
        // All primitives on the CPU side should be 4 byte aligned
        SkASSERT(SkIsAlign4(reinterpret_cast<intptr_t>(src)));

        // Type and validation layout
        SkASSERT(SkSLTypeCanBeUniformValue(type));
        SkASSERT(SkSLTypeVecLength(type) == N); // Matrix types should have been flattened already
        if constexpr (Half) {
            SkASSERT(SkSLTypeIsFloatType(type));
            SkASSERT(!SkSLTypeIsFullPrecisionNumericType(type));
            SkASSERT(!LayoutRules::UseFullPrecision(layout));
        } else {
            SkASSERT(SkSLTypeIsFullPrecisionNumericType(type) ||
                     LayoutRules::UseFullPrecision(layout));
        }
    }
#endif
};

template<int N, bool Half>
void UniformManager::write(const void* src, SkSLType type) {
    using L = LayoutTraits<N, Half>;
    SkDEBUGCODE(L::Validate(src, type, fLayout);)

    // Layouts diverge in how vec3 size is determined for non-array usage
    char* dst = (N == 3 && LayoutRules::PadVec3Size(fLayout))
            ? this->append(L::kAlign, L::kSize + L::kElemSize)
            : this->append(L::kAlign, L::kSize);
    SkASSERT(this->checkExpected(dst, type, Uniform::kNonArray));

    L::Copy(src, dst);
    if (N == 3 && LayoutRules::PadVec3Size(fLayout)) {
        memset(dst + L::kSize, 0, L::kElemSize);
    }
}

template<int N, bool Half>
void UniformManager::writeArray(const void* src, int count, SkSLType type) {
    using L = LayoutTraits<N, Half>;
    static constexpr int kSrcStride = N * 4; // Source data is always in multiples of 4 bytes.

    SkDEBUGCODE(L::Validate(src, type, fLayout);)
    SkASSERT(count > 0);

    if (Half || N == 3 || (N != 4 && LayoutRules::AlignArraysAsVec4(fLayout))) {
        // A non-dense array (N == 3 is always padded to vec4, or the Layout requires it),
        // or we have to perform half conversion so iterate over each element.
        static constexpr int kStride  = Half ? L::kAlign : 4*L::kElemSize;
        SkASSERT(!(Half && LayoutRules::AlignArraysAsVec4(fLayout))); // should be exclusive

        const char* srcBytes = reinterpret_cast<const char*>(src);
        char* dst = this->append(kStride, kStride*count);
        SkASSERT(this->checkExpected(dst, type, count));

        for (int i = 0; i < count; ++i) {
            L::Copy(srcBytes, dst);
            if constexpr (kStride - L::kSize > 0) {
                memset(dst + L::kSize, 0, kStride - L::kSize);
            }

            dst += kStride;
            srcBytes += kSrcStride;
        }
    } else {
        // A dense array with no type conversion, so copy in one go.
        SkASSERT(L::kAlign == L::kSize && kSrcStride == L::kSize);
        char* dst = this->append(L::kAlign, L::kSize*count);
        SkASSERT(this->checkExpected(dst, type, count));

        memcpy(dst, src, L::kSize*count);
    }
}

void UniformManager::alignTo(int alignment) {
    SkASSERT(alignment >= 1 && SkIsPow2(alignment));
    if ((fStorage.size() & (alignment - 1)) != 0) {
        this->append(alignment, /*size=*/0);
    }
}

char* UniformManager::append(int alignment, int size) {
    // The base alignment for a struct should have been calculated for the current layout using
    // UniformOffsetCalculator, so every field appended within the struct should have an alignment
    // less than or equal to that base alignment.
    SkASSERT(fStructBaseAlignment <= 0 || alignment <= fStructBaseAlignment);

    const int offset = fStorage.size();
    const int padding = SkAlignTo(offset, alignment) - offset;

    // These are just asserts not aborts because SkSL compilation imposes limits on the size of
    // runtime effect arrays, and internal shaders should not be using excessive lengths.
    SkASSERT(std::numeric_limits<int>::max() - alignment >= offset);
    SkASSERT(std::numeric_limits<int>::max() - size >= padding);

    char* dst = fStorage.append(size + padding);
    if (padding > 0) {
        memset(dst, 0, padding);
        dst += padding;
    }

    fReqAlignment = std::max(fReqAlignment, alignment);
    return dst;
}

}  // namespace skgpu::graphite

#endif // skgpu_UniformManager_DEFINED
