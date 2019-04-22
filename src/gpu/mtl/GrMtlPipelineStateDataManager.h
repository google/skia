/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateDataManager_DEFINED
#define GrMtlPipelineStateDataManager_DEFINED

#include "glsl/GrGLSLProgramDataManager.h"
#include "GrMtlUniformHandler.h"
#include "SkAutoMalloc.h"

#import <Metal/Metal.h>

class GrMtlBuffer;
class GrMtlGpu;

class GrMtlPipelineStateDataManager : public GrGLSLProgramDataManager {
public:
    typedef GrMtlUniformHandler::UniformInfoArray UniformInfoArray;

    GrMtlPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t geometryUniformSize,
                                  uint32_t fragmentUniformSize);

    void set1i(UniformHandle, int32_t) const override;
    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set1f(UniformHandle, float v0) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2i(UniformHandle, int32_t, int32_t) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2f(UniformHandle, float, float) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3i(UniformHandle, int32_t, int32_t, int32_t) const override;
    void set3iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set3f(UniformHandle, float, float, float) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set4i(UniformHandle, int32_t, int32_t, int32_t, int32_t) const override;
    void set4iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
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
        SK_ABORT("Only supported in NVPR, which is not in Metal");
    }

    void uploadAndBindUniformBuffers(GrMtlGpu* gpu,
                                     id<MTLRenderCommandEncoder> renderCmdEncoder) const;
    void resetDirtyBits();

private:
    struct Uniform {
        uint32_t fBinding;
        uint32_t fOffset;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    template<int N> inline void setMatrices(UniformHandle, int arrayCount,
                                            const float matrices[]) const;

    void* getBufferPtrAndMarkDirty(const Uniform& uni) const;

    uint32_t fGeometryUniformSize;
    uint32_t fFragmentUniformSize;

    SkTArray<Uniform, true> fUniforms;

    mutable SkAutoMalloc fGeometryUniformData;
    mutable SkAutoMalloc fFragmentUniformData;
    mutable bool         fGeometryUniformsDirty;
    mutable bool         fFragmentUniformsDirty;
};

#endif
