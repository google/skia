/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "gl/GrGLShaderVar.h"
#include "gl/GrGLSL.h"
#include "GrAllocator.h"

#include "SkTArray.h"

class GrGpuGL;
class SkMatrix;
class GrGLProgram;
class GrGLShaderBuilder;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : public SkRefCnt {
public:
    // Opaque handle to a uniform
    class UniformHandle {
    public:
        /** Creates a reference to an unifrom of a GrGLShaderBuilder.
         * The ref can be used to set the uniform with corresponding the GrGLProgramDataManager.*/
        static UniformHandle CreateFromUniformIndex(int i);

        bool isValid() const { return -1 != fValue; }

        bool operator==(const UniformHandle& other) const { return other.fValue == fValue; }

        UniformHandle()
            : fValue(-1) {
        }

    private:
        UniformHandle(int value)
            : fValue(value) {
            SkASSERT(isValid());
        }

        int toProgramDataIndex() const { SkASSERT(isValid()); return fValue; }
        int toShaderBuilderIndex() const { return toProgramDataIndex(); }

        int fValue;
        friend class GrGLProgramDataManager; // For accessing toProgramDataIndex().
        friend class GrGLShaderBuilder; // For accessing toShaderBuilderIndex().
    };

    GrGLProgramDataManager(GrGpuGL*, GrGLProgram*, const GrGLShaderBuilder&);

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

    SkTArray<Uniform, true> fUniforms;
    GrGpuGL* fGpu;

    typedef SkRefCnt INHERITED;
};

#endif
