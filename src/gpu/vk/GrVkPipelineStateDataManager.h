/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateDataManager_DEFINED
#define GrVkPipelineStateDataManager_DEFINED

#include "glsl/GrGLSLProgramDataManager.h"

#include "vk/GrVkUniformHandler.h"

class GrVkGpu;
class GrVkUniformBuffer;

class GrVkPipelineStateDataManager : public GrGLSLProgramDataManager {
public:
    typedef GrVkUniformHandler::UniformInfoArray UniformInfoArray;

    GrVkPipelineStateDataManager(const UniformInfoArray&,
                                 uint32_t vertexUniformSize,
                                 uint32_t fragmentUniformSize);

    void set1f(UniformHandle, float v0) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2f(UniformHandle, float, float) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3f(UniformHandle, float, float, float) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set4f(UniformHandle, float, float, float, float) const override;
    void set4fv(UniformHandle, int arrayCount, const float v[]) const override;
    // matrices are column-major, the first two upload a single matrix, the latter two upload
    // arrayCount matrices into a uniform array.
    void setMatrix2f(UniformHandle, const float matrix[]) const override;
    void setMatrix3f(UniformHandle, const float matrix[]) const override;
    void setMatrix4f(UniformHandle, const float matrix[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const override;

    // for nvpr only
    void setPathFragmentInputTransform(VaryingHandle u, int components,
                                       const SkMatrix& matrix) const override {
        SkFAIL("Only supported in NVPR, which is not in vulkan");
    }

    void uploadUniformBuffers(const GrVkGpu* gpu,
                              GrVkUniformBuffer* vertexBuffer,
                              GrVkUniformBuffer* fragmentBuffer) const;
private:
    struct Uniform {
        uint32_t fBinding;
        uint32_t fOffset;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
            uint32_t    fSetNumber;
        );
    };

    template<int N> inline void setMatrices(UniformHandle, int arrayCount,
                                            const float matrices[]) const;

    void* getBufferPtrAndMarkDirty(const Uniform& uni) const;

    uint32_t fVertexUniformSize;
    uint32_t fFragmentUniformSize;

    SkTArray<Uniform, true> fUniforms;

    mutable SkAutoMalloc fVertexUniformData;
    mutable SkAutoMalloc fFragmentUniformData;
    mutable bool         fVertexUniformsDirty;
    mutable bool         fFragmentUniformsDirty;
};

#endif
