/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

const char* GrGLSLFragmentShaderBuilder::kDstColorName = "_dstColor";

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
    // There's an illegal GrBlendEquation at the end there, hence the -1.
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kLayoutQualifierNames) ==
                     kGrBlendEquationCnt - kFirstAdvancedGrBlendEquation - 1);
}

uint8_t GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(GrSurfaceOrigin origin) {
    SkASSERT(kTopLeft_GrSurfaceOrigin == origin || kBottomLeft_GrSurfaceOrigin == origin);
    return origin + 1;

    GR_STATIC_ASSERT(0 == kTopLeft_GrSurfaceOrigin);
    GR_STATIC_ASSERT(1 == kBottomLeft_GrSurfaceOrigin);
}

GrGLSLFragmentShaderBuilder::GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program)
        : GrGLSLFragmentBuilder(program) {
    fSubstageIndices.push_back(0);
}

SkString GrGLSLFragmentShaderBuilder::ensureCoords2D(const GrShaderVar& coords) {
    if (kFloat3_GrSLType != coords.getType() && kHalf3_GrSLType != coords.getType()) {
        SkASSERT(kFloat2_GrSLType == coords.getType() || kHalf2_GrSLType == coords.getType());
        return coords.getName();
    }

    SkString coords2D;
    coords2D.printf("%s_ensure2D", coords.c_str());
    this->codeAppendf("\tfloat2 %s = %s.xy / %s.z;", coords2D.c_str(), coords.c_str(),
                      coords.c_str());
    return coords2D;
}

const char* GrGLSLFragmentShaderBuilder::sampleOffsets() {
    SkASSERT(CustomFeatures::kSampleLocations & fProgramBuilder->header().processorFeatures());
    SkDEBUGCODE(fUsedProcessorFeaturesThisStage_DebugOnly |= CustomFeatures::kSampleLocations);
    SkDEBUGCODE(fUsedProcessorFeaturesAllStages_DebugOnly |= CustomFeatures::kSampleLocations);
    return "_sampleOffsets";
}

void GrGLSLFragmentShaderBuilder::maskOffMultisampleCoverage(
        const char* mask, ScopeFlags scopeFlags) {
    const GrShaderCaps& shaderCaps = *fProgramBuilder->shaderCaps();
    if (!shaderCaps.sampleVariablesSupport()) {
        SkDEBUGFAIL("Attempted to mask sample coverage without support.");
        return;
    }
    if (const char* extension = shaderCaps.sampleVariablesExtensionString()) {
        this->addFeature(1 << kSampleVariables_GLSLPrivateFeature, extension);
    }

    if (!fHasModifiedSampleMask) {
        fHasModifiedSampleMask = true;
        if (ScopeFlags::kTopLevel != scopeFlags) {
            this->codePrependf("gl_SampleMask[0] = ~0;");
        }
        if (!(ScopeFlags::kInsideLoop & scopeFlags)) {
            this->codeAppendf("gl_SampleMask[0] = (%s);", mask);
            return;
        }
    }

    this->codeAppendf("gl_SampleMask[0] &= (%s);", mask);
}

void GrGLSLFragmentShaderBuilder::applyFnToMultisampleMask(
        const char* fn, const char* grad, ScopeFlags scopeFlags) {
    SkASSERT(CustomFeatures::kSampleLocations & fProgramBuilder->header().processorFeatures());
    SkDEBUGCODE(fUsedProcessorFeaturesThisStage_DebugOnly |= CustomFeatures::kSampleLocations);
    SkDEBUGCODE(fUsedProcessorFeaturesAllStages_DebugOnly |= CustomFeatures::kSampleLocations);

    int sampleCnt = fProgramBuilder->effectiveSampleCnt();
    SkASSERT(sampleCnt > 1);

    this->codeAppendf("{");

    if (!grad) {
        SkASSERT(fProgramBuilder->shaderCaps()->shaderDerivativeSupport());
        // In order to use HW derivatives, our neighbors within the same primitive must also be
        // executing the same code. A per-pixel branch makes this pre-condition impossible to
        // fulfill.
        SkASSERT(!(ScopeFlags::kInsidePerPixelBranch & scopeFlags));
        this->codeAppendf("float2 grad = float2(dFdx(%s), dFdy(%s));", fn, fn);
        this->codeAppendf("float fnwidth = fwidth(%s);", fn);
        grad = "grad";
    } else {
        this->codeAppendf("float fnwidth = abs(%s.x) + abs(%s.y);", grad, grad);
    }

    this->codeAppendf("int mask = 0;");
    this->codeAppendf("if (%s*2 < fnwidth) {", fn);  // Are ANY samples inside the implicit fn?
    this->codeAppendf(    "if (%s*-2 >= fnwidth) {", fn);  // Are ALL samples inside the implicit?
    this->codeAppendf(        "mask = ~0;");
    this->codeAppendf(    "} else for (int i = 0; i < %i; ++i) {", sampleCnt);
    this->codeAppendf(        "float fnsample = dot(%s, _sampleOffsets[i]) + %s;", grad, fn);
    this->codeAppendf(        "if (fnsample < 0) {");
    this->codeAppendf(            "mask |= (1 << i);");
    this->codeAppendf(        "}");
    this->codeAppendf(    "}");
    this->codeAppendf("}");
    this->maskOffMultisampleCoverage("mask", scopeFlags);

    this->codeAppendf("}");
}

const char* GrGLSLFragmentShaderBuilder::dstColor() {
    SkDEBUGCODE(fHasReadDstColorThisStage_DebugOnly = true;)

    const GrShaderCaps* shaderCaps = fProgramBuilder->shaderCaps();
    if (shaderCaps->fbFetchSupport()) {
        this->addFeature(1 << kFramebufferFetch_GLSLPrivateFeature,
                         shaderCaps->fbFetchExtensionString());

        // Some versions of this extension string require declaring custom color output on ES 3.0+
        const char* fbFetchColorName = "sk_LastFragColor";
        if (shaderCaps->fbFetchNeedsCustomOutput()) {
            this->enableCustomOutput();
            fOutputs[fCustomColorOutputIndex].setTypeModifier(GrShaderVar::kInOut_TypeModifier);
            fbFetchColorName = DeclaredColorOutputName();
            // Set the dstColor to an intermediate variable so we don't override it with the output
            this->codeAppendf("half4 %s = %s;", kDstColorName, fbFetchColorName);
        } else {
            return fbFetchColorName;
        }
    }
    return kDstColorName;
}

void GrGLSLFragmentShaderBuilder::enableAdvancedBlendEquationIfNeeded(GrBlendEquation equation) {
    SkASSERT(GrBlendEquationIsAdvanced(equation));

    const GrShaderCaps& caps = *fProgramBuilder->shaderCaps();
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
        fOutputs.push_back().set(kHalf4_GrSLType, DeclaredColorOutputName(),
                                 GrShaderVar::kOut_TypeModifier);
        fProgramBuilder->finalizeFragmentOutputColor(fOutputs.back());
    }
}

void GrGLSLFragmentShaderBuilder::enableSecondaryOutput() {
    SkASSERT(!fHasSecondaryOutput);
    fHasSecondaryOutput = true;
    const GrShaderCaps& caps = *fProgramBuilder->shaderCaps();
    if (const char* extension = caps.secondaryOutputExtensionString()) {
        this->addFeature(1 << kBlendFuncExtended_GLSLPrivateFeature, extension);
    }

    // If the primary output is declared, we must declare also the secondary output
    // and vice versa, since it is not allowed to use a built-in gl_FragColor and a custom
    // output. The condition also co-incides with the condition in whici GLES SL 2.0
    // requires the built-in gl_SecondaryFragColorEXT, where as 3.0 requires a custom output.
    if (caps.mustDeclareFragmentShaderOutput()) {
        fOutputs.push_back().set(kHalf4_GrSLType, DeclaredSecondaryColorOutputName(),
                                 GrShaderVar::kOut_TypeModifier);
        fProgramBuilder->finalizeFragmentSecondaryColor(fOutputs.back());
    }
}

const char* GrGLSLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? DeclaredColorOutputName() : "sk_FragColor";
}

bool GrGLSLFragmentShaderBuilder::primaryColorOutputIsInOut() const {
    return fHasCustomColorOutput &&
           fOutputs[fCustomColorOutputIndex].getTypeModifier() == GrShaderVar::kInOut_TypeModifier;
}

void GrGLSLFragmentBuilder::declAppendf(const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    inputs().appendVAList(fmt, argp);
    va_end(argp);
}

const char* GrGLSLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    if (this->hasSecondaryOutput()) {
        return (fProgramBuilder->shaderCaps()->mustDeclareFragmentShaderOutput())
                ? DeclaredSecondaryColorOutputName()
                : "gl_SecondaryFragColorEXT";
    }
    return nullptr;
}

GrSurfaceOrigin GrGLSLFragmentShaderBuilder::getSurfaceOrigin() const {
    SkASSERT(fProgramBuilder->header().hasSurfaceOriginKey());
    return static_cast<GrSurfaceOrigin>(fProgramBuilder->header().fSurfaceOriginKey-1);

    GR_STATIC_ASSERT(0 == kTopLeft_GrSurfaceOrigin);
    GR_STATIC_ASSERT(1 == kBottomLeft_GrSurfaceOrigin);
}

void GrGLSLFragmentShaderBuilder::onFinalize() {
    SkASSERT(fProgramBuilder->header().processorFeatures()
                     == fUsedProcessorFeaturesAllStages_DebugOnly);

    if (CustomFeatures::kSampleLocations & fProgramBuilder->header().processorFeatures()) {
        const SkTArray<SkPoint>& sampleLocations =
                fProgramBuilder->renderTarget()->renderTargetPriv().getSampleLocations();
        this->definitions().append("const float2 _sampleOffsets[] = float2[](");
        for (int i = 0; i < sampleLocations.count(); ++i) {
            SkPoint offset = sampleLocations[i] - SkPoint::Make(.5f, .5f);
            if (kBottomLeft_GrSurfaceOrigin == this->getSurfaceOrigin()) {
                offset.fY = -offset.fY;
            }
            this->definitions().appendf("float2(%f, %f)", offset.x(), offset.y());
            this->definitions().append((i + 1 != sampleLocations.count()) ? ", " : ");");
        }
    }

    fProgramBuilder->varyingHandler()->getFragDecls(&this->inputs(), &this->outputs());
}

void GrGLSLFragmentShaderBuilder::onBeforeChildProcEmitCode() {
    SkASSERT(fSubstageIndices.count() >= 1);
    fSubstageIndices.push_back(0);
    // second-to-last value in the fSubstageIndices stack is the index of the child proc
    // at that level which is currently emitting code.
    fMangleString.appendf("_c%d", fSubstageIndices[fSubstageIndices.count() - 2]);
}

void GrGLSLFragmentShaderBuilder::onAfterChildProcEmitCode() {
    SkASSERT(fSubstageIndices.count() >= 2);
    fSubstageIndices.pop_back();
    fSubstageIndices.back()++;
    int removeAt = fMangleString.findLastOf('_');
    fMangleString.remove(removeAt, fMangleString.size() - removeAt);
}
