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
#include "include/core/SkTileMode.h"

class SkPaint;

namespace skgpu {

class Uniform;

// A single, fully specified combination resulting from a PaintCombo (i.e., it corresponds to a
// specific SkPaint)
struct Combination {
    bool operator==(const Combination& other) const {
        return fShaderType == other.fShaderType &&
               fTileMode == other.fTileMode &&
               fBlendMode == other.fBlendMode;
    }

    ShaderCombo::ShaderType fShaderType = ShaderCombo::ShaderType::kNone;
    SkTileMode fTileMode = SkTileMode::kRepeat;
    SkBlendMode fBlendMode = SkBlendMode::kSrc;
};

struct UniformData {
    static std::unique_ptr<UniformData> Make(int count,
                                             const Uniform* uniforms,
                                             size_t dataSize);

    ~UniformData() {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    const int fCount;
    const Uniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
#ifdef SK_DEBUG
    const size_t fDataSize;
#endif

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
#ifdef SK_DEBUG
            , fDataSize(dataSize)
#endif
    {
    }
};

std::tuple<Combination, std::unique_ptr<UniformData>> ExtractCombo(const SkPaint&);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
