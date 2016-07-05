/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSLFragmentShaderBuilder.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "gl/GrGLGpu.h"
#include "glsl/GrGLSL.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"

const char* GrGLSLFragmentShaderBuilder::kDstTextureColorName = "_dstColor";

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
    , fHasInitializedSampleMask(false) {
    fSubstageIndices.push_back(0);
#ifdef SK_DEBUG
    fUsedProcessorFeatures = GrProcessor::kNone_RequiredFeatures;
    fHasReadDstColor = false;
#endif
}

bool GrGLSLFragmentShaderBuilder::enableFeature(GLSLFeature feature) {
    const GrGLSLCaps& glslCaps = *fProgramBuilder->glslCaps();
    switch (feature) {
        case kStandardDerivatives_GLSLFeature:
            if (!glslCaps.shaderDerivativeSupport()) {
                return false;
            }
            if (const char* extension = glslCaps.shaderDerivativeExtensionString()) {
                this->addFeature(1 << kStandardDerivatives_GLSLFeature, extension);
            }
            return true;
        case kPixelLocalStorage_GLSLFeature:
            if (glslCaps.pixelLocalStorageSize() <= 0) {
                return false;
            }
            this->addFeature(1 << kPixelLocalStorage_GLSLFeature,
                             "GL_EXT_shader_pixel_local_storage");
            return true;
        case kMultisampleInterpolation_GLSLFeature:
            if (!glslCaps.multisampleInterpolationSupport()) {
                return false;
            }
            if (const char* extension = glslCaps.multisampleInterpolationExtensionString()) {
                this->addFeature(1 << kMultisampleInterpolation_GLSLFeature, extension);
            }
            return true;
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
    SkDEBUGCODE(fUsedProcessorFeatures |= GrProcessor::kFragmentPosition_RequiredFeature;)

    const GrGLSLCaps* glslCaps = fProgramBuilder->glslCaps();
    // We only declare "gl_FragCoord" when we're in the case where we want to use layout qualifiers
    // to reverse y. Otherwise it isn't necessary and whether the "in" qualifier appears in the
    // declaration varies in earlier GLSL specs. So it is simpler to omit it.
    if (kTopLeft_GrSurfaceOrigin == this->getSurfaceOrigin()) {
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
                                    "origin_upper_left");
            fSetupFragPosition = true;
        }
        return "gl_FragCoord";
    } else {
        static const char* kTempName = "tmpXYFragCoord";
        static const char* kCoordName = "fragCoordYDown";
        if (!fSetupFragPosition) {
            const char* rtHeightName;

            fProgramBuilder->addRTHeightUniform("RTHeight", &rtHeightName);

            // The Adreno compiler seems to be very touchy about access to "gl_FragCoord".
            // Accessing glFragCoord.zw can cause a program to fail to link. Additionally,
            // depending on the surrounding code, accessing .xy with a uniform involved can
            // do the same thing. Copying gl_FragCoord.xy into a temp vec2 beforehand
            // (and only accessing .xy) seems to "fix" things.
            const char* precision = glslCaps->usesPrecisionModifiers() ? "highp " : "";
            this->codePrependf("\t%svec4 %s = vec4(%s.x, %s - %s.y, 1.0, 1.0);\n",
                               precision, kCoordName, kTempName, rtHeightName, kTempName);
            this->codePrependf("%svec2 %s = gl_FragCoord.xy;", precision, kTempName);
            fSetupFragPosition = true;
        }
        SkASSERT(fProgramBuilder->fUniformHandles.fRTHeightUni.isValid());
        return kCoordName;
    }
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
    const GrGLSLCaps& glslCaps = *fProgramBuilder->glslCaps();
    if (!glslCaps.sampleVariablesSupport()) {
        SkDEBUGFAIL("Attempted to mask sample coverage without support.");
        return;
    }
    if (const char* extension = glslCaps.sampleVariablesExtensionString()) {
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
    const GrGLSLCaps& glslCaps = *fProgramBuilder->glslCaps();
    if (!glslCaps.sampleMaskOverrideCoverageSupport()) {
        SkDEBUGFAIL("Attempted to override sample coverage without support.");
        return;
    }
    SkASSERT(glslCaps.sampleVariablesSupport());
    if (const char* extension = glslCaps.sampleVariablesExtensionString()) {
        this->addFeature(1 << kSampleVariables_GLSLPrivateFeature, extension);
    }
    if (this->addFeature(1 << kSampleMaskOverrideCoverage_GLSLPrivateFeature,
                         "GL_NV_sample_mask_override_coverage")) {
        // Redeclare gl_SampleMask with layout(override_coverage) if we haven't already.
        fOutputs.push_back().set(kInt_GrSLType, GrShaderVar::kOut_TypeModifier,
                                 "gl_SampleMask", 1, kHigh_GrSLPrecision,
                                 "override_coverage");
    }
    this->codeAppendf("gl_SampleMask[0] = %s;", mask);
    fHasInitializedSampleMask = true;
}

const char* GrGLSLFragmentShaderBuilder::dstColor() {
    SkDEBUGCODE(fHasReadDstColor = true;)

    const char* override = fProgramBuilder->primitiveProcessor().getDestColorOverride();
    if (override != nullptr) {
        return override;
    }

    const GrGLSLCaps* glslCaps = fProgramBuilder->glslCaps();
    if (glslCaps->fbFetchSupport()) {
        this->addFeature(1 << kFramebufferFetch_GLSLPrivateFeature,
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
        fProgramBuilder->finalizeFragmentOutputColor(fOutputs.back());
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
        fProgramBuilder->finalizeFragmentSecondaryColor(fOutputs.back());
    }
}

const char* GrGLSLFragmentShaderBuilder::getPrimaryColorOutputName() const {
    return fHasCustomColorOutput ? DeclaredColorOutputName() : "gl_FragColor";
}

void GrGLSLFragmentBuilder::declAppendf(const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    inputs().appendVAList(fmt, argp);
    va_end(argp);
}

const char* GrGLSLFragmentShaderBuilder::getSecondaryColorOutputName() const {
    const GrGLSLCaps& caps = *fProgramBuilder->glslCaps();
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
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision,
                                                 *fProgramBuilder->glslCaps(),
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
    const GrGpu::MultisampleSpecs& specs = rtp.getMultisampleSpecs(pipeline.getStencil());
    SkSTArray<16, SkPoint, true> offsets;
    offsets.push_back_n(specs.fEffectiveSampleCnt);
    m.mapPoints(offsets.begin(), specs.fSampleLocations.get(), specs.fEffectiveSampleCnt);
    this->definitions().append("const ");
    if (fProgramBuilder->glslCaps()->usesPrecisionModifiers()) {
        this->definitions().append("highp ");
    }
    this->definitions().appendf("vec2 %s[] = vec2[](", name);
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
