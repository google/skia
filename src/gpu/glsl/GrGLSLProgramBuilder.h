/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProgramBuilder_DEFINED
#define GrGLSLProgramBuilder_DEFINED

#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "GrProgramDesc.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLXferProcessor.h"

class GrShaderVar;
class GrGLSLVaryingHandler;
class SkString;
class GrShaderCaps;

typedef SkSTArray<8, GrGLSLFragmentProcessor*, true> GrGLSLFragProcs;

class GrGLSLProgramBuilder {
public:
    using UniformHandle      = GrGLSLUniformHandler::UniformHandle;
    using SamplerHandle      = GrGLSLUniformHandler::SamplerHandle;
    using TexelBufferHandle  = GrGLSLUniformHandler::TexelBufferHandle;
    using ImageStorageHandle = GrGLSLUniformHandler::ImageStorageHandle;

    virtual ~GrGLSLProgramBuilder() {}

    virtual const GrCaps* caps() const = 0;
    const GrShaderCaps* shaderCaps() const { return this->caps()->shaderCaps(); }

    const GrPrimitiveProcessor& primitiveProcessor() const { return fPrimProc; }
    const GrPipeline& pipeline() const { return fPipeline; }
    GrProgramDesc* desc() { return fDesc; }
    const GrProgramDesc::KeyHeader& header() const { return fDesc->header(); }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const;

    const GrShaderVar& samplerVariable(SamplerHandle handle) const {
        return this->uniformHandler()->samplerVariable(handle);
    }

    GrSwizzle samplerSwizzle(SamplerHandle handle) const {
        return this->uniformHandler()->samplerSwizzle(handle);
    }

    const GrShaderVar& texelBufferVariable(TexelBufferHandle handle) const {
        return this->uniformHandler()->texelBufferVariable(handle);
    }

    const GrShaderVar& imageStorageVariable(ImageStorageHandle handle) const {
        return this->uniformHandler()->imageStorageVariable(handle);
    }

    // Handles for program uniforms (other than per-effect uniforms)
    struct BuiltinUniformHandles {
        UniformHandle       fRTAdjustmentUni;

        // We use the render target height to provide a y-down frag coord when specifying
        // origin_upper_left is not supported.
        UniformHandle       fRTHeightUni;
    };

    // Used to add a uniform for the RenderTarget height (used for frag position) without mangling
    // the name of the uniform inside of a stage.
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

    GrGLSLVertexBuilder         fVS;
    GrGLSLGeometryBuilder       fGS;
    GrGLSLFragmentShaderBuilder fFS;

    int fStageIndex;

    const GrPipeline&           fPipeline;
    const GrPrimitiveProcessor& fPrimProc;
    GrProgramDesc*              fDesc;

    BuiltinUniformHandles fUniformHandles;

    GrGLSLPrimitiveProcessor* fGeometryProcessor;
    GrGLSLXferProcessor* fXferProcessor;
    GrGLSLFragProcs fFragmentProcessors;

protected:
    explicit GrGLSLProgramBuilder(const GrPipeline&,
                                  const GrPrimitiveProcessor&,
                                  GrProgramDesc*);

    void addFeature(GrShaderFlags shaders, uint32_t featureBit, const char* extensionName);

    bool emitAndInstallProcs();

    void cleanupFragmentProcessors();

    void finalizeShaders();

private:
    // reset is called by program creator between each processor's emit code.  It increments the
    // stage offset for variable name mangling, and also ensures verfication variables in the
    // fragment shader are cleared.
    void reset() {
        this->addStage();
        SkDEBUGCODE(fFS.resetVerification();)
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

    void emitAndInstallPrimProc(const GrPrimitiveProcessor&,
                                SkString* outputColor,
                                SkString* outputCoverage);
    void emitAndInstallFragProcs(SkString* colorInOut, SkString* coverageInOut);
    SkString emitAndInstallFragProc(const GrFragmentProcessor&,
                                    int index,
                                    int transformedCoordVarsIdx,
                                    const SkString& input,
                                    SkString output);
    void emitAndInstallXferProc(const SkString& colorIn, const SkString& coverageIn);
    void emitSamplersAndImageStorages(const GrResourceIOProcessor& processor,
                                      SkTArray<SamplerHandle>* outTexSamplerHandles,
                                      SkTArray<TexelBufferHandle>* outTexelBufferHandles,
                                      SkTArray<ImageStorageHandle>* outImageStorageHandles);
    SamplerHandle emitSampler(GrSLType samplerType, GrPixelConfig, const char* name,
                              GrShaderFlags visibility);
    TexelBufferHandle emitTexelBuffer(GrPixelConfig, const char* name, GrShaderFlags visibility);
    ImageStorageHandle emitImageStorage(const GrResourceIOProcessor::ImageStorageAccess&,
                                        const char* name);
    void emitFSOutputSwizzle(bool hasSecondaryOutput);
    void updateSamplerCounts(GrShaderFlags visibility);
    bool checkSamplerCounts();
    bool checkImageStorageCounts();

#ifdef SK_DEBUG
    void verify(const GrPrimitiveProcessor&);
    void verify(const GrXferProcessor&);
    void verify(const GrFragmentProcessor&);
#endif

    // These are used to check that we don't excede the allowable number of resources in a shader.
    // The sampler counts include both normal texure samplers as well as texel buffers.
    int                         fNumVertexSamplers;
    int                         fNumGeometrySamplers;
    int                         fNumFragmentSamplers;
    int                         fNumVertexImageStorages;
    int                         fNumGeometryImageStorages;
    int                         fNumFragmentImageStorages;
    SkSTArray<4, GrShaderVar>   fTransformedCoordVars;
};

#endif
