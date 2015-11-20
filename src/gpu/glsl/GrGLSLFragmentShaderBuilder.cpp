/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLFragmentShaderBuilder.h"
#include "GrRenderTarget.h"
#include "glsl/GrGLSL.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVarying.h"

const char* GrGLSLFragmentShaderBuilder::kDstTextureColorName = "_dstColor";

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

GrGLSLFragmentShaderBuilder::FragPosKey
GrGLSLFragmentShaderBuilder::KeyForFragmentPosition(const GrRenderTarget* dst) {
    if (kTopLeft_GrSurfaceOrigin == dst->origin()) {
        return kTopLeftFragPosRead_FragPosKey;
    } else {
        return kBottomLeftFragPosRead_FragPosKey;
    }
}

GrGLSLFragmentShaderBuilder::GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program,
                                                         uint8_t fragPosKey)
    : INHERITED(program)
    , fSetupFragPosition(false)
    , fTopLeftFragPosRead(kTopLeftFragPosRead_FragPosKey == fragPosKey)
    , fCustomColorOutputIndex(-1)
    , fHasReadDstColor(false)
    , fHasReadFragmentPosition(false) {
}

bool GrGLSLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    switch (feature) {
        case kStandardDerivatives_GLSLFeature: {
            if (!fProgramBuilder->glslCaps()->shaderDerivativeSupport()) {
                return false;
            }
            const char* extension = fProgramBuilder->glslCaps()->shaderDerivativeExtensionString();
            if (extension) {
                this->addFeature(1 << kStandardDerivatives_GLSLFeature, extension);
            }
            return true;
        }
        default:
            SkFAIL("Unexpected GLSLFeature requested.");
            return false;
    }
}

SkString GrGLSLFragmentShaderBuilder::ensureFSCoords2D(const GrGLSLTransformedCoordsArray& coords,
                                                       int index) {
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

const char* GrGLSLFragmentShaderBuilder::fragmentPosition() {
    fHasReadFragmentPosition = true;

    const GrGLSLCaps* glslCaps = fProgramBuilder->glslCaps();
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (fTopLeftFragPosRead) {
        fSetupFragPosition = true;
        return "gl_FragCoord";
    } else if (const char* extension = glslCaps->fragCoordConventionsExtensionString()) {
        if (!fSetupFragPosition) {
            if (glslCaps->generation() < k150_GrGLSLGeneration) {
                this->addFeature(1 << kFragCoordConventions_GLSLPrivateFeature,
                                 extension);
            }
            fInputs.push_back().set(kVec4f_GrSLType,
                                    GrGLSLShaderVar::kIn_TypeModifier,
                                    "gl_FragCoord",
                                    kDefault_GrSLPrecision,
                                    GrGLSLShaderVar::kUpperLeft_Origin);
            fSetupFragPosition = true;
        }
        return "gl_FragCoord";
    } else {
        static const char* kTempName = "tmpXYFragCoord";
        static const char* kCoordName = "fragCoordYDown";
        if (!fSetupFragPosition) {
            SkASSERT(!fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
            const char* rtHeightName;

            fProgramBuilder->fUniformHandles.fRTHeightUni =
                    fProgramBuilder->addFragPosUniform(GrGLSLProgramBuilder::kFragment_Visibility,
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

const char* GrGLSLFragmentShaderBuilder::dstColor() {
    fHasReadDstColor = true;

    const GrGLSLCaps* glslCaps = fProgramBuilder->glslCaps();
    if (glslCaps->fbFetchSupport()) {
        this->addFeature(1 << (GrGLSLFragmentShaderBuilder::kLastGLSLPrivateFeature + 1),
                         glslCaps->fbFetchExtensionString());

        // Some versions of this extension string require declaring custom color output on ES 3.0+
        const char* fbFetchColorName = glslCaps->fbFetchColorName();
        if (glslCaps->fbFetchNeedsCustomOutput()) {
            this->enableCustomOutput();
            fOutputs[fCustomColorOutputIndex].setTypeModifier(GrShaderVar::kInOut_TypeModifier);
            fbFetchColorName = DeclaredColorOutputName();
        }
        return fbFetchColorName;
    } else {
        return kDstTextureColorName;
    } 
}

void GrGLSLFragmentShaderBuilder::enableAdvancedBlendEquationIfNeeded(GrBlendEquation equation) {
    SkASSERT(GrBlendEquationIsAdvanced(equation));

    const GrGLSLCaps& caps = *fProgramBuilder->glslCaps();
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

void GrGLSLFragmentShaderBuilder::enableCustomOutput() {
    if (!fHasCustomColorOutput) {
        fHasCustomColorOutput = true;
        fCustomColorOutputIndex = fOutputs.count();
        fOutputs.push_back().set(kVec4f_GrSLType,
                                 GrGLSLShaderVar::kOut_TypeModifier,
                                 DeclaredColorOutputName());
    }
}

void GrGLSLFragmentShaderBuilder::enableSecondaryOutput() {
    SkASSERT(!fHasSecondaryOutput);
    fHasSecondaryOutput = true;
    const GrGLSLCaps& caps = *fProgramBuilder->glslCaps();
    if (const char* extension = caps.secondaryOutputExtensionString()) {
        this->addFeature(1 << kBlendFuncExtended_GLSLPrivateFeature, extension);
    }

    // If the primary output is declared, we must declare also the secondary output
    // and vice versa, since it is not allowed to use a built-in gl_FragColor and a custom
    // output. The condition also co-incides with the condition in whici GLES SL 2.0
    // requires the built-in gl_SecondaryFragColorEXT, where as 3.0 requires a custom output.
    if (caps.mustDeclareFragmentShaderOutput()) {
        fOutputs.push_back().set(kVec4f_GrSLType, GrGLSLShaderVar::kOut_TypeModifier,
                                 DeclaredSecondaryColorOutputName());
    }
}

const char* GrGLSLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? DeclaredColorOutputName() : "gl_FragColor";
}

const char* GrGLSLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    const GrGLSLCaps& caps = *fProgramBuilder->glslCaps();
    return caps.mustDeclareFragmentShaderOutput() ? DeclaredSecondaryColorOutputName()
                                                  : "gl_SecondaryFragColorEXT";
}

void GrGLSLFragmentShaderBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getFragDecls(&this->inputs(), &this->outputs());
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision,
                                                 *fProgramBuilder->glslCaps(),
                                                 &this->precisionQualifier());
}

void GrGLSLFragmentBuilder::onBeforeChildProcEmitCode() {
    SkASSERT(fSubstageIndices.count() >= 1);
    fSubstageIndices.push_back(0);
    // second-to-last value in the fSubstageIndices stack is the index of the child proc
    // at that level which is currently emitting code.
    fMangleString.appendf("_c%d", fSubstageIndices[fSubstageIndices.count() - 2]);
}

void GrGLSLFragmentBuilder::onAfterChildProcEmitCode() {
    SkASSERT(fSubstageIndices.count() >= 2);
    fSubstageIndices.pop_back();
    fSubstageIndices.back()++;
    int removeAt = fMangleString.findLastOf('_');
    fMangleString.remove(removeAt, fMangleString.size() - removeAt);
}

