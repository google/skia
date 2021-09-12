/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformDataManager_DEFINED
#define GrUniformDataManager_DEFINED

#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/core/SkAutoMalloc.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

#include <vector>

class GrProgramInfo;

/**
 * Subclass of GrGLSLProgramDataManager used to store uniforms for a program in a CPU buffer that
 * can be uploaded to a UBO.
 */
class GrUniformDataManager : public GrGLSLProgramDataManager {
public:
    enum class Layout {
        kStd140,
        kStd430,
        kMetal, /** This is our own self-imposed layout we use for Metal. */
    };

    struct NewUniform {
        size_t   indexInProcessor = ~0;
        GrSLType type             = kVoid_GrSLType;
        int      count            = 0;
        uint32_t offset           = 0;
    };

    using ProcessorUniforms = std::vector<NewUniform>;
    using ProgramUniforms   = std::vector<ProcessorUniforms>;

    GrUniformDataManager(ProgramUniforms,
                         Layout layout,
                         uint32_t uniformCount,
                         uint32_t uniformSize);

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

    // For the uniform data to be dirty so that we will reupload on the next use.
    void markDirty() { fUniformsDirty = true; }

    void setUniforms(const GrProgramInfo& info);

protected:
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

    uint32_t fUniformSize;

    SkTArray<Uniform, true> fUniforms;

    mutable SkAutoMalloc fUniformData;
    mutable bool         fUniformsDirty;

private:
    class UniformManager {
    public:
        UniformManager(ProgramUniforms, Layout layout);
        bool writeUniforms(const GrProgramInfo& info, void* buffer);

    private:
        ProgramUniforms fUniforms;
        Layout          fLayout;
    };

    UniformManager fUniformManager;
};

#endif
