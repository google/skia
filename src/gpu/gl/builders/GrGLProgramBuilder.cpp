/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLProgram.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "gl/GrGLUniformHandle.h"
#include "GrCoordTransform.h"
#include "GrDrawEffect.h"
#include "../GrGpuGL.h"
#include "GrGLFragmentShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrTexture.h"
#include "GrGLVertexShaderBuilder.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

namespace {
#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

// number of each input/output type in a single allocation block
static const int kVarsPerBlock = 8;

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool GrGLProgramBuilder::genProgram(const GrEffectStage* colorStages[],
                                    const GrEffectStage* coverageStages[]) {
    const GrGLProgramDesc::KeyHeader& header = this->desc().getHeader();

    fFS.emitCodeBeforeEffects();

    ///////////////////////////////////////////////////////////////////////////
    // get the initial color and coverage to feed into the first effect in each effect chain

    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (GrGLProgramDesc::kUniform_ColorInput == header.fColorInput) {
        const char* name;
        fUniformHandles.fColorUni =
            this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                             kVec4f_GrSLType,
                             "Color",
                             &name);
        inputColor = GrGLSLExpr4(name);
    }

    if (GrGLProgramDesc::kUniform_ColorInput == header.fCoverageInput) {
        const char* name;
        fUniformHandles.fCoverageUni =
            this->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                             kVec4f_GrSLType,
                             "Coverage",
                             &name);
        inputCoverage = GrGLSLExpr4(name);
    } else if (GrGLProgramDesc::kSolidWhite_ColorInput == header.fCoverageInput) {
        inputCoverage = GrGLSLExpr4(1);
    }

    this->emitCodeBeforeEffects(&inputColor, &inputCoverage);

    ///////////////////////////////////////////////////////////////////////////
    // emit the per-effect code for both color and coverage effects

    GrGLProgramDesc::EffectKeyProvider colorKeyProvider(
        &this->desc(), GrGLProgramDesc::EffectKeyProvider::kColor_EffectType);
    fColorEffects.reset(this->createAndEmitEffects(colorStages,
                                                   this->desc().numColorEffects(),
                                                   colorKeyProvider,
                                                   &inputColor));

    GrGLProgramDesc::EffectKeyProvider coverageKeyProvider(
        &this->desc(), GrGLProgramDesc::EffectKeyProvider::kCoverage_EffectType);
    fCoverageEffects.reset(this->createAndEmitEffects(coverageStages,
                                                      this->desc().numCoverageEffects(),
                                                      coverageKeyProvider,
                                                      &inputCoverage));

    this->emitCodeAfterEffects();

    fFS.emitCodeAfterEffects(inputColor, inputCoverage);

    if (!this->finish()) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGpuGL* gpu,
                                       const GrGLProgramDesc& desc)
    : fFragOnly(!desc.getHeader().fRequiresVertexShader &&
                gpu->glCaps().pathRenderingSupport() &&
                gpu->glPathRendering()->texturingMode() == GrGLPathRendering::FixedFunction_TexturingMode)
    , fTexCoordSetCnt(0)
    , fProgramID(0)
    , fFS(this, desc)
    , fSeparableVaryingInfos(kVarsPerBlock)
    , fDesc(desc)
    , fGpu(gpu)
    , fUniforms(kVarsPerBlock) {
}

void GrGLProgramBuilder::nameVariable(SkString* out, char prefix, const char* name) {
    if ('\0' == prefix) {
        *out = name;
    } else {
        out->printf("%c%s", prefix, name);
    }
    if (fCodeStage.inStageCode()) {
        if (out->endsWith('_')) {
            // Names containing "__" are reserved.
            out->append("x");
        }
        out->appendf("_Stage%d", fCodeStage.stageIndex());
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

    if (NULL != outName) {
        *outName = uni.fVariable.c_str();
    }
    return GrGLProgramDataManager::UniformHandle::CreateFromUniformIndex(fUniforms.count() - 1);
}

void GrGLProgramBuilder::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(this->ctxInfo(), out);
        out->append(";\n");
    }
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

void GrGLProgramBuilder::createAndEmitEffects(GrGLProgramEffectsBuilder* programEffectsBuilder,
                                              const GrEffectStage* effectStages[],
                                              int effectCnt,
                                              const GrGLProgramDesc::EffectKeyProvider& keyProvider,
                                              GrGLSLExpr4* fsInOutColor) {
    bool effectEmitted = false;

    GrGLSLExpr4 inColor = *fsInOutColor;
    GrGLSLExpr4 outColor;

    for (int e = 0; e < effectCnt; ++e) {
        SkASSERT(NULL != effectStages[e] && NULL != effectStages[e]->getEffect());
        const GrEffectStage& stage = *effectStages[e];

        CodeStage::AutoStageRestore csar(&fCodeStage, &stage);

        if (inColor.isZeros()) {
            SkString inColorName;

            // Effects have no way to communicate zeros, they treat an empty string as ones.
            this->nameVariable(&inColorName, '\0', "input");
            fFS.codeAppendf("\tvec4 %s = %s;\n", inColorName.c_str(), inColor.c_str());
            inColor = inColorName;
        }

        // create var to hold stage result
        SkString outColorName;
        this->nameVariable(&outColorName, '\0', "output");
        fFS.codeAppendf("\tvec4 %s;\n", outColorName.c_str());
        outColor = outColorName;


        programEffectsBuilder->emitEffect(stage,
                                          keyProvider.get(e),
                                          outColor.c_str(),
                                          inColor.isOnes() ? NULL : inColor.c_str(),
                                          fCodeStage.stageIndex());

        inColor = outColor;
        effectEmitted = true;
    }

    if (effectEmitted) {
        *fsInOutColor = outColor;
    }
}

bool GrGLProgramBuilder::finish() {
    SkASSERT(0 == fProgramID);
    GL_CALL_RET(fProgramID, CreateProgram());
    if (!fProgramID) {
        return false;
    }

    SkTDArray<GrGLuint> shadersToDelete;

    if (!this->compileAndAttachShaders(fProgramID, &shadersToDelete)) {
        GL_CALL(DeleteProgram(fProgramID));
        return false;
    }

    this->bindProgramLocations(fProgramID);

    GL_CALL(LinkProgram(fProgramID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = !fGpu->ctxInfo().isChromium();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        GrGLint linked = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(fProgramID, GR_GL_LINK_STATUS, &linked));
        if (!linked) {
            GrGLint infoLen = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramiv(fProgramID, GR_GL_INFO_LOG_LENGTH, &infoLen));
            SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
            if (infoLen > 0) {
                // retrieve length even though we don't need it to workaround
                // bug in chrome cmd buffer param validation.
                GrGLsizei length = GR_GL_INIT_ZERO;
                GL_CALL(GetProgramInfoLog(fProgramID,
                                          infoLen+1,
                                          &length,
                                          (char*)log.get()));
                GrPrintf((char*)log.get());
            }
            SkDEBUGFAIL("Error linking program");
            GL_CALL(DeleteProgram(fProgramID));
            fProgramID = 0;
            return false;
        }
    }

    this->resolveProgramLocations(fProgramID);

    for (int i = 0; i < shadersToDelete.count(); ++i) {
      GL_CALL(DeleteShader(shadersToDelete[i]));
    }

    return true;
}

bool GrGLProgramBuilder::compileAndAttachShaders(GrGLuint programId,
                                                 SkTDArray<GrGLuint>* shaderIds) const {
    return fFS.compileAndAttachShaders(programId, shaderIds);
}

void GrGLProgramBuilder::bindProgramLocations(GrGLuint programId) {
    fFS.bindProgramLocations(programId);

    // skbug.com/2056
    bool usingBindUniform = fGpu->glInterface()->fFunctions.fBindUniformLocation != NULL;
    if (usingBindUniform) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GL_CALL(BindUniformLocation(programId, i, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = i;
        }
    }
}

void GrGLProgramBuilder::resolveProgramLocations(GrGLuint programId) {
    bool usingBindUniform = fGpu->glInterface()->fFunctions.fBindUniformLocation != NULL;
    if (!usingBindUniform) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GrGLint location;
            GL_CALL_RET(location,
                        GetUniformLocation(programId, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = location;
        }
    }

    int count = fSeparableVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location,
                    GetProgramResourceLocation(programId,
                                               GR_GL_FRAGMENT_INPUT,
                                               fSeparableVaryingInfos[i].fVariable.c_str()));
        fSeparableVaryingInfos[i].fLocation = location;
    }
}

const GrGLContextInfo& GrGLProgramBuilder::ctxInfo() const {
    return fGpu->ctxInfo();
}

////////////////////////////////////////////////////////////////////////////////

GrGLFullProgramBuilder::GrGLFullProgramBuilder(GrGpuGL* gpu,
                                              const GrGLProgramDesc& desc)
    : INHERITED(gpu, desc)
    , fGS(this)
    , fVS(this) {
}

void GrGLFullProgramBuilder::emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) {
    fVS.emitCodeBeforeEffects(color, coverage);
}

void GrGLFullProgramBuilder::emitCodeAfterEffects() {
    fVS.emitCodeAfterEffects();
}

void GrGLFullProgramBuilder::addVarying(GrSLType type,
                                        const char* name,
                                        const char** vsOutName,
                                        const char** fsInName) {
    fVS.addVarying(type, name, vsOutName);

    SkString* fsInputName = fVS.fOutputs.back().accessName();

#if GR_GL_EXPERIMENTAL_GS
    if (desc().getHeader().fExperimentalGS) {
       // TODO let the caller use these names
       fGS.addVarying(type, fsInputName->c_str(), NULL);
       fsInputName = fGS.fOutputs.back().accessName();
    }
#endif
    fFS.addVarying(type, fsInputName->c_str(), fsInName);
}

GrGLFullProgramBuilder::VaryingHandle
GrGLFullProgramBuilder::addSeparableVarying(GrSLType type,
                                            const char* name,
                                            const char** vsOutName,
                                            const char** fsInName) {
    addVarying(type, name, vsOutName, fsInName);
    SeparableVaryingInfo& varying = fSeparableVaryingInfos.push_back();
    varying.fVariable = fFS.fInputs.back();
    return VaryingHandle::CreateFromSeparableVaryingIndex(fSeparableVaryingInfos.count() - 1);
}


GrGLProgramEffects* GrGLFullProgramBuilder::createAndEmitEffects(
        const GrEffectStage* effectStages[],
        int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider,
        GrGLSLExpr4* inOutFSColor) {

    GrGLVertexProgramEffectsBuilder programEffectsBuilder(this, effectCnt);
    this->INHERITED::createAndEmitEffects(&programEffectsBuilder,
                                          effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return programEffectsBuilder.finish();
}

bool GrGLFullProgramBuilder::compileAndAttachShaders(GrGLuint programId,
                                                     SkTDArray<GrGLuint>* shaderIds) const {
    return INHERITED::compileAndAttachShaders(programId, shaderIds)
         && fVS.compileAndAttachShaders(programId, shaderIds)
#if GR_GL_EXPERIMENTAL_GS
         && (!desc().getHeader().fExperimentalGS
                 || fGS.compileAndAttachShaders(programId, shaderIds))
#endif
         ;
}

void GrGLFullProgramBuilder::bindProgramLocations(GrGLuint programId) {
    fVS.bindProgramLocations(programId);
    INHERITED::bindProgramLocations(programId);
}

////////////////////////////////////////////////////////////////////////////////

GrGLFragmentOnlyProgramBuilder::GrGLFragmentOnlyProgramBuilder(GrGpuGL* gpu,
                                                               const GrGLProgramDesc& desc)
    : INHERITED(gpu, desc) {
    SkASSERT(!desc.getHeader().fRequiresVertexShader);
    SkASSERT(gpu->glCaps().pathRenderingSupport());
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fColorInput);
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fCoverageInput);
}

int GrGLFragmentOnlyProgramBuilder::addTexCoordSets(int count) {
    int firstFreeCoordSet = fTexCoordSetCnt;
    fTexCoordSetCnt += count;
    SkASSERT(gpu()->glCaps().maxFixedFunctionTextureCoords() >= fTexCoordSetCnt);
    return firstFreeCoordSet;
}

GrGLProgramEffects* GrGLFragmentOnlyProgramBuilder::createAndEmitEffects(
        const GrEffectStage* effectStages[], int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider, GrGLSLExpr4* inOutFSColor) {

    GrGLPathTexGenProgramEffectsBuilder pathTexGenEffectsBuilder(this,
                                                                 effectCnt);
    this->INHERITED::createAndEmitEffects(&pathTexGenEffectsBuilder,
                                          effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return pathTexGenEffectsBuilder.finish();
}
