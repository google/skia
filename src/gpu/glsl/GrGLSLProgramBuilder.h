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
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLTextureSampler.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLXferProcessor.h"

class GrGLSLCaps;
class GrGLSLShaderVar;
class GrGLSLVaryingHandler;

typedef SkSTArray<8, GrGLSLFragmentProcessor*, true> GrGLSLFragProcs;

class GrGLSLProgramBuilder {
public:
    typedef GrGpu::DrawArgs DrawArgs;
    typedef GrGLSLUniformHandler::ShaderVisibility ShaderVisibility;
    typedef GrGLSLUniformHandler::UniformHandle UniformHandle;

    virtual ~GrGLSLProgramBuilder() {}

    virtual const GrCaps* caps() const = 0;
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

    GrGLSLPrimitiveProcessor* fGeometryProcessor;
    GrGLSLXferProcessor* fXferProcessor;
    GrGLSLFragProcs fFragmentProcessors;

protected:
    explicit GrGLSLProgramBuilder(const DrawArgs& args);

    bool emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage, int maxTextures);

    void cleanupFragmentProcessors();

private:
    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->addStage();
        fFS.reset();
    }
    void addStage() { fStageIndex++; }

    class AutoStageAdvance {
    public:
        AutoStageAdvance(GrGLSLProgramBuilder* pb)
            : fPB(pb) {
            fPB->reset();
            // Each output to the fragment processor gets its own code section
            fPB->fFS.nextStage();
        }
        ~AutoStageAdvance() {}
    private:
        GrGLSLProgramBuilder* fPB;
    };

    // Generates a possibly mangled name for a stage variable and writes it to the fragment shader.
    // If GrGLSLExpr4 has a valid name then it will use that instead
    void nameExpression(GrGLSLExpr4*, const char* baseName);

    void emitAndInstallPrimProc(const GrPrimitiveProcessor&,
                                GrGLSLExpr4* outputColor,
                                GrGLSLExpr4* outputCoverage);
    void emitAndInstallFragProcs(int procOffset, int numProcs, GrGLSLExpr4* inOut);
    void emitAndInstallFragProc(const GrFragmentProcessor&,
                                int index,
                                const GrGLSLExpr4& input,
                                GrGLSLExpr4* output);
    void emitAndInstallXferProc(const GrXferProcessor&,
                                const GrGLSLExpr4& colorIn,
                                const GrGLSLExpr4& coverageIn,
                                bool ignoresCoverage);
    void emitFSOutputSwizzle(bool hasSecondaryOutput);

    void verify(const GrPrimitiveProcessor&);
    void verify(const GrXferProcessor&);
    void verify(const GrFragmentProcessor&);

    virtual void emitSamplers(const GrProcessor& processor,
                              GrGLSLTextureSampler::TextureSamplerArray* outSamplers) = 0;

    GrGLSLPrimitiveProcessor::TransformsIn  fCoordTransforms;
    GrGLSLPrimitiveProcessor::TransformsOut fOutCoords;
};

#endif
