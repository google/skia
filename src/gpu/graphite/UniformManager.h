/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTDArray.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Uniform.h"

class SkM44;
class SkMatrix;
struct SkPoint;
struct SkPoint3;
struct SkRect;
struct SkSize;
struct SkV2;
struct SkV4;

namespace skgpu::graphite {

enum class CType : unsigned;

class UniformDataBlock;

class UniformOffsetCalculator {
public:
    UniformOffsetCalculator(Layout layout, uint32_t startingOffset);

    size_t size() const { return fOffset; }

    // Calculates the correctly aligned offset to accommodate `count` instances of `type` and
    // advances the internal offset. Returns the correctly aligned start offset.
    //
    // After a call to this method, `size()` will return the offset to the end of `count` instances
    // of `type` (while the return value equals the aligned start offset). Subsequent calls will
    // calculate the new start offset starting at `size()`.
    size_t advanceOffset(SkSLType type, unsigned int count);

protected:
    SkSLType getUniformTypeForLayout(SkSLType type);
    void setLayout(Layout);

    using WriteUniformFn = uint32_t (*)(SkSLType type,
                                        CType ctype,
                                        void *dest,
                                        int n,
                                        const void *src);

    WriteUniformFn fWriteUniform;
    Layout fLayout;  // TODO: eventually 'fLayout' will not need to be stored
    uint32_t fOffset = 0;
};

class UniformManager : public UniformOffsetCalculator {
public:
    UniformManager(Layout layout) : UniformOffsetCalculator(layout, /*startingOffset=*/0) {}

    UniformDataBlock finishUniformDataBlock();
    size_t size() const { return fStorage.size(); }

    void resetWithNewLayout(Layout);
    void reset();

    // Write a single instance of `type` from the data block referenced by `src`.
    void write(SkSLType type, const void* src);

    // Write an array of `type` with `count` elements from the data block referenced by `src`.
    // Does nothing if `count` is 0.
    void writeArray(SkSLType type, const void* src, unsigned int count);

    // Copy from `src` using Uniform array-count semantics.
    void write(const Uniform&, const uint8_t* src);

    void write(const SkM44&);
    void write(const SkPMColor4f&);
    void writePaintColor(const SkPMColor4f&);
    void write(const SkRect&);
    void write(const SkV2&);
    void write(const SkV4&);
    void write(const SkSize&);
    void write(const SkPoint&);
    void write(const SkPoint3&);
    void write(float f);
    void write(int);

    void writeArray(SkSpan<const SkColor4f>);
    void writeArray(SkSpan<const SkPMColor4f>);
    void writeArray(SkSpan<const float>);

    void writeHalf(float f);
    void writeHalf(const SkMatrix&);
    void writeHalf(const SkM44&);
    void writeHalf(const SkColor4f&);
    void writeHalfArray(SkSpan<const float>);

    // Debug only utilities used for debug assertions and tests.
    void checkReset() const;
    void setExpectedUniforms(SkSpan<const Uniform>);
    void checkExpected(SkSLType, unsigned int count);
    void doneWithExpectedUniforms();

private:
    // Writes a single element of the given `type` if `count` == 0 (aka Uniform::kNonArray).
    // Writes an array of `count` elements if `count` > 0, obeying any array layout constraints.
    //
    // Do not call this method directly for any new write()/writeArray() overloads. Instead
    // call the write(SkSLType, const void*) and writeArray(SkSLType, const void*, unsigned int)
    // overloads which correctly abstract the array vs non-array semantics.
    void writeInternal(SkSLType type, unsigned int count, const void* src);

    // The paint color is treated special and we only add its uniform once.
    bool fWrotePaintColor = false;
#ifdef SK_DEBUG
    SkSpan<const Uniform> fExpectedUniforms;
    int fExpectedUniformIndex = 0;
#endif // SK_DEBUG

    SkTDArray<char> fStorage;
    uint32_t fReqAlignment = 0;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
