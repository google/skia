/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "GrPipeline.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/GrGLUniformHandler.h"
#include "gl/GrGLVaryingHandler.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrFragmentProcessor;
class GrGLContextInfo;
class GrGLSLShaderBuilder;
class GrGLSLCaps;

class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * @return true if generation was successful.
     */
    static GrGLProgram* CreateProgram(const DrawArgs&, GrGLGpu*);

    const GrCaps* caps() const override;
    const GrGLSLCaps* glslCaps() const override;

    GrGLGpu* gpu() const { return fGpu; }

private:
    GrGLProgramBuilder(GrGLGpu*, const DrawArgs&);

    void emitSamplers(const GrProcessor&,
                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers) override;

    bool compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds); 
    GrGLProgram* finalize();
    void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID);
    void resolveProgramResourceLocations(GrGLuint programID);
    void cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs);
    void cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs);

    // Subclasses create different programs
    GrGLProgram* createProgram(GrGLuint programID);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }


    GrGLGpu* fGpu;
    typedef GrGLSLUniformHandler::UniformHandle UniformHandle;
    SkTArray<UniformHandle> fSamplerUniforms;

    GrGLVaryingHandler        fVaryingHandler;
    GrGLUniformHandler        fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED; 
};
#endif
