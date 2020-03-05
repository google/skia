/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramDataManager_DEFINED
#define GrDawnProgramDataManager_DEFINED

#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "src/gpu/dawn/GrDawnUniformHandler.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "dawn/webgpu_cpp.h"

#include "src/core/SkAutoMalloc.h"

class GrDawnGpu;
class GrDawnUniformBuffer;

class GrDawnProgramDataManager : public GrGLSLProgramDataManager {
public:
    typedef GrDawnUniformHandler::UniformInfoArray UniformInfoArray;

    GrDawnProgramDataManager(const UniformInfoArray&, uint32_t uniformBufferSize);

    void set1i(UniformHandle, int32_t) const override;
    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2i(UniformHandle, int32_t, int32_t) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set3i(UniformHandle, int32_t, int32_t, int32_t) const override;
    void set3iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set4i(UniformHandle, int32_t, int32_t, int32_t, int32_t) const override;
    void set4iv(UniformHandle, int arrayCount, const int32_t v[]) const override;

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
        SK_ABORT("Only supported in NVPR, which is not in Dawn");
    }

    void uploadUniformBuffers(void* dest) const;

    uint32_t uniformBufferSize() const { return fUniformBufferSize; }
private:
    struct Uniform {
        uint32_t fOffset;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    template<int N> inline void setMatrices(UniformHandle, int arrayCount,
                                            const float matrices[]) const;

    void* getBufferPtrAndMarkDirty(const Uniform& uni) const;

    uint32_t fUniformBufferSize;

    SkTArray<Uniform, true> fUniforms;

    mutable SkAutoMalloc fUniformData;
    mutable bool         fUniformsDirty;
};

#endif
