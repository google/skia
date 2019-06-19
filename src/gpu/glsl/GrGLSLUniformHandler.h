/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLUniformHandler_DEFINED
#define GrGLSLUniformHandler_DEFINED

#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

// variable names beginning with this prefix will not be mangled
#define GR_NO_MANGLE_PREFIX "sk_"

class GrGLSLProgramBuilder;
class GrSamplerState;
class GrTexture;

// Handles for program uniforms (other than per-effect uniforms)
struct GrGLSLBuiltinUniformHandles {
    GrGLSLProgramDataManager::UniformHandle fRTAdjustmentUni;
    // Render target width, used to implement sk_Width
    GrGLSLProgramDataManager::UniformHandle fRTWidthUni;
    // Render target height, used to implement sk_Height and to calculate sk_FragCoord when
    // origin_upper_left is not supported.
    GrGLSLProgramDataManager::UniformHandle fRTHeightUni;
};

class GrGLSLUniformHandler {
public:
    virtual ~GrGLSLUniformHandler() {}

    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;

    GR_DEFINE_RESOURCE_HANDLE_CLASS(SamplerHandle);

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of GrShaderFlag values indicating from which shaders the uniform
        should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not nullptr
        then it will refer to the final uniform name after return. Use the addUniformArray variant
        to add an array of uniforms. */
    UniformHandle addUniform(uint32_t visibility,
                             GrSLType type,
                             const char* name,
                             const char** outName = nullptr) {
        SkASSERT(!GrSLTypeIsCombinedSamplerType(type));
        return this->addUniformArray(visibility, type, name, 0, outName);
    }

    UniformHandle addUniformArray(uint32_t visibility,
                                  GrSLType type,
                                  const char* name,
                                  int arrayCount,
                                  const char** outName = nullptr) {
        SkASSERT(!GrSLTypeIsCombinedSamplerType(type));
        bool mangle = strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX));
        return this->internalAddUniformArray(visibility, type, name, mangle, arrayCount, outName);
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
    virtual const char * samplerVariable(SamplerHandle) const = 0;
    // Only called if GrShaderCaps(:textureSwizzleAppliedInShader() == true.
    virtual GrSwizzle samplerSwizzle(SamplerHandle) const = 0;

    virtual SamplerHandle addSampler(const GrTexture*, const GrSamplerState&, const char* name,
                                     const GrShaderCaps*) = 0;

    virtual UniformHandle internalAddUniformArray(uint32_t visibility,
                                                  GrSLType type,
                                                  const char* name,
                                                  bool mangleName,
                                                  int arrayCount,
                                                  const char** outName) = 0;

    virtual void appendUniformDecls(GrShaderFlags visibility, SkString*) const = 0;

    friend class GrGLSLProgramBuilder;
};

#endif
