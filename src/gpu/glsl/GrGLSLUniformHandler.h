/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLUniformHandler_DEFINED
#define GrGLSLUniformHandler_DEFINED

#include "GrGLSLProgramDataManager.h"
#include "GrShaderVar.h"
#include "GrSwizzle.h"

class GrGLSLProgramBuilder;

class GrGLSLUniformHandler {
public:
    virtual ~GrGLSLUniformHandler() {}

    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;
    GR_DEFINE_RESOURCE_HANDLE_CLASS(SamplerHandle);
    GR_DEFINE_RESOURCE_HANDLE_CLASS(TexelBufferHandle);
    GR_DEFINE_RESOURCE_HANDLE_CLASS(ImageStorageHandle);

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of GrShaderFlag values indicating from which shaders the uniform
        should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not nullptr
        then it will refer to the final uniform name after return. Use the addUniformArray variant
        to add an array of uniforms. */
    UniformHandle addUniform(uint32_t visibility,
                             GrSLType type,
                             GrSLPrecision precision,
                             const char* name,
                             const char** outName = nullptr) {
        SkASSERT(!GrSLTypeIsCombinedSamplerType(type));
        return this->addUniformArray(visibility, type, precision, name, 0, outName);
    }

    UniformHandle addUniformArray(uint32_t visibility,
                                  GrSLType type,
                                  GrSLPrecision precision,
                                  const char* name,
                                  int arrayCount,
                                  const char** outName = nullptr) {
        SkASSERT(!GrSLTypeIsCombinedSamplerType(type));
        return this->internalAddUniformArray(visibility, type, precision, name, true, arrayCount,
                                             outName);
    }

    virtual const GrShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

protected:
    explicit GrGLSLUniformHandler(GrGLSLProgramBuilder* program) : fProgramBuilder(program) {}

    // This is not owned by the class
    GrGLSLProgramBuilder* fProgramBuilder;

private:
    virtual const GrShaderVar& samplerVariable(SamplerHandle) const = 0;
    virtual GrSwizzle samplerSwizzle(SamplerHandle) const = 0;

    virtual SamplerHandle addSampler(uint32_t visibility, GrSwizzle, GrSLType, GrSLPrecision,
                                     const char* name) = 0;

    virtual const GrShaderVar& texelBufferVariable(TexelBufferHandle) const = 0;
    virtual TexelBufferHandle addTexelBuffer(uint32_t visibility, GrSLPrecision,
                                             const char* name) = 0;

    virtual const GrShaderVar& imageStorageVariable(ImageStorageHandle) const = 0;
    virtual ImageStorageHandle addImageStorage(uint32_t visibility, GrSLType type,
                                               GrImageStorageFormat, GrSLMemoryModel, GrSLRestrict,
                                               GrIOType, const char* name) = 0;

    virtual UniformHandle internalAddUniformArray(uint32_t visibility,
                                                  GrSLType type,
                                                  GrSLPrecision precision,
                                                  const char* name,
                                                  bool mangleName,
                                                  int arrayCount,
                                                  const char** outName) = 0;

    virtual void appendUniformDecls(GrShaderFlags visibility, SkString*) const = 0;

    friend class GrGLSLProgramBuilder;
};

#endif
