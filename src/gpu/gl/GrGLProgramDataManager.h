/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "GrAllocator.h"
#include "GrShaderVar.h"
#include "gl/GrGLTypes.h"
#include "glsl/GrGLSLProgramDataManager.h"

#include "SkTArray.h"

class GrGLGpu;
class SkMatrix;
class GrGLProgram;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : public GrGLSLProgramDataManager {
public:
    struct UniformInfo {
        GrShaderVar fVariable;
        uint32_t        fVisibility;
        GrGLint         fLocation;
    };

    struct VaryingInfo {
        GrShaderVar fVariable;
        GrGLint         fLocation;
    };

    // This uses an allocator rather than array so that the GrShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<UniformInfo> UniformInfoArray;
    typedef GrTAllocator<VaryingInfo> VaryingInfoArray;

    GrGLProgramDataManager(GrGLGpu*, GrGLuint programID, const UniformInfoArray&,
                           const VaryingInfoArray&);


    void setSamplers(const UniformInfoArray& samplers) const;
    void setImageStorages(const UniformInfoArray &images) const;

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
    *  array of uniforms. arrayCount must be <= the array count of the uniform.
    */
    void set1i(UniformHandle, int32_t) const override;
    void set1iv(UniformHandle, int arrayCount, const int v[]) const override;
    void set1f(UniformHandle, float v0) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2f(UniformHandle, float, float) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3f(UniformHandle, float, float, float) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
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

    // for nvpr only
    void setPathFragmentInputTransform(VaryingHandle u, int components,
                                       const SkMatrix& matrix) const override;

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

    enum {
        kUnusedPathProcVarying = -1,
    };
    struct PathProcVarying {
        GrGLint     fLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    template<int N> inline void setMatrices(UniformHandle, int arrayCount,
                                            const float matrices[]) const;

    SkTArray<Uniform, true> fUniforms;
    SkTArray<PathProcVarying, true> fPathProcVaryings;
    GrGLGpu* fGpu;
    GrGLuint fProgramID;

    typedef GrGLSLProgramDataManager INHERITED;
};

#endif
