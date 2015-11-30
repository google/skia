/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramBuilder.h"

#include "GrAutoLocaleSetter.h"
#include "GrCoordTransform.h"
#include "GrGLProgramBuilder.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "gl/builders/GrGLShaderStringBuilder.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLTextureSampler.h"
#include "glsl/GrGLSLXferProcessor.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLProgram* GrGLProgramBuilder::CreateProgram(const DrawArgs& args, GrGLGpu* gpu) {
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    SkAutoTDelete<GrGLProgramBuilder> builder(new GrGLProgramBuilder(gpu, args));

    GrGLProgramBuilder* pb = builder.get();

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // seed correctly here
    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!pb->emitAndInstallProcs(&inputColor, &inputCoverage)) {
        return nullptr;
    }

    return pb->finalize();
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu, const DrawArgs& args)
    : INHERITED(args)
    , fGeometryProcessor(nullptr)
    , fXferProcessor(nullptr)
    , fGpu(gpu)
    , fUniforms(kVarsPerBlock)
    , fSamplerUniforms(4)
    , fVaryingHandler(this) {
}

GrGLSLProgramDataManager::UniformHandle GrGLProgramBuilder::internalAddUniformArray(
                                                                uint32_t visibility,
                                                                GrSLType type,
                                                                GrSLPrecision precision,
                                                                const char* name,
                                                                bool mangleName,
                                                                int count,
                                                                const char** outName) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_Visibility | kFragment_Visibility);
    SkASSERT(0 == (~kVisibilityMask & visibility));
    SkASSERT(0 != visibility);
    SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
    uni.fVariable.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);
    // TODO this is a bit hacky, lets think of a better way.  Basically we need to be able to use
    // the uniform view matrix name in the GP, and the GP is immutable so it has to tell the PB
    // exactly what name it wants to use for the uniform view matrix.  If we prefix anythings, then
    // the names will mismatch.  I think the correct solution is to have all GPs which need the
    // uniform view matrix, they should upload the view matrix in their setData along with regular
    // uniforms.
    char prefix = 'u';
    if ('u' == name[0]) {
        prefix = '\0';
    }
    this->nameVariable(uni.fVariable.accessName(), prefix, name, mangleName);
    uni.fVariable.setArrayCount(count);
    uni.fVisibility = visibility;
    uni.fVariable.setPrecision(precision);

    if (outName) {
        *outName = uni.fVariable.c_str();
    }
    return GrGLSLProgramDataManager::UniformHandle(fUniforms.count() - 1);
}

void GrGLProgramBuilder::onAppendUniformDecls(ShaderVisibility visibility, SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & visibility) {
            fUniforms[i].fVariable.appendDecl(this->glslCaps(), out);
            out->append(";\n");
        }
    }
}

const GrGLSLCaps* GrGLProgramBuilder::glslCaps() const {
    return this->fGpu->ctxInfo().caps()->glslCaps();
}

bool GrGLProgramBuilder::emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage) {
    // First we loop over all of the installed processors and collect coord transforms.  These will
    // be sent to the GrGLSLPrimitiveProcessor in its emitCode function
    const GrPrimitiveProcessor& primProc = this->primitiveProcessor();
    int totalTextures = primProc.numTextures();
    const int maxTextureUnits = fGpu->glCaps().maxFragmentTextureUnits();

    for (int i = 0; i < this->pipeline().numFragmentProcessors(); i++) {
        const GrFragmentProcessor& processor = this->pipeline().getFragmentProcessor(i);

        if (!primProc.hasTransformedLocalCoords()) {
            SkTArray<const GrCoordTransform*, true>& procCoords = fCoordTransforms.push_back();
            processor.gatherCoordTransforms(&procCoords);
        }

        totalTextures += processor.numTextures();
        if (totalTextures >= maxTextureUnits) {
            GrCapsDebugf(fGpu->caps(), "Program would use too many texture units\n");
            return false;
        }
    }

    this->emitAndInstallProc(primProc, inputColor, inputCoverage);

    fFragmentProcessors.reset(new GrGLInstalledFragProcs);
    int numProcs = this->pipeline().numFragmentProcessors();
    this->emitAndInstallFragProcs(0, this->pipeline().numColorFragmentProcessors(), inputColor);
    this->emitAndInstallFragProcs(this->pipeline().numColorFragmentProcessors(), numProcs,
                                  inputCoverage);
    this->emitAndInstallXferProc(*this->pipeline().getXferProcessor(), *inputColor, *inputCoverage,
                                 this->pipeline().ignoresCoverage());
    return true;
}

void GrGLProgramBuilder::emitAndInstallFragProcs(int procOffset,
                                                 int numProcs,
                                                 GrGLSLExpr4* inOut) {
    for (int i = procOffset; i < numProcs; ++i) {
        GrGLSLExpr4 output;
        const GrFragmentProcessor& fp = this->pipeline().getFragmentProcessor(i);
        this->emitAndInstallProc(fp, i, *inOut, &output);
        *inOut = output;
    }
}

void GrGLProgramBuilder::nameExpression(GrGLSLExpr4* output, const char* baseName) {
    // create var to hold stage result.  If we already have a valid output name, just use that
    // otherwise create a new mangled one.  This name is only valid if we are reordering stages
    // and have to tell stage exactly where to put its output.
    SkString outName;
    if (output->isValid()) {
        outName = output->c_str();
    } else {
        this->nameVariable(&outName, '\0', baseName);
    }
    fFS.codeAppendf("vec4 %s;", outName.c_str());
    *output = outName;
}

// TODO Processors cannot output zeros because an empty string is all 1s
// the fix is to allow effects to take the GrGLSLExpr4 directly
void GrGLProgramBuilder::emitAndInstallProc(const GrFragmentProcessor& fp,
                                            int index,
                                            const GrGLSLExpr4& input,
                                            GrGLSLExpr4* output) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);
    this->nameExpression(output, "output");

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("{ // Stage %d, %s\n", fStageIndex, fp.name());
    fFS.codeAppend(openBrace.c_str());

    this->emitAndInstallProc(fp, index, output->c_str(), input.isOnes() ? nullptr : input.c_str());

    fFS.codeAppend("}");
}

void GrGLProgramBuilder::emitAndInstallProc(const GrPrimitiveProcessor& proc,
                                            GrGLSLExpr4* outputColor,
                                            GrGLSLExpr4* outputCoverage) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);
    this->nameExpression(outputColor, "outputColor");
    this->nameExpression(outputCoverage, "outputCoverage");

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("{ // Stage %d, %s\n", fStageIndex, proc.name());
    fFS.codeAppend(openBrace.c_str());
    fVS.codeAppendf("// Primitive Processor %s\n", proc.name());

    this->emitAndInstallProc(proc, outputColor->c_str(), outputCoverage->c_str());

    fFS.codeAppend("}");
}

void GrGLProgramBuilder::emitAndInstallProc(const GrFragmentProcessor& fp,
                                            int index,
                                            const char* outColor,
                                            const char* inColor) {
    GrGLInstalledFragProc* ifp = new GrGLInstalledFragProc;

    ifp->fGLProc.reset(fp.createGLSLInstance());

    SkSTArray<4, GrGLSLTextureSampler> samplers(fp.numTextures());
    this->emitSamplers(fp, &samplers, ifp);

    GrGLSLFragmentProcessor::EmitArgs args(this,
                                           &fFS,
                                           this->glslCaps(),
                                           fp,
                                           outColor,
                                           inColor,
                                           fOutCoords[index],
                                           samplers);
    ifp->fGLProc->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(fp);
    fFragmentProcessors->fProcs.push_back(ifp);
}

void GrGLProgramBuilder::emitAndInstallProc(const GrPrimitiveProcessor& gp,
                                            const char* outColor,
                                            const char* outCoverage) {
    SkASSERT(!fGeometryProcessor);
    fGeometryProcessor = new GrGLInstalledGeoProc;

    fGeometryProcessor->fGLProc.reset(gp.createGLSLInstance(*fGpu->glCaps().glslCaps()));

    SkSTArray<4, GrGLSLTextureSampler> samplers(gp.numTextures());
    this->emitSamplers(gp, &samplers, fGeometryProcessor);

    GrGLSLGeometryProcessor::EmitArgs args(this,
                                           &fVS,
                                           &fFS,
                                           &fVaryingHandler,
                                           this->glslCaps(),
                                           gp,
                                           outColor,
                                           outCoverage,
                                           samplers,
                                           fCoordTransforms,
                                           &fOutCoords);
    fGeometryProcessor->fGLProc->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(gp);
}

void GrGLProgramBuilder::emitAndInstallXferProc(const GrXferProcessor& xp,
                                                const GrGLSLExpr4& colorIn,
                                                const GrGLSLExpr4& coverageIn,
                                                bool ignoresCoverage) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);

    SkASSERT(!fXferProcessor);
    fXferProcessor = new GrGLInstalledXferProc;

    fXferProcessor->fGLProc.reset(xp.createGLSLInstance());

    // Enable dual source secondary output if we have one
    if (xp.hasSecondaryOutput()) {
        fFS.enableSecondaryOutput();
    }

    if (this->glslCaps()->mustDeclareFragmentShaderOutput()) {
        fFS.enableCustomOutput();
    }

    SkString openBrace;
    openBrace.printf("{ // Xfer Processor: %s\n", xp.name());
    fFS.codeAppend(openBrace.c_str());

    SkSTArray<4, GrGLSLTextureSampler> samplers(xp.numTextures());
    this->emitSamplers(xp, &samplers, fXferProcessor);

    GrGLSLXferProcessor::EmitArgs args(this,
                                       &fFS,
                                       this->glslCaps(),
                                       xp, colorIn.c_str(),
                                       ignoresCoverage ? nullptr : coverageIn.c_str(),
                                       fFS.getPrimaryColorOutputName(),
                                       fFS.getSecondaryColorOutputName(),
                                       samplers);
    fXferProcessor->fGLProc->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(xp);
    fFS.codeAppend("}");
}

void GrGLProgramBuilder::verify(const GrPrimitiveProcessor& gp) {
    SkASSERT(fFS.hasReadFragmentPosition() == gp.willReadFragmentPosition());
}

void GrGLProgramBuilder::verify(const GrXferProcessor& xp) {
    SkASSERT(fFS.hasReadDstColor() == xp.willReadDstColor());
}

void GrGLProgramBuilder::verify(const GrFragmentProcessor& fp) {
    SkASSERT(fFS.hasReadFragmentPosition() == fp.willReadFragmentPosition());
}

static GrSLType get_sampler_type(const GrTextureAccess& access) {
    GrGLTexture* glTexture = static_cast<GrGLTexture*>(access.getTexture());
    if (glTexture->target() == GR_GL_TEXTURE_EXTERNAL) {
        return kSamplerExternal_GrSLType;
    } else {
        SkASSERT(glTexture->target() == GR_GL_TEXTURE_2D);
        return kSampler2D_GrSLType;
    }
}

template <class Proc>
void GrGLProgramBuilder::emitSamplers(const GrProcessor& processor,
                                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers,
                                      GrGLInstalledProc<Proc>* ip) {
    SkDEBUGCODE(ip->fSamplersIdx = fSamplerUniforms.count();)
    int numTextures = processor.numTextures();
    UniformHandle* localSamplerUniforms = fSamplerUniforms.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("Sampler%d", t);
        GrSLType samplerType = get_sampler_type(processor.textureAccess(t));
        localSamplerUniforms[t] = this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                   samplerType, kDefault_GrSLPrecision,
                                                   name.c_str());
        SkNEW_APPEND_TO_TARRAY(outSamplers, GrGLSLTextureSampler,
                               (localSamplerUniforms[t], processor.textureAccess(t)));
        if (kSamplerExternal_GrSLType == samplerType) {
            const char* externalFeatureString = this->glslCaps()->externalTextureExtensionString();
            // We shouldn't ever create a GrGLTexture that requires external sampler type 
            SkASSERT(externalFeatureString);
            fFS.addFeature(1 << GrGLSLFragmentShaderBuilder::kExternalTexture_GLSLPrivateFeature,
                           externalFeatureString);
        }
    }
}

bool GrGLProgramBuilder::compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   shader.fCompilerStrings.begin(),
                                                   shader.fCompilerStringLengths.begin(),
                                                   shader.fCompilerStrings.count(),
                                                   gpu->stats());

    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;

    return true;
}

GrGLProgram* GrGLProgramBuilder::finalize() {
    // verify we can get a program id
    GrGLuint programID;
    GL_CALL_RET(programID, CreateProgram());
    if (0 == programID) {
        return nullptr;
    }

    // compile shaders and bind attributes / uniforms
    SkTDArray<GrGLuint> shadersToDelete;
    fVS.finalize(kVertex_Visibility);
    if (!this->compileAndAttachShaders(fVS, programID, GR_GL_VERTEX_SHADER, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    // NVPR actually requires a vertex shader to compile
    bool useNvpr = primitiveProcessor().isPathRendering();
    if (!useNvpr) {
        const GrPrimitiveProcessor& primProc = this->primitiveProcessor();

        int vaCount = primProc.numAttribs();
        for (int i = 0; i < vaCount; i++) {
            GL_CALL(BindAttribLocation(programID, i, primProc.getAttrib(i).fName));
        }
    }

    fFS.finalize(kFragment_Visibility);
    if (!this->compileAndAttachShaders(fFS, programID, GR_GL_FRAGMENT_SHADER, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    this->bindProgramResourceLocations(programID);

    GL_CALL(LinkProgram(programID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = kChromium_GrGLDriver != fGpu->ctxInfo().driver();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        checkLinkStatus(programID);
    }
    this->resolveProgramResourceLocations(programID);

    this->cleanupShaders(shadersToDelete);

    return this->createProgram(programID);
}

void GrGLProgramBuilder::bindProgramResourceLocations(GrGLuint programID) {
    if (fGpu->glCaps().bindUniformLocationSupport()) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GL_CALL(BindUniformLocation(programID, i, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = i;
        }
    }

    const GrGLCaps& caps = this->gpu()->glCaps();
    if (fFS.hasCustomColorOutput() && caps.bindFragDataLocationSupport()) {
        GL_CALL(BindFragDataLocation(programID, 0,
                                     GrGLSLFragmentShaderBuilder::DeclaredColorOutputName()));
    }
    if (fFS.hasSecondaryOutput() && caps.glslCaps()->mustDeclareFragmentShaderOutput()) {
        GL_CALL(BindFragDataLocationIndexed(programID, 0, 1,
                                  GrGLSLFragmentShaderBuilder::DeclaredSecondaryColorOutputName()));
    }

    // handle NVPR separable varyings
    if (!fGpu->glCaps().shaderCaps()->pathRenderingSupport() ||
        !fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fVaryingHandler.fPathProcVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GL_CALL(BindFragmentInputLocation(programID, i,
                                       fVaryingHandler.fPathProcVaryingInfos[i].fVariable.c_str()));
        fVaryingHandler.fPathProcVaryingInfos[i].fLocation = i;
    }
}

bool GrGLProgramBuilder::checkLinkStatus(GrGLuint programID) {
    GrGLint linked = GR_GL_INIT_ZERO;
    GL_CALL(GetProgramiv(programID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(programID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramInfoLog(programID,
                                      infoLen+1,
                                      &length,
                                      (char*)log.get()));
            SkDebugf("%s", (char*)log.get());
        }
        SkDEBUGFAIL("Error linking program");
        GL_CALL(DeleteProgram(programID));
        programID = 0;
    }
    return SkToBool(linked);
}

void GrGLProgramBuilder::resolveProgramResourceLocations(GrGLuint programID) {
    if (!fGpu->glCaps().bindUniformLocationSupport()) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = location;
        }
    }

    // handle NVPR separable varyings
    if (!fGpu->glCaps().shaderCaps()->pathRenderingSupport() ||
        !fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fVaryingHandler.fPathProcVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location, GetProgramResourceLocation(
                                       programID,
                                       GR_GL_FRAGMENT_INPUT,
                                       fVaryingHandler.fPathProcVaryingInfos[i].fVariable.c_str()));
        fVaryingHandler.fPathProcVaryingInfos[i].fLocation = location;
    }
}

void GrGLProgramBuilder::cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs) {
    GL_CALL(DeleteProgram(programID));
    cleanupShaders(shaderIDs);
}
void GrGLProgramBuilder::cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs) {
    for (int i = 0; i < shaderIDs.count(); ++i) {
      GL_CALL(DeleteShader(shaderIDs[i]));
    }
}

GrGLProgram* GrGLProgramBuilder::createProgram(GrGLuint programID) {
    return new GrGLProgram(fGpu, this->desc(), fUniformHandles, programID, fUniforms,
                           fVaryingHandler.fPathProcVaryingInfos,
                           fGeometryProcessor, fXferProcessor, fFragmentProcessors.get(),
                           &fSamplerUniforms);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLInstalledFragProcs::~GrGLInstalledFragProcs() {
    int numProcs = fProcs.count();
    for (int i = 0; i < numProcs; ++i) {
        delete fProcs[i];
    }
}
