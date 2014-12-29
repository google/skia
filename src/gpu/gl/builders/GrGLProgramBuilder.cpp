/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramBuilder.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "gl/GrGLUniformHandle.h"
#include "../GrGLXferProcessor.h"
#include "../GrGLGpu.h"
#include "GrCoordTransform.h"
#include "GrGLLegacyNvprProgramBuilder.h"
#include "GrGLNvprProgramBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

//////////////////////////////////////////////////////////////////////////////

const int GrGLProgramBuilder::kVarsPerBlock = 8;

GrGLProgram* GrGLProgramBuilder::CreateProgram(const GrOptDrawState& optState, GrGLGpu* gpu) {
    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    SkAutoTDelete<GrGLProgramBuilder> builder(CreateProgramBuilder(optState,
                                                                   optState.hasGeometryProcessor(),
                                                                   gpu));

    GrGLProgramBuilder* pb = builder.get();
    const GrGLProgramDescBuilder::GLKeyHeader& header = GrGLProgramDescBuilder::GetHeader(pb->desc());

    // emit code to read the dst copy texture, if necessary
    if (GrGLFragmentShaderBuilder::kNoDstRead_DstReadKey != header.fDstReadKey &&
        !gpu->glCaps().fbFetchSupport()) {
        pb->fFS.emitCodeToReadDstTexture();
    }

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // seed correctly here
    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    pb->emitAndInstallProcs(&inputColor, &inputCoverage);

    return pb->finalize();
}

GrGLProgramBuilder* GrGLProgramBuilder::CreateProgramBuilder(const GrOptDrawState& optState,
                                                             bool hasGeometryProcessor,
                                                             GrGLGpu* gpu) {
    const GrProgramDesc& desc = optState.programDesc();
    if (GrGLProgramDescBuilder::GetHeader(desc).fUseNvpr) {
        SkASSERT(gpu->glCaps().pathRenderingSupport());
        SkASSERT(!hasGeometryProcessor);
        if (gpu->glPathRendering()->texturingMode() ==
            GrGLPathRendering::FixedFunction_TexturingMode) {
            return SkNEW_ARGS(GrGLLegacyNvprProgramBuilder, (gpu, optState));
        } else {
            return SkNEW_ARGS(GrGLNvprProgramBuilder, (gpu, optState));
        }
    } else {
        return SkNEW_ARGS(GrGLProgramBuilder, (gpu, optState));
    }
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu, const GrOptDrawState& optState)
    : fVS(this)
    , fGS(this)
    , fFS(this, optState.programDesc().header().fFragPosKey)
    , fOutOfStage(true)
    , fStageIndex(-1)
    , fGeometryProcessor(NULL)
    , fXferProcessor(NULL)
    , fOptState(optState)
    , fDesc(optState.programDesc())
    , fGpu(gpu)
    , fUniforms(kVarsPerBlock) {
}

void GrGLProgramBuilder::addVarying(const char* name,
                                    GrGLVarying* varying,
                                    GrSLPrecision fsPrecision) {
    SkASSERT(varying);
    if (varying->vsVarying()) {
        fVS.addVarying(name, varying);
    }
    if (fOptState.hasGeometryProcessor() && fOptState.getGeometryProcessor()->willUseGeoShader()) {
        fGS.addVarying(name, varying);
    }
    if (varying->fsVarying()) {
        fFS.addVarying(varying, fsPrecision);
    }
}

void GrGLProgramBuilder::addPassThroughAttribute(const GrGeometryProcessor::GrAttribute* input,
                                                 const char* output) {
    GrSLType type = GrVertexAttribTypeToSLType(input->fType);
    GrGLVertToFrag v(type);
    this->addVarying(input->fName, &v);
    fVS.codeAppendf("%s = %s;", v.vsOut(), input->fName);
    fFS.codeAppendf("%s = %s;", output, v.fsIn());
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

void GrGLProgramBuilder::emitAndInstallProcs(GrGLSLExpr4* inputColor, GrGLSLExpr4* inputCoverage) {
    if (fOptState.hasGeometryProcessor()) {
        fVS.setupUniformViewMatrix();

        fVS.codeAppend("gl_PointSize = 1.0;");

        // Setup position
        // TODO it'd be possible to remove these from the vertexshader builder and have them
        // be outputs from the emit call.  We don't do this because emitargs is constant.  It would
        // be easy to change this though
        fVS.codeAppendf("vec3 %s;", fVS.glPosition());
        fVS.codeAppendf("vec2 %s;", fVS.positionCoords());
        fVS.codeAppendf("vec2 %s;", fVS.localCoords());

        const GrGeometryProcessor& gp = *fOptState.getGeometryProcessor();
        fVS.emitAttributes(gp);
    }

    const GrPrimitiveProcessor& primProc = *fOptState.getPrimitiveProcessor();
    this->emitAndInstallProc(primProc, inputColor, inputCoverage);

    fFragmentProcessors.reset(SkNEW(GrGLInstalledFragProcs));
    int numProcs = fOptState.numFragmentStages();
    this->emitAndInstallFragProcs(0, fOptState.numColorStages(), inputColor);
    this->emitAndInstallFragProcs(fOptState.numColorStages(), numProcs,  inputCoverage);

    if (fOptState.hasGeometryProcessor()) {
        fVS.transformToNormalizedDeviceSpace();
    }

    this->emitAndInstallXferProc(*fOptState.getXferProcessor(), *inputColor, *inputCoverage);
}

void GrGLProgramBuilder::emitAndInstallFragProcs(int procOffset,
                                                 int numProcs,
                                                 GrGLSLExpr4* inOut) {
    for (int e = procOffset; e < numProcs; ++e) {
        GrGLSLExpr4 output;
        const GrPendingFragmentStage& stage = fOptState.getFragmentStage(e);
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

    this->emitAndInstallProc(proc, output->c_str(), input.isOnes() ? NULL : input.c_str());

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

    this->emitAndInstallProc(proc, outputColor->c_str(), outputCoverage->c_str());

    fFS.codeAppend("}");
}

void GrGLProgramBuilder::emitAndInstallProc(const GrPendingFragmentStage& fs,
                                            const char* outColor,
                                            const char* inColor) {
    GrGLInstalledFragProc* ifp = SkNEW(GrGLInstalledFragProc);

    const GrFragmentProcessor& fp = *fs.processor();
    ifp->fGLProc.reset(fp.createGLInstance());

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(fp.numTextures());
    this->emitSamplers(fp, &samplers, ifp);

    // Fragment processors can have coord transforms
    SkSTArray<2, GrGLProcessor::TransformedCoords> coords(fp.numTransforms());
    this->emitTransforms(fs, &coords, ifp);

    ifp->fGLProc->emitCode(this, fp, outColor, inColor, coords, samplers);

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

    const GrBatchTracker& bt = fOptState.getBatchTracker();
    fGeometryProcessor->fGLProc.reset(gp.createGLInstance(bt));

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(gp.numTextures());
    this->emitSamplers(gp, &samplers, fGeometryProcessor);

    GrGLGeometryProcessor::EmitArgs args(this, gp, bt, outColor, outCoverage, samplers);
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

    // On any post 1.10 GLSL supporting GPU, we declare custom output
    if (k110_GrGLSLGeneration != fFS.fProgramBuilder->gpu()->glslGeneration()) {
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
    // TODO: Once will readDst is only xp enable this assert and remove it from the
    // FragmentProcessor verify()
    //SkASSERT(fFS.hasReadDstColor() == xp.willReadDstColor());
}

void GrGLProgramBuilder::verify(const GrFragmentProcessor& fp) {
    SkASSERT(fFS.hasReadFragmentPosition() == fp.willReadFragmentPosition());
    SkASSERT(fFS.hasReadDstColor() == fp.willReadDstColor());
}

void GrGLProgramBuilder::emitTransforms(const GrPendingFragmentStage& stage,
                                        GrGLProcessor::TransformedCoordsArray* outCoords,
                                        GrGLInstalledFragProc* ifp) {
    const GrFragmentProcessor* processor = stage.processor();
    int numTransforms = processor->numTransforms();
    ifp->fTransforms.push_back_n(numTransforms);

    for (int t = 0; t < numTransforms; t++) {
        const char* uniName = "StageMatrix";
        GrSLType varyingType;

        GrCoordSet coordType = processor->coordTransform(t).sourceCoords();
        const SkMatrix& localMatrix = fOptState.getPrimitiveProcessor()->localMatrix();
        uint32_t type = processor->coordTransform(t).getMatrix().getType();
        if (kLocal_GrCoordSet == coordType) {
            type |= localMatrix.getType();
        }
        varyingType = SkToBool(SkMatrix::kPerspective_Mask & type) ? kVec3f_GrSLType :
                                                                     kVec2f_GrSLType;
        GrSLPrecision precision = processor->coordTransform(t).precision();

        SkString suffixedUniName;
        if (0 != t) {
            suffixedUniName.append(uniName);
            suffixedUniName.appendf("_%i", t);
            uniName = suffixedUniName.c_str();
        }
        ifp->fTransforms[t].fHandle = this->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                                       kMat33f_GrSLType, precision,
                                                       uniName,
                                                       &uniName).toShaderBuilderIndex();

        const char* varyingName = "MatrixCoord";
        SkString suffixedVaryingName;
        if (0 != t) {
            suffixedVaryingName.append(varyingName);
            suffixedVaryingName.appendf("_%i", t);
            varyingName = suffixedVaryingName.c_str();
        }

        GrGLVertToFrag v(varyingType);
        this->addVarying(varyingName, &v, precision);
        fCoordVaryings.push_back(TransformVarying(v, uniName, coordType));

        SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLProcessor::TransformedCoords,
                               (SkString(v.fsIn()), varyingType));
    }
}

void GrGLProgramBuilder::emitSamplers(const GrProcessor& processor,
                                      GrGLProcessor::TextureSamplerArray* outSamplers,
                                      GrGLInstalledProc* ip) {
    int numTextures = processor.numTextures();
    ip->fSamplers.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("Sampler%d", t);
        ip->fSamplers[t].fUniform = this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                     kSampler2D_GrSLType, kDefault_GrSLPrecision,
                                                     name.c_str());
        SkNEW_APPEND_TO_TARRAY(outSamplers, GrGLProcessor::TextureSampler,
                               (ip->fSamplers[t].fUniform, processor.textureAccess(t)));
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
    if (!(GrGLProgramDescBuilder::GetHeader(fDesc).fUseNvpr &&
          fGpu->glPathRendering()->texturingMode() ==
          GrGLPathRendering::FixedFunction_TexturingMode)) {
        if (!fVS.compileAndAttachShaders(programID, &shadersToDelete)) {
            this->cleanupProgram(programID, shadersToDelete);
            return NULL;
        }

        // Non fixed function NVPR actually requires a vertex shader to compile
        if (fOptState.hasGeometryProcessor()) {
            fVS.bindVertexAttributes(programID);
        }
    }

    if (!fFS.compileAndAttachShaders(programID, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return NULL;
    }

    bool usingBindUniform = fGpu->glInterface()->fFunctions.fBindUniformLocation != NULL;
    if (usingBindUniform) {
        this->bindUniformLocations(programID);
    }
    fFS.bindFragmentShaderLocations(programID);
    GL_CALL(LinkProgram(programID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = !fGpu->ctxInfo().isChromium();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        checkLinkStatus(programID);
    }
    if (!usingBindUniform) {
        this->resolveUniformLocations(programID);
    }

    this->cleanupShaders(shadersToDelete);

    return this->createProgram(programID);
}

void GrGLProgramBuilder::bindUniformLocations(GrGLuint programID) {
    int count = fUniforms.count();
    for (int i = 0; i < count; ++i) {
        GL_CALL(BindUniformLocation(programID, i, fUniforms[i].fVariable.c_str()));
        fUniforms[i].fLocation = i;
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
            SkDebugf((char*)log.get());
        }
        SkDEBUGFAIL("Error linking program");
        GL_CALL(DeleteProgram(programID));
        programID = 0;
    }
    return SkToBool(linked);
}

void GrGLProgramBuilder::resolveUniformLocations(GrGLuint programID) {
    int count = fUniforms.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location, GetUniformLocation(programID, fUniforms[i].fVariable.c_str()));
        fUniforms[i].fLocation = location;
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
    return SkNEW_ARGS(GrGLProgram, (fGpu, fDesc, fUniformHandles, programID, fUniforms,
                                    fGeometryProcessor, fXferProcessor, fFragmentProcessors.get()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLInstalledFragProcs::~GrGLInstalledFragProcs() {
    int numProcs = fProcs.count();
    for (int e = 0; e < numProcs; ++e) {
        SkDELETE(fProcs[e]);
    }
}
