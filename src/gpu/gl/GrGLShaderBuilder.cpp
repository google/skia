/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLUniformHandle.h"
#include "GrCoordTransform.h"
#include "GrDrawEffect.h"
#include "GrGpuGL.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

// number of each input/output type in a single allocation block
static const int kVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int kMaxFSOutputs = 2;

// ES2 FS only guarantees mediump and lowp support
static const GrGLShaderVar::Precision kDefaultFragmentPrecision = GrGLShaderVar::kMedium_Precision;

typedef GrGLUniformManager::UniformHandle UniformHandle;

SK_CONF_DECLARE(bool, c_PrintShaders, "gpu.printShaders", false,
                "Print the source code for all shaders generated.");

///////////////////////////////////////////////////////////////////////////////

namespace {

inline const char* color_attribute_name() { return "aColor"; }
inline const char* coverage_attribute_name() { return "aCoverage"; }
inline const char* declared_color_output_name() { return "fsColorOut"; }
inline const char* dual_source_output_name() { return "dualSourceOut"; }
inline const char* sample_function_name(GrSLType type, GrGLSLGeneration glslGen) {
    if (kVec2f_GrSLType == type) {
        return glslGen >= k130_GrGLSLGeneration ? "texture" : "texture2D";
    } else {
        SkASSERT(kVec3f_GrSLType == type);
        return glslGen >= k130_GrGLSLGeneration ? "textureProj" : "texture2DProj";
    }
}

void append_texture_lookup(SkString* out,
                           GrGpuGL* gpu,
                           const char* samplerName,
                           const char* coordName,
                           uint32_t configComponentMask,
                           const char* swizzle,
                           GrSLType varyingType = kVec2f_GrSLType) {
    SkASSERT(NULL != coordName);

    out->appendf("%s(%s, %s)",
                 sample_function_name(varyingType, gpu->glslGeneration()),
                 samplerName,
                 coordName);

    char mangledSwizzle[5];

    // The swizzling occurs using texture params instead of shader-mangling if ARB_texture_swizzle
    // is available.
    if (!gpu->glCaps().textureSwizzleSupport() &&
        (kA_GrColorComponentFlag == configComponentMask)) {
        char alphaChar = gpu->glCaps().textureRedSupport() ? 'r' : 'a';
        int i;
        for (i = 0; '\0' != swizzle[i]; ++i) {
            mangledSwizzle[i] = alphaChar;
        }
        mangledSwizzle[i] ='\0';
        swizzle = mangledSwizzle;
    }
    // For shader prettiness we omit the swizzle rather than appending ".rgba".
    if (memcmp(swizzle, "rgba", 4)) {
        out->appendf(".%s", swizzle);
    }
}

}

static const char kDstCopyColorName[] = "_dstColor";

///////////////////////////////////////////////////////////////////////////////

bool GrGLShaderBuilder::GenProgram(GrGpuGL* gpu,
                                   GrGLUniformManager* uman,
                                   const GrGLProgramDesc& desc,
                                   const GrEffectStage* inColorStages[],
                                   const GrEffectStage* inCoverageStages[],
                                   GenProgramOutput* output) {
    SkAutoTDelete<GrGLShaderBuilder> builder;
    if (desc.getHeader().fHasVertexCode ||!gpu->shouldUseFixedFunctionTexturing()) {
        builder.reset(SkNEW_ARGS(GrGLFullShaderBuilder, (gpu, uman, desc)));
    } else {
        builder.reset(SkNEW_ARGS(GrGLFragmentOnlyShaderBuilder, (gpu, uman, desc)));
    }
    if (builder->genProgram(inColorStages, inCoverageStages)) {
        *output = builder->getOutput();
        return true;
    }
    return false;
}

bool GrGLShaderBuilder::genProgram(const GrEffectStage* colorStages[],
                                   const GrEffectStage* coverageStages[]) {
    const GrGLProgramDesc::KeyHeader& header = this->desc().getHeader();

    ///////////////////////////////////////////////////////////////////////////
    // emit code to read the dst copy texture, if necessary
    if (kNoDstRead_DstReadKey != header.fDstReadKey &&
        GrGLCaps::kNone_FBFetchType == fGpu->glCaps().fbFetchType()) {
        bool topDown = SkToBool(kTopLeftOrigin_DstReadKeyBit & header.fDstReadKey);
        const char* dstCopyTopLeftName;
        const char* dstCopyCoordScaleName;
        const char* dstCopySamplerName;
        uint32_t configMask;
        if (SkToBool(kUseAlphaConfig_DstReadKeyBit & header.fDstReadKey)) {
            configMask = kA_GrColorComponentFlag;
        } else {
            configMask = kRGBA_GrColorComponentFlags;
        }
        fOutput.fUniformHandles.fDstCopySamplerUni =
            this->addUniform(kFragment_Visibility, kSampler2D_GrSLType, "DstCopySampler",
                             &dstCopySamplerName);
        fOutput.fUniformHandles.fDstCopyTopLeftUni =
            this->addUniform(kFragment_Visibility, kVec2f_GrSLType, "DstCopyUpperLeft",
                             &dstCopyTopLeftName);
        fOutput.fUniformHandles.fDstCopyScaleUni =
            this->addUniform(kFragment_Visibility, kVec2f_GrSLType, "DstCopyCoordScale",
                             &dstCopyCoordScaleName);
        const char* fragPos = this->fragmentPosition();
        this->fsCodeAppend("\t// Read color from copy of the destination.\n");
        this->fsCodeAppendf("\tvec2 _dstTexCoord = (%s.xy - %s) * %s;\n",
                            fragPos, dstCopyTopLeftName, dstCopyCoordScaleName);
        if (!topDown) {
            this->fsCodeAppend("\t_dstTexCoord.y = 1.0 - _dstTexCoord.y;\n");
        }
        this->fsCodeAppendf("\tvec4 %s = ", kDstCopyColorName);
        append_texture_lookup(&fFSCode,
                              fGpu,
                              dstCopySamplerName,
                              "_dstTexCoord",
                              configMask,
                              "rgba");
        this->fsCodeAppend(";\n\n");
    }

    ///////////////////////////////////////////////////////////////////////////
    // get the initial color and coverage to feed into the first effect in each effect chain

    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (GrGLProgramDesc::kUniform_ColorInput == header.fColorInput) {
        const char* name;
        fOutput.fUniformHandles.fColorUni =
            this->addUniform(GrGLShaderBuilder::kFragment_Visibility, kVec4f_GrSLType, "Color",
                             &name);
        inputColor = GrGLSLExpr4(name);
    }

    if (GrGLProgramDesc::kUniform_ColorInput == header.fCoverageInput) {
        const char* name;
        fOutput.fUniformHandles.fCoverageUni =
            this->addUniform(GrGLShaderBuilder::kFragment_Visibility, kVec4f_GrSLType, "Coverage",
                             &name);
        inputCoverage = GrGLSLExpr4(name);
    } else if (GrGLProgramDesc::kSolidWhite_ColorInput == header.fCoverageInput) {
        inputCoverage = GrGLSLExpr4(1);
    }

    if (k110_GrGLSLGeneration != fGpu->glslGeneration()) {
        fFSOutputs.push_back().set(kVec4f_GrSLType,
                                   GrGLShaderVar::kOut_TypeModifier,
                                   declared_color_output_name());
        fHasCustomColorOutput = true;
    }

    this->emitCodeBeforeEffects(&inputColor, &inputCoverage);

    ///////////////////////////////////////////////////////////////////////////
    // emit the per-effect code for both color and coverage effects

    GrGLProgramDesc::EffectKeyProvider colorKeyProvider(
        &this->desc(), GrGLProgramDesc::EffectKeyProvider::kColor_EffectType);
    fOutput.fColorEffects.reset(this->createAndEmitEffects(colorStages,
                                                           this->desc().numColorEffects(),
                                                           colorKeyProvider,
                                                           &inputColor));

    GrGLProgramDesc::EffectKeyProvider coverageKeyProvider(
        &this->desc(), GrGLProgramDesc::EffectKeyProvider::kCoverage_EffectType);
    fOutput.fCoverageEffects.reset(this->createAndEmitEffects(coverageStages,
                                   this->desc().numCoverageEffects(),
                                   coverageKeyProvider,
                                   &inputCoverage));

    this->emitCodeAfterEffects();

    ///////////////////////////////////////////////////////////////////////////
    // write the secondary color output if necessary
    if (GrGLProgramDesc::CoverageOutputUsesSecondaryOutput(header.fCoverageOutput)) {
        const char* secondaryOutputName = this->enableSecondaryOutput();

        // default coeff to ones for kCoverage_DualSrcOutput
        GrGLSLExpr4 coeff(1);
        if (GrGLProgramDesc::kSecondaryCoverageISA_CoverageOutput == header.fCoverageOutput) {
            // Get (1-A) into coeff
            coeff = GrGLSLExpr4::VectorCast(GrGLSLExpr1(1) - inputColor.a());
        } else if (GrGLProgramDesc::kSecondaryCoverageISC_CoverageOutput ==
                   header.fCoverageOutput){
            // Get (1-RGBA) into coeff
            coeff = GrGLSLExpr4(1) - inputColor;
        }
        // Get coeff * coverage into modulate and then write that to the dual source output.
        this->fsCodeAppendf("\t%s = %s;\n", secondaryOutputName, (coeff * inputCoverage).c_str());
    }

    ///////////////////////////////////////////////////////////////////////////
    // combine color and coverage as frag color

    // Get "color * coverage" into fragColor
    GrGLSLExpr4 fragColor = inputColor * inputCoverage;
    // Now tack on "+(1-coverage)dst onto the frag color if we were asked to do so.
    if (GrGLProgramDesc::kCombineWithDst_CoverageOutput == header.fCoverageOutput) {
        GrGLSLExpr4 dstCoeff = GrGLSLExpr4(1) - inputCoverage;

        GrGLSLExpr4 dstContribution = dstCoeff * GrGLSLExpr4(this->dstColor());

        fragColor = fragColor + dstContribution;
    }
    this->fsCodeAppendf("\t%s = %s;\n", this->getColorOutputName(), fragColor.c_str());

    if (!this->finish()) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////

GrGLShaderBuilder::GrGLShaderBuilder(GrGpuGL* gpu,
                                     GrGLUniformManager* uniformManager,
                                     const GrGLProgramDesc& desc)
    : fDesc(desc)
    , fGpu(gpu)
    , fUniformManager(SkRef(uniformManager))
    , fFSFeaturesAddedMask(0)
    , fFSInputs(kVarsPerBlock)
    , fFSOutputs(kMaxFSOutputs)
    , fUniforms(kVarsPerBlock)
    , fSetupFragPosition(false)
    , fTopLeftFragPosRead(kTopLeftFragPosRead_FragPosKey == desc.getHeader().fFragPosKey)
    , fHasCustomColorOutput(false)
    , fHasSecondaryOutput(false) {
}

bool GrGLShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature:
            if (!fGpu->glCaps().shaderDerivativeSupport()) {
                return false;
            }
            if (kGLES_GrGLStandard == fGpu->glStandard()) {
                this->addFSFeature(1 << kStandardDerivatives_GLSLFeature,
                                   "GL_OES_standard_derivatives");
            }
            return true;
        default:
            SkFAIL("Unexpected GLSLFeature requested.");
            return false;
    }
}

bool GrGLShaderBuilder::enablePrivateFeature(GLSLPrivateFeature feature) {
    switch (feature) {
        case kFragCoordConventions_GLSLPrivateFeature:
            if (!fGpu->glCaps().fragCoordConventionsSupport()) {
                return false;
            }
            if (fGpu->glslGeneration() < k150_GrGLSLGeneration) {
                this->addFSFeature(1 << kFragCoordConventions_GLSLPrivateFeature,
                                   "GL_ARB_fragment_coord_conventions");
            }
            return true;
        case kEXTShaderFramebufferFetch_GLSLPrivateFeature:
            if (GrGLCaps::kEXT_FBFetchType != fGpu->glCaps().fbFetchType()) {
                return false;
            }
            this->addFSFeature(1 << kEXTShaderFramebufferFetch_GLSLPrivateFeature,
                               "GL_EXT_shader_framebuffer_fetch");
            return true;
        case kNVShaderFramebufferFetch_GLSLPrivateFeature:
            if (GrGLCaps::kNV_FBFetchType != fGpu->glCaps().fbFetchType()) {
                return false;
            }
            this->addFSFeature(1 << kNVShaderFramebufferFetch_GLSLPrivateFeature,
                               "GL_NV_shader_framebuffer_fetch");
            return true;
        default:
            SkFAIL("Unexpected GLSLPrivateFeature requested.");
            return false;
    }
}

void GrGLShaderBuilder::addFSFeature(uint32_t featureBit, const char* extensionName) {
    if (!(featureBit & fFSFeaturesAddedMask)) {
        fFSExtensions.appendf("#extension %s: require\n", extensionName);
        fFSFeaturesAddedMask |= featureBit;
    }
}

void GrGLShaderBuilder::nameVariable(SkString* out, char prefix, const char* name) {
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

const char* GrGLShaderBuilder::dstColor() {
    if (fCodeStage.inStageCode()) {
        const GrEffect* effect = fCodeStage.effectStage()->getEffect();
        if (!effect->willReadDstColor()) {
            SkDEBUGFAIL("GrGLEffect asked for dst color but its generating GrEffect "
                         "did not request access.");
            return "";
        }
    }
    static const char kFBFetchColorName[] = "gl_LastFragData[0]";
    GrGLCaps::FBFetchType fetchType = fGpu->glCaps().fbFetchType();
    if (GrGLCaps::kEXT_FBFetchType == fetchType) {
        SkAssertResult(this->enablePrivateFeature(kEXTShaderFramebufferFetch_GLSLPrivateFeature));
        return kFBFetchColorName;
    } else if (GrGLCaps::kNV_FBFetchType == fetchType) {
        SkAssertResult(this->enablePrivateFeature(kNVShaderFramebufferFetch_GLSLPrivateFeature));
        return kFBFetchColorName;
    } else if (fOutput.fUniformHandles.fDstCopySamplerUni.isValid()) {
        return kDstCopyColorName;
    } else {
        return "";
    }
}

void GrGLShaderBuilder::appendTextureLookup(SkString* out,
                                            const GrGLShaderBuilder::TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) const {
    append_texture_lookup(out,
                          fGpu,
                          this->getUniformCStr(sampler.samplerUniform()),
                          coordName,
                          sampler.configComponentMask(),
                          sampler.swizzle(),
                          varyingType);
}

void GrGLShaderBuilder::fsAppendTextureLookup(const GrGLShaderBuilder::TextureSampler& sampler,
                                              const char* coordName,
                                              GrSLType varyingType) {
    this->appendTextureLookup(&fFSCode, sampler, coordName, varyingType);
}

void GrGLShaderBuilder::fsAppendTextureLookupAndModulate(
                                            const char* modulation,
                                            const GrGLShaderBuilder::TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) {
    SkString lookup;
    this->appendTextureLookup(&lookup, sampler, coordName, varyingType);
    fFSCode.append((GrGLSLExpr4(modulation) * GrGLSLExpr4(lookup)).c_str());
}

GrGLShaderBuilder::DstReadKey GrGLShaderBuilder::KeyForDstRead(const GrTexture* dstCopy,
                                                               const GrGLCaps& caps) {
    uint32_t key = kYesDstRead_DstReadKeyBit;
    if (GrGLCaps::kNone_FBFetchType != caps.fbFetchType()) {
        return key;
    }
    SkASSERT(NULL != dstCopy);
    if (!caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(dstCopy->config())) {
        // The fact that the config is alpha-only must be considered when generating code.
        key |= kUseAlphaConfig_DstReadKeyBit;
    }
    if (kTopLeft_GrSurfaceOrigin == dstCopy->origin()) {
        key |= kTopLeftOrigin_DstReadKeyBit;
    }
    SkASSERT(static_cast<DstReadKey>(key) == key);
    return static_cast<DstReadKey>(key);
}

GrGLShaderBuilder::FragPosKey GrGLShaderBuilder::KeyForFragmentPosition(const GrRenderTarget* dst,
                                                                        const GrGLCaps&) {
    if (kTopLeft_GrSurfaceOrigin == dst->origin()) {
        return kTopLeftFragPosRead_FragPosKey;
    } else {
        return kBottomLeftFragPosRead_FragPosKey;
    }
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
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisibilityMask = kVertex_Visibility | kFragment_Visibility);
    SkASSERT(0 == (~kVisibilityMask & visibility));
    SkASSERT(0 != visibility);

    BuilderUniform& uni = fUniforms.push_back();
    UniformHandle h = GrGLUniformManager::UniformHandle::CreateFromUniformIndex(fUniforms.count() - 1);
    SkDEBUGCODE(UniformHandle h2 =)
    fUniformManager->appendUniform(type, count);
    // We expect the uniform manager to initially have no uniforms and that all uniforms are added
    // by this function. Therefore, the handles should match.
    SkASSERT(h2 == h);
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

    return h;
}

SkString GrGLShaderBuilder::ensureFSCoords2D(const TransformedCoordsArray& coords, int index) {
    if (kVec3f_GrSLType != coords[index].type()) {
        SkASSERT(kVec2f_GrSLType == coords[index].type());
        return coords[index].getName();
    }

    SkString coords2D("coords2D");
    if (0 != index) {
        coords2D.appendf("_%i", index);
    }
    this->fsCodeAppendf("\tvec2 %s = %s.xy / %s.z;",
                        coords2D.c_str(), coords[index].c_str(), coords[index].c_str());
    return coords2D;
}

const char* GrGLShaderBuilder::fragmentPosition() {
    if (fCodeStage.inStageCode()) {
        const GrEffect* effect = fCodeStage.effectStage()->getEffect();
        if (!effect->willReadFragmentPosition()) {
            SkDEBUGFAIL("GrGLEffect asked for frag position but its generating GrEffect "
                         "did not request access.");
            return "";
        }
    }
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (fTopLeftFragPosRead) {
        fSetupFragPosition = true;
        return "gl_FragCoord";
    } else if (fGpu->glCaps().fragCoordConventionsSupport()) {
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
            // temporarily change the stage index because we're inserting non-stage code.
            CodeStage::AutoStageRestore csar(&fCodeStage, NULL);

            SkASSERT(!fOutput.fUniformHandles.fRTHeightUni.isValid());
            const char* rtHeightName;

            fOutput.fUniformHandles.fRTHeightUni =
                this->addUniform(kFragment_Visibility, kFloat_GrSLType, "RTHeight", &rtHeightName);

            // Using glFragCoord.zw for the last two components tickles an Adreno driver bug that
            // causes programs to fail to link. Making this function return a vec2() didn't fix the
            // problem but using 1.0 for the last two components does.
            this->fFSCode.prependf("\tvec4 %s = vec4(gl_FragCoord.x, %s - gl_FragCoord.y, 1.0, "
                                   "1.0);\n", kCoordName, rtHeightName);
            fSetupFragPosition = true;
        }
        SkASSERT(fOutput.fUniformHandles.fRTHeightUni.isValid());
        return kCoordName;
    }
}

void GrGLShaderBuilder::fsEmitFunction(GrSLType returnType,
                                       const char* name,
                                       int argCnt,
                                       const GrGLShaderVar* args,
                                       const char* body,
                                       SkString* outName) {
    fFSFunctions.append(GrGLSLTypeString(returnType));
    this->nameVariable(outName, '\0', name);
    fFSFunctions.appendf(" %s", outName->c_str());
    fFSFunctions.append("(");
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(this->ctxInfo(), &fFSFunctions);
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
                                               GrGLStandard standard,
                                               SkString* str) {
    // Desktop GLSL has added precision qualifiers but they don't do anything.
    if (kGLES_GrGLStandard == standard) {
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
                SkFAIL("Default precision now allowed.");
            default:
                SkFAIL("Unknown precision value.");
        }
    }
}
}

void GrGLShaderBuilder::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(this->ctxInfo(), out);
        out->append(";\n");
    }
}

void GrGLShaderBuilder::appendUniformDecls(ShaderVisibility visibility,
                                           SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & visibility) {
            fUniforms[i].fVariable.appendDecl(this->ctxInfo(), out);
            out->append(";\n");
        }
    }
}

void GrGLShaderBuilder::createAndEmitEffects(GrGLProgramEffectsBuilder* programEffectsBuilder,
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
            this->fsCodeAppendf("\tvec4 %s = %s;\n", inColorName.c_str(), inColor.c_str());
            inColor = inColorName;
        }

        // create var to hold stage result
        SkString outColorName;
        this->nameVariable(&outColorName, '\0', "output");
        this->fsCodeAppendf("\tvec4 %s;\n", outColorName.c_str());
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

const char* GrGLShaderBuilder::getColorOutputName() const {
    return fHasCustomColorOutput ? declared_color_output_name() : "gl_FragColor";
}

const char* GrGLShaderBuilder::enableSecondaryOutput() {
    if (!fHasSecondaryOutput) {
        fFSOutputs.push_back().set(kVec4f_GrSLType,
                                   GrGLShaderVar::kOut_TypeModifier,
                                   dual_source_output_name());
        fHasSecondaryOutput = true;
    }
    return dual_source_output_name();
}

bool GrGLShaderBuilder::finish() {
    SkASSERT(0 == fOutput.fProgramID);
    GL_CALL_RET(fOutput.fProgramID, CreateProgram());
    if (!fOutput.fProgramID) {
        return false;
    }

    SkTDArray<GrGLuint> shadersToDelete;

    if (!this->compileAndAttachShaders(fOutput.fProgramID, &shadersToDelete)) {
        GL_CALL(DeleteProgram(fOutput.fProgramID));
        return false;
    }

    this->bindProgramLocations(fOutput.fProgramID);
    if (fUniformManager->isUsingBindUniform()) {
        fUniformManager->getUniformLocations(fOutput.fProgramID, fUniforms);
    }

    GL_CALL(LinkProgram(fOutput.fProgramID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = !fGpu->ctxInfo().isChromium();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        GrGLint linked = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(fOutput.fProgramID, GR_GL_LINK_STATUS, &linked));
        if (!linked) {
            GrGLint infoLen = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramiv(fOutput.fProgramID, GR_GL_INFO_LOG_LENGTH, &infoLen));
            SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
            if (infoLen > 0) {
                // retrieve length even though we don't need it to workaround
                // bug in chrome cmd buffer param validation.
                GrGLsizei length = GR_GL_INIT_ZERO;
                GL_CALL(GetProgramInfoLog(fOutput.fProgramID,
                                          infoLen+1,
                                          &length,
                                          (char*)log.get()));
                GrPrintf((char*)log.get());
            }
            SkDEBUGFAIL("Error linking program");
            GL_CALL(DeleteProgram(fOutput.fProgramID));
            fOutput.fProgramID = 0;
            return false;
        }
    }

    if (!fUniformManager->isUsingBindUniform()) {
        fUniformManager->getUniformLocations(fOutput.fProgramID, fUniforms);
    }

    for (int i = 0; i < shadersToDelete.count(); ++i) {
      GL_CALL(DeleteShader(shadersToDelete[i]));
    }

    return true;
}

// Compiles a GL shader and attaches it to a program. Returns the shader ID if
// successful, or 0 if not.
static GrGLuint attach_shader(const GrGLContext& glCtx,
                              GrGLuint programId,
                              GrGLenum type,
                              const SkString& shaderSrc) {
    const GrGLInterface* gli = glCtx.interface();

    GrGLuint shaderId;
    GR_GL_CALL_RET(gli, shaderId, CreateShader(type));
    if (0 == shaderId) {
        return 0;
    }

    const GrGLchar* sourceStr = shaderSrc.c_str();
    GrGLint sourceLength = static_cast<GrGLint>(shaderSrc.size());
    GR_GL_CALL(gli, ShaderSource(shaderId, 1, &sourceStr, &sourceLength));
    GR_GL_CALL(gli, CompileShader(shaderId));

    // Calling GetShaderiv in Chromium is quite expensive. Assume success in release builds.
    bool checkCompiled = !glCtx.isChromium();
#ifdef SK_DEBUG
    checkCompiled = true;
#endif
    if (checkCompiled) {
        GrGLint compiled = GR_GL_INIT_ZERO;
        GR_GL_CALL(gli, GetShaderiv(shaderId, GR_GL_COMPILE_STATUS, &compiled));

        if (!compiled) {
            GrGLint infoLen = GR_GL_INIT_ZERO;
            GR_GL_CALL(gli, GetShaderiv(shaderId, GR_GL_INFO_LOG_LENGTH, &infoLen));
            SkAutoMalloc log(sizeof(char)*(infoLen+1)); // outside if for debugger
            if (infoLen > 0) {
                // retrieve length even though we don't need it to workaround bug in Chromium cmd
                // buffer param validation.
                GrGLsizei length = GR_GL_INIT_ZERO;
                GR_GL_CALL(gli, GetShaderInfoLog(shaderId, infoLen+1,
                                                 &length, (char*)log.get()));
                GrPrintf(shaderSrc.c_str());
                GrPrintf("\n%s", log.get());
            }
            SkDEBUGFAIL("Shader compilation failed!");
            GR_GL_CALL(gli, DeleteShader(shaderId));
            return 0;
        }
    }

    TRACE_EVENT_INSTANT1(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), "skia_gpu::GLShader",
                         TRACE_EVENT_SCOPE_THREAD, "shader", TRACE_STR_COPY(shaderSrc.c_str()));
    if (c_PrintShaders) {
        GrPrintf(shaderSrc.c_str());
        GrPrintf("\n");
    }

    // Attach the shader, but defer deletion until after we have linked the program.
    // This works around a bug in the Android emulator's GLES2 wrapper which
    // will immediately delete the shader object and free its memory even though it's
    // attached to a program, which then causes glLinkProgram to fail.
    GR_GL_CALL(gli, AttachShader(programId, shaderId));

    return shaderId;
}

bool GrGLShaderBuilder::compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const {
    SkString fragShaderSrc(GrGetGLSLVersionDecl(this->ctxInfo()));
    fragShaderSrc.append(fFSExtensions);
    append_default_precision_qualifier(kDefaultFragmentPrecision,
                                       fGpu->glStandard(),
                                       &fragShaderSrc);
    this->appendUniformDecls(kFragment_Visibility, &fragShaderSrc);
    this->appendDecls(fFSInputs, &fragShaderSrc);
    // We shouldn't have declared outputs on 1.10
    SkASSERT(k110_GrGLSLGeneration != fGpu->glslGeneration() || fFSOutputs.empty());
    this->appendDecls(fFSOutputs, &fragShaderSrc);
    fragShaderSrc.append(fFSFunctions);
    fragShaderSrc.append("void main() {\n");
    fragShaderSrc.append(fFSCode);
    fragShaderSrc.append("}\n");

    GrGLuint fragShaderId = attach_shader(fGpu->glContext(), programId, GR_GL_FRAGMENT_SHADER, fragShaderSrc);
    if (!fragShaderId) {
        return false;
    }

    *shaderIds->append() = fragShaderId;

    return true;
}

void GrGLShaderBuilder::bindProgramLocations(GrGLuint programId) const {
    if (fHasCustomColorOutput) {
        GL_CALL(BindFragDataLocation(programId, 0, declared_color_output_name()));
    }
    if (fHasSecondaryOutput) {
        GL_CALL(BindFragDataLocationIndexed(programId, 0, 1, dual_source_output_name()));
    }
}

const GrGLContextInfo& GrGLShaderBuilder::ctxInfo() const {
    return fGpu->ctxInfo();
}

////////////////////////////////////////////////////////////////////////////////

GrGLFullShaderBuilder::GrGLFullShaderBuilder(GrGpuGL* gpu,
                                             GrGLUniformManager* uniformManager,
                                             const GrGLProgramDesc& desc)
    : INHERITED(gpu, uniformManager, desc)
    , fVSAttrs(kVarsPerBlock)
    , fVSOutputs(kVarsPerBlock)
    , fGSInputs(kVarsPerBlock)
    , fGSOutputs(kVarsPerBlock) {
}

void GrGLFullShaderBuilder::emitCodeBeforeEffects(GrGLSLExpr4* color, GrGLSLExpr4* coverage) {
    const GrGLProgramDesc::KeyHeader& header = this->desc().getHeader();

    fOutput.fHasVertexShader = true;

    fPositionVar = &fVSAttrs.push_back();
    fPositionVar->set(kVec2f_GrSLType, GrGLShaderVar::kAttribute_TypeModifier, "aPosition");
    if (-1 != header.fLocalCoordAttributeIndex) {
        fLocalCoordsVar = &fVSAttrs.push_back();
        fLocalCoordsVar->set(kVec2f_GrSLType,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             "aLocalCoords");
    } else {
        fLocalCoordsVar = fPositionVar;
    }

    const char* viewMName;
    fOutput.fUniformHandles.fViewMatrixUni =
        this->addUniform(GrGLShaderBuilder::kVertex_Visibility, kMat33f_GrSLType, "ViewM",
                          &viewMName);

    // Transform the position into Skia's device coords.
    this->vsCodeAppendf("\tvec3 pos3 = %s * vec3(%s, 1);\n",
                        viewMName, fPositionVar->c_str());

    // we output point size in the GS if present
    if (header.fEmitsPointSize
#if GR_GL_EXPERIMENTAL_GS
        && !header.fExperimentalGS
#endif
        ) {
        this->vsCodeAppend("\tgl_PointSize = 1.0;\n");
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fColorInput) {
        this->addAttribute(kVec4f_GrSLType, color_attribute_name());
        const char *vsName, *fsName;
        this->addVarying(kVec4f_GrSLType, "Color", &vsName, &fsName);
        this->vsCodeAppendf("\t%s = %s;\n", vsName, color_attribute_name());
        *color = fsName;
    }

    if (GrGLProgramDesc::kAttribute_ColorInput == header.fCoverageInput) {
        this->addAttribute(kVec4f_GrSLType, coverage_attribute_name());
        const char *vsName, *fsName;
        this->addVarying(kVec4f_GrSLType, "Coverage", &vsName, &fsName);
        this->vsCodeAppendf("\t%s = %s;\n", vsName, coverage_attribute_name());
        *coverage = fsName;
    }
}

void GrGLFullShaderBuilder::emitCodeAfterEffects() {
    const char* rtAdjustName;
    fOutput.fUniformHandles.fRTAdjustmentUni =
        this->addUniform(GrGLShaderBuilder::kVertex_Visibility, kVec4f_GrSLType, "rtAdjustment",
                         &rtAdjustName);

    // Transform from Skia's device coords to GL's normalized device coords.
    this->vsCodeAppendf(
        "\tgl_Position = vec4(dot(pos3.xz, %s.xy), dot(pos3.yz, %s.zw), 0, pos3.z);\n",
        rtAdjustName, rtAdjustName);
}

bool GrGLFullShaderBuilder::addAttribute(GrSLType type, const char* name) {
    for (int i = 0; i < fVSAttrs.count(); ++i) {
        const GrGLShaderVar& attr = fVSAttrs[i];
        // if attribute already added, don't add it again
        if (attr.getName().equals(name)) {
            SkASSERT(attr.getType() == type);
            return false;
        }
    }
    fVSAttrs.push_back().set(type,
                             GrGLShaderVar::kAttribute_TypeModifier,
                             name);
    return true;
}

bool GrGLFullShaderBuilder::addEffectAttribute(int attributeIndex,
                                               GrSLType type,
                                               const SkString& name) {
    if (!this->addAttribute(type, name.c_str())) {
        return false;
    }

    fEffectAttributes.push_back().set(attributeIndex, name);
    return true;
}

void GrGLFullShaderBuilder::addVarying(GrSLType type,
                                       const char* name,
                                       const char** vsOutName,
                                       const char** fsInName) {
    fVSOutputs.push_back();
    fVSOutputs.back().setType(type);
    fVSOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
    this->nameVariable(fVSOutputs.back().accessName(), 'v', name);

    if (vsOutName) {
        *vsOutName = fVSOutputs.back().getName().c_str();
    }
    // input to FS comes either from VS or GS
    const SkString* fsName;
#if GR_GL_EXPERIMENTAL_GS
    if (this->desc().getHeader().fExperimentalGS) {
        // if we have a GS take each varying in as an array
        // and output as non-array.
        fGSInputs.push_back();
        fGSInputs.back().setType(type);
        fGSInputs.back().setTypeModifier(GrGLShaderVar::kVaryingIn_TypeModifier);
        fGSInputs.back().setUnsizedArray();
        *fGSInputs.back().accessName() = fVSOutputs.back().getName();
        fGSOutputs.push_back();
        fGSOutputs.back().setType(type);
        fGSOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
        this->nameVariable(fGSOutputs.back().accessName(), 'g', name);
        fsName = fGSOutputs.back().accessName();
    } else
#endif
    {
        fsName = fVSOutputs.back().accessName();
    }
    this->fsInputAppend().set(type, GrGLShaderVar::kVaryingIn_TypeModifier, *fsName);
    if (fsInName) {
        *fsInName = fsName->c_str();
    }
}

const SkString* GrGLFullShaderBuilder::getEffectAttributeName(int attributeIndex) const {
    const AttributePair* attribEnd = fEffectAttributes.end();
    for (const AttributePair* attrib = fEffectAttributes.begin(); attrib != attribEnd; ++attrib) {
        if (attrib->fIndex == attributeIndex) {
            return &attrib->fName;
        }
    }

    return NULL;
}

GrGLProgramEffects* GrGLFullShaderBuilder::createAndEmitEffects(
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

bool GrGLFullShaderBuilder::compileAndAttachShaders(GrGLuint programId,
                                                    SkTDArray<GrGLuint>* shaderIds) const {
    const GrGLContext& glCtx = this->gpu()->glContext();
    SkString vertShaderSrc(GrGetGLSLVersionDecl(this->ctxInfo()));
    this->appendUniformDecls(kVertex_Visibility, &vertShaderSrc);
    this->appendDecls(fVSAttrs, &vertShaderSrc);
    this->appendDecls(fVSOutputs, &vertShaderSrc);
    vertShaderSrc.append("void main() {\n");
    vertShaderSrc.append(fVSCode);
    vertShaderSrc.append("}\n");
    GrGLuint vertShaderId = attach_shader(glCtx, programId, GR_GL_VERTEX_SHADER, vertShaderSrc);
    if (!vertShaderId) {
        return false;
    }
    *shaderIds->append() = vertShaderId;

#if GR_GL_EXPERIMENTAL_GS
    if (this->desc().getHeader().fExperimentalGS) {
        SkASSERT(this->ctxInfo().glslGeneration() >= k150_GrGLSLGeneration);
        SkString geomShaderSrc(GrGetGLSLVersionDecl(this->ctxInfo()));
        geomShaderSrc.append("layout(triangles) in;\n"
                             "layout(triangle_strip, max_vertices = 6) out;\n");
        this->appendDecls(fGSInputs, &geomShaderSrc);
        this->appendDecls(fGSOutputs, &geomShaderSrc);
        geomShaderSrc.append("void main() {\n");
        geomShaderSrc.append("\tfor (int i = 0; i < 3; ++i) {\n"
                             "\t\tgl_Position = gl_in[i].gl_Position;\n");
        if (this->desc().getHeader().fEmitsPointSize) {
            geomShaderSrc.append("\t\tgl_PointSize = 1.0;\n");
        }
        SkASSERT(fGSInputs.count() == fGSOutputs.count());
        for (int i = 0; i < fGSInputs.count(); ++i) {
            geomShaderSrc.appendf("\t\t%s = %s[i];\n",
                                  fGSOutputs[i].getName().c_str(),
                                  fGSInputs[i].getName().c_str());
        }
        geomShaderSrc.append("\t\tEmitVertex();\n"
                             "\t}\n"
                             "\tEndPrimitive();\n");
        geomShaderSrc.append("}\n");
        GrGLuint geomShaderId = attach_shader(glCtx, programId, GR_GL_GEOMETRY_SHADER, geomShaderSrc);
        if (!geomShaderId) {
            return false;
        }
        *shaderIds->append() = geomShaderId;
    }
#endif

    return this->INHERITED::compileAndAttachShaders(programId, shaderIds);
}

void GrGLFullShaderBuilder::bindProgramLocations(GrGLuint programId) const {
    this->INHERITED::bindProgramLocations(programId);

    const GrGLProgramDesc::KeyHeader& header = this->desc().getHeader();

    // Bind the attrib locations to same values for all shaders
    SkASSERT(-1 != header.fPositionAttributeIndex);
    GL_CALL(BindAttribLocation(programId,
                               header.fPositionAttributeIndex,
                               fPositionVar->c_str()));
    if (-1 != header.fLocalCoordAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fLocalCoordAttributeIndex,
                                   fLocalCoordsVar->c_str()));
    }
    if (-1 != header.fColorAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fColorAttributeIndex,
                                   color_attribute_name()));
    }
    if (-1 != header.fCoverageAttributeIndex) {
        GL_CALL(BindAttribLocation(programId,
                                   header.fCoverageAttributeIndex,
                                   coverage_attribute_name()));
    }

    const AttributePair* attribEnd = fEffectAttributes.end();
    for (const AttributePair* attrib = fEffectAttributes.begin(); attrib != attribEnd; ++attrib) {
         GL_CALL(BindAttribLocation(programId, attrib->fIndex, attrib->fName.c_str()));
    }
}

////////////////////////////////////////////////////////////////////////////////

GrGLFragmentOnlyShaderBuilder::GrGLFragmentOnlyShaderBuilder(GrGpuGL* gpu,
                                                             GrGLUniformManager* uniformManager,
                                                             const GrGLProgramDesc& desc)
    : INHERITED(gpu, uniformManager, desc) {
    SkASSERT(!desc.getHeader().fHasVertexCode);
    SkASSERT(gpu->glCaps().pathRenderingSupport());
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fColorInput);
    SkASSERT(GrGLProgramDesc::kAttribute_ColorInput != desc.getHeader().fCoverageInput);
}

int GrGLFragmentOnlyShaderBuilder::addTexCoordSets(int count) {
    int firstFreeCoordSet = fOutput.fTexCoordSetCnt;
    fOutput.fTexCoordSetCnt += count;
    SkASSERT(gpu()->glCaps().maxFixedFunctionTextureCoords() >= fOutput.fTexCoordSetCnt);
    return firstFreeCoordSet;
}

GrGLProgramEffects* GrGLFragmentOnlyShaderBuilder::createAndEmitEffects(
        const GrEffectStage* effectStages[],
        int effectCnt,
        const GrGLProgramDesc::EffectKeyProvider& keyProvider,
        GrGLSLExpr4* inOutFSColor) {

    GrGLPathTexGenProgramEffectsBuilder pathTexGenEffectsBuilder(this,
                                                                 effectCnt);
    this->INHERITED::createAndEmitEffects(&pathTexGenEffectsBuilder,
                                          effectStages,
                                          effectCnt,
                                          keyProvider,
                                          inOutFSColor);
    return pathTexGenEffectsBuilder.finish();
}
