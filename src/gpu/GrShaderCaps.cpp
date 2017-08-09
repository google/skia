/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrShaderCaps.h"

#include "GrContextOptions.h"
#include "SkJSONWriter.h"

////////////////////////////////////////////////////////////////////////////////////////////

static const char* shader_type_to_string(GrShaderType type) {
    switch (type) {
    case kVertex_GrShaderType:
        return "vertex";
    case kGeometry_GrShaderType:
        return "geometry";
    case kFragment_GrShaderType:
        return "fragment";
    }
    return "";
}

static const char* precision_to_string(GrSLPrecision p) {
    switch (p) {
    case kLow_GrSLPrecision:
        return "low";
    case kMedium_GrSLPrecision:
        return "medium";
    case kHigh_GrSLPrecision:
        return "high";
    default:
        SkFAIL("Unexpected precision type.");
        return "";
    }
}

GrShaderCaps::GrShaderCaps(const GrContextOptions& options) {
    fGLSLGeneration = k330_GrGLSLGeneration;
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fIntegerSupport = false;
    fTexelBufferSupport = false;
    fImageLoadStoreSupport = false;
    fShaderPrecisionVaries = false;
    fDropsTileOnZeroDivide = false;
    fFBFetchSupport = false;
    fFBFetchNeedsCustomOutput = false;
    fBindlessTextureSupport = false;
    fUsesPrecisionModifiers = false;
    fCanUseAnyFunctionInShader = true;
    fCanUseMinAndAbsTogether = true;
    fCanUseFractForNegativeValues = true;
    fMustForceNegatedAtanParamToFloat = false;
    fAtan2ImplementedAsAtanYOverX = false;
    fRequiresLocalOutputColorForFBFetch = false;
    fMustImplementGSInvocationsWithLoop = false;
    fMustObfuscateUniformColor = false;
    fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    fFlatInterpolationSupport = false;
    fNoPerspectiveInterpolationSupport = false;
    fMultisampleInterpolationSupport = false;
    fSampleVariablesSupport = false;
    fSampleMaskOverrideCoverageSupport = false;
    fExternalTextureSupport = false;
    fTexelFetchSupport = false;
    fVertexIDSupport = false;

    fVersionDeclString = nullptr;
    fShaderDerivativeExtensionString = nullptr;
    fFragCoordConventionsExtensionString = nullptr;
    fSecondaryOutputExtensionString = nullptr;
    fExternalTextureExtensionString = nullptr;
    fTexelBufferExtensionString = nullptr;
    fNoPerspectiveInterpolationExtensionString = nullptr;
    fMultisampleInterpolationExtensionString = nullptr;
    fSampleVariablesExtensionString = nullptr;
    fFBFetchColorName = nullptr;
    fFBFetchExtensionString = nullptr;
    fImageLoadStoreExtensionString = nullptr;
    fMaxVertexSamplers = 0;
    fMaxGeometrySamplers = 0;
    fMaxFragmentSamplers = 0;
    fMaxCombinedSamplers = 0;
    fMaxVertexImageStorages = 0;
    fMaxGeometryImageStorages = 0;
    fMaxFragmentImageStorages = 0;
    fMaxCombinedImageStorages   = 0;
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
}

void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    static const char* gNY[] = { "NO", "YES" };
    writer->appendString("Shader Derivative Support", gNY[fShaderDerivativeSupport]);
    writer->appendString("Geometry Shader Support", gNY[fGeometryShaderSupport]);
    writer->appendString("Path Rendering Support", gNY[fPathRenderingSupport]);
    writer->appendString("Dst Read In Shader Support", gNY[fDstReadInShaderSupport]);
    writer->appendString("Dual Source Blending Support", gNY[fDualSourceBlendingSupport]);
    writer->appendString("Integer Support", gNY[fIntegerSupport]);
    writer->appendString("Texel Buffer Support", gNY[fTexelBufferSupport]);
    writer->appendString("Image Load Store Support", gNY[fImageLoadStoreSupport]);

    writer->appendString("Variable Precision", gNY[fShaderPrecisionVaries]);

    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        GrShaderType shaderType = static_cast<GrShaderType>(s);
        writer->beginArray(SkStringPrintf("%s precisions",
                                          shader_type_to_string(shaderType)).c_str());
        // TODO: Structured JSON?
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            if (fFloatPrecisions[s][p].supported()) {
                GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                writer->appendString(SkStringPrintf("%s: log_low: %d log_high: %d bits: %d",
                                                    precision_to_string(precision),
                                                    fFloatPrecisions[s][p].fLogRangeLow,
                                                    fFloatPrecisions[s][p].fLogRangeHigh,
                                                    fFloatPrecisions[s][p].fBits).c_str());
            }
        }
        writer->endArray();
    }

    static const char* kAdvBlendEqInteractionStr[] = {
        "Not Supported",
        "Automatic",
        "General Enable",
        "Specific Enables",
    };
    GR_STATIC_ASSERT(0 == kNotSupported_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(1 == kAutomatic_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(2 == kGeneralEnable_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(3 == kSpecificEnables_AdvBlendEqInteraction);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kAdvBlendEqInteractionStr) == kLast_AdvBlendEqInteraction + 1);

    writer->appendString("FB Fetch Support", gNY[fFBFetchSupport]);
    writer->appendString("Drops tile on zero divide", gNY[fDropsTileOnZeroDivide]);
    writer->appendString("Bindless texture support", gNY[fBindlessTextureSupport]);
    writer->appendString("Uses precision modifiers", gNY[fUsesPrecisionModifiers]);
    writer->appendString("Can use any() function", gNY[fCanUseAnyFunctionInShader]);
    writer->appendString("Can use min() and abs() together", gNY[fCanUseMinAndAbsTogether]);
    writer->appendString("Can use fract() for negative values", gNY[fCanUseFractForNegativeValues]);
    writer->appendString("Must force negated atan param to float",
                         gNY[fMustForceNegatedAtanParamToFloat]);
    writer->appendString("Must use local out color for FBFetch",
                         gNY[fRequiresLocalOutputColorForFBFetch]);
    writer->appendString("Must implement geo shader invocations with loop",
                         gNY[fMustImplementGSInvocationsWithLoop]);
    writer->appendString("Must obfuscate uniform color", gNY[fMustObfuscateUniformColor]);
    writer->appendString("Must guard division even after explicit zero check",
                         gNY[fMustGuardDivisionEvenAfterExplicitZeroCheck]);
    writer->appendString("Flat interpolation support", gNY[fFlatInterpolationSupport]);
    writer->appendString("No perspective interpolation support",
                         gNY[fNoPerspectiveInterpolationSupport]);
    writer->appendString("Multisample interpolation support",
                         gNY[fMultisampleInterpolationSupport]);
    writer->appendString("Sample variables support", gNY[fSampleVariablesSupport]);
    writer->appendString("Sample mask override coverage support",
                         gNY[fSampleMaskOverrideCoverageSupport]);
    writer->appendString("External texture support", gNY[fExternalTextureSupport]);
    writer->appendString("texelFetch support", gNY[fTexelFetchSupport]);
    writer->appendString("sk_VertexID support", gNY[fVertexIDSupport]);

    writer->appendS32("Max VS Samplers", fMaxVertexSamplers);
    writer->appendS32("Max GS Samplers", fMaxGeometrySamplers);
    writer->appendS32("Max FS Samplers", fMaxFragmentSamplers);
    writer->appendS32("Max Combined Samplers", fMaxFragmentSamplers);
    writer->appendS32("Max VS Image Storages", fMaxVertexImageStorages);
    writer->appendS32("Max GS Image Storages", fMaxGeometryImageStorages);
    writer->appendS32("Max FS Image Storages", fMaxFragmentImageStorages);
    writer->appendS32("Max Combined Image Storages", fMaxFragmentImageStorages);
    writer->appendString("Advanced blend equation interaction",
                         kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);

    writer->endObject();
}

void GrShaderCaps::initSamplerPrecisionTable() {
    // Determine the largest precision qualifiers that are effectively the same as lowp/mediump.
    //   e.g. if lowp == mediump, then use mediump instead of lowp.
    GrSLPrecision effectiveMediumP[kGrShaderTypeCount];
    GrSLPrecision effectiveLowP[kGrShaderTypeCount];
    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        const PrecisionInfo* info = fFloatPrecisions[s];
        effectiveMediumP[s] = info[kHigh_GrSLPrecision] == info[kMedium_GrSLPrecision] ?
                                  kHigh_GrSLPrecision : kMedium_GrSLPrecision;
        effectiveLowP[s] = info[kMedium_GrSLPrecision] == info[kLow_GrSLPrecision] ?
                               effectiveMediumP[s] : kLow_GrSLPrecision;
    }

    // Determine which precision qualifiers should be used with samplers.
    for (int visibility = 0; visibility < (1 << kGrShaderTypeCount); ++visibility) {
        GrSLPrecision mediump = kHigh_GrSLPrecision;
        GrSLPrecision lowp = kHigh_GrSLPrecision;
        for (int s = 0; s < kGrShaderTypeCount; ++s) {
            if (visibility & (1 << s)) {
                mediump = SkTMin(mediump, effectiveMediumP[s]);
                lowp = SkTMin(lowp, effectiveLowP[s]);
            }

            GR_STATIC_ASSERT(0 == kLow_GrSLPrecision);
            GR_STATIC_ASSERT(1 == kMedium_GrSLPrecision);
            GR_STATIC_ASSERT(2 == kHigh_GrSLPrecision);

            GR_STATIC_ASSERT((1 << kVertex_GrShaderType) == kVertex_GrShaderFlag);
            GR_STATIC_ASSERT((1 << kGeometry_GrShaderType) == kGeometry_GrShaderFlag);
            GR_STATIC_ASSERT((1 << kFragment_GrShaderType) == kFragment_GrShaderFlag);
            GR_STATIC_ASSERT(3 == kGrShaderTypeCount);
        }

        uint8_t* table = fSamplerPrecisions[visibility];
        table[kUnknown_GrPixelConfig]        = lowp;
        table[kAlpha_8_GrPixelConfig]        = lowp;
        table[kGray_8_GrPixelConfig]         = lowp;
        table[kRGB_565_GrPixelConfig]        = lowp;
        table[kRGBA_4444_GrPixelConfig]      = lowp;
        table[kRGBA_8888_GrPixelConfig]      = lowp;
        table[kBGRA_8888_GrPixelConfig]      = lowp;
        table[kSRGBA_8888_GrPixelConfig]     = lowp;
        table[kSBGRA_8888_GrPixelConfig]     = lowp;
        table[kRGBA_8888_sint_GrPixelConfig] = lowp;
        table[kRGBA_float_GrPixelConfig]     = kHigh_GrSLPrecision;
        table[kRG_float_GrPixelConfig]       = kHigh_GrSLPrecision;
        table[kAlpha_half_GrPixelConfig]     = mediump;
        table[kRGBA_half_GrPixelConfig]      = mediump;

        GR_STATIC_ASSERT(14 == kGrPixelConfigCnt);
    }
}

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
    fDualSourceBlendingSupport = fDualSourceBlendingSupport && !options.fSuppressDualSourceBlending;
}
