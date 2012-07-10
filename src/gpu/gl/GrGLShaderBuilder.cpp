/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"

namespace {

// number of each input/output type in a single allocation block
static const int sVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int sMaxFSOutputs = 2;

}

// Architectural assumption: always 2-d input coords.
// Likely to become non-constant and non-static, perhaps even
// varying by stage, if we use 1D textures for gradients!
//const int GrGLShaderBuilder::fCoordDims = 2;

GrGLShaderBuilder::GrGLShaderBuilder()
    : fVSUnis(sVarsPerBlock)
    , fVSAttrs(sVarsPerBlock)
    , fVSOutputs(sVarsPerBlock)
    , fGSInputs(sVarsPerBlock)
    , fGSOutputs(sVarsPerBlock)
    , fFSInputs(sVarsPerBlock)
    , fFSUnis(sVarsPerBlock)
    , fFSOutputs(sMaxFSOutputs)
    , fUsesGS(false)
    , fVaryingDims(0)
    , fComplexCoord(false) {

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

const GrGLShaderVar& GrGLShaderBuilder::addUniform(VariableLifetime lifetime,
                                             GrSLType type,
                                             const char* name,
                                             int stageNum,
                                             int count) {
    GrAssert(name && strlen(name));

    GrGLShaderVar* var = NULL;
    if (kVertex_VariableLifetime & lifetime) {
        var = &fVSUnis.push_back();
    } else {
        GrAssert(kFragment_VariableLifetime & lifetime);
        var = &fFSUnis.push_back();
    }
    var->setType(type);
    var->setTypeModifier(GrGLShaderVar::kUniform_TypeModifier);
    var->setName(name);
    if (stageNum >= 0) {
        var->accessName()->appendS32(stageNum);
    }
    var->setArrayCount(count);

    if ((kVertex_VariableLifetime |
        kFragment_VariableLifetime) == lifetime) {
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

