/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DUniformHandler_DEFINED
#define GrD3DUniformHandler_DEFINED

#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/GrTAllocator.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrD3DUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    ~GrD3DUniformHandler() override;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return dummyVar;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }


    int numUniforms() const override {
        return 0;
    }

    UniformInfo& uniform(int idx) override {
        return dummyInfo;
    }

    /**
    * Returns the offset that the RTHeight synthetic uniform should use if it needs to be created.
    */
    uint32_t getRTHeightOffset() const {
        // TODO: figure this out
        return 0;
    }

private:
    UniformInfo dummyInfo;
    GrShaderVar dummyVar;

    explicit GrD3DUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program) {
    }

    UniformHandle internalAddUniformArray(const GrFragmentProcessor* owner,
                                          uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle addSampler(const GrBackendFormat&,
                             GrSamplerState,
                             const GrSwizzle&,
                             const char* name,
                             const GrShaderCaps*) override;

    const char* samplerVariable(SamplerHandle handle) const override {
        return "";
    }
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override {
        return GrSwizzle("rgba");
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    friend class GrD3DPipelineStateBuilder;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
