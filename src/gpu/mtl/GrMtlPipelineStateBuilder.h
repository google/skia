/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"
#include "src/gpu/mtl/GrMtlVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"

#import <metal/metal.h>

class GrMtlGpu;
class GrMtlPipelineState;

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    static GrMtlPipelineState* CreatePipelineState(const GrPrimitiveProcessor&,
                                                   const GrPipeline&,
                                                   GrProgramDesc*,
                                                   GrMtlGpu*);

private:
    GrMtlPipelineStateBuilder(const GrPrimitiveProcessor&, const GrPipeline&,
                              GrProgramDesc*, GrMtlGpu*);

    const GrCaps* caps() const override;

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }

    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }

    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    id<MTLLibrary> createMtlShaderLibrary(const GrGLSLShaderBuilder& builder,
                                          SkSL::Program::Kind kind,
                                          const SkSL::Program::Settings& settings,
                                          GrProgramDesc* desc);

    GrMtlPipelineState* finalize(const GrPrimitiveProcessor&, const GrPipeline&, GrProgramDesc*);

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
