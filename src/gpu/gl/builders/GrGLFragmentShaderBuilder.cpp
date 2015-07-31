/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLFragmentShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLGLSL.h"
#include "glsl/GrGLSLCaps.h"

#define GL_CALL(X) GR_GL_CALL(fProgramBuilder->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fProgramBuilder->gpu()->glInterface(), R, X)

const char* GrGLFragmentShaderBuilder::kDstTextureColorName = "_dstColor";
static const char* declared_color_output_name() { return "fsColorOut"; }
static const char* declared_secondary_color_output_name() { return "fsSecondaryColorOut"; }

static const char* specific_layout_qualifier_name(GrBlendEquation equation) {
    SkASSERT(GrBlendEquationIsAdvanced(equation));

    static const char* kLayoutQualifierNames[] = {
        "blend_support_screen",
        "blend_support_overlay",
        "blend_support_darken",
        "blend_support_lighten",
        "blend_support_colordodge",
        "blend_support_colorburn",
        "blend_support_hardlight",
        "blend_support_softlight",
        "blend_support_difference",
        "blend_support_exclusion",
        "blend_support_multiply",
        "blend_support_hsl_hue",
        "blend_support_hsl_saturation",
        "blend_support_hsl_color",
        "blend_support_hsl_luminosity"
    };
    return kLayoutQualifierNames[equation - kFirstAdvancedGrBlendEquation];

    GR_STATIC_ASSERT(0 == kScreen_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(1 == kOverlay_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(2 == kDarken_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(3 == kLighten_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(4 == kColorDodge_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(5 == kColorBurn_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(6 == kHardLight_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(7 == kSoftLight_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(8 == kDifference_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(9 == kExclusion_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(10 == kMultiply_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(11 == kHSLHue_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(12 == kHSLSaturation_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(13 == kHSLColor_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(14 == kHSLLuminosity_GrBlendEquation - kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kLayoutQualifierNames) ==
                     kGrBlendEquationCnt - kFirstAdvancedGrBlendEquation);
}

GrGLFragmentShaderBuilder::DstReadKey
GrGLFragmentShaderBuilder::KeyForDstRead(const GrTexture* dstTexture, const GrGLCaps& caps) {
    uint32_t key = kYesDstRead_DstReadKeyBit;
    if (caps.glslCaps()->fbFetchSupport()) {
        return key;
    }
    SkASSERT(dstTexture);
    if (!caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(dstTexture->config())) {
        // The fact that the config is alpha-only must be considered when generating code.
        key |= kUseAlphaConfig_DstReadKeyBit;
    }
    if (kTopLeft_GrSurfaceOrigin == dstTexture->origin()) {
        key |= kTopLeftOrigin_DstReadKeyBit;
    }
    SkASSERT(static_cast<DstReadKey>(key) == key);
    return static_cast<DstReadKey>(key);
}

GrGLFragmentShaderBuilder::FragPosKey
GrGLFragmentShaderBuilder::KeyForFragmentPosition(const GrRenderTarget* dst, const GrGLCaps&) {
    if (kTopLeft_GrSurfaceOrigin == dst->origin()) {
        return kTopLeftFragPosRead_FragPosKey;
    } else {
        return kBottomLeftFragPosRead_FragPosKey;
    }
}

GrGLFragmentShaderBuilder::GrGLFragmentShaderBuilder(GrGLProgramBuilder* program,
                                                     uint8_t fragPosKey)
    : INHERITED(program)
    , fHasCustomColorOutput(false)
    , fHasSecondaryOutput(false)
    , fSetupFragPosition(false)
    , fTopLeftFragPosRead(kTopLeftFragPosRead_FragPosKey == fragPosKey)
    , fCustomColorOutputIndex(-1)
    , fHasReadDstColor(false)
    , fHasReadFragmentPosition(false) {
}

bool GrGLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature: {
            GrGLGpu* gpu = fProgramBuilder->gpu();
            if (!gpu->glCaps().shaderCaps()->shaderDerivativeSupport()) {
                return false;
            }
            if (kGLES_GrGLStandard == gpu->glStandard() &&
                k110_GrGLSLGeneration == gpu->glslGeneration()) {
                this->addFeature(1 << kStandardDerivatives_GLSLFeature,
                                 "GL_OES_standard_derivatives");
            }
            return true;
        }
        default:
            SkFAIL("Unexpected GLSLFeature requested.");
            return false;
    }
}

SkString GrGLFragmentShaderBuilder::ensureFSCoords2D(
        const GrGLProcessor::TransformedCoordsArray& coords, int index) {
    if (kVec3f_GrSLType != coords[index].getType()) {
        SkASSERT(kVec2f_GrSLType == coords[index].getType());
        return coords[index].getName();
    }

    SkString coords2D("coords2D");
    if (0 != index) {
        coords2D.appendf("_%i", index);
    }
    this->codeAppendf("\tvec2 %s = %s.xy / %s.z;",
                      coords2D.c_str(), coords[index].c_str(), coords[index].c_str());
    return coords2D;
}

const char* GrGLFragmentShaderBuilder::fragmentPosition() {
    fHasReadFragmentPosition = true;

    GrGLGpu* gpu = fProgramBuilder->gpu();
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (fTopLeftFragPosRead) {
        fSetupFragPosition = true;
        return "gl_FragCoord";
    } else if (gpu->glCaps().fragCoordConventionsSupport()) {
        if (!fSetupFragPosition) {
            if (gpu->glslGeneration() < k150_GrGLSLGeneration) {
                this->addFeature(1 << kFragCoordConventions_GLSLPrivateFeature,
                                 "GL_ARB_fragment_coord_conventions");
            }
            fInputs.push_back().set(kVec4f_GrSLType,
                                    GrGLShaderVar::kIn_TypeModifier,
                                    "gl_FragCoord",
                                    kDefault_GrSLPrecision,
                                    GrGLShaderVar::kUpperLeft_Origin);
            fSetupFragPosition = true;
        }
        return "gl_FragCoord";
    } else {
        static const char* kTempName = "tmpXYFragCoord";
        static const char* kCoordName = "fragCoordYDown";
        if (!fSetupFragPosition) {
            // temporarily change the stage index because we're inserting non-stage code.
            GrGLProgramBuilder::AutoStageRestore asr(fProgramBuilder);
            SkASSERT(!fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
            const char* rtHeightName;

            fProgramBuilder->fUniformHandles.fRTHeightUni =
                    fProgramBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                                kFloat_GrSLType,
                                                kDefault_GrSLPrecision,
                                                "RTHeight",
                                                &rtHeightName);

            // The Adreno compiler seems to be very touchy about access to "gl_FragCoord".
            // Accessing glFragCoord.zw can cause a program to fail to link. Additionally,
            // depending on the surrounding code, accessing .xy with a uniform involved can
            // do the same thing. Copying gl_FragCoord.xy into a temp vec2 beforehand 
            // (and only accessing .xy) seems to "fix" things.
            this->codePrependf("\tvec4 %s = vec4(%s.x, %s - %s.y, 1.0, 1.0);\n",
                               kCoordName, kTempName, rtHeightName, kTempName);
            this->codePrependf("vec2 %s = gl_FragCoord.xy;", kTempName);
            fSetupFragPosition = true;
        }
        SkASSERT(fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
        return kCoordName;
    }
}

const char* GrGLFragmentShaderBuilder::dstColor() {
    fHasReadDstColor = true;

    GrGLGpu* gpu = fProgramBuilder->gpu();
    if (gpu->glCaps().glslCaps()->fbFetchSupport()) {
        this->addFeature(1 << (GrGLFragmentShaderBuilder::kLastGLSLPrivateFeature + 1),
                         gpu->glCaps().glslCaps()->fbFetchExtensionString());

        // Some versions of this extension string require declaring custom color output on ES 3.0+
        const char* fbFetchColorName = gpu->glCaps().glslCaps()->fbFetchColorName();
        if (gpu->glCaps().glslCaps()->fbFetchNeedsCustomOutput()) {
            this->enableCustomOutput();
            fOutputs[fCustomColorOutputIndex].setTypeModifier(GrShaderVar::kInOut_TypeModifier);
            fbFetchColorName = declared_color_output_name();
        }
        return fbFetchColorName;
    } else {
        return kDstTextureColorName;
    } 
}

void GrGLFragmentShaderBuilder::enableAdvancedBlendEquationIfNeeded(GrBlendEquation equation) {
    SkASSERT(GrBlendEquationIsAdvanced(equation));

    const GrGLSLCaps& caps = *fProgramBuilder->gpu()->glCaps().glslCaps();
    if (!caps.mustEnableAdvBlendEqs()) {
        return;
    }

    this->addFeature(1 << kBlendEquationAdvanced_GLSLPrivateFeature,
                     "GL_KHR_blend_equation_advanced");
    if (caps.mustEnableSpecificAdvBlendEqs()) {
        this->addLayoutQualifier(specific_layout_qualifier_name(equation), kOut_InterfaceQualifier);
    } else {
        this->addLayoutQualifier("blend_support_all_equations", kOut_InterfaceQualifier);
    }
}

void GrGLFragmentShaderBuilder::enableCustomOutput() {
    if (!fHasCustomColorOutput) {
        fHasCustomColorOutput = true;
        fCustomColorOutputIndex = fOutputs.count();
        fOutputs.push_back().set(kVec4f_GrSLType,
                                 GrGLShaderVar::kOut_TypeModifier,
                                 declared_color_output_name());
    }
}

void GrGLFragmentShaderBuilder::enableSecondaryOutput() {
    SkASSERT(!fHasSecondaryOutput);
    fHasSecondaryOutput = true;
    if (kGLES_GrGLStandard == fProgramBuilder->gpu()->ctxInfo().standard()) {
        this->addFeature(1 << kBlendFuncExtended_GLSLPrivateFeature, "GL_EXT_blend_func_extended");
    }

    // If the primary output is declared, we must declare also the secondary output
    // and vice versa, since it is not allowed to use a built-in gl_FragColor and a custom
    // output. The condition also co-incides with the condition in whici GLES SL 2.0
    // requires the built-in gl_SecondaryFragColorEXT, where as 3.0 requires a custom output.
    const GrGLSLCaps& caps = *fProgramBuilder->gpu()->glCaps().glslCaps();
    if (caps.mustDeclareFragmentShaderOutput()) {
        fOutputs.push_back().set(kVec4f_GrSLType, GrGLShaderVar::kOut_TypeModifier,
                                 declared_secondary_color_output_name());
    }
}

const char* GrGLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? declared_color_output_name() : "gl_FragColor";
}

const char* GrGLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    const GrGLSLCaps& caps = *fProgramBuilder->gpu()->glCaps().glslCaps();
    return caps.mustDeclareFragmentShaderOutput() ? declared_secondary_color_output_name()
                                                  : "gl_SecondaryFragColorEXT";
}

bool GrGLFragmentShaderBuilder::compileAndAttachShaders(GrGLuint programId,
                                                        SkTDArray<GrGLuint>* shaderIds) {
    GrGLGpu* gpu = fProgramBuilder->gpu();
    this->versionDecl() = GrGLGetGLSLVersionDecl(gpu->ctxInfo());
    GrGLAppendGLSLDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision,
                                                   gpu->glStandard(),
                                                   &this->precisionQualifier());
    this->compileAndAppendLayoutQualifiers();
    fProgramBuilder->appendUniformDecls(GrGLProgramBuilder::kFragment_Visibility,
                                        &this->uniforms());
    this->appendDecls(fInputs, &this->inputs());
    // We shouldn't have declared outputs on 1.10
    SkASSERT(k110_GrGLSLGeneration != gpu->glslGeneration() || fOutputs.empty());
    this->appendDecls(fOutputs, &this->outputs());
    return this->finalize(programId, GR_GL_FRAGMENT_SHADER, shaderIds);
}

void GrGLFragmentShaderBuilder::bindFragmentShaderLocations(GrGLuint programID) {
    const GrGLCaps& caps = fProgramBuilder->gpu()->glCaps();
    if (fHasCustomColorOutput && caps.bindFragDataLocationSupport()) {
        GL_CALL(BindFragDataLocation(programID, 0, declared_color_output_name()));
    }
    if (fHasSecondaryOutput && caps.glslCaps()->mustDeclareFragmentShaderOutput()) {
        GL_CALL(BindFragDataLocationIndexed(programID, 0, 1,
                                            declared_secondary_color_output_name()));
    }
}

void GrGLFragmentShaderBuilder::addVarying(GrGLVarying* v, GrSLPrecision fsPrec) {
    v->fFsIn = v->fVsOut;
    if (v->fGsOut) {
        v->fFsIn = v->fGsOut;
    }
    fInputs.push_back().set(v->fType, GrGLShaderVar::kVaryingIn_TypeModifier, v->fFsIn, fsPrec);
}
