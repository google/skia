/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

class GrShaderVar;
class GrGLSLVaryingHandler;
class SkString;
class GrShaderCaps;

class GrGLSLProgramBuilder {
public:
    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;

    virtual ~GrGLSLProgramBuilder() {}

    virtual const GrCaps* caps() const = 0;
    const GrShaderCaps* shaderCaps() const { return this->caps()->shaderCaps(); }

    const GrPrimitiveProcessor& primitiveProcessor() const { return fPrimProc; }
    const GrTextureProxy* const* primProcProxies() const { return fPrimProcProxies; }
    int effectiveSampleCnt() const {
        SkASSERT(GrProcessor::CustomFeatures::kSampleLocations & header().processorFeatures());
        return fRenderTarget->renderTargetPriv().getSampleLocations().count();
    }
    const SkTArray<SkPoint>& getSampleLocations() const {
        return fRenderTarget->renderTargetPriv().getSampleLocations();
    }
    int numSamples() const { return fNumSamples; }
    GrSurfaceOrigin origin() const { return fOrigin; }
    const GrPipeline& pipeline() const { return fPipeline; }
    GrProgramDesc* desc() { return fDesc; }
    const GrProgramDesc::KeyHeader& header() const { return fDesc->header(); }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const;

    const char* samplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->samplerVariable(handle);
    }

    GrSwizzle samplerSwizzle(SamplerHandle handle) const {
        if (this->caps()->shaderCaps()->textureSwizzleAppliedInShader()) {
            return this->uniformHandler()->samplerSwizzle(handle);
        }
        return GrSwizzle::RGBA();
    }

    // Used to add a uniform for the RenderTarget width (used for sk_Width) without mangling
    // the name of the uniform inside of a stage.
    void addRTWidthUniform(const char* name);

    // Used to add a uniform for the RenderTarget height (used for sk_Height and frag position)
    // without mangling the name of the uniform inside of a stage.
    void addRTHeightUniform(const char* name);

    // Generates a name for a variable. The generated string will be name prefixed by the prefix
    // char (unless the prefix is '\0'). It also will mangle the name to be stage-specific unless
    // explicitly asked not to.
    void nameVariable(SkString* out, char prefix, const char* name, bool mangle = true);

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

    const GrRenderTarget*        fRenderTarget;
    const int                    fNumSamples;
    const GrSurfaceOrigin        fOrigin;
    const GrPipeline&            fPipeline;
    const GrPrimitiveProcessor&  fPrimProc;
    const GrTextureProxy* const* fPrimProcProxies;

    GrProgramDesc*               fDesc;

    GrGLSLBuiltinUniformHandles  fUniformHandles;

    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    int fFragmentProcessorCnt;

protected:
    explicit GrGLSLProgramBuilder(GrRenderTarget* renderTarget,
                                  int numSamples,
                                  GrSurfaceOrigin origin,
                                  const GrPrimitiveProcessor&,
                                  const GrTextureProxy* const primProcProxies[],
                                  const GrPipeline&,
                                  GrProgramDesc*);

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

    void emitAndInstallPrimProc(SkString* outputColor, SkString* outputCoverage);
    void emitAndInstallFragProcs(SkString* colorInOut, SkString* coverageInOut);
    SkString emitAndInstallFragProc(const GrFragmentProcessor&,
                                    int index,
                                    int transformedCoordVarsIdx,
                                    const SkString& input,
                                    SkString output,
                                    SkTArray<std::unique_ptr<GrGLSLFragmentProcessor>>*);
    void emitAndInstallXferProc(const SkString& colorIn, const SkString& coverageIn);
    SamplerHandle emitSampler(const GrTexture*, const GrSamplerState&, const GrSwizzle&,
                              const char* name);
    bool checkSamplerCounts();

#ifdef SK_DEBUG
    void verify(const GrPrimitiveProcessor&);
    void verify(const GrFragmentProcessor&);
    void verify(const GrXferProcessor&);
#endif

    // These are used to check that we don't excede the allowable number of resources in a shader.
    int fNumFragmentSamplers;
    SkSTArray<4, GrGLSLPrimitiveProcessor::TransformVar> fTransformedCoordVars;
};

#endif
