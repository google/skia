/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "src/gpu/GrPipeline.h"
#include "src/gpu/gl/GrGLProgram.h"
#include "src/gpu/gl/GrGLProgramDataManager.h"
#include "src/gpu/gl/GrGLUniformHandler.h"
#include "src/gpu/gl/GrGLVaryingHandler.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/sksl/ir/SkSLProgram.h"

class GrFragmentProcessor;
class GrGLContextInfo;
class GrProgramDesc;
class GrGLSLShaderBuilder;
class GrShaderCaps;

class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * This function may modify the GrProgramDesc by setting the surface origin
     * key to 0 (unspecified) if it turns out the program does not care about
     * the surface origin.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(GrRenderTarget*, GrSurfaceOrigin,
                                      const GrPrimitiveProcessor&,
                                      const GrTextureProxy* const primProcProxies[],
                                      const GrPipeline&,
                                      GrProgramDesc*,
                                      GrGLGpu*);

    const GrCaps* caps() const override;

    GrGLGpu* gpu() const { return fGpu; }

private:
    GrGLProgramBuilder(GrGLGpu*, GrRenderTarget*, GrSurfaceOrigin,
                       const GrPipeline&, const GrPrimitiveProcessor&,
                       const GrTextureProxy* const primProcProxies[], GrProgramDesc*);

    void addInputVars(const SkSL::Program::Inputs& inputs);
    bool compileAndAttachShaders(const SkSL::String& glsl,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 const SkSL::Program::Inputs& inputs,
                                 GrContextOptions::ShaderErrorHandler* errorHandler);

    void computeCountsAndStrides(GrGLuint programID, const GrPrimitiveProcessor& primProc,
                                 bool bindAttribLocations);
    void storeShaderInCache(const SkSL::Program::Inputs& inputs, GrGLuint programID,
                            const SkSL::String shaders[], bool isSkSL);
    GrGLProgram* finalize();
    void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID, GrContextOptions::ShaderErrorHandler* errorHandler,
                         SkSL::String* sksl[], const SkSL::String glsl[]);
    void resolveProgramResourceLocations(GrGLuint programID, bool force);
    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    GrGLProgram* createProgram(GrGLuint programID);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrGLGpu*              fGpu;
    GrGLVaryingHandler    fVaryingHandler;
    GrGLUniformHandler    fUniformHandler;

    std::unique_ptr<GrGLProgram::Attribute[]> fAttributes;
    int fVertexAttributeCnt;
    int fInstanceAttributeCnt;
    size_t fVertexStride;
    size_t fInstanceStride;

    // shader pulled from cache. Data is organized as:
    // SkSL::Program::Inputs inputs
    // int binaryFormat
    // (all remaining bytes) char[] binary
    sk_sp<SkData> fCached;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
