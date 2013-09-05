/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUniformManager_DEFINED
#define GrGLUniformManager_DEFINED

#include "gl/GrGLShaderVar.h"
#include "gl/GrGLSL.h"
#include "GrAllocator.h"

#include "SkTArray.h"

class GrGpuGL;
class SkMatrix;

/** Manages a program's uniforms.
*/
class GrGLUniformManager {
public:
    // Opaque handle to a uniform
    class UniformHandle {
    public:
        static UniformHandle CreateFromUniformIndex(int i);

        bool isValid() const { return 0 != fValue; }

        bool operator==(const UniformHandle& other) const { return other.fValue == fValue; }

        UniformHandle()
            : fValue(0) {
        }

    private:
        UniformHandle(int value)
            : fValue(~value) {
            SkASSERT(isValid());
        }

        int toUniformIndex() const { SkASSERT(isValid()); return ~fValue; }

        int fValue;
        friend class GrGLUniformManager; // For accessing toUniformIndex().
    };

    GrGLUniformManager(GrGpuGL* gpu) : fGpu(gpu) {}

    UniformHandle appendUniform(GrSLType type, int arrayCount = GrGLShaderVar::kNonArray);

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. offset + arrayCount must be <= the array count of the uniform.
     */
    void setSampler(UniformHandle, GrGLint texUnit) const;
    void set1f(UniformHandle, GrGLfloat v0) const;
    void set1fv(UniformHandle, int offset, int arrayCount, const GrGLfloat v[]) const;
    void set2f(UniformHandle, GrGLfloat, GrGLfloat) const;
    void set2fv(UniformHandle, int offset, int arrayCount, const GrGLfloat v[]) const;
    void set3f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set3fv(UniformHandle, int offset, int arrayCount, const GrGLfloat v[]) const;
    void set4f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set4fv(UniformHandle, int offset, int arrayCount, const GrGLfloat v[]) const;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    void setMatrix3f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix4f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix3fv(UniformHandle, int offset, int arrayCount, const GrGLfloat matrices[]) const;
    void setMatrix4fv(UniformHandle, int offset, int arrayCount, const GrGLfloat matrices[]) const;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const;

    struct BuilderUniform {
        GrGLShaderVar fVariable;
        uint32_t      fVisibility;
    };
    // This uses an allocator rather than array so that the GrGLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<BuilderUniform> BuilderUniformArray;

    /**
     * Called by the GrGLShaderBuilder to get GL locations for all uniforms.
     */
    void getUniformLocations(GrGLuint programID, const BuilderUniformArray& uniforms);

    /**
     * Called by the GrGLShaderBuilder to access the array by the handle (index).
     */
    const BuilderUniform& getBuilderUniform(const BuilderUniformArray&, GrGLUniformManager::UniformHandle) const;

private:
    enum {
        kUnusedUniform = -1,
    };

    struct Uniform {
        GrGLint     fVSLocation;
        GrGLint     fFSLocation;
        GrSLType    fType;
        int         fArrayCount;
    };

    SkTArray<Uniform, true> fUniforms;
    GrGpuGL* fGpu;
};

#endif
