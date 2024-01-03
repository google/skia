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

//////////////////////////////////////////////////////////////////////////////

UniformDataBlock UniformManager::finishUniformDataBlock() {
    size_t size = SkAlignTo(fStorage.size(), fReqAlignment);
    size_t paddingSize = size - fStorage.size();
    if (paddingSize > 0) {
        char* padding = fStorage.append(paddingSize);
        memset(padding, 0, paddingSize);
    }
    return UniformDataBlock(SkSpan(fStorage.begin(), size));
}

void UniformManager::resetWithNewLayout(Layout layout) {
    fStorage.clear();
    fLayout = layout;
    fReqAlignment = 0;
    fWrotePaintColor = false;

#ifdef SK_DEBUG
    fOffsetCalculator = UniformOffsetCalculator(layout, 0);
    fExpectedUniforms = {};
    fExpectedUniformIndex = 0;
#endif
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

#ifdef SK_DEBUG

bool UniformManager::checkExpected(const void* dst, SkSLType type, int count) {
    if (fExpectedUniformIndex >= (int) fExpectedUniforms.size()) {
        // A write() outside of a UniformExpectationsVisitor or too many uniforms written for what
        // is expected.
        return false;
    }

    const Uniform& expected = fExpectedUniforms[fExpectedUniformIndex++];
    if (!SkSLTypeCanBeUniformValue(expected.type())) {
        // Not all types are supported as uniforms or supported by UniformManager
        return false;
    }

    auto [expectedType, expectedCount] = adjust_for_matrix_type(expected.type(), expected.count());
    if (expectedType != type || expectedCount != count) {
        return false;
    }

    if (dst) {
        // If we have 'dst', it's the aligned starting offset of the uniform being checked, so
        // subtracting the address of the first byte in fStorage gives us the offset.
        int offset = static_cast<int>(reinterpret_cast<intptr_t>(dst) -
                                      reinterpret_cast<intptr_t>(fStorage.data()));
        // Pass original expected type and count to the offset calculator for validation.
        if (offset != fOffsetCalculator.advanceOffset(expected.type(), expected.count())) {
            return false;
        }
        if (fReqAlignment != fOffsetCalculator.requiredAlignment()) {
            return false;
        }
        // And if it is the paint color uniform, we should not have already written it
        return !(fWrotePaintColor && expected.isPaintColor());
    } else {
        // If 'dst' is null, it's an already-visited paint color uniform, so it's not being written
        // and not changing the offset.
        SkASSERT(fWrotePaintColor);
        return expected.isPaintColor();
    }
}

bool UniformManager::isReset() const {
    return fStorage.empty();
}

void UniformManager::setExpectedUniforms(SkSpan<const Uniform> expected) {
    fExpectedUniforms = expected;
    fExpectedUniformIndex = 0;
}

void UniformManager::doneWithExpectedUniforms() {
    SkASSERT(fExpectedUniformIndex == static_cast<int>(fExpectedUniforms.size()));
    fExpectedUniforms = {};
}

#endif // SK_DEBUG

} // namespace skgpu::graphite
