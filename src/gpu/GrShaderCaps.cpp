/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrShaderCaps.h"

#include "GrContextOptions.h"

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
    fMustForceNegatedAtanParamToFloat = false;
    fAtan2ImplementedAsAtanYOverX = false;
    fRequiresLocalOutputColorForFBFetch = false;
    fMustImplementGSInvocationsWithLoop = false;
    fFlatInterpolationSupport = false;
    fNoPerspectiveInterpolationSupport = false;
    fMultisampleInterpolationSupport = false;
    fSampleVariablesSupport = false;
    fSampleMaskOverrideCoverageSupport = false;
    fExternalTextureSupport = false;
    fTexelFetchSupport = false;

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

SkString GrShaderCaps::dump() const {
    SkString r;
    static const char* gNY[] = { "NO", "YES" };
    r.appendf("Shader Derivative Support          : %s\n", gNY[fShaderDerivativeSupport]);
    r.appendf("Geometry Shader Support            : %s\n", gNY[fGeometryShaderSupport]);
    r.appendf("Path Rendering Support             : %s\n", gNY[fPathRenderingSupport]);
    r.appendf("Dst Read In Shader Support         : %s\n", gNY[fDstReadInShaderSupport]);
    r.appendf("Dual Source Blending Support       : %s\n", gNY[fDualSourceBlendingSupport]);
    r.appendf("Integer Support                    : %s\n", gNY[fIntegerSupport]);
    r.appendf("Texel Buffer Support               : %s\n", gNY[fTexelBufferSupport]);
    r.appendf("Image Load Store Support           : %s\n", gNY[fImageLoadStoreSupport]);

    r.appendf("Shader Float Precisions (varies: %s):\n", gNY[fShaderPrecisionVaries]);

    for (int s = 0; s < kGrShaderTypeCount; ++s) {
        GrShaderType shaderType = static_cast<GrShaderType>(s);
        r.appendf("\t%s:\n", shader_type_to_string(shaderType));
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            if (fFloatPrecisions[s][p].supported()) {
                GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                r.appendf("\t\t%s: log_low: %d log_high: %d bits: %d\n",
                    precision_to_string(precision),
                    fFloatPrecisions[s][p].fLogRangeLow,
                    fFloatPrecisions[s][p].fLogRangeHigh,
                    fFloatPrecisions[s][p].fBits);
            }
        }
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

    r.appendf("--- GLSL-Specific ---\n");

    r.appendf("FB Fetch Support: %s\n", (fFBFetchSupport ? "YES" : "NO"));
    r.appendf("Drops tile on zero divide: %s\n", (fDropsTileOnZeroDivide ? "YES" : "NO"));
    r.appendf("Bindless texture support: %s\n", (fBindlessTextureSupport ? "YES" : "NO"));
    r.appendf("Uses precision modifiers: %s\n", (fUsesPrecisionModifiers ? "YES" : "NO"));
    r.appendf("Can use any() function: %s\n", (fCanUseAnyFunctionInShader ? "YES" : "NO"));
    r.appendf("Can use min() and abs() together: %s\n", (fCanUseMinAndAbsTogether ? "YES" : "NO"));
    r.appendf("Must force negated atan param to float: %s\n", (fMustForceNegatedAtanParamToFloat ?
                                                               "YES" : "NO"));
    r.appendf("Must use local out color for FBFetch: %s\n", (fRequiresLocalOutputColorForFBFetch ?
                                                             "YES" : "NO"));
    r.appendf("Must implement geo shader invocations with loop : %s\n",
              (fMustImplementGSInvocationsWithLoop ? "YES" : "NO"));
    r.appendf("Flat interpolation support: %s\n", (fFlatInterpolationSupport ?  "YES" : "NO"));
    r.appendf("No perspective interpolation support: %s\n", (fNoPerspectiveInterpolationSupport ?
                                                             "YES" : "NO"));
    r.appendf("Multisample interpolation support: %s\n", (fMultisampleInterpolationSupport ?
                                                          "YES" : "NO"));
    r.appendf("Sample variables support: %s\n", (fSampleVariablesSupport ? "YES" : "NO"));
    r.appendf("Sample mask override coverage support: %s\n", (fSampleMaskOverrideCoverageSupport ?
                                                              "YES" : "NO"));
    r.appendf("External texture support: %s\n", (fExternalTextureSupport ? "YES" : "NO"));
    r.appendf("texelFetch support: %s\n", (fTexelFetchSupport ? "YES" : "NO"));
    r.appendf("Max VS Samplers: %d\n", fMaxVertexSamplers);
    r.appendf("Max GS Samplers: %d\n", fMaxGeometrySamplers);
    r.appendf("Max FS Samplers: %d\n", fMaxFragmentSamplers);
    r.appendf("Max Combined Samplers: %d\n", fMaxFragmentSamplers);
    r.appendf("Max VS Image Storages: %d\n", fMaxVertexImageStorages);
    r.appendf("Max GS Image Storages: %d\n", fMaxGeometryImageStorages);
    r.appendf("Max FS Image Storages: %d\n", fMaxFragmentImageStorages);
    r.appendf("Max Combined Image Storages: %d\n", fMaxFragmentImageStorages);
    r.appendf("Advanced blend equation interaction: %s\n",
              kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);
    return r;
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
        table[kETC1_GrPixelConfig]           = lowp;
        table[kRGBA_float_GrPixelConfig]     = kHigh_GrSLPrecision;
        table[kRG_float_GrPixelConfig]       = kHigh_GrSLPrecision;
        table[kAlpha_half_GrPixelConfig]     = mediump;
        table[kRGBA_half_GrPixelConfig]      = mediump;

        GR_STATIC_ASSERT(15 == kGrPixelConfigCnt);
    }
}

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
    fDualSourceBlendingSupport = fDualSourceBlendingSupport && !options.fSuppressDualSourceBlending;
}
