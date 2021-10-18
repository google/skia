/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformDataManager_DEFINED
#define skgpu_UniformDataManager_DEFINED

#include <vector>
#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "src/core/SkAutoMalloc.h"

namespace skgpu {

/**
 * Used to store uniforms for a program in a CPU buffer that can be uploaded to a UBO.
 */
class UniformDataManager {
public:
    enum class Layout {
        kStd140,
        kStd430,
        kMetal, /** This is our own self-imposed layout we use for Metal. */
    };

    struct NewUniform {
        size_t indexInProcessor = ~0;
        SLType type = SLType::kVoid;
        int count = 0;
        uint32_t offset = 0;
    };

    UniformDataManager(Layout layout,
                       uint32_t uniformCount,
                       uint32_t uniformSize);

    // For the uniform data to be dirty so that we will reupload on the next use.
    void markDirty() { fUniformsDirty = true; }

    // TODO: replace GrProgramInfo w/ something more generic
    void setUniforms(/*const GrProgramInfo &info*/);

protected:
    struct Uniform {
        uint32_t fOffset;
        SkDEBUGCODE(
                SLType fType;
                int fArrayCount;
        );
    };

    uint32_t fUniformSize;

    std::vector<Uniform> fUniforms;

    mutable SkAutoMalloc fUniformData;
    mutable bool fUniformsDirty;

private:
    class UniformManager {
    public:
        UniformManager(Layout layout);

        // TODO: replace GrProgramInfo w/ something more generic
        bool writeUniforms(/*const GrProgramInfo &info,*/ void *buffer);

    private:
        using ProgramUniforms = std::vector<std::vector<NewUniform>>;

        ProgramUniforms fUniforms;
        Layout fLayout;
    };

    UniformManager fUniformManager;
};

} // namespace skgpu

#endif // skgpu_UniformDataManager_DEFINED
