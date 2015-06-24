/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "gl/GrGLShaderVar.h"
#include "GrAllocator.h"

#include "SkTArray.h"

class GrGLGpu;
class SkMatrix;
class GrGLProgram;
class GrGLProgramBuilder;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : SkNoncopyable {
public:
    // Opaque handle to a uniform
    class ShaderResourceHandle {
    public:
        bool isValid() const { return -1 != fValue; }
        ShaderResourceHandle()
            : fValue(-1) {
        }
    protected:
        ShaderResourceHandle(int value)
            : fValue(value) {
            SkASSERT(isValid());
        }
        int fValue;
    };

    class UniformHandle : public ShaderResourceHandle {
    public:
        /** Creates a reference to an unifrom of a GrGLShaderBuilder.
         * The ref can be used to set the uniform with corresponding the GrGLProgramDataManager.*/
        static UniformHandle CreateFromUniformIndex(int i);
        UniformHandle() { }
        bool operator==(const UniformHandle& other) const { return other.fValue == fValue; }
    private:
        UniformHandle(int value) : ShaderResourceHandle(value) { }
        int toProgramDataIndex() const { SkASSERT(isValid()); return fValue; }
        int toShaderBuilderIndex() const { return toProgramDataIndex(); }

        friend class GrGLProgramDataManager; // For accessing toProgramDataIndex().
        friend class GrGLProgramBuilder; // For accessing toShaderBuilderIndex().
        friend class GrGLGeometryProcessor;
    };

    struct UniformInfo {
        GrGLShaderVar fVariable;
        uint32_t      fVisibility;
        GrGLint       fLocation;
    };

    // This uses an allocator rather than array so that the GrGLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<UniformInfo> UniformInfoArray;

    GrGLProgramDataManager(GrGLGpu*, const UniformInfoArray&);

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    void setSampler(UniformHandle, GrGLint texUnit) const;
    void set1f(UniformHandle, GrGLfloat v0) const;
    void set1fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set2f(UniformHandle, GrGLfloat, GrGLfloat) const;
    void set2fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set3f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set3fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    void set4f(UniformHandle, GrGLfloat, GrGLfloat, GrGLfloat, GrGLfloat) const;
    void set4fv(UniformHandle, int arrayCount, const GrGLfloat v[]) const;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    void setMatrix3f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix4f(UniformHandle, const GrGLfloat matrix[]) const;
    void setMatrix3fv(UniformHandle, int arrayCount, const GrGLfloat matrices[]) const;
    void setMatrix4fv(UniformHandle, int arrayCount, const GrGLfloat matrices[]) const;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const;

private:
    enum {
        kUnusedUniform = -1,
    };

    struct Uniform {
        GrGLint     fVSLocation;
        GrGLint     fFSLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    SkDEBUGCODE(void printUnused(const Uniform&) const;)

    SkTArray<Uniform, true> fUniforms;
    GrGLGpu* fGpu;

    typedef SkNoncopyable INHERITED;
};

#endif
