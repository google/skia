/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

class GrGLSLCaps;
class GrGLSLShaderVar;
class GrGLSLVaryingHandler;

class GrGLSLProgramBuilder {
public:
    typedef GrGpu::DrawArgs DrawArgs;
    typedef GrGLSLUniformHandler::ShaderVisibility ShaderVisibility;
    typedef GrGLSLUniformHandler::UniformHandle UniformHandle;

    virtual ~GrGLSLProgramBuilder() {}

    virtual const GrGLSLCaps* glslCaps() const = 0;

    const GrPrimitiveProcessor& primitiveProcessor() const { return *fArgs.fPrimitiveProcessor; }
    const GrPipeline& pipeline() const { return *fArgs.fPipeline; }
    const GrProgramDesc& desc() const { return *fArgs.fDesc; }
    const GrProgramDesc::KeyHeader& header() const { return fArgs.fDesc->header(); }

    void appendUniformDecls(ShaderVisibility, SkString*) const;

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fRTAdjustmentUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;
    };

    // Used to add a uniform in the vertex shader for transforming into normalized device space.
    void addRTAdjustmentUniform(GrSLPrecision precision, const char* name, const char** outName);
    const char* rtAdjustment() const { return "rtAdjustment"; }
 
    // Used to add a uniform for the RenderTarget height (used for frag position) without mangling
    // the name of the uniform inside of a stage.
    void addRTHeightUniform(const char* name, const char** outName);

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also will mangle the name to be stage-specific unless
    // explicitly asked not to.
    void nameVariable(SkString* out, char prefix, const char* name, bool mangle = true);

    virtual GrGLSLUniformHandler* uniformHandler() = 0;
    virtual const GrGLSLUniformHandler* uniformHandler() const = 0;
    virtual GrGLSLVaryingHandler* varyingHandler() = 0;

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    GrGLSLVertexBuilder         fVS;
    GrGLSLGeometryBuilder       fGS;
    GrGLSLFragmentShaderBuilder fFS;

    int fStageIndex;

    const DrawArgs& fArgs;

    BuiltinUniformHandles fUniformHandles;

protected:
    explicit GrGLSLProgramBuilder(const DrawArgs& args);
};

#endif
