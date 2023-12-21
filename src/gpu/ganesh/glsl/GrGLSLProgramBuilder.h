/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/sksl/SkSLCompiler.h"

#include <vector>

class GrProgramDesc;
class GrRenderTarget;
class GrShaderVar;
class GrGLSLVaryingHandler;
class SkString;
struct GrShaderCaps;

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
    bool snapVerticesToPixelCenters() const {
        return fProgramInfo.pipeline().snapVerticesToPixelCenters();
    }
    bool hasPointSize() const { return fProgramInfo.primitiveType() == GrPrimitiveType::kPoints; }

    const GrProgramDesc& desc() const { return fDesc; }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const;

    const char* samplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->samplerVariable(handle);
    }

    skgpu::Swizzle samplerSwizzle(SamplerHandle handle) const {
        return this->uniformHandler()->samplerSwizzle(handle);
    }

    const char* inputSamplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->inputSamplerVariable(handle);
    }

    skgpu::Swizzle inputSamplerSwizzle(SamplerHandle handle) const {
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
     * Emits samplers for TextureEffect fragment processors as needed. `fp` can be a TextureEffect,
     * or a tree containing zero or more TextureEffects.
     */
    bool emitTextureSamplersForFPs(const GrFragmentProcessor& fp,
                                   GrFragmentProcessor::ProgramImpl& impl,
                                   int* samplerIndex);

    /**
     * advanceStage is called by program creator between each processor's emit code.  It increments
     * the stage index for variable name mangling, and also ensures verification variables in the
     * fragment shader are cleared.
     */
    void advanceStage() {
        fStageIndex++;
        SkDEBUGCODE(fFS.debugOnly_resetPerStageVerification();)
        fFS.nextStage();
    }

    /** Adds the SkSL function that implements an FP assuming its children are already written. */
    void writeFPFunction(const GrFragmentProcessor& fp, GrFragmentProcessor::ProgramImpl& impl);

    /**
     * Returns a function-call invocation of `fp` in string form, passing the appropriate
     * combination of `inputColor`, `destColor` and `fLocalCoordsVar` for the FP.
     */
    std::string invokeFP(const GrFragmentProcessor& fp,
                         const GrFragmentProcessor::ProgramImpl& impl,
                         const char* inputColor,
                         const char* destColor,
                         const char* coords) const;
    /**
     * If the FP's coords are unused or all uses have been lifted to interpolated varyings then
     * don't put coords in the FP's function signature or call sites.
     */
    bool fragmentProcessorHasCoordsParam(const GrFragmentProcessor*) const;

    virtual GrGLSLUniformHandler* uniformHandler() = 0;
    virtual const GrGLSLUniformHandler* uniformHandler() const = 0;
    virtual GrGLSLVaryingHandler* varyingHandler() = 0;

    // Used for backend customization of the secondary color variable from the fragment processor.
    // Only used if the output is explicitly declared in the shaders.
    virtual void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {}

    // number of each input/output type in a single allocation block, used by many builders
    static const int kVarsPerBlock;

    GrGLSLVertexBuilder          fVS;
    GrGLSLFragmentShaderBuilder  fFS;

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
    SkString getMangleSuffix() const;

    // Generates a possibly mangled name for a stage variable and writes it to the fragment shader.
    void nameExpression(SkString*, const char* baseName);

    bool emitAndInstallPrimProc(SkString* outputColor, SkString* outputCoverage);
    bool emitAndInstallDstTexture();
    /** Adds the root FPs */
    bool emitAndInstallFragProcs(SkString* colorInOut, SkString* coverageInOut);
    /** Adds a single root FP tree. */
    SkString emitRootFragProc(const GrFragmentProcessor& fp,
                              GrFragmentProcessor::ProgramImpl& impl,
                              const SkString& input,
                              SkString output);
    /** Recursive step to write out children FPs' functions before parent's. */
    void writeChildFPFunctions(const GrFragmentProcessor& fp,
                               GrFragmentProcessor::ProgramImpl& impl);
    bool emitAndInstallXferProc(const SkString& colorIn, const SkString& coverageIn);
    SamplerHandle emitSampler(const GrBackendFormat&, GrSamplerState, const skgpu::Swizzle&,
                              const char* name);
    SamplerHandle emitInputSampler(const skgpu::Swizzle& swizzle, const char* name);
    bool checkSamplerCounts();

#ifdef SK_DEBUG
    void verify(const GrGeometryProcessor&);
    void verify(const GrFragmentProcessor&);
    void verify(const GrXferProcessor&);
#endif

    // This is used to check that we don't excede the allowable number of resources in a shader.
    int fNumFragmentSamplers;

    GrGeometryProcessor::ProgramImpl::FPCoordsMap fFPCoordsMap;
    GrShaderVar                                   fLocalCoordsVar;

    /**
     * Each root processor has an stage index. The GP is stage 0. The first root FP is stage 1,
     * the second root FP is stage 2, etc. The XP's stage index is last and its value depends on
     * how many root FPs there are. Names are mangled by appending _S<stage-index>.
     */
    int fStageIndex = -1;

    /**
     * When emitting FP stages we track the children FPs as "substages" and do additional name
     * mangling based on where in the FP hierarchy we are. The first FP is stage index 1. It's first
     * child would be substage 0 of stage 1. If that FP also has three children then its third child
     * would be substage 2 of stubstage 0 of stage 1 and would be mangled as "_S1_c0_c2".
     */
    skia_private::TArray<int> fSubstageIndices;
};

#endif
