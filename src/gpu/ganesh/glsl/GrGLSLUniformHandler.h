/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLUniformHandler_DEFINED
#define GrGLSLUniformHandler_DEFINED

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrResourceHandle.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"

#include <string.h>
#include <cstdint>

class GrBackendFormat;
class GrGLSLProgramBuilder;
class GrProcessor;
class GrSamplerState;
struct GrShaderCaps;

// variable names beginning with this prefix will not be mangled
#define GR_NO_MANGLE_PREFIX "sk_"

// Handles for program uniforms (other than per-effect uniforms)
struct GrGLSLBuiltinUniformHandles {
    GrGLSLProgramDataManager::UniformHandle fRTAdjustmentUni;
    // Render target flip uniform (used for dFdy, sk_Clockwise, and sk_FragCoord)
    GrGLSLProgramDataManager::UniformHandle fRTFlipUni;
    // Destination texture origin and scale, used when dest-texture readback is enabled.
    GrGLSLProgramDataManager::UniformHandle fDstTextureCoordsUni;
};

class GrGLSLUniformHandler {
public:
    struct UniformInfo {
        GrShaderVar        fVariable;
        uint32_t           fVisibility;
        const GrProcessor* fOwner;
        SkString           fRawName;
    };

    virtual ~GrGLSLUniformHandler() {}

    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;

    GR_DEFINE_RESOURCE_HANDLE_CLASS(SamplerHandle)

    /** Add a uniform variable to the current program, that has visibility in one or more shaders.
        visibility is a bitfield of GrShaderFlag values indicating from which shaders the uniform
        should be accessible. At least one bit must be set. Geometry shader uniforms are not
        supported at this time. The actual uniform name will be mangled. If outName is not nullptr
        then it will refer to the final uniform name after return. Use the addUniformArray variant
        to add an array of uniforms. */
    UniformHandle addUniform(const GrProcessor* owner,
                             uint32_t visibility,
                             SkSLType type,
                             const char* name,
                             const char** outName = nullptr) {
        SkASSERT(!SkSLTypeIsCombinedSamplerType(type));
        return this->addUniformArray(owner, visibility, type, name, 0, outName);
    }

    UniformHandle addUniformArray(const GrProcessor* owner,
                                  uint32_t visibility,
                                  SkSLType type,
                                  const char* name,
                                  int arrayCount,
                                  const char** outName = nullptr) {
        SkASSERT(!SkSLTypeIsCombinedSamplerType(type));
        bool mangle = strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX));
        return this->internalAddUniformArray(owner, visibility, type, name, mangle, arrayCount,
                                             outName);
    }

    virtual const GrShaderVar& getUniformVariable(UniformHandle u) const = 0;

    /**
     * Shortcut for getUniformVariable(u).c_str()
     */
    virtual const char* getUniformCStr(UniformHandle u) const = 0;

    virtual int numUniforms() const = 0;

    virtual UniformInfo& uniform(int idx) = 0;
    virtual const UniformInfo& uniform(int idx) const = 0;

    // Looks up a uniform that was added by 'owner' with the given 'rawName' (pre-mangling).
    // If there is no such uniform, a variable with type kVoid is returned.
    GrShaderVar getUniformMapping(const GrProcessor& owner, SkString rawName) const;

    // Like getUniformMapping(), but if the uniform is found it also marks it as accessible in
    // the vertex shader.
    GrShaderVar liftUniformToVertexShader(const GrProcessor& owner, SkString rawName);

protected:
    explicit GrGLSLUniformHandler(GrGLSLProgramBuilder* program) : fProgramBuilder(program) {}

    // This is not owned by the class
    GrGLSLProgramBuilder* fProgramBuilder;

private:
    virtual const char * samplerVariable(SamplerHandle) const = 0;
    virtual skgpu::Swizzle samplerSwizzle(SamplerHandle) const = 0;

    virtual const char* inputSamplerVariable(SamplerHandle) const {
        SkDEBUGFAIL("Trying to get input sampler from unsupported backend");
        return nullptr;
    }
    virtual skgpu::Swizzle inputSamplerSwizzle(SamplerHandle) const {
        SkDEBUGFAIL("Trying to get input sampler swizzle from unsupported backend");
        return {};
    }

    virtual SamplerHandle addSampler(const GrBackendFormat&, GrSamplerState, const skgpu::Swizzle&,
                                     const char* name, const GrShaderCaps*) = 0;

    virtual SamplerHandle addInputSampler(const skgpu::Swizzle& swizzle, const char* name) {
        SkDEBUGFAIL("Trying to add input sampler to unsupported backend");
        return {};
    }

    virtual UniformHandle internalAddUniformArray(const GrProcessor* owner,
                                                  uint32_t visibility,
                                                  SkSLType type,
                                                  const char* name,
                                                  bool mangleName,
                                                  int arrayCount,
                                                  const char** outName) = 0;

    virtual void appendUniformDecls(GrShaderFlags visibility, SkString*) const = 0;

    friend class GrGLSLProgramBuilder;
};

#endif
