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
#include "../GrGpuGL.h"
#include "GrCoordTransform.h"
#include "GrGLLegacyNvprProgramBuilder.h"
#include "GrGLNvprProgramBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;

//////////////////////////////////////////////////////////////////////////////

const int GrGLProgramBuilder::kVarsPerBlock = 8;

GrGLProgram* GrGLProgramBuilder::CreateProgram(const GrOptDrawState& optState,
                                               GrGpu::DrawType drawType,
                                               GrGpuGL* gpu) {
    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    SkAutoTDelete<GrGLProgramBuilder> builder(CreateProgramBuilder(optState,
                                                                   drawType,
                                                                   optState.hasGeometryProcessor(),
                                                                   gpu));

    GrGLProgramBuilder* pb = builder.get();
    const GrGLProgramDescBuilder::GLKeyHeader& header = GrGLProgramDescBuilder::GetHeader(pb->desc());

    // emit code to read the dst copy texture, if necessary
    if (GrGLFragmentShaderBuilder::kNoDstRead_DstReadKey != header.fDstReadKey
            && !gpu->glCaps().fbFetchSupport()) {
        pb->fFS.emitCodeToReadDstTexture();
    }

    // get the initial color and coverage to feed into the first effect in each effect chain
    GrGLSLExpr4 inputColor;
    GrGLSLExpr1 inputCoverage;
    pb->setupUniformColorAndCoverageIfNeeded(&inputColor,  &inputCoverage);

    // if we have a vertex shader(we don't only if we are using NVPR or NVPR ES), then we may have
    // to setup a few more things like builtin vertex attributes
    bool hasVertexShader = !(header.fUseNvpr &&
                             gpu->glPathRendering()->texturingMode() ==
                             GrGLPathRendering::FixedFunction_TexturingMode);
    if (hasVertexShader) {
        pb->fVS.setupLocalCoords();
        pb->fVS.transformGLToSkiaCoords();
        if (header.fEmitsPointSize) {
            pb->fVS.codeAppend("gl_PointSize = 1.0;");
        }
        if (GrProgramDesc::kAttribute_ColorInput == header.fColorInput) {
            pb->fVS.setupBuiltinVertexAttribute("Color", &inputColor);
        }
        if (GrProgramDesc::kAttribute_ColorInput == header.fCoverageInput) {
            pb->fVS.setupBuiltinVertexAttribute("Coverage", &inputCoverage);
        }
    }

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // remove this cast to a vec4.
    GrGLSLExpr4 inputCoverageVec4 = GrGLSLExpr4::VectorCast(inputCoverage);

    pb->emitAndInstallProcs(optState, &inputColor, &inputCoverageVec4);

    if (hasVertexShader) {
        pb->fVS.transformSkiaToGLCoords();
    }

    // write the secondary color output if necessary
    if (GrProgramDesc::kNone_SecondaryOutputType != header.fSecondaryOutputType) {
        pb->fFS.enableSecondaryOutput(inputColor, inputCoverageVec4);
    }

    pb->fFS.combineColorAndCoverage(inputColor, inputCoverageVec4);

    return pb->finalize();
}

GrGLProgramBuilder*
GrGLProgramBuilder::CreateProgramBuilder(const GrOptDrawState& optState,
                                         GrGpu::DrawType drawType,
                                         bool hasGeometryProcessor,
                                         GrGpuGL* gpu) {
    const GrProgramDesc& desc = optState.programDesc();
    if (GrGLProgramDescBuilder::GetHeader(desc).fUseNvpr) {
        SkASSERT(gpu->glCaps().pathRenderingSupport());
        SkASSERT(GrProgramDesc::kAttribute_ColorInput != desc.header().fColorInput);
        SkASSERT(GrProgramDesc::kAttribute_ColorInput != desc.header().fCoverageInput);
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

GrGLProgramBuilder::GrGLProgramBuilder(GrGpuGL* gpu, const GrOptDrawState& optState)
    : fVS(this)
    , fGS(this)
    , fFS(this, optState.programDesc().header().fFragPosKey)
    , fOutOfStage(true)
    , fStageIndex(-1)
    , fGeometryProcessor(NULL)
    , fOptState(optState)
    , fDesc(optState.programDesc())
    , fGpu(gpu)
    , fUniforms(kVarsPerBlock) {
}

void GrGLProgramBuilder::addVarying(const char* name,
                                    GrGLVarying* varying,
                                    GrGLShaderVar::Precision fsPrecision) {
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

GrGLProgramDataManager::UniformHandle GrGLProgramBuilder::addUniformArray(uint32_t visibility,
                                                                          GrSLType type,
                                                                          const char* name,
                                                                          int count,
                                                                          const char** outName) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_Visibility | kFragment_Visibility);
    SkASSERT(0 == (~kVisibilityMask & visibility));
    SkASSERT(0 != visibility);

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
    uni.fVariable.setTypeModifier(GrGLShaderVar::kUniform_TypeModifier);
    this->nameVariable(uni.fVariable.accessName(), 'u', name);
    uni.fVariable.setArrayCount(count);
    uni.fVisibility = visibility;

    // If it is visible in both the VS and FS, the precision must match.
    // We declare a default FS precision, but not a default VS. So set the var
    // to use the default FS precision.
    if ((kVertex_Visibility | kFragment_Visibility) == visibility) {
        // the fragment and vertex precisions must match
        uni.fVariable.setPrecision(kDefaultFragmentPrecision);
    }

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

void GrGLProgramBuilder::setupUniformColorAndCoverageIfNeeded(GrGLSLExpr4* inputColor,
                                                              GrGLSLExpr1* inputCoverage) {
    const GrProgramDesc::KeyHeader& header = this->header();
    if (GrProgramDesc::kUniform_ColorInput == header.fColorInput) {
        const char* name;
        fUniformHandles.fColorUni =
            this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                             kVec4f_GrSLType,
                             "Color",
                             &name);
        *inputColor = GrGLSLExpr4(name);
    } else if (GrProgramDesc::kAllOnes_ColorInput == header.fColorInput) {
        *inputColor = GrGLSLExpr4(1);
    }
    if (GrProgramDesc::kUniform_ColorInput == header.fCoverageInput) {
        const char* name;
        fUniformHandles.fCoverageUni =
            this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                             kFloat_GrSLType,
                             "Coverage",
                             &name);
        *inputCoverage = GrGLSLExpr1(name);
    } else if (GrProgramDesc::kAllOnes_ColorInput == header.fCoverageInput) {
        *inputCoverage = GrGLSLExpr1(1);
    }
}

void GrGLProgramBuilder::emitAndInstallProcs(const GrOptDrawState& optState,
                                             GrGLSLExpr4* inputColor,
                                             GrGLSLExpr4* inputCoverage) {
    fFragmentProcessors.reset(SkNEW(GrGLInstalledFragProcs));
    int numProcs = optState.numFragmentStages();
    this->emitAndInstallFragProcs(0, optState.numColorStages(), inputColor);
    if (optState.hasGeometryProcessor()) {
        const GrGeometryProcessor& gp = *optState.getGeometryProcessor();
        fVS.emitAttributes(gp);
        ProcKeyProvider keyProvider(&fDesc,
                                    ProcKeyProvider::kGeometry_ProcessorType,
                                    GrGLProgramDescBuilder::kProcessorKeyOffsetsAndLengthOffset);
        GrGLSLExpr4 output;
        this->emitAndInstallProc<GrGeometryProcessor>(gp, 0, keyProvider, *inputCoverage, &output);
        *inputCoverage = output;
    }
    this->emitAndInstallFragProcs(optState.numColorStages(), numProcs,  inputCoverage);
}

void GrGLProgramBuilder::emitAndInstallFragProcs(int procOffset, int numProcs, GrGLSLExpr4* inOut) {
    ProcKeyProvider keyProvider(&fDesc,
                                ProcKeyProvider::kFragment_ProcessorType,
                                GrGLProgramDescBuilder::kProcessorKeyOffsetsAndLengthOffset);
    for (int e = procOffset; e < numProcs; ++e) {
        GrGLSLExpr4 output;
        const GrFragmentStage& stage = fOptState.getFragmentStage(e);
        this->emitAndInstallProc<GrFragmentStage>(stage, e, keyProvider, *inOut, &output);
        *inOut = output;
    }
}

// TODO Processors cannot output zeros because an empty string is all 1s
// the fix is to allow effects to take the GrGLSLExpr4 directly
template <class Proc>
void GrGLProgramBuilder::emitAndInstallProc(const Proc& proc,
                                            int index,
                                            const ProcKeyProvider& keyProvider,
                                            const GrGLSLExpr4& input,
                                            GrGLSLExpr4* output) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);

    // create var to hold stage result
    SkString outColorName;
    this->nameVariable(&outColorName, '\0', "output");
    fFS.codeAppendf("vec4 %s;", outColorName.c_str());
    *output = outColorName;

    // Enclose custom code in a block to avoid namespace conflicts
    SkString openBrace;
    openBrace.printf("{ // Stage %d\n", fStageIndex);
    fFS.codeAppend(openBrace.c_str());

    this->emitAndInstallProc(proc, keyProvider.get(index), output->c_str(),
                             input.isOnes() ? NULL : input.c_str());

    fFS.codeAppend("}");
}

void GrGLProgramBuilder::emitAndInstallProc(const GrFragmentStage& fs,
                                            const GrProcessorKey& key,
                                            const char* outColor,
                                            const char* inColor) {
    GrGLInstalledFragProc* ifp = SkNEW_ARGS(GrGLInstalledFragProc, (fVS.hasLocalCoords()));

    const GrFragmentProcessor& fp = *fs.getProcessor();
    ifp->fGLProc.reset(fp.getFactory().createGLInstance(fp));

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(fp.numTextures());
    this->emitSamplers(fp, &samplers, ifp);

    // Fragment processors can have coord transforms
    SkSTArray<2, GrGLProcessor::TransformedCoords> coords(fp.numTransforms());
    this->emitTransforms(fs, &coords, ifp);

    ifp->fGLProc->emitCode(this, fp, key, outColor, inColor, coords, samplers);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(fp);
    fFragmentProcessors->fProcs.push_back(ifp);
}

void GrGLProgramBuilder::emitAndInstallProc(const GrGeometryProcessor& gp,
                                            const GrProcessorKey& key,
                                            const char* outColor,
                                            const char* inColor) {
    SkASSERT(!fGeometryProcessor);
    fGeometryProcessor = SkNEW(GrGLInstalledGeoProc);

    fGeometryProcessor->fGLProc.reset(gp.getFactory().createGLInstance(gp));

    SkSTArray<4, GrGLProcessor::TextureSampler> samplers(gp.numTextures());
    this->emitSamplers(gp, &samplers, fGeometryProcessor);

    GrGLGeometryProcessor::EmitArgs args(this, gp, key, outColor, inColor, samplers);
    fGeometryProcessor->fGLProc->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    verify(gp);
}

void GrGLProgramBuilder::verify(const GrGeometryProcessor& gp) {
    SkASSERT(fFS.hasReadFragmentPosition() == gp.willReadFragmentPosition());
}

void GrGLProgramBuilder::verify(const GrFragmentProcessor& fp) {
    SkASSERT(fFS.hasReadFragmentPosition() == fp.willReadFragmentPosition());
    SkASSERT(fFS.hasReadDstColor() == fp.willReadDstColor());
}

void GrGLProgramBuilder::emitTransforms(const GrFragmentStage& effectStage,
                                        GrGLProcessor::TransformedCoordsArray* outCoords,
                                        GrGLInstalledFragProc* ifp) {
    const GrFragmentProcessor* effect = effectStage.getProcessor();
    int numTransforms = effect->numTransforms();
    ifp->fTransforms.push_back_n(numTransforms);

    for (int t = 0; t < numTransforms; t++) {
        const char* uniName = "StageMatrix";
        GrSLType varyingType =
                effectStage.isPerspectiveCoordTransform(t, fVS.hasLocalCoords()) ?
                        kVec3f_GrSLType :
                        kVec2f_GrSLType;

        SkString suffixedUniName;
        if (0 != t) {
            suffixedUniName.append(uniName);
            suffixedUniName.appendf("_%i", t);
            uniName = suffixedUniName.c_str();
        }
        ifp->fTransforms[t].fHandle = this->addUniform(GrGLProgramBuilder::kVertex_Visibility,
                                                       kMat33f_GrSLType,
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
        this->addVarying(varyingName, &v);

        const GrGLShaderVar& coords =
                kPosition_GrCoordSet == effect->coordTransform(t).sourceCoords() ?
                                          fVS.positionAttribute() :
                                          fVS.localCoordsAttribute();

        // varying = matrix * coords (logically)
        SkASSERT(kVec2f_GrSLType == varyingType || kVec3f_GrSLType == varyingType);
        if (kVec2f_GrSLType == varyingType) {
            fVS.codeAppendf("%s = (%s * vec3(%s, 1)).xy;",
                            v.vsOut(), uniName, coords.c_str());
        } else {
            fVS.codeAppendf("%s = %s * vec3(%s, 1);",
                            v.vsOut(), uniName, coords.c_str());
        }
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
                                                     kSampler2D_GrSLType,
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
    if (!fFS.compileAndAttachShaders(programID, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return NULL;
    }
    if (!(GrGLProgramDescBuilder::GetHeader(fDesc).fUseNvpr &&
          fGpu->glPathRendering()->texturingMode() ==
          GrGLPathRendering::FixedFunction_TexturingMode)) {
        if (!fVS.compileAndAttachShaders(programID, &shadersToDelete)) {
            this->cleanupProgram(programID, shadersToDelete);
            return NULL;
        }
        fVS.bindVertexAttributes(programID);
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
                                    fGeometryProcessor, fFragmentProcessors.get()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLInstalledFragProcs::~GrGLInstalledFragProcs() {
    int numProcs = fProcs.count();
    for (int e = 0; e < numProcs; ++e) {
        SkDELETE(fProcs[e]);
    }
}
