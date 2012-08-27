/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLUniformHandle.h"
#include "GrTexture.h"

// number of each input/output type in a single allocation block
static const int kVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int kMaxFSOutputs = 2;

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;

typedef GrGLUniformManager::UniformHandle UniformHandle;
///////////////////////////////////////////////////////////////////////////////

static SkString build_sampler_string(GrGLShaderBuilder::SamplerMode samplerMode) {
    SkString sampler("texture2D");
    switch (samplerMode) {
      case GrGLShaderBuilder::kDefault_SamplerMode:
          break;
      case GrGLShaderBuilder::kProj_SamplerMode:
          sampler.append("Proj");
          break;
      case GrGLShaderBuilder::kExplicitDivide_SamplerMode:
          break;
    }

    return sampler;
}

static bool texture_requires_alpha_to_red_swizzle(const GrGLCaps& caps,
                                                  const GrTextureAccess& access) {
    return GrPixelConfigIsAlphaOnly(access.getTexture()->config()) && caps.textureRedSupport() &&
        access.referencesAlpha();
}

static SkString build_swizzle_string(const GrTextureAccess& textureAccess,
                                     const GrGLCaps& caps) {
    const GrTextureAccess::Swizzle& swizzle = textureAccess.getSwizzle();
    if (0 == swizzle[0]) {
        return SkString("");
    }

    SkString swizzleOut(".");
    bool alphaIsRed = texture_requires_alpha_to_red_swizzle(caps, textureAccess);
    for (int offset = 0; offset < 4 && swizzle[offset]; ++offset) {
        if (alphaIsRed && 'a' == swizzle[offset]) {
            swizzleOut.appendf("r");
        } else {
            swizzleOut.appendf("%c", swizzle[offset]);
        }
    }

    return swizzleOut;
}

///////////////////////////////////////////////////////////////////////////////

// Architectural assumption: always 2-d input coords.
// Likely to become non-constant and non-static, perhaps even
// varying by stage, if we use 1D textures for gradients!
//const int GrGLShaderBuilder::fCoordDims = 2;

GrGLShaderBuilder::GrGLShaderBuilder(const GrGLContextInfo& ctx, GrGLUniformManager& uniformManager)
    : fUniforms(kVarsPerBlock)
    , fVSAttrs(kVarsPerBlock)
    , fVSOutputs(kVarsPerBlock)
    , fGSInputs(kVarsPerBlock)
    , fGSOutputs(kVarsPerBlock)
    , fFSInputs(kVarsPerBlock)
    , fFSOutputs(kMaxFSOutputs)
    , fUsesGS(false)
    , fVaryingDims(0)
    , fComplexCoord(false)
    , fContext(ctx)
    , fUniformManager(uniformManager)
    , fCurrentStage(kNonStageIdx) {
}

void GrGLShaderBuilder::computeSwizzle(uint32_t configFlags) {
    fSwizzle = "";
    if (configFlags & GrGLProgram::StageDesc::kSmearAlpha_InConfigFlag) {
        GrAssert(!(configFlags &
                   GrGLProgram::StageDesc::kSmearRed_InConfigFlag));
        fSwizzle = ".aaaa";
    } else if (configFlags & GrGLProgram::StageDesc::kSmearRed_InConfigFlag) {
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

void GrGLShaderBuilder::setupTextureAccess(int stageNum) {
    SkString retval;

    SamplerMode mode = kDefault_SamplerMode;
    // FIXME: we aren't currently using Proj.
    if (fVaryingDims != fCoordDims) {
        mode = kExplicitDivide_SamplerMode;
    }

    switch (mode) {
        case kDefault_SamplerMode:
            GrAssert(fVaryingDims == fCoordDims);
            // Do nothing
            break;
        case kProj_SamplerMode:
            // Do nothing
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
    fTexFunc = build_sampler_string(mode);
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

void GrGLShaderBuilder::emitCustomTextureLookup(SamplerMode samplerMode,
                                                const GrTextureAccess& textureAccess,
                                                const char* samplerName,
                                                const char* coordName) {
    GrAssert(samplerName && coordName);
    SkString sampler = build_sampler_string(samplerMode);
    SkString swizzle = build_swizzle_string(textureAccess, fContext.caps());

    fFSCode.appendf("%s( %s, %s)%s;\n", sampler.c_str(), samplerName,
                    coordName, swizzle.c_str());
}

GrCustomStage::StageKey GrGLShaderBuilder::KeyForTextureAccess(const GrTextureAccess& access,
                                                               const GrGLCaps& caps) {
    GrCustomStage::StageKey key = 0;

    // Assume that swizzle support implies that we never have to modify a shader to adjust
    // for texture format/swizzle settings.
    if (caps.textureSwizzleSupport()) {
        return key;
    }

    if (texture_requires_alpha_to_red_swizzle(caps, access)) {
        key = 1;
    }

    return key;
}

GrGLUniformManager::UniformHandle GrGLShaderBuilder::addUniformArray(uint32_t visibility,
                                                                     GrSLType type,
                                                                     const char* name,
                                                                     int count,
                                                                     const char** outName) {
    GrAssert(name && strlen(name));
    static const uint32_t kVisibilityMask = kVertex_ShaderType | kFragment_ShaderType;
    GrAssert(0 == (~kVisibilityMask & visibility));
    GrAssert(0 != visibility);

    BuilderUniform& uni = fUniforms.push_back();
    UniformHandle h = index_to_handle(fUniforms.count() - 1);
    GR_DEBUGCODE(UniformHandle h2 =)
    fUniformManager.appendUniform(type, count);
    // We expect the uniform manager to initially have no uniforms and that all uniforms are added
    // by this function. Therefore, the handles should match.
    GrAssert(h2 == h);
    uni.fVariable.setType(type);
    uni.fVariable.setTypeModifier(GrGLShaderVar::kUniform_TypeModifier);
    SkString* uniName = uni.fVariable.accessName();
    if (kNonStageIdx == fCurrentStage) {
        uniName->printf("u%s", name);
    } else {
        uniName->printf("u%s%d", name, fCurrentStage);
    }
    uni.fVariable.setArrayCount(count);
    uni.fVisibility = visibility;

    // If it is visible in both the VS and FS, the precision must match.
    // We declare a default FS precision, but not a default VS. So set the var
    // to use the default FS precision.
    if ((kVertex_ShaderType | kFragment_ShaderType) == visibility) {
        // the fragment and vertex precisions must match
        uni.fVariable.setPrecision(kDefaultFragmentPrecision);
    }

    if (NULL != outName) {
        *outName = uni.fVariable.c_str();
    }

    return h;
}

const GrGLShaderVar& GrGLShaderBuilder::getUniformVariable(UniformHandle u) const {
    return fUniforms[handle_to_index(u)].fVariable;
}

void GrGLShaderBuilder::addVarying(GrSLType type,
                                   const char* name,
                                   const char** vsOutName,
                                   const char** fsInName) {
    fVSOutputs.push_back();
    fVSOutputs.back().setType(type);
    fVSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
    if (kNonStageIdx == fCurrentStage) {
        fVSOutputs.back().accessName()->printf("v%s", name);
    } else {
        fVSOutputs.back().accessName()->printf("v%s%d", name, fCurrentStage);
    }
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
        if (kNonStageIdx == fCurrentStage) {
            fGSOutputs.back().accessName()->printf("g%s", name);
        } else {
            fGSOutputs.back().accessName()->printf("g%s%d", name, fCurrentStage);
        }
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

void GrGLShaderBuilder::emitFunction(ShaderType shader,
                                     GrSLType returnType,
                                     const char* name,
                                     int argCnt,
                                     const GrGLShaderVar* args,
                                     const char* body,
                                     SkString* outName) {
    GrAssert(kFragment_ShaderType == shader);
    fFSFunctions.append(GrGLShaderVar::TypeString(returnType));
    if (kNonStageIdx != fCurrentStage) {
        outName->printf(" %s_%d", name, fCurrentStage);
    } else {
        *outName = name;
    }
    fFSFunctions.append(*outName);
    fFSFunctions.append("(");
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(fContext, &fFSFunctions);
        if (i < argCnt - 1) {
            fFSFunctions.append(", ");
        }
    }
    fFSFunctions.append(") {\n");
    fFSFunctions.append(body);
    fFSFunctions.append("}\n\n");
}

namespace {

inline void append_default_precision_qualifier(GrGLShaderVar::Precision p,
                                               GrGLBinding binding,
                                               SkString* str) {
    // Desktop GLSL has added precision qualifiers but they don't do anything.
    if (kES2_GrGLBinding == binding) {
        switch (p) {
            case GrGLShaderVar::kHigh_Precision:
                str->append("precision highp float;\n");
                break;
            case GrGLShaderVar::kMedium_Precision:
                str->append("precision mediump float;\n");
                break;
            case GrGLShaderVar::kLow_Precision:
                str->append("precision lowp float;\n");
                break;
            case GrGLShaderVar::kDefault_Precision:
                GrCrash("Default precision now allowed.");
            default:
                GrCrash("Unknown precision value.");
        }
    }
}
}

void GrGLShaderBuilder::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(fContext, out);
        out->append(";\n");
    }
}

void GrGLShaderBuilder::appendUniformDecls(ShaderType stype, SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & stype) {
            fUniforms[i].fVariable.appendDecl(fContext, out);
            out->append(";\n");
        }
    }
}

void GrGLShaderBuilder::getShader(ShaderType type, SkString* shaderStr) const {
    switch (type) {
        case kVertex_ShaderType:
            *shaderStr = fHeader;
            this->appendUniformDecls(kVertex_ShaderType, shaderStr);
            this->appendDecls(fVSAttrs, shaderStr);
            this->appendDecls(fVSOutputs, shaderStr);
            shaderStr->append(fVSCode);
            break;
        case kGeometry_ShaderType:
            if (fUsesGS) {
                *shaderStr = fHeader;
                shaderStr->append(fGSHeader);
                this->appendDecls(fGSInputs, shaderStr);
                this->appendDecls(fGSOutputs, shaderStr);
                shaderStr->append(fGSCode);
            } else {
                shaderStr->reset();
            }
            break;
        case kFragment_ShaderType:
            *shaderStr = fHeader;
            append_default_precision_qualifier(kDefaultFragmentPrecision,
                                               fContext.binding(),
                                               shaderStr);
            this->appendUniformDecls(kFragment_ShaderType, shaderStr);
            this->appendDecls(fFSInputs, shaderStr);
            // We shouldn't have declared outputs on 1.10
            GrAssert(k110_GrGLSLGeneration != fContext.glslGeneration() || fFSOutputs.empty());
            this->appendDecls(fFSOutputs, shaderStr);
            shaderStr->append(fFSFunctions);
            shaderStr->append(fFSCode);
            break;
    }
 }

void GrGLShaderBuilder::finished(GrGLuint programID) {
    fUniformManager.getUniformLocations(programID, fUniforms);
}
