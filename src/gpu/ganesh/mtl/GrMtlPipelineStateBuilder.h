/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "include/gpu/GrContextOptions.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/mtl/GrMtlUniformHandler.h"
#include "src/gpu/ganesh/mtl/GrMtlVaryingHandler.h"
#include "src/sksl/ir/SkSLProgram.h"

#import <Metal/Metal.h>

class GrProgramDesc;
class GrProgramInfo;
class GrMtlCaps;
class GrMtlGpu;
class GrMtlPipelineState;
class SkReadBuffer;

namespace SkSL { class Compiler; }

struct GrMtlPrecompiledLibraries {
    // TODO: wrap these in sk_cfp<> or unique_ptr<> when we remove ARC
    id<MTLLibrary> fVertexLibrary;
    id<MTLLibrary> fFragmentLibrary;
    bool fRTFlip = false;
};

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
     *
     * The returned GrMtlPipelineState implements the supplied GrProgramInfo.
     *
     * @return the created pipeline if generation was successful; nullptr otherwise
     */
    static GrMtlPipelineState* CreatePipelineState(
                                       GrMtlGpu*,
                                       const GrProgramDesc&,
                                       const GrProgramInfo&,
                                       const GrMtlPrecompiledLibraries* precompiledLibs = nullptr);

    static bool PrecompileShaders(GrMtlGpu*, const SkData&,
                                  GrMtlPrecompiledLibraries* precompiledLibs);

private:
    GrMtlPipelineStateBuilder(GrMtlGpu*, const GrProgramDesc&, const GrProgramInfo&);

    GrMtlPipelineState* finalize(const GrProgramDesc&, const GrProgramInfo&,
                                 const GrMtlPrecompiledLibraries* precompiledLibraries);

    const GrCaps* caps() const override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    id<MTLLibrary> compileMtlShaderLibrary(const std::string& shader,
                                           SkSL::Program::Interface,
                                           GrContextOptions::ShaderErrorHandler* errorHandler);
    void storeShadersInCache(const std::string shaders[],
                             const SkSL::Program::Interface[],
                             SkSL::ProgramSettings*,
                             sk_sp<SkData>,
                             bool isSkSL);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    using INHERITED = GrGLSLProgramBuilder;
};
#endif
