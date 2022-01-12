/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextUtils_DEFINED
#define skgpu_ContextUtils_DEFINED

#include "experimental/graphite/include/Context.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"

enum class CodeSnippetID : uint8_t;
class SkUniquePaintParamsID;

namespace skgpu {

class PaintParams;
class Uniform;

class UniformData : public SkRefCnt {
public:

    // TODO: should we require a name (e.g., "gradient_uniforms") for each uniform block so
    // we can better name the Metal FS uniform struct?
    static sk_sp<UniformData> Make(int count,
                                   const Uniform* uniforms,
                                   size_t dataSize);

    ~UniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    int count() const { return fCount; }
    const Uniform* uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index >= 0 && index < fCount);
        return fOffsets[index];
    }
    char* data() { return fData; }
    size_t dataSize() const { return fDataSize; }

private:
    UniformData(int count,
                const Uniform* uniforms,
                uint32_t* offsets,
                char* data,
                size_t dataSize)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    const int fCount;
    const Uniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

std::tuple<SkUniquePaintParamsID, sk_sp<UniformData>> ExtractPaintData(Context*,
                                                                       const PaintParams&);
SkSpan<const Uniform> GetUniforms(CodeSnippetID);

// TODO: Temporary way to get at SkSL snippet for handling the given shader type, which will be
// embedded in the fragment function's body. It has access to the vertex output via a "interpolated"
// variable, and must have a statement that writes to a float4 "out.color". Its uniforms (as defined
// by GetUniforms(type)) are available as a variable named "uniforms".
const char* GetShaderSkSL(CodeSnippetID);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
