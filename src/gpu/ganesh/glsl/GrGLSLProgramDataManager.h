/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramDataManager_DEFINED
#define GrGLSLProgramDataManager_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/gpu/ganesh/GrResourceHandle.h"

#include <cstdint>

class SkM44;
class SkMatrix;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLSLProgramDataManager {
public:
    GR_DEFINE_RESOURCE_HANDLE_CLASS(UniformHandle)

    virtual ~GrGLSLProgramDataManager() {}

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    virtual void set1i(UniformHandle, int32_t) const = 0;
    virtual void set1iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set1f(UniformHandle, float v0) const = 0;
    virtual void set1fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set2i(UniformHandle, int32_t, int32_t) const = 0;
    virtual void set2iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set2f(UniformHandle, float, float) const = 0;
    virtual void set2fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set3i(UniformHandle, int32_t, int32_t, int32_t) const = 0;
    virtual void set3iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set3f(UniformHandle, float, float, float) const = 0;
    virtual void set3fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    virtual void set4i(UniformHandle, int32_t, int32_t, int32_t, int32_t) const = 0;
    virtual void set4iv(UniformHandle, int arrayCount, const int v[]) const = 0;
    virtual void set4f(UniformHandle, float, float, float, float) const = 0;
    virtual void set4fv(UniformHandle, int arrayCount, const float v[]) const = 0;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    virtual void setMatrix2f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix3f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix4f(UniformHandle, const float matrix[]) const = 0;
    virtual void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;
    virtual void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;
    virtual void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const = 0;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const;
    // convenience method for uploading a SkMatrix to a 4x4 matrix uniform
    void setSkM44(UniformHandle, const SkM44&) const;

    enum class Specialized : bool { kNo = false, kYes = true };

    /**
     * Sets runtime effect uniform values. The Specialized span is optional. If it is non-empty
     * then it should be the same size as the Uniform span. Uniforms for which the Specialized value
     * is kYes will be skipped and the UniformHandle span should have an entry for each
     * kNo value. If Specialized is empty then the Uniform and UniformHandle spans should have the
     * same size.
     */
    void setRuntimeEffectUniforms(SkSpan<const SkRuntimeEffect::Uniform>,
                                  SkSpan<const UniformHandle>,
                                  SkSpan<const Specialized>,
                                  const void* src) const;

    void setRuntimeEffectUniforms(SkSpan<const SkRuntimeEffect::Uniform> uniforms,
                                  SkSpan<const UniformHandle>            handles,
                                  const void*                            src) const {
        this->setRuntimeEffectUniforms(uniforms, handles, {}, src);
    }

protected:
    GrGLSLProgramDataManager() {}

private:
    GrGLSLProgramDataManager(const GrGLSLProgramDataManager&) = delete;
    GrGLSLProgramDataManager& operator=(const GrGLSLProgramDataManager&) = delete;

    using INHERITED = SkNoncopyable;
};

#endif
