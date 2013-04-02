/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLUniformHandle.h"
#include "GrDrawEffect.h"
#include "GrTexture.h"

// number of each input/output type in a single allocation block
static const int kVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int kMaxFSOutputs = 2;

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;

typedef GrGLUniformManager::UniformHandle UniformHandle;
///////////////////////////////////////////////////////////////////////////////

namespace {

inline const char* sample_function_name(GrSLType type, GrGLSLGeneration glslGen) {
    if (kVec2f_GrSLType == type) {
        return glslGen >= k130_GrGLSLGeneration ? "texture" : "texture2D";
    } else {
        GrAssert(kVec3f_GrSLType == type);
        return glslGen >= k130_GrGLSLGeneration ? "textureProj" : "texture2DProj";
    }
}

/**
 * Do we need to either map r,g,b->a or a->r. configComponentMask indicates which channels are
 * present in the texture's config. swizzleComponentMask indicates the channels present in the
 * shader swizzle.
 */
inline bool swizzle_requires_alpha_remapping(const GrGLCaps& caps,
                                             uint32_t configComponentMask,
                                             uint32_t swizzleComponentMask) {
    if (caps.textureSwizzleSupport()) {
        // Any remapping is handled using texture swizzling not shader modifications.
        return false;
    }
    // check if the texture is alpha-only
    if (kA_GrColorComponentFlag == configComponentMask) {
        if (caps.textureRedSupport() && (kA_GrColorComponentFlag & swizzleComponentMask)) {
            // we must map the swizzle 'a's to 'r'.
            return true;
        }
        if (kRGB_GrColorComponentFlags & swizzleComponentMask) {
            // The 'r', 'g', and/or 'b's must be mapped to 'a' according to our semantics that
            // alpha-only textures smear alpha across all four channels when read.
            return true;
        }
    }
    return false;
}

void append_swizzle(SkString* outAppend,
                    const GrGLShaderBuilder::TextureSampler& texSampler,
                    const GrGLCaps& caps) {
    const char* swizzle = texSampler.swizzle();
    char mangledSwizzle[5];

    // The swizzling occurs using texture params instead of shader-mangling if ARB_texture_swizzle
    // is available.
    if (!caps.textureSwizzleSupport() &&
        (kA_GrColorComponentFlag == texSampler.configComponentMask())) {
        char alphaChar = caps.textureRedSupport() ? 'r' : 'a';
        int i;
        for (i = 0; '\0' != swizzle[i]; ++i) {
            mangledSwizzle[i] = alphaChar;
        }
        mangledSwizzle[i] ='\0';
        swizzle = mangledSwizzle;
    }
    // For shader prettiness we omit the swizzle rather than appending ".rgba".
    if (memcmp(swizzle, "rgba", 4)) {
        outAppend->appendf(".%s", swizzle);
    }
}

}

static const char kDstColorName[] = "_dstColor";

///////////////////////////////////////////////////////////////////////////////

GrGLShaderBuilder::GrGLShaderBuilder(const GrGLContextInfo& ctxInfo,
                                     GrGLUniformManager& uniformManager,
                                     const GrGLProgramDesc& desc)
    : fUniforms(kVarsPerBlock)
    , fVSAttrs(kVarsPerBlock)
    , fVSOutputs(kVarsPerBlock)
    , fGSInputs(kVarsPerBlock)
    , fGSOutputs(kVarsPerBlock)
    , fFSInputs(kVarsPerBlock)
    , fFSOutputs(kMaxFSOutputs)
    , fCtxInfo(ctxInfo)
    , fUniformManager(uniformManager)
    , fCurrentStageIdx(kNonStageIdx)
    , fFSFeaturesAddedMask(0)
#if GR_GL_EXPERIMENTAL_GS
    , fUsesGS(desc.fExperimentalGS)
#else
    , fUsesGS(false)
#endif
    , fSetupFragPosition(false)
    , fRTHeightUniform(GrGLUniformManager::kInvalidUniformHandle)
    , fDstCopyTopLeftUniform (GrGLUniformManager::kInvalidUniformHandle)
    , fDstCopyScaleUniform (GrGLUniformManager::kInvalidUniformHandle) {

    fPositionVar = &fVSAttrs.push_back();
    fPositionVar->set(kVec2f_GrSLType, GrGLShaderVar::kAttribute_TypeModifier, "aPosition");
    if (-1 != desc.fLocalCoordAttributeIndex) {
        fLocalCoordsVar = &fVSAttrs.push_back();
        fLocalCoordsVar->set(kVec2f_GrSLType,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             "aLocalCoords");
    } else {
        fLocalCoordsVar = fPositionVar;
    }
    if (kNoDstRead_DstReadKey != desc.fDstRead) {
        bool topDown = SkToBool(kTopLeftOrigin_DstReadKeyBit & desc.fDstRead);
        const char* dstCopyTopLeftName;
        const char* dstCopyCoordScaleName;
        uint32_t configMask;
        if (SkToBool(kUseAlphaConfig_DstReadKeyBit & desc.fDstRead)) {
            configMask = kA_GrColorComponentFlag;
        } else {
            configMask = kRGBA_GrColorComponentFlags;
        }
        fDstCopySampler.init(this, configMask, "rgba", 0);

        fDstCopyTopLeftUniform = this->addUniform(kFragment_ShaderType,
                                                  kVec2f_GrSLType,
                                                  "DstCopyUpperLeft",
                                                  &dstCopyTopLeftName);
        fDstCopyScaleUniform     = this->addUniform(kFragment_ShaderType,
                                                    kVec2f_GrSLType,
                                                    "DstCopyCoordScale",
                                                    &dstCopyCoordScaleName);
        const char* fragPos = this->fragmentPosition();
        this->fsCodeAppend("\t// Read color from copy of the destination.\n");
        this->fsCodeAppendf("\tvec2 _dstTexCoord = (%s.xy - %s) * %s;\n",
                            fragPos, dstCopyTopLeftName, dstCopyCoordScaleName);
        if (!topDown) {
            this->fsCodeAppend("\t_dstTexCoord.y = 1.0 - _dstTexCoord.y;\n");
        }
        this->fsCodeAppendf("\tvec4 %s = ", kDstColorName);
        this->appendTextureLookup(kFragment_ShaderType, fDstCopySampler, "_dstTexCoord");
        this->fsCodeAppend(";\n\n");
    }
}

bool GrGLShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature:
            if (!fCtxInfo.caps()->shaderDerivativeSupport()) {
                return false;
            }
            if (kES2_GrGLBinding == fCtxInfo.binding()) {
                this->addFSFeature(1 << kStandardDerivatives_GLSLFeature,
                                   "GL_OES_standard_derivatives");
            }
            return true;
        default:
            GrCrash("Unexpected GLSLFeature requested.");
            return false;
    }
}

bool GrGLShaderBuilder::enablePrivateFeature(GLSLPrivateFeature feature) {
    switch (feature) {
        case kFragCoordConventions_GLSLPrivateFeature:
            if (!fCtxInfo.caps()->fragCoordConventionsSupport()) {
                return false;
            }
            if (fCtxInfo.glslGeneration() < k150_GrGLSLGeneration) {
                this->addFSFeature(1 << kFragCoordConventions_GLSLPrivateFeature,
                                   "GL_ARB_fragment_coord_conventions");
            }
            return true;
        default:
            GrCrash("Unexpected GLSLPrivateFeature requested.");
            return false;
    }
}

void GrGLShaderBuilder::addFSFeature(uint32_t featureBit, const char* extensionName) {
    if (!(featureBit & fFSFeaturesAddedMask)) {
        fFSExtensions.appendf("#extension %s: require\n", extensionName);
        fFSFeaturesAddedMask |= featureBit;
    }
}

const char* GrGLShaderBuilder::dstColor() const {
    if (fDstCopySampler.isInitialized()) {
        return kDstColorName;
    } else {
        return NULL;
    }
}

void GrGLShaderBuilder::codeAppendf(ShaderType type, const char format[], va_list args) {
    SkString* string = NULL;
    switch (type) {
        case kVertex_ShaderType:
            string = &fVSCode;
            break;
        case kGeometry_ShaderType:
            string = &fGSCode;
            break;
        case kFragment_ShaderType:
            string = &fFSCode;
            break;
        default:
            GrCrash("Invalid shader type");
    }
    string->appendf(format, args);
}

void GrGLShaderBuilder::codeAppend(ShaderType type, const char* str) {
    SkString* string = NULL;
    switch (type) {
        case kVertex_ShaderType:
            string = &fVSCode;
            break;
        case kGeometry_ShaderType:
            string = &fGSCode;
            break;
        case kFragment_ShaderType:
            string = &fFSCode;
            break;
        default:
            GrCrash("Invalid shader type");
    }
    string->append(str);
}

void GrGLShaderBuilder::appendTextureLookup(SkString* out,
                                            const GrGLShaderBuilder::TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) const {
    GrAssert(NULL != coordName);

    out->appendf("%s(%s, %s)",
                 sample_function_name(varyingType, fCtxInfo.glslGeneration()),
                 this->getUniformCStr(sampler.fSamplerUniform),
                 coordName);
    append_swizzle(out, sampler, *fCtxInfo.caps());
}

void GrGLShaderBuilder::appendTextureLookup(ShaderType type,
                                            const GrGLShaderBuilder::TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) {
    GrAssert(kFragment_ShaderType == type);
    this->appendTextureLookup(&fFSCode, sampler, coordName, varyingType);
}

void GrGLShaderBuilder::appendTextureLookupAndModulate(
                                            ShaderType type,
                                            const char* modulation,
                                            const GrGLShaderBuilder::TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) {
    GrAssert(kFragment_ShaderType == type);
    SkString lookup;
    this->appendTextureLookup(&lookup, sampler, coordName, varyingType);
    GrGLSLModulate4f(&fFSCode, modulation, lookup.c_str());
}

GrBackendEffectFactory::EffectKey GrGLShaderBuilder::KeyForTextureAccess(
                                                            const GrTextureAccess& access,
                                                            const GrGLCaps& caps) {
    uint32_t configComponentMask = GrPixelConfigComponentMask(access.getTexture()->config());
    if (swizzle_requires_alpha_remapping(caps, configComponentMask, access.swizzleMask())) {
        return 1;
    } else {
        return 0;
    }
}

GrGLShaderBuilder::DstReadKey GrGLShaderBuilder::KeyForDstRead(const GrTexture* dstCopy,
                                                               const GrGLCaps& caps) {
    uint32_t key = kYesDstRead_DstReadKeyBit;
    if (!caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(dstCopy->config())) {
        // The fact that the config is alpha-only must be considered when generating code.
        key |= kUseAlphaConfig_DstReadKeyBit;
    }
    if (kTopLeft_GrSurfaceOrigin == dstCopy->origin()) {
        key |= kTopLeftOrigin_DstReadKeyBit;
    }
    GrAssert(static_cast<DstReadKey>(key) == key);
    return static_cast<DstReadKey>(key);
}

const GrGLenum* GrGLShaderBuilder::GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps) {
    if (caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(config)) {
        if (caps.textureRedSupport()) {
            static const GrGLenum gRedSmear[] = { GR_GL_RED, GR_GL_RED, GR_GL_RED, GR_GL_RED };
            return gRedSmear;
        } else {
            static const GrGLenum gAlphaSmear[] = { GR_GL_ALPHA, GR_GL_ALPHA,
                                                    GR_GL_ALPHA, GR_GL_ALPHA };
            return gAlphaSmear;
        }
    } else {
        static const GrGLenum gStraight[] = { GR_GL_RED, GR_GL_GREEN, GR_GL_BLUE, GR_GL_ALPHA };
        return gStraight;
    }
}

GrGLUniformManager::UniformHandle GrGLShaderBuilder::addUniformArray(uint32_t visibility,
                                                                     GrSLType type,
                                                                     const char* name,
                                                                     int count,
                                                                     const char** outName) {
    GrAssert(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_ShaderType | kFragment_ShaderType);
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
    if (kNonStageIdx == fCurrentStageIdx) {
        uniName->printf("u%s", name);
    } else {
        uniName->printf("u%s%d", name, fCurrentStageIdx);
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

bool GrGLShaderBuilder::addAttribute(GrSLType type,
                                     const char* name) {
    for (int i = 0; i < fVSAttrs.count(); ++i) {
        const GrGLShaderVar& attr = fVSAttrs[i];
        // if attribute already added, don't add it again
        if (attr.getName().equals(name)) {
            GrAssert(attr.getType() == type);
            return false;
        }
    }
    fVSAttrs.push_back().set(type,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             name);
    return true;
}

void GrGLShaderBuilder::addVarying(GrSLType type,
                                   const char* name,
                                   const char** vsOutName,
                                   const char** fsInName) {
    fVSOutputs.push_back();
    fVSOutputs.back().setType(type);
    fVSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
    if (kNonStageIdx == fCurrentStageIdx) {
        fVSOutputs.back().accessName()->printf("v%s", name);
    } else {
        fVSOutputs.back().accessName()->printf("v%s%d", name, fCurrentStageIdx);
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
        if (kNonStageIdx == fCurrentStageIdx) {
            fGSOutputs.back().accessName()->printf("g%s", name);
        } else {
            fGSOutputs.back().accessName()->printf("g%s%d", name, fCurrentStageIdx);
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

const char* GrGLShaderBuilder::fragmentPosition() {
#if 1
    if (fCtxInfo.caps()->fragCoordConventionsSupport()) {
        if (!fSetupFragPosition) {
            SkAssertResult(this->enablePrivateFeature(kFragCoordConventions_GLSLPrivateFeature));
            fFSInputs.push_back().set(kVec4f_GrSLType,
                                      GrGLShaderVar::kIn_TypeModifier,
                                      "gl_FragCoord",
                                      GrGLShaderVar::kDefault_Precision,
                                      GrGLShaderVar::kUpperLeft_Origin);
            fSetupFragPosition = true;
        }
        return "gl_FragCoord";
    } else {
        static const char* kCoordName = "fragCoordYDown";
        if (!fSetupFragPosition) {
            GrAssert(GrGLUniformManager::kInvalidUniformHandle == fRTHeightUniform);
            const char* rtHeightName;

            // temporarily change the stage index because we're inserting a uniform whose name
            // shouldn't be mangled to be stage-specific.
            int oldStageIdx = fCurrentStageIdx;
            fCurrentStageIdx = kNonStageIdx;
            fRTHeightUniform = this->addUniform(kFragment_ShaderType,
                                                kFloat_GrSLType,
                                                "RTHeight",
                                                &rtHeightName);
            fCurrentStageIdx = oldStageIdx;

            this->fFSCode.prependf("\tvec4 %s = vec4(gl_FragCoord.x, %s - gl_FragCoord.y, gl_FragCoord.zw);\n",
                                   kCoordName, rtHeightName);
            fSetupFragPosition = true;
        }
        GrAssert(GrGLUniformManager::kInvalidUniformHandle != fRTHeightUniform);
        return kCoordName;
    }
#else
    // This is the path we'll need to use once we have support for TopLeft
    // render targets.
    if (!fSetupFragPosition) {
        fFSInputs.push_back().set(kVec4f_GrSLType,
                                  GrGLShaderVar::kIn_TypeModifier,
                                  "gl_FragCoord",
                                  GrGLShaderVar::kDefault_Precision);
        fSetupFragPosition = true;
    }
    return "gl_FragCoord";
#endif
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
    if (kNonStageIdx != fCurrentStageIdx) {
        outName->printf(" %s_%d", name, fCurrentStageIdx);
    } else {
        *outName = name;
    }
    fFSFunctions.append(*outName);
    fFSFunctions.append("(");
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(fCtxInfo, &fFSFunctions);
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
        vars[i].appendDecl(fCtxInfo, out);
        out->append(";\n");
    }
}

void GrGLShaderBuilder::appendUniformDecls(ShaderType stype, SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & stype) {
            fUniforms[i].fVariable.appendDecl(fCtxInfo, out);
            out->append(";\n");
        }
    }
}

void GrGLShaderBuilder::getShader(ShaderType type, SkString* shaderStr) const {
    const char* version = GrGetGLSLVersionDecl(fCtxInfo.binding(), fCtxInfo.glslGeneration());

    switch (type) {
        case kVertex_ShaderType:
            *shaderStr = version;
            this->appendUniformDecls(kVertex_ShaderType, shaderStr);
            this->appendDecls(fVSAttrs, shaderStr);
            this->appendDecls(fVSOutputs, shaderStr);
            shaderStr->append("void main() {\n");
            shaderStr->append(fVSCode);
            shaderStr->append("}\n");
            break;
        case kGeometry_ShaderType:
            if (fUsesGS) {
                *shaderStr = version;
                shaderStr->append(fGSHeader);
                this->appendDecls(fGSInputs, shaderStr);
                this->appendDecls(fGSOutputs, shaderStr);
                shaderStr->append("void main() {\n");
                shaderStr->append(fGSCode);
                shaderStr->append("}\n");
            } else {
                shaderStr->reset();
            }
            break;
        case kFragment_ShaderType:
            *shaderStr = version;
            shaderStr->append(fFSExtensions);
            append_default_precision_qualifier(kDefaultFragmentPrecision,
                                               fCtxInfo.binding(),
                                               shaderStr);
            this->appendUniformDecls(kFragment_ShaderType, shaderStr);
            this->appendDecls(fFSInputs, shaderStr);
            // We shouldn't have declared outputs on 1.10
            GrAssert(k110_GrGLSLGeneration != fCtxInfo.glslGeneration() || fFSOutputs.empty());
            this->appendDecls(fFSOutputs, shaderStr);
            shaderStr->append(fFSFunctions);
            shaderStr->append("void main() {\n");
            shaderStr->append(fFSCode);
            shaderStr->append("}\n");
            break;
    }
 }

void GrGLShaderBuilder::finished(GrGLuint programID) {
    fUniformManager.getUniformLocations(programID, fUniforms);
}

GrGLEffect* GrGLShaderBuilder::createAndEmitGLEffect(
                                const GrEffectStage& stage,
                                GrGLEffect::EffectKey key,
                                const char* fsInColor,
                                const char* fsOutColor,
                                SkTArray<GrGLUniformManager::UniformHandle, true>* samplerHandles) {
    GrAssert(NULL != stage.getEffect());

    const GrEffectRef& effect = *stage.getEffect();
    int numTextures = effect->numTextures();
    SkSTArray<8, GrGLShaderBuilder::TextureSampler> textureSamplers;
    textureSamplers.push_back_n(numTextures);
    for (int i = 0; i < numTextures; ++i) {
        textureSamplers[i].init(this, &effect->textureAccess(i), i);
        samplerHandles->push_back(textureSamplers[i].fSamplerUniform);
    }
    GrDrawEffect drawEffect(stage, this->hasExplicitLocalCoords());

    int numAttributes = stage.getVertexAttribIndexCount();
    const int* attributeIndices = stage.getVertexAttribIndices();
    SkSTArray<GrEffect::kMaxVertexAttribs, SkString> attributeNames;
    for (int i = 0; i < numAttributes; ++i) {
        SkString attributeName("aAttr");
        attributeName.appendS32(attributeIndices[i]);

        if (this->addAttribute(effect->vertexAttribType(i), attributeName.c_str())) {
            fEffectAttributes.push_back().set(attributeIndices[i], attributeName);
        }
    }

    GrGLEffect* glEffect = effect->getFactory().createGLInstance(drawEffect);

    // Enclose custom code in a block to avoid namespace conflicts
    this->fVSCode.appendf("\t{ // %s\n", glEffect->name());
    this->fFSCode.appendf("\t{ // %s \n", glEffect->name());

    glEffect->emitCode(this,
                       drawEffect,
                       key,
                       fsOutColor,
                       fsInColor,
                       textureSamplers);
    this->fVSCode.appendf("\t}\n");
    this->fFSCode.appendf("\t}\n");

    return glEffect;
}

const SkString* GrGLShaderBuilder::getEffectAttributeName(int attributeIndex) const {
    const AttributePair* attribEnd = this->getEffectAttributes().end();
    for (const AttributePair* attrib = this->getEffectAttributes().begin();
         attrib != attribEnd;
         ++attrib) {
        if (attrib->fIndex == attributeIndex) {
            return &attrib->fName;
        }
    }

    return NULL;
}
