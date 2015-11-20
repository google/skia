/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramDataManager_DEFINED
#define GrGLSLProgramDataManager_DEFINED

#include "SkTypes.h"

class SkMatrix;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLSLProgramDataManager : SkNoncopyable {
public:
    // Opaque handle to a resource
    class ShaderResourceHandle {
    public:
        ShaderResourceHandle(int value)
            : fValue(value) {
            SkASSERT(this->isValid());
        }

        ShaderResourceHandle()
            : fValue(kInvalid_ShaderResourceHandle) {
        }

        bool operator==(const ShaderResourceHandle& other) const { return other.fValue == fValue; }
        bool isValid() const { return kInvalid_ShaderResourceHandle != fValue; }
        int toIndex() const { SkASSERT(this->isValid()); return fValue; }

    private:
        static const int kInvalid_ShaderResourceHandle = -1;
        int fValue;
    };

    typedef ShaderResourceHandle UniformHandle;

    virtual ~GrGLSLProgramDataManager() {}

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    virtual void set1f(UniformHandle, float v0) const = 0;
    virtual void set1fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set2f(UniformHandle, float, float) const = 0;
    virtual void set2fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set3f(UniformHandle, float, float, float) const = 0;
    virtual void set3fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set4f(UniformHandle, float, float, float, float) const = 0;
    virtual void set4fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    virtual void setMatrix3f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix4f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;
    virtual void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    virtual void setSkMatrix(UniformHandle, const SkMatrix&) const = 0;

    // for nvpr only
    typedef ShaderResourceHandle VaryingHandle;
    virtual void setPathFragmentInputTransform(VaryingHandle u, int components,
                                               const SkMatrix& matrix) const = 0;

protected:
    GrGLSLProgramDataManager() {}

private:

    typedef SkNoncopyable INHERITED;
};

#endif
