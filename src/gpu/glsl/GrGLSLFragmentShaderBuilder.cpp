/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLFragmentShaderBuilder.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrShaderCaps.h"
#include "gl/GrGLGpu.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "../private/GrGLSL.h"

const char* GrGLSLFragmentShaderBuilder::kDstColorName = "_dstColor";

static const char* sample_offset_array_name(GrGLSLFPFragmentBuilder::Coordinates coords) {
    static const char* kArrayNames[] = {
        "deviceSpaceSampleOffsets",
        "windowSpaceSampleOffsets"
    };
    return kArrayNames[coords];

    GR_STATIC_ASSERT(0 == GrGLSLFPFragmentBuilder::kSkiaDevice_Coordinates);
    GR_STATIC_ASSERT(1 == GrGLSLFPFragmentBuilder::kGLSLWindow_Coordinates);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kArrayNames) == GrGLSLFPFragmentBuilder::kLast_Coordinates + 1);
}

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

uint8_t GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(GrSurfaceOrigin origin) {
    SkASSERT(kTopLeft_GrSurfaceOrigin == origin || kBottomLeft_GrSurfaceOrigin == origin);
    return origin;

    GR_STATIC_ASSERT(1 == kTopLeft_GrSurfaceOrigin);
    GR_STATIC_ASSERT(2 == kBottomLeft_GrSurfaceOrigin);
}

GrGLSLFragmentShaderBuilder::GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program)
    : GrGLSLFragmentBuilder(program)
    , fSetupFragPosition(false)
    , fHasCustomColorOutput(false)
    , fCustomColorOutputIndex(-1)
    , fHasSecondaryOutput(false)
    , fUsedSampleOffsetArrays(0)
    , fHasInitializedSampleMask(false)
    , fDefaultPrecision(kMedium_GrSLPrecision) {
    fSubstageIndices.push_back(0);
#ifdef SK_DEBUG
    fUsedProcessorFeatures = GrProcessor::kNone_RequiredFeatures;
    fHasReadDstColor = false;
#endif
}

bool GrGLSLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    const GrShaderCaps& shaderCaps = *fProgramBuilder->shaderCaps();
    switch (feature) {
        case kMultisampleInterpolation_GLSLFeature:
            if (!shaderCaps.multisampleInterpolationSupport()) {
                return false;
            }
            if (const char* extension = shaderCaps.multisampleInterpolationExtensionString()) {
                this->addFeature(1 << kMultisampleInterpolation_GLSLFeature, extension);
            }
            return true;
        default:
            SkFAIL("Unexpected GLSLFeature requested.");
            return false;
    }
}

SkString GrGLSLFragmentShaderBuilder::ensureCoords2D(const GrShaderVar& coords) {
    if (kVec3f_GrSLType != coords.getType()) {
        SkASSERT(kVec2f_GrSLType == coords.getType());
        return coords.getName();
    }

    SkString coords2D;
    coords2D.printf("%s_ensure2D", coords.c_str());
    this->codeAppendf("\tvec2 %s = %s.xy / %s.z;", coords2D.c_str(), coords.c_str(),
                      coords.c_str());
    return coords2D;
}

void GrGLSLFragmentShaderBuilder::appendOffsetToSample(const char* sampleIdx, Coordinates coords) {
    SkASSERT(fProgramBuilder->header().fSamplePatternKey);
    SkDEBUGCODE(fUsedProcessorFeatures |= GrProcessor::kSampleLocations_RequiredFeature);
    if (kTopLeft_GrSurfaceOrigin == this->getSurfaceOrigin()) {
        // With a top left origin, device and window space are equal, so we only use device coords.
        coords = kSkiaDevice_Coordinates;
    }
    this->codeAppendf("%s[%s]", sample_offset_array_name(coords), sampleIdx);
    fUsedSampleOffsetArrays |= (1 << coords);
}

void GrGLSLFragmentShaderBuilder::maskSampleCoverage(const char* mask, bool invert) {
    const GrShaderCaps& shaderCaps = *fProgramBuilder->shaderCaps();
    if (!shaderCaps.sampleVariablesSupport()) {
        SkDEBUGFAIL("Attempted to mask sample coverage without support.");
        return;
    }
    if (const char* extension = shaderCaps.sampleVariablesExtensionString()) {
        this->addFeature(1 << kSampleVariables_GLSLPrivateFeature, extension);
    }
    if (!fHasInitializedSampleMask) {
        this->codePrependf("gl_SampleMask[0] = -1;");
        fHasInitializedSampleMask = true;
    }
    if (invert) {
        this->codeAppendf("gl_SampleMask[0] &= ~(%s);", mask);
    } else {
        this->codeAppendf("gl_SampleMask[0] &= %s;", mask);
    }
}

void GrGLSLFragmentShaderBuilder::overrideSampleCoverage(const char* mask) {
    const GrShaderCaps& shaderCaps = *fProgramBuilder->shaderCaps();
    if (!shaderCaps.sampleMaskOverrideCoverageSupport()) {
        SkDEBUGFAIL("Attempted to override sample coverage without support.");
        return;
    }
    SkASSERT(shaderCaps.sampleVariablesSupport());
    if (const char* extension = shaderCaps.sampleVariablesExtensionString()) {
        this->addFeature(1 << kSampleVariables_GLSLPrivateFeature, extension);
    }
    if (this->addFeature(1 << kSampleMaskOverrideCoverage_GLSLPrivateFeature,
                         "GL_NV_sample_mask_override_coverage")) {
        // Redeclare gl_SampleMask with layout(override_coverage) if we haven't already.
        fOutputs.push_back().set(kInt_GrSLType, "gl_SampleMask", 1, GrShaderVar::kOut_TypeModifier,
                                 kHigh_GrSLPrecision, "override_coverage");
    }
    this->codeAppendf("gl_SampleMask[0] = %s;", mask);
    fHasInitializedSampleMask = true;
}

void GrGLSLFragmentShaderBuilder::elevateDefaultPrecision(GrSLPrecision precision) {
    fDefaultPrecision = SkTMax(fDefaultPrecision, precision);
}

const char* GrGLSLFragmentShaderBuilder::dstColor() {
    SkDEBUGCODE(fHasReadDstColor = true;)

    const char* override = fProgramBuilder->primitiveProcessor().getDestColorOverride();
    if (override != nullptr) {
        return override;
    }

    const GrShaderCaps* shaderCaps = fProgramBuilder->shaderCaps();
    if (shaderCaps->fbFetchSupport()) {
        this->addFeature(1 << kFramebufferFetch_GLSLPrivateFeature,
                         shaderCaps->fbFetchExtensionString());

        // Some versions of this extension string require declaring custom color output on ES 3.0+
        const char* fbFetchColorName = shaderCaps->fbFetchColorName();
        if (shaderCaps->fbFetchNeedsCustomOutput()) {
            this->enableCustomOutput();
            fOutputs[fCustomColorOutputIndex].setTypeModifier(GrShaderVar::kInOut_TypeModifier);
            fbFetchColorName = DeclaredColorOutputName();
            // Set the dstColor to an intermediate variable so we don't override it with the output
            this->codeAppendf("vec4 %s = %s;", kDstColorName, fbFetchColorName);
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
        fOutputs.push_back().set(kVec4f_GrSLType, DeclaredColorOutputName(),
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
        fOutputs.push_back().set(kVec4f_GrSLType, DeclaredSecondaryColorOutputName(),
                                 GrShaderVar::kOut_TypeModifier);
        fProgramBuilder->finalizeFragmentSecondaryColor(fOutputs.back());
    }
}

const char* GrGLSLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? DeclaredColorOutputName() : "sk_FragColor";
}

void GrGLSLFragmentBuilder::declAppendf(const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    inputs().appendVAList(fmt, argp);
    va_end(argp);
}

const char* GrGLSLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    const GrShaderCaps& caps = *fProgramBuilder->shaderCaps();
    return caps.mustDeclareFragmentShaderOutput() ? DeclaredSecondaryColorOutputName()
                                                  : "gl_SecondaryFragColorEXT";
}

GrSurfaceOrigin GrGLSLFragmentShaderBuilder::getSurfaceOrigin() const {
    SkASSERT(fProgramBuilder->header().fSurfaceOriginKey);
    return static_cast<GrSurfaceOrigin>(fProgramBuilder->header().fSurfaceOriginKey);

    GR_STATIC_ASSERT(1 == kTopLeft_GrSurfaceOrigin);
    GR_STATIC_ASSERT(2 == kBottomLeft_GrSurfaceOrigin);
}

void GrGLSLFragmentShaderBuilder::onFinalize() {
    fProgramBuilder->varyingHandler()->getFragDecls(&this->inputs(), &this->outputs());
    GrGLSLAppendDefaultFloatPrecisionDeclaration(fDefaultPrecision,
                                                 *fProgramBuilder->shaderCaps(),
                                                 &this->precisionQualifier());
    if (fUsedSampleOffsetArrays & (1 << kSkiaDevice_Coordinates)) {
        this->defineSampleOffsetArray(sample_offset_array_name(kSkiaDevice_Coordinates),
                                      SkMatrix::MakeTrans(-0.5f, -0.5f));
    }
    if (fUsedSampleOffsetArrays & (1 << kGLSLWindow_Coordinates)) {
        // With a top left origin, device and window space are equal, so we only use device coords.
        SkASSERT(kBottomLeft_GrSurfaceOrigin == this->getSurfaceOrigin());
        SkMatrix m;
        m.setScale(1, -1);
        m.preTranslate(-0.5f, -0.5f);
        this->defineSampleOffsetArray(sample_offset_array_name(kGLSLWindow_Coordinates), m);
    }
}

void GrGLSLFragmentShaderBuilder::defineSampleOffsetArray(const char* name, const SkMatrix& m) {
    SkASSERT(fProgramBuilder->caps()->sampleLocationsSupport());
    const GrPipeline& pipeline = fProgramBuilder->pipeline();
    const GrRenderTargetPriv& rtp = pipeline.getRenderTarget()->renderTargetPriv();
    const GrGpu::MultisampleSpecs& specs = rtp.getMultisampleSpecs(pipeline);
    SkSTArray<16, SkPoint, true> offsets;
    offsets.push_back_n(specs.fEffectiveSampleCnt);
    m.mapPoints(offsets.begin(), specs.fSampleLocations, specs.fEffectiveSampleCnt);
    this->definitions().appendf("const highp vec2 %s[] = vec2[](", name);
    for (int i = 0; i < specs.fEffectiveSampleCnt; ++i) {
        this->definitions().appendf("vec2(%f, %f)", offsets[i].x(), offsets[i].y());
        this->definitions().append(i + 1 != specs.fEffectiveSampleCnt ? ", " : ");\n");
    }
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
