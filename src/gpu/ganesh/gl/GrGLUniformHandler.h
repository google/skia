/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUniformHandler_DEFINED
#define GrGLUniformHandler_DEFINED

#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/gl/GrGLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <cstdint>

class GrBackendFormat;
class GrGLCaps;
class GrGLGpu;
class GrGLSLProgramBuilder;
class GrProcessor;
class SkString;
enum class SkSLType : char;
struct GrShaderCaps;

class GrGLUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms.item(u.toIndex()).fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    int numUniforms() const override {
        return fUniforms.count();
    }

    UniformInfo& uniform(int idx) override {
        return fUniforms.item(idx);
    }
    const UniformInfo& uniform(int idx) const override {
        return fUniforms.item(idx);
    }

private:
    explicit GrGLUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock) {}

    UniformHandle internalAddUniformArray(const GrProcessor* owner,
                                          uint32_t visibility,
                                          SkSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle addSampler(const GrBackendFormat&, GrSamplerState, const skgpu::Swizzle&,
                             const char* name, const GrShaderCaps*) override;

    const char* samplerVariable(SamplerHandle handle) const override {
        return fSamplers.item(handle.toIndex()).fVariable.c_str();
    }

    skgpu::Swizzle samplerSwizzle(SamplerHandle handle) const override {
        return fSamplerSwizzles[handle.toIndex()];
    }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;

    // Manually set uniform locations for all our uniforms.
    void bindUniformLocations(GrGLuint programID, const GrGLCaps& caps);

    // Updates the loction of the Uniforms if we cannot bind uniform locations manually
    void getUniformLocations(GrGLuint programID, const GrGLCaps& caps, bool force);

    const GrGLGpu* glGpu() const;

    typedef GrGLProgramDataManager::GLUniformInfo GLUniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    UniformInfoArray         fUniforms;
    UniformInfoArray         fSamplers;
    skia_private::TArray<skgpu::Swizzle> fSamplerSwizzles;

    friend class GrGLProgramBuilder;

    using INHERITED = GrGLSLUniformHandler;
};

#endif
