/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"

// number of each input/output type in a single allocation block
static const int kVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int kMaxFSOutputs = 2;

// Architectural assumption: always 2-d input coords.
// Likely to become non-constant and non-static, perhaps even
// varying by stage, if we use 1D textures for gradients!
//const int GrGLShaderBuilder::fCoordDims = 2;

GrGLShaderBuilder::GrGLShaderBuilder(const GrGLContextInfo& ctx)
    : fVSUnis(kVarsPerBlock)
    , fVSAttrs(kVarsPerBlock)
    , fVSOutputs(kVarsPerBlock)
    , fGSInputs(kVarsPerBlock)
    , fGSOutputs(kVarsPerBlock)
    , fFSInputs(kVarsPerBlock)
    , fFSUnis(kVarsPerBlock)
    , fFSOutputs(kMaxFSOutputs)
    , fUsesGS(false)
    , fVaryingDims(0)
    , fComplexCoord(false)
    , fContext(ctx) {

}

void GrGLShaderBuilder::computeSwizzle(uint32_t configFlags) {
   static const uint32_t kMulByAlphaMask =
        (GrGLProgram::StageDesc::kMulRGBByAlpha_RoundUp_InConfigFlag |
         GrGLProgram::StageDesc::kMulRGBByAlpha_RoundDown_InConfigFlag);

    fSwizzle = "";
    if (configFlags & GrGLProgram::StageDesc::kSwapRAndB_InConfigFlag) {
        GrAssert(!(configFlags &
                   GrGLProgram::StageDesc::kSmearAlpha_InConfigFlag));
        GrAssert(!(configFlags &
                   GrGLProgram::StageDesc::kSmearRed_InConfigFlag));
        fSwizzle = ".bgra";
    } else if (configFlags & GrGLProgram::StageDesc::kSmearAlpha_InConfigFlag) {
        GrAssert(!(configFlags & kMulByAlphaMask));
        GrAssert(!(configFlags &
                   GrGLProgram::StageDesc::kSmearRed_InConfigFlag));
        fSwizzle = ".aaaa";
    } else if (configFlags & GrGLProgram::StageDesc::kSmearRed_InConfigFlag) {
        GrAssert(!(configFlags & kMulByAlphaMask));
        GrAssert(!(configFlags &
                   GrGLProgram::StageDesc::kSmearAlpha_InConfigFlag));
        fSwizzle = ".rrrr";
    }
}

void GrGLShaderBuilder::computeModulate(const char* fsInColor) {
    if (NULL != fsInColor) {
        fModulate.printf(" * %s", fsInColor);
    } else {
        fModulate.reset();
    }
}

void GrGLShaderBuilder::setupTextureAccess(SamplerMode samplerMode,
                                           int stageNum) {
    SkString retval;

    fTexFunc = "texture2D";
    switch (samplerMode) {
        case kDefault_SamplerMode:
            GrAssert(fVaryingDims == fCoordDims);
            // Do nothing
            break;
        case kProj_SamplerMode:
            fTexFunc.append("Proj");
            break;
        case kExplicitDivide_SamplerMode:
            retval = "inCoord";
            retval.appendS32(stageNum);
            fFSCode.appendf("\t%s %s = %s%s / %s%s;\n",
                GrGLShaderVar::TypeString
                    (GrSLFloatVectorType(fCoordDims)),
                retval.c_str(),
                fSampleCoords.c_str(),
                GrGLSLVectorNonhomogCoords(fVaryingDims),
                fSampleCoords.c_str(),
                GrGLSLVectorHomogCoord(fVaryingDims));
            fSampleCoords = retval;
            break;
    }
    fComplexCoord = false;
}

void GrGLShaderBuilder::emitTextureLookup(const char* samplerName,
                                          const char* coordName) {
    if (NULL == coordName) {
        coordName = fSampleCoords.c_str();
    }
    fFSCode.appendf("%s(%s, %s)", fTexFunc.c_str(), samplerName, coordName);
}

void GrGLShaderBuilder::emitDefaultFetch(const char* outColor,
                                         const char* samplerName) {
    fFSCode.appendf("\t%s = ", outColor);
    this->emitTextureLookup(samplerName);
    fFSCode.appendf("%s%s;\n", fSwizzle.c_str(), fModulate.c_str());
}

const GrGLShaderVar& GrGLShaderBuilder::addUniform(uint32_t visibility,
                                                   GrSLType type,
                                                   const char* name,
                                                   int stageNum,
                                                   int count) {
    GrAssert(name && strlen(name));
    static const uint32_t kVisibilityMask = kVertex_ShaderType | kFragment_ShaderType;
    GrAssert(0 == (~kVisibilityMask & visibility));
    GrAssert(0 != visibility);

    GrGLShaderVar* var = NULL;
    if (kVertex_ShaderType & visibility) {
        var = &fVSUnis.push_back();
    } else {
        GrAssert(kFragment_ShaderType & visibility);
        var = &fFSUnis.push_back();
    }
    var->setType(type);
    var->setTypeModifier(GrGLShaderVar::kUniform_TypeModifier);
    var->setName(name);
    if (stageNum >= 0) {
        var->accessName()->appendS32(stageNum);
    }
    var->setArrayCount(count);

    if ((kVertex_ShaderType | kFragment_ShaderType) == visibility) {
        fFSUnis.push_back(*var);
        // If it's shared between VS and FS, VS must override
        // default highp and specify mediump.
        var->setEmitPrecision(true);
    }

    return *var;
}

void GrGLShaderBuilder::addVarying(GrSLType type,
                                   const char* name,
                                   const char** vsOutName,
                                   const char** fsInName) {
    fVSOutputs.push_back();
    fVSOutputs.back().setType(type);
    fVSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
    fVSOutputs.back().accessName()->printf("v%s", name);
    if (vsOutName) {
        *vsOutName = fVSOutputs.back().getName().c_str();
    }
    // input to FS comes either from VS or GS
    const SkString* fsName;
    if (fUsesGS) {
        // if we have a GS take each varying in as an array
        // and output as non-array.
        fGSInputs.push_back();
        fGSInputs.back().setType(type);
        fGSInputs.back().setTypeModifier(GrGLShaderVar::kIn_TypeModifier);
        fGSInputs.back().setUnsizedArray();
        *fGSInputs.back().accessName() = fVSOutputs.back().getName();
        fGSOutputs.push_back();
        fGSOutputs.back().setType(type);
        fGSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
        fGSOutputs.back().accessName()->printf("g%s", name);
        fsName = fGSOutputs.back().accessName();
    } else {
        fsName = fVSOutputs.back().accessName();
    }
    fFSInputs.push_back();
    fFSInputs.back().setType(type);
    fFSInputs.back().setTypeModifier(GrGLShaderVar::kIn_TypeModifier);
    fFSInputs.back().setName(*fsName);
    if (fsInName) {
        *fsInName = fsName->c_str();
    }
}


void GrGLShaderBuilder::addVarying(GrSLType type,
                                   const char* name,
                                   int stageNum,
                                   const char** vsOutName,
                                   const char** fsInName) {
    SkString nameWithStage(name);
    nameWithStage.appendS32(stageNum);
    this->addVarying(type, nameWithStage.c_str(), vsOutName, fsInName);
}

namespace {
void append_decls(const GrGLShaderBuilder::VarArray& vars,
                  const GrGLContextInfo& ctx,
                  SkString* string) {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(ctx, string);
    }
}
}

void GrGLShaderBuilder::getShader(ShaderType type, SkString* shaderStr) const {
    switch (type) {
        case kVertex_ShaderType:
            *shaderStr = fHeader;
            append_decls(fVSUnis, fContext, shaderStr);
            append_decls(fVSAttrs, fContext, shaderStr);
            append_decls(fVSOutputs, fContext, shaderStr);
            shaderStr->append(fVSCode);
            break;
        case kGeometry_ShaderType:
            if (fUsesGS) {
                *shaderStr = fHeader;
                shaderStr->append(fGSHeader);
                append_decls(fGSInputs, fContext, shaderStr);
                append_decls(fGSOutputs, fContext, shaderStr);
                shaderStr->append(fGSCode);
            } else {
                shaderStr->reset();
            }
            break;
        case kFragment_ShaderType:
            *shaderStr = fHeader;
            shaderStr->append(GrGetGLSLShaderPrecisionDecl(fContext.binding()));
            append_decls(fFSUnis, fContext, shaderStr);
            append_decls(fFSInputs, fContext, shaderStr);
            // We shouldn't have declared outputs on 1.10
            GrAssert(k110_GrGLSLGeneration != fContext.glslGeneration() || fFSOutputs.empty());
            append_decls(fFSOutputs, fContext, shaderStr);
            shaderStr->append(fFSFunctions);
            shaderStr->append(fFSCode);
            break;
    }

 }
