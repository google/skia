/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramBuilder_DEFINED
#define GrGLProgramBuilder_DEFINED

#include "include/gpu/GrContextOptions.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/gl/GrGLProgram.h"
#include "src/gpu/ganesh/gl/GrGLProgramDataManager.h"
#include "src/gpu/ganesh/gl/GrGLUniformHandler.h"
#include "src/gpu/ganesh/gl/GrGLVaryingHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/sksl/ir/SkSLProgram.h"

class GrFragmentProcessor;
class GrGLContextInfo;
class GrProgramDesc;
class GrGLSLShaderBuilder;
struct GrShaderCaps;

struct GrGLPrecompiledProgram {
    GrGLPrecompiledProgram(GrGLuint programID = 0,
                           SkSL::Program::Interface intf = SkSL::Program::Interface())
            : fProgramID(programID), fInterface(intf) {}

    GrGLuint fProgramID;
    SkSL::Program::Interface fInterface;
};

class GrGLProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
     *
     * The program implements what is specified in the stages given as input.
     * After successful generation, the builder result objects are available
     * to be used.
     * If a GL program has already been created, the program ID and interface can
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

private:
    GrGLProgramBuilder(GrGLGpu*, const GrProgramDesc&, const GrProgramInfo&);

    void addInputVars(const SkSL::Program::Interface&);
    bool compileAndAttachShaders(const std::string& glsl,
                                 GrGLuint programId,
                                 GrGLenum type,
                                 SkTDArray<GrGLuint>* shaderIds,
                                 bool shaderWasCached,
                                 GrContextOptions::ShaderErrorHandler* errorHandler);

    void computeCountsAndStrides(GrGLuint programID,
                                 const GrGeometryProcessor&,
                                 bool bindAttribLocations);
    void storeShaderInCache(const SkSL::Program::Interface&,
                            GrGLuint programID,
                            const std::string shaders[],
                            bool isSkSL,
                            SkSL::ProgramSettings* settings);
    sk_sp<GrGLProgram> finalize(const GrGLPrecompiledProgram*);
    void bindProgramResourceLocations(GrGLuint programID);
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
    // SkSL::Program::Interface interface
    // int binaryFormat
    // (all remaining bytes) char[] binary
    sk_sp<SkData> fCached;

    using INHERITED = GrGLSLProgramBuilder;
};
#endif
