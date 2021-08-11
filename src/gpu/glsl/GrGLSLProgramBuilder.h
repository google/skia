/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/sksl/SkSLCompiler.h"

#include <vector>

class GrProgramDesc;
class GrRenderTarget;
class GrShaderVar;
class GrGLSLVaryingHandler;
class SkString;
class GrShaderCaps;

class GrGLSLProgramBuilder {
public:
    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;

    virtual ~GrGLSLProgramBuilder();

    virtual const GrCaps* caps() const = 0;
    const GrShaderCaps* shaderCaps() const { return this->caps()->shaderCaps(); }

    GrSurfaceOrigin origin() const { return fProgramInfo.origin(); }
    const GrPipeline& pipeline() const { return fProgramInfo.pipeline(); }
    const GrGeometryProcessor& geometryProcessor() const { return fProgramInfo.geomProc(); }
    GrProcessor::CustomFeatures processorFeatures() const {
        return fProgramInfo.requestedFeatures();
    }
    bool snapVerticesToPixelCenters() const {
        return fProgramInfo.pipeline().snapVerticesToPixelCenters();
    }
    bool hasPointSize() const { return fProgramInfo.primitiveType() == GrPrimitiveType::kPoints; }
    virtual SkSL::Compiler* shaderCompiler() const = 0;

    const GrProgramDesc& desc() const { return fDesc; }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const;

    const char* samplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->samplerVariable(handle);
    }

    GrSwizzle samplerSwizzle(SamplerHandle handle) const {
        return this->uniformHandler()->samplerSwizzle(handle);
    }

    const char* inputSamplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->inputSamplerVariable(handle);
    }

    GrSwizzle inputSamplerSwizzle(SamplerHandle handle) const {
        return this->uniformHandler()->inputSamplerSwizzle(handle);
    }

    // Used to add a uniform for render target flip (used for dFdy, sk_Clockwise, and sk_FragCoord)
    // without mangling the name of the uniform inside of a stage.
    void addRTFlipUniform(const char* name);

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also will mangle the name to be stage-specific unless
    // explicitly asked not to. `nameVariable` can also be used to generate names for functions or
    // other types of symbols where unique names are important.
    SkString nameVariable(char prefix, const char* name, bool mangle = true);

    /**
     * If computation of a FP's input coords have been lifted to the vertex shader, this method
     * retrieves the name of the coords varying in the FS. The FP code should read this directly
     * rather than writing a function that takes a float2 coord.
     */
    GrShaderVar varyingCoordsForFragmentProcessor(const GrFragmentProcessor*);

    /**
     * If the FP's coords are unused or all uses have been lifted to interpolated varyings then
     * don't put coords in the FP's function signature or call sites.
     */
    bool fragmentProcessorHasCoordsParam(const GrFragmentProcessor*);

    virtual GrGLSLUniformHandler* uniformHandler() = 0;
    virtual const GrGLSLUniformHandler* uniformHandler() const = 0;
    virtual GrGLSLVaryingHandler* varyingHandler() = 0;

    // Used for backend customization of the output color and secondary color variables from the
    // fragment processor. Only used if the outputs are explicitly declared in the shaders
    virtual void finalizeFragmentOutputColor(GrShaderVar& outputColor) {}
    virtual void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {}

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    GrGLSLVertexBuilder          fVS;
    GrGLSLGeometryBuilder        fGS;
    GrGLSLFragmentShaderBuilder  fFS;

    int fStageIndex;

    const GrProgramDesc&         fDesc;
    const GrProgramInfo&         fProgramInfo;

    GrGLSLBuiltinUniformHandles  fUniformHandles;

    std::unique_ptr<GrGeometryProcessor::ProgramImpl>               fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl>                   fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>>  fFPImpls;

    SamplerHandle fDstTextureSamplerHandle;
    GrSurfaceOrigin fDstTextureOrigin;

protected:
    explicit GrGLSLProgramBuilder(const GrProgramDesc&, const GrProgramInfo&);

    void addFeature(GrShaderFlags shaders, uint32_t featureBit, const char* extensionName);

    bool emitAndInstallProcs();

    void finalizeShaders();

    bool fragColorIsInOut() const { return fFS.primaryColorOutputIsInOut(); }

private:
    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->addStage();
        SkDEBUGCODE(fFS.debugOnly_resetPerStageVerification();)
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
    void nameExpression(SkString*, const char* baseName);

    bool emitAndInstallPrimProc(SkString* outputColor, SkString* outputCoverage);
    bool emitAndInstallDstTexture();
    bool emitAndInstallFragProcs(SkString* colorInOut, SkString* coverageInOut);
    SkString emitFragProc(const GrFragmentProcessor&,
                          GrFragmentProcessor::ProgramImpl&,
                          const SkString& input,
                          SkString output);
    bool emitAndInstallXferProc(const SkString& colorIn, const SkString& coverageIn);
    SamplerHandle emitSampler(const GrBackendFormat&, GrSamplerState, const GrSwizzle&,
                              const char* name);
    SamplerHandle emitInputSampler(const GrSwizzle& swizzle, const char* name);
    bool checkSamplerCounts();

#ifdef SK_DEBUG
    void verify(const GrGeometryProcessor&);
    void verify(const GrFragmentProcessor&);
    void verify(const GrXferProcessor&);
#endif

    // These are used to check that we don't excede the allowable number of resources in a shader.
    int fNumFragmentSamplers;
    GrGeometryProcessor::ProgramImpl::FPCoordsMap fFPCoordsMap;
};

#endif
