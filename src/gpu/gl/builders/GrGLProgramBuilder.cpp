/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramBuilder.h"

#include "gl/GrGLGeometryProcessor.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLPathProcessor.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "gl/GrGLUniformHandle.h"
#include "gl/GrGLXferProcessor.h"
#include "glsl/GrGLSLCaps.h"
#include "GrAutoLocaleSetter.h"
#include "GrCoordTransform.h"
#include "GrGLPathProgramBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

const int GrGLProgramBuilder::kVarsPerBlock = 8;

GrGLProgram* GrGLProgramBuilder::CreateProgram(const DrawArgs& args, GrGLGpu* gpu) {
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    SkAutoTDelete<GrGLProgramBuilder> builder(CreateProgramBuilder(args, gpu));

    GrGLProgramBuilder* pb = builder.get();

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // seed correctly here
    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!pb->emitAndInstallProcs(&inputColor, &inputCoverage)) {
        return NULL;
    }

    return pb->finalize();
}

GrGLProgramBuilder* GrGLProgramBuilder::CreateProgramBuilder(const DrawArgs& args,
                                                             GrGLGpu* gpu) {
    if (args.fPrimitiveProcessor->isPathRendering()) {
        SkASSERT(gpu->glCaps().shaderCaps()->pathRenderingSupport() &&
                 !args.fPrimitiveProcessor->willUseGeoShader() &&
                 args.fPrimitiveProcessor->numAttribs() == 0);
        return SkNEW_ARGS(GrGLPathProgramBuilder, (gpu, args));
    } else {
        return SkNEW_ARGS(GrGLProgramBuilder, (gpu, args));
    }
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu, const DrawArgs& args)
    : fVS(this)
    , fGS(this)
    , fFS(this, args.fDesc->header().fFragPosKey)
    , fOutOfStage(true)
    , fStageIndex(-1)
    , fGeometryProcessor(NULL)
    , fXferProcessor(NULL)
    , fArgs(args)
    , fGpu(gpu)
    , fUniforms(kVarsPerBlock)
    , fSamplerUniforms(4) {
}

void GrGLProgramBuilder::addVarying(const char* name,
                                    GrGLVarying* varying,
                                    GrSLPrecision fsPrecision) {
    SkASSERT(varying);
    if (varying->vsVarying()) {
        fVS.addVarying(name, varying);
    }
    if (this->primitiveProcessor().willUseGeoShader()) {
        fGS.addVarying(name, varying);
    }
    if (varying->fsVarying()) {
        fFS.addVarying(varying, fsPrecision);
    }
}

void GrGLProgramBuilder::addPassThroughAttribute(const GrPrimitiveProcessor::Attribute* input,
                                                 const char* output) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLVertToFrag v(type);
    this->addVarying(input->fName, &v);
    fVS.codeAppendf("%s = %s;", v.vsOut(), input->fName);
    fFS.codeAppendf("%s = %s;", output, v.fsIn());
}

GrGLProgramBuilder::SeparableVaryingHandle GrGLProgramBuilder::addSeparableVarying(const char*,
                                                                                   GrGLVertToFrag*,
                                                                                   GrSLPrecision) {
    // This call is not used for non-NVPR backends. However, the polymorphism between
    // GrPrimitiveProcessor, GrGLPrimitiveProcessor and GrGLProgramBuilder does not allow for
    // a system where GrGLPathProcessor would be able to refer to a primitive-specific builder
    // that would understand separable varyings. Thus separable varyings need to be present
    // early in the inheritance chain of builders.
    SkASSERT(false);
    return SeparableVaryingHandle();
}

void GrGLProgramBuilder::nameVariable(SkString* out, char prefix, const char* name) {
    if ('\0' == prefix) {
        *out = name;
    } else {
        out->printf("%c%s", prefix, name);
    }
    if (!fOutOfStage) {
        if (out->endsWith('_')) {
            // Names containing "__" are reserved.
            out->append("x");
        }
        out->appendf("_Stage%d", fStageIndex);
    }
}

GrGLProgramDataManager::UniformHandle GrGLProgramBuilder::addUniformArray(
                                                                uint32_t visibility,
                                                                GrSLType type,
                                                                GrSLPrecision precision,
                                                                const char* name,
                                                                int count,
                                                                const char** outName) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_Visibility | kFragment_Visibility);
    SkASSERT(0 == (~kVisibilityMask & visibility));
    SkASSERT(0 != visibility);
    SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeIsFloatType(type));

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
    uni.fVariable.setTypeModifier(GrGLShaderVar::kUniform_TypeModifier);
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
    this->nameVariable(uni.fVariable.accessName(), prefix, name);
    uni.fVariable.setArrayCount(count);
    uni.fVisibility = visibility;
    uni.fVariable.setPrecision(precision);

    if (outName) {
        *outName = uni.fVariable.c_str();
    }
    return GrGLProgramDataManager::UniformHandle::CreateFromUniformIndex(fUniforms.count() - 1);
}

void GrGLProgramBuilder::appendUniformDecls(ShaderVisibility visibility,
                                            SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & visibility) {
            fUniforms[i].fVariable.appendDecl(this->ctxInfo(), out);
            out->append(";\n");
        }
    }
}

const GrGLContextInfo& GrGLProgramBuilder::ctxInfo() const {
    return fGpu->ctxInfo();
}

static void append_gr_fp_coord_transforms(const GrFragmentProcessor* processor,
                                          SkTArray<const GrCoordTransform*, true>* procCoords) {
    // add the coord transforms of this processor
    for (int i = 0; i < processor->numTransforms(); ++i) {
        procCoords->push_back(&processor->coordTransform(i));
    }
    // recursively add the coord transforms of this processor's child processors
    for (int i = 0; i < processor->numChildProcessors(); ++i) {
        append_gr_fp_coord_transforms(processor->childProcessor(i), procCoords);
    }
}

bool GrGLProgramBuilder::emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage) {
    // First we loop over all of the installed processors and collect coord transforms.  These will
    // be sent to the GrGLPrimitiveProcessor in its emitCode function
    const GrPrimitiveProcessor& primProc = this->primitiveProcessor();
    int totalTextures = primProc.numTextures();
    const int maxTextureUnits = fGpu->glCaps().maxFragmentTextureUnits();

    for (int i = 0; i < this->pipeline().numFragmentStages(); i++) {
        const GrFragmentProcessor* processor = this->pipeline().getFragmentStage(i).processor();

        if (!primProc.hasTransformedLocalCoords()) {
            SkSTArray<2, const GrCoordTransform*, true>& procCoords = fCoordTransforms.push_back();

            append_gr_fp_coord_transforms(processor, &procCoords);
        }

        totalTextures += processor->numTexturesIncludeChildProcs();
        if (totalTextures >= maxTextureUnits) {
            GrCapsDebugf(fGpu->caps(), "Program would use too many texture units\n");
            return false;
        }
    }

    this->emitAndInstallProc(primProc, inputColor, inputCoverage);

    fFragmentProcessors.reset(SkNEW(GrGLInstalledFragProcs));
    int numProcs = this->pipeline().numFragmentStages();
    this->emitAndInstallFragProcs(0, this->pipeline().numColorFragmentStages(), inputColor);
    this->emitAndInstallFragProcs(this->pipeline().numColorFragmentStages(), numProcs,
                                  inputCoverage);
    this->emitAndInstallXferProc(*this->pipeline().getXferProcessor(), *inputColor, *inputCoverage);
    return true;
}

void GrGLProgramBuilder::emitAndInstallFragProcs(int procOffset,
                                                 int numProcs,
                                                 GrGLSLExpr4* inOut) {
    for (int e = procOffset; e < numProcs; ++e) {
        GrGLSLExpr4 output;
        const GrPendingFragmentStage& stage = this->pipeline().getFragmentStage(e);
        this->emitAndInstallProc(stage, e, *inOut, &output);
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
void GrGLProgramBuilder::emitAndInstallProc(const GrPendingFragmentStage& proc,
                                            int index,
                                            const GrGLSLExpr4& input,
                                            GrGLSLExpr4* output) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);
    this->nameExpression(output, "output");

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("{ // Stage %d, %s\n", fStageIndex, proc.name());
    fFS.codeAppend(openBrace.c_str());

    this->emitAndInstallProc(proc, index, output->c_str(), input.isOnes() ? NULL : input.c_str());

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

void GrGLProgramBuilder::emitAndInstallProc(const GrPendingFragmentStage& fs,
                                            int index,
                                            const char* outColor,
                                            const char* inColor) {
    GrGLInstalledFragProc* ifp = SkNEW(GrGLInstalledFragProc);

    const GrFragmentProcessor& fp = *fs.processor();
    ifp->fGLProc.reset(fp.createGLInstance());

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(fp.numTexturesIncludeChildProcs());
    this->emitSamplers(fp, &samplers, ifp);

    GrGLFragmentProcessor::EmitArgs args(this, fp, outColor, inColor, fOutCoords[index], samplers);
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
    fGeometryProcessor = SkNEW(GrGLInstalledGeoProc);

    const GrBatchTracker& bt = this->batchTracker();
    fGeometryProcessor->fGLProc.reset(gp.createGLInstance(bt, *fGpu->glCaps().glslCaps()));

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(gp.numTextures());
    this->emitSamplers(gp, &samplers, fGeometryProcessor);

    GrGLGeometryProcessor::EmitArgs args(this, gp, bt, outColor, outCoverage, samplers,
                                         fCoordTransforms, &fOutCoords);
    fGeometryProcessor->fGLProc->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(gp);
}

void GrGLProgramBuilder::emitAndInstallXferProc(const GrXferProcessor& xp,
                                                const GrGLSLExpr4& colorIn,
                                                const GrGLSLExpr4& coverageIn) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);

    SkASSERT(!fXferProcessor);
    fXferProcessor = SkNEW(GrGLInstalledXferProc);

    fXferProcessor->fGLProc.reset(xp.createGLInstance());

    // Enable dual source secondary output if we have one
    if (xp.hasSecondaryOutput()) {
        fFS.enableSecondaryOutput();
    }

    if (this->ctxInfo().caps()->glslCaps()->mustDeclareFragmentShaderOutput()) {
        fFS.enableCustomOutput();
    }

    SkString openBrace;
    openBrace.printf("{ // Xfer Processor: %s\n", xp.name());
    fFS.codeAppend(openBrace.c_str());

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(xp.numTextures());
    this->emitSamplers(xp, &samplers, fXferProcessor);

    GrGLXferProcessor::EmitArgs args(this, xp, colorIn.c_str(), coverageIn.c_str(),
                                     fFS.getPrimaryColorOutputName(),
                                     fFS.getSecondaryColorOutputName(), samplers);
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

template <class Proc>
void GrGLProgramBuilder::emitSamplers(const GrProcessor& processor,
                                      GrGLProcessor::TextureSamplerArray* outSamplers,
                                      GrGLInstalledProc<Proc>* ip) {
    SkDEBUGCODE(ip->fSamplersIdx = fSamplerUniforms.count();)
    int numTextures = processor.numTextures();
    UniformHandle* localSamplerUniforms = fSamplerUniforms.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("Sampler%d", t);
        localSamplerUniforms[t] = this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                   kSampler2D_GrSLType, kDefault_GrSLPrecision,
                                                   name.c_str());
        SkNEW_APPEND_TO_TARRAY(outSamplers, GrGLProcessor::TextureSampler,
                               (localSamplerUniforms[t], processor.textureAccess(t)));
    }
}

GrGLProgram* GrGLProgramBuilder::finalize() {
    // verify we can get a program id
    GrGLuint programID;
    GL_CALL_RET(programID, CreateProgram());
    if (0 == programID) {
        return NULL;
    }

    // compile shaders and bind attributes / uniforms
    SkTDArray<GrGLuint> shadersToDelete;

    if (!fVS.compileAndAttachShaders(programID, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return NULL;
    }

    // NVPR actually requires a vertex shader to compile
    bool useNvpr = primitiveProcessor().isPathRendering();
    if (!useNvpr) {
        fVS.bindVertexAttributes(programID);
    }

    if (!fFS.compileAndAttachShaders(programID, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return NULL;
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
    bool usingBindUniform = fGpu->glInterface()->fFunctions.fBindUniformLocation != NULL;
    if (usingBindUniform) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GL_CALL(BindUniformLocation(programID, i, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = i;
        }
    }

    fFS.bindFragmentShaderLocations(programID);
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
    bool usingBindUniform = fGpu->glInterface()->fFunctions.fBindUniformLocation != NULL;
    if (!usingBindUniform) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = location;
        }
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
    return SkNEW_ARGS(GrGLProgram, (fGpu, this->desc(), fUniformHandles, programID, fUniforms,
                                    fGeometryProcessor, fXferProcessor, fFragmentProcessors.get(),
                                    &fSamplerUniforms));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLInstalledFragProcs::~GrGLInstalledFragProcs() {
    int numProcs = fProcs.count();
    for (int e = 0; e < numProcs; ++e) {
        SkDELETE(fProcs[e]);
    }
}
