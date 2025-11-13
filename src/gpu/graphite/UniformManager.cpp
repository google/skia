/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/UniformManager.h"

#include "src/gpu/graphite/PipelineData.h"

// ensure that these types are the sizes the uniform data is expecting
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(float) == 4);
static_assert(sizeof(SkHalf) == 2);

namespace skgpu::graphite {

int UniformOffsetCalculator::advanceOffset(SkSLType type, int count) {
    SkASSERT(SkSLTypeCanBeUniformValue(type));

    int dimension = SkSLTypeMatrixSize(type);
    if (dimension > 0) {
        // All SkSL matrices are square and can be interpreted as an array of column vectors
        count = std::max(count, 1) * dimension;
    } else {
        dimension = SkSLTypeVecLength(type);
    }
    SkASSERT(1 <= dimension && dimension <= 4);

    // Bump dimension up to 4 if the array or vec3 consumes 4 primitives per element
    // NOTE: This affects the size, alignment already rounds up to a power of 2 automatically.
    const bool isArray = count > Uniform::kNonArray;
    if ((isArray && LayoutRules::AlignArraysAsVec4(fLayout)) ||
        (dimension == 3 && (isArray || LayoutRules::PadVec3Size(fLayout)))) {
        dimension = 4;
    }

    const int primitiveSize = LayoutRules::UseFullPrecision(fLayout) ||
                              SkSLTypeIsFullPrecisionNumericType(type) ? 4 : 2;
    const int align = SkNextPow2(dimension) * primitiveSize;
    const int alignedOffset = SkAlignTo(fOffset, align);
    fOffset = alignedOffset + dimension * primitiveSize * std::max(count, 1);
    fReqAlignment = std::max(fReqAlignment, align);

    return alignedOffset;
}

int UniformOffsetCalculator::advanceStruct(const UniformOffsetCalculator& substruct, int count) {
    SkASSERT(substruct.fLayout == fLayout); // Invalid if the layout rules used aren't consistent

    // If array element strides are forced to 16-byte alignment, structs must also have their
    // base alignment rounded up to 16-byte alignment, which should have been accounted for in
    // 'substruct's constructor.
    const int baseAlignment = substruct.requiredAlignment();
    SkASSERT(!LayoutRules::AlignArraysAsVec4(fLayout) || SkIsAlign16(baseAlignment));

    // Per layout rule #9, the struct size must be padded to its base alignment
    // (see https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159).
    const int alignedSize = SkAlignTo(substruct.size(), baseAlignment);

    const int alignedOffset = SkAlignTo(fOffset, baseAlignment);
    fOffset = alignedOffset + alignedSize * std::max(count, 1);
    fReqAlignment = std::max(fReqAlignment, baseAlignment);

    return alignedOffset;
}

//////////////////////////////////////////////////////////////////////////////

void UniformManager::resetWithNewLayout(Layout layout) {
    fStorage.clear();
    fLayout = layout;
    fStartingOffset = 0;
    fReqAlignment = 0;
    fStructBaseAlignment = 0;
    fWrotePaintColor = false;

#ifdef SK_DEBUG
    fOffsetCalculator = UniformOffsetCalculator::ForTopLevel(layout);
    fSubstructCalculator = {};
    fExpectedUniforms = {};
    fExpectedUniformIndex = 0;
#endif
}

void UniformManager::rewindToOffset() {
    fStorage.resize(fStartingOffset);
}

static std::pair<SkSLType, int> adjust_for_matrix_type(SkSLType type, int count) {
    // All Layouts flatten matrices and arrays of matrices into arrays of columns, so update
    // 'type' to be the column type and either multiply 'count' by the number of columns for
    // arrays of matrices, or set to exactly the number of columns for a "non-array" matrix.
    switch(type) {
        case SkSLType::kFloat2x2: return {SkSLType::kFloat2, 2*std::max(1, count)};
        case SkSLType::kFloat3x3: return {SkSLType::kFloat3, 3*std::max(1, count)};
        case SkSLType::kFloat4x4: return {SkSLType::kFloat4, 4*std::max(1, count)};

        case SkSLType::kHalf2x2:  return {SkSLType::kHalf2,  2*std::max(1, count)};
        case SkSLType::kHalf3x3:  return {SkSLType::kHalf3,  3*std::max(1, count)};
        case SkSLType::kHalf4x4:  return {SkSLType::kHalf4,  4*std::max(1, count)};

        // Otherwise leave type and count alone.
        default:                  return {type, count};
    }
}

void UniformManager::write(const Uniform& u, const void* data) {
    SkASSERT(SkSLTypeCanBeUniformValue(u.type()));
    SkASSERT(!u.isPaintColor()); // Must go through writePaintColor()

    auto [type, count] = adjust_for_matrix_type(u.type(), u.count());
    SkASSERT(SkSLTypeMatrixSize(type) < 0); // Matrix types should have been flattened

    const bool fullPrecision = LayoutRules::UseFullPrecision(fLayout) || !IsHalfVector(type);
    if (count == Uniform::kNonArray) {
        if (fullPrecision) {
            switch(SkSLTypeVecLength(type)) {
                case 1: this->write<1, /*Half=*/false>(data, type); break;
                case 2: this->write<2, /*Half=*/false>(data, type); break;
                case 3: this->write<3, /*Half=*/false>(data, type); break;
                case 4: this->write<4, /*Half=*/false>(data, type); break;
            }
        } else {
            switch(SkSLTypeVecLength(type)) {
                case 1: this->write<1, /*Half=*/true>(data, type); break;
                case 2: this->write<2, /*Half=*/true>(data, type); break;
                case 3: this->write<3, /*Half=*/true>(data, type); break;
                case 4: this->write<4, /*Half=*/true>(data, type); break;
            }
        }
    } else {
        if (fullPrecision) {
            switch(SkSLTypeVecLength(type)) {
                case 1: this->writeArray<1, /*Half=*/false>(data, count, type); break;
                case 2: this->writeArray<2, /*Half=*/false>(data, count, type); break;
                case 3: this->writeArray<3, /*Half=*/false>(data, count, type); break;
                case 4: this->writeArray<4, /*Half=*/false>(data, count, type); break;
            }
        } else {
            switch(SkSLTypeVecLength(type)) {
                case 1: this->writeArray<1, /*Half=*/true>(data, count, type); break;
                case 2: this->writeArray<2, /*Half=*/true>(data, count, type); break;
                case 3: this->writeArray<3, /*Half=*/true>(data, count, type); break;
                case 4: this->writeArray<4, /*Half=*/true>(data, count, type); break;
            }
        }
    }
}

#if defined(SK_DEBUG)

void UniformManager::checkBeginStruct(int baseAlignment) {
    // Wrote a struct field before the struct was started
    SkASSERT(fExpectedUniformIndex == 0);

    // Not expecting to start a struct (layout must be valid)
    SkASSERT(fSubstructCalculator.layout() != Layout::kInvalid);

    // Somehow already started a substruct (base alignment should be <= 0 initially)
    SkASSERT(fStructBaseAlignment <= 0);

    // Empty substructs are not allowed
    SkASSERT(!fExpectedUniforms.empty());

    // Assume the expected uniforms describe the whole substruct
    auto structCalculator = UniformOffsetCalculator::ForStruct(fLayout);
    for (const Uniform& f : fExpectedUniforms) {
        structCalculator.advanceOffset(f.type(), f.count());
    }

    // Calculated alignment must match the passed base alignment
    SkASSERT(baseAlignment == structCalculator.requiredAlignment());

    fSubstructStartingOffset = fOffsetCalculator.advanceStruct(structCalculator);
}

void UniformManager::checkEndStruct() {
    // Didn't write all the expected fields before ending the struct
    SkASSERT(fExpectedUniformIndex == (int)fExpectedUniforms.size());

    // Not expecting a struct (layout must be valid)
    SkASSERT(fSubstructCalculator.layout() != Layout::kInvalid);

    // Missing a beginStruct() (base alignment must be > 0 if we are in a struct)
    SkASSERT(fStructBaseAlignment > 0);

    // `fStructCalculator` should now have been advanced equivalently to the substruct calculator
    // used in checkBeginStruct() to calculate the expected starting offset.
    const int structSize = SkAlignTo(fSubstructCalculator.size(),
                                     fSubstructCalculator.requiredAlignment());

    // Somehow didn't end on the correct boundary
    SkASSERT(fStorage.size() == fSubstructStartingOffset + structSize);

    // UniformManager's alignment got out of sync with expected alignment
    SkASSERT(fReqAlignment == fOffsetCalculator.requiredAlignment());
    SkASSERT(fReqAlignment >= fSubstructCalculator.requiredAlignment());

    // Reset the substruct calculator to mark that the struct has been completed
    fSubstructCalculator = {};
}

void UniformManager::checkExpected(const void* dst, SkSLType type, int count) {
    // A write() outside of a UniformExpectationsVisitor or too many uniforms written for what
    // is expected.
    SkASSERT(fExpectedUniformIndex < SkTo<int>(fExpectedUniforms.size()));

    if (fSubstructCalculator.layout() != Layout::kInvalid) {
        // A write() that should be inside a struct, but missing a call to beginStruct()
        SkASSERT(fStructBaseAlignment > 0);
    } else {
        // A substruct was started when it shouldn't have been.
        SkASSERT(fStructBaseAlignment <= 0);
    }

    const Uniform& expected = fExpectedUniforms[fExpectedUniformIndex++];
    // Not all types are supported as uniforms or supported by UniformManager
    SkASSERT(SkSLTypeCanBeUniformValue(expected.type()));

    auto [expectedType, expectedCount] = adjust_for_matrix_type(expected.type(), expected.count());
    SkASSERT(expectedType == type && expectedCount == count);

    if (dst) {
        // If we have 'dst', it's the aligned starting offset of the uniform being checked, so
        // subtracting the address of the first byte in fStorage gives us the offset.
        int offset = static_cast<int>(reinterpret_cast<intptr_t>(dst) -
                                      reinterpret_cast<intptr_t>(fStorage.data())
                                      - fStartingOffset);

        if (fSubstructCalculator.layout() == Layout::kInvalid) {
            // Pass original expected type and count to the offset calculator for validation.
            SkASSERT(offset == fOffsetCalculator.advanceOffset(expected.type(), expected.count()));
            SkASSERT(fReqAlignment == fOffsetCalculator.requiredAlignment());

            // And if it is the paint color uniform, we should not have already written it.
            SkASSERT(!(fWrotePaintColor && expected.isPaintColor()));
        } else {
            int relOffset = fSubstructCalculator.advanceOffset(expected.type(), expected.count());
            SkASSERT(offset == fSubstructStartingOffset + relOffset);

            // The overall required alignment might already be higher from prior fields, but should
            // be at least what's required by the substruct.
            SkASSERT(fReqAlignment >= fSubstructCalculator.requiredAlignment());

            // And it should not be a paint color uniform within a substruct.
            SkASSERT(!expected.isPaintColor());
        }
    } else {
        // If 'dst' is null, it's an already-visited paint color uniform, so it's not being written
        // and not changing the offset, and should not be part of a substruct.
        SkASSERT(fWrotePaintColor);
        SkASSERT(fSubstructCalculator.layout() == Layout::kInvalid);
        SkASSERT(expected.isPaintColor());
    }
}

bool UniformManager::isReset() const {
    return fStorage.empty();
}

void UniformManager::setExpectedUniforms(SkSpan<const Uniform> expected, bool isSubstruct) {
    fExpectedUniforms = expected;
    fExpectedUniformIndex = 0;

    if (isSubstruct) {
        // Start collecting the subsequent uniforms with a 0-based offset to determine their
        // relative layout and required base alignment of the entire struct.
        fSubstructCalculator = UniformOffsetCalculator::ForStruct(fLayout);
    } else {
        // Expected uniforms will advance fOffsetCalculator directly
        SkASSERT(fSubstructCalculator.layout() == Layout::kInvalid);
    }
}

void UniformManager::doneWithExpectedUniforms() {
    SkASSERT(fExpectedUniformIndex == static_cast<int>(fExpectedUniforms.size()));
    // Any expected substruct should have been ended and validated inside endStruct(); if this fails
    // it means there is a missing endStruct().
    SkASSERT(fSubstructCalculator.layout() == Layout::kInvalid);
    fExpectedUniforms = {};
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
