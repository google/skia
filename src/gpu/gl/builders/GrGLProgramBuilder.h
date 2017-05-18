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
#include "ir/SkSLProgram.h"

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
    static GrGLProgram* CreateProgram(const GrPipeline&,
                                      const GrPrimitiveProcessor&,
                                      GrProgramDesc*,
                                      GrGLGpu*);

    const GrCaps* caps() const override;

    GrGLGpu* gpu() const { return fGpu; }

private:
    GrGLProgramBuilder(GrGLGpu*, const GrPipeline&, const GrPrimitiveProcessor&,
                       GrProgramDesc*);

    bool compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 const SkSL::Program::Settings& settings,
                                 SkSL::Program::Inputs* outInputs);
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


    GrGLGpu*              fGpu;
    GrGLVaryingHandler    fVaryingHandler;
    GrGLUniformHandler    fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
