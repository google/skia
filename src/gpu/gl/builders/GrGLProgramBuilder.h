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
struct GrShaderCaps;

struct GrGLPrecompiledProgram {
    GrGLPrecompiledProgram(GrGLuint programID = 0,
                           SkSL::Program::Inputs inputs = SkSL::Program::Inputs())
        : fProgramID(programID)
        , fInputs(inputs) {}

    GrGLuint fProgramID;
    SkSL::Program::Inputs fInputs;
};

class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * If a GL program has already been created, the program ID and inputs can
     * be supplied to skip the shader compilation.
     * @return the created program if generation was successful.
     */
    static sk_sp<GrGLProgram> CreateProgram(GrDirectContext*,
                                            const GrProgramDesc&,
                                            const GrProgramInfo&,
                                            const GrGLPrecompiledProgram* = nullptr);

    static bool PrecompileProgram(GrDirectContext*, GrGLPrecompiledProgram*, const SkData&);

    const GrCaps* caps() const override;

    GrGLGpu* gpu() const { return fGpu; }

    SkSL::Compiler* shaderCompiler() const override;

private:
    GrGLProgramBuilder(GrGLGpu*, const GrProgramDesc&, const GrProgramInfo&);

    void addInputVars(const SkSL::Program::Inputs& inputs);
    bool compileAndAttachShaders(const SkSL::String& glsl,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 GrContextOptions::ShaderErrorHandler* errorHandler);

    void computeCountsAndStrides(GrGLuint programID,
                                 const GrGeometryProcessor&,
                                 bool bindAttribLocations);
    void storeShaderInCache(const SkSL::Program::Inputs& inputs, GrGLuint programID,
                            const SkSL::String shaders[], bool isSkSL,
                            SkSL::Program::Settings* settings);
    sk_sp<GrGLProgram> finalize(const GrGLPrecompiledProgram*);
    void bindProgramResourceLocations(GrGLuint programID);
    bool checkLinkStatus(GrGLuint programID, GrContextOptions::ShaderErrorHandler* errorHandler,
                         SkSL::String* sksl[], const SkSL::String glsl[]);
    void resolveProgramResourceLocations(GrGLuint programID, bool force);

    // Subclasses create different programs
    sk_sp<GrGLProgram> createProgram(GrGLuint programID);

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

    using INHERITED = GrGLSLProgramBuilder;
};
#endif
