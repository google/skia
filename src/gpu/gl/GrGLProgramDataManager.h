/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkTBlockList.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

#include <vector>

class GrGLGpu;
class SkMatrix;
class GrGLProgram;
class GrGLContext;
struct GrGLInterface;
class GrProgramInfo;
class GrUniformAggregator;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : public GrGLSLProgramDataManager {
public:
    struct GLUniformInfo : public GrGLSLUniformHandler::UniformInfo {
        GrGLint fLocation;
    };

    struct VaryingInfo {
        GrShaderVar fVariable;
        GrGLint     fLocation;
    };

    // This uses a SkTBlockList rather than SkTArray/std::vector so that the GrShaderVars
    // don't move in memory after they are inserted. Users of GrGLShaderBuilder get refs to the vars
    // and ptrs to their name strings. Otherwise, we'd have to hand out copies.
    typedef SkTBlockList<GLUniformInfo> UniformInfoArray;
    typedef SkTBlockList<VaryingInfo>   VaryingInfoArray;

    GrGLProgramDataManager(GrGLGpu*,
                           const UniformInfoArray&,
                           GrGLuint programID,
                           const GrUniformAggregator& aggregator);

    void setSamplerUniforms(const UniformInfoArray& samplers, int startUnit) const;

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
    *  array of uniforms. arrayCount must be <= the array count of the uniform.
    */
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
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    void setMatrix2f(UniformHandle, const float matrix[]) const override;
    void setMatrix3f(UniformHandle, const float matrix[]) const override;
    void setMatrix4f(UniformHandle, const float matrix[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const override;

    void setUniforms(const GrProgramInfo& info);

private:
    enum {
        kUnusedUniform = -1,
    };

    struct Uniform {
        GrGLint     fLocation;
#ifdef SK_DEBUG
        GrSLType    fType;
        int         fArrayCount;
#endif
    };

    template<int N> inline void setMatrices(UniformHandle, int arrayCount,
                                            const float matrices[]) const;

    SkTArray<Uniform, true> fUniforms;
    GrGLGpu* fGpu;

    class UniformManager {
    public:
        UniformManager(const GrUniformAggregator&,
                       GrGLuint programID,
                       GrGLint firstUnusedUniformID,  // used with BindUniformLocation
                       const GrGLContext& ctx);

        void setUniforms(const GrGLInterface* gl, const GrProgramInfo& info);

    private:
        struct Uniform {
            size_t   indexInProcessor = -1;
            GrSLType type             = kVoid_GrSLType;
            int      count            = 0;
            GrGLint  location         = -1;
        };
        using ProcessorUniforms = std::vector<Uniform>;
        std::vector<ProcessorUniforms> fUniforms;
    };

    UniformManager fManager;

    using INHERITED = GrGLSLProgramDataManager;
};

#endif
