/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLSLCaps.h"

#include "GrContextOptions.h"

////////////////////////////////////////////////////////////////////////////////////////////

GrGLSLCaps::GrGLSLCaps(const GrContextOptions& options) {
    fGLSLGeneration = k330_GrGLSLGeneration;

    fDropsTileOnZeroDivide = false;
    fFBFetchSupport = false;
    fFBFetchNeedsCustomOutput = false;
    fBindlessTextureSupport = false;
    fUsesPrecisionModifiers = false;
    fCanUseAnyFunctionInShader = true;
    fCanUseMinAndAbsTogether = true;
    fMustForceNegatedAtanParamToFloat = false;
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
    fMaxVertexSamplers = 0;
    fMaxGeometrySamplers = 0;
    fMaxFragmentSamplers = 0;
    fMaxCombinedSamplers = 0;
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
}

SkString GrGLSLCaps::dump() const {
    SkString r = INHERITED::dump();

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
    r.appendf("Advanced blend equation interaction: %s\n",
              kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);
    return r;
}

void GrGLSLCaps::initSamplerPrecisionTable() {
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
        table[kUnknown_GrPixelConfig]    = kDefault_GrSLPrecision;
        table[kAlpha_8_GrPixelConfig]    = lowp;
        table[kIndex_8_GrPixelConfig]    = lowp;
        table[kRGB_565_GrPixelConfig]    = lowp;
        table[kRGBA_4444_GrPixelConfig]  = lowp;
        table[kRGBA_8888_GrPixelConfig]  = lowp;
        table[kBGRA_8888_GrPixelConfig]  = lowp;
        table[kSRGBA_8888_GrPixelConfig] = lowp;
        table[kSBGRA_8888_GrPixelConfig] = lowp;
        table[kETC1_GrPixelConfig]       = lowp;
        table[kLATC_GrPixelConfig]       = lowp;
        table[kR11_EAC_GrPixelConfig]    = lowp;
        table[kASTC_12x12_GrPixelConfig] = lowp;
        table[kRGBA_float_GrPixelConfig] = kHigh_GrSLPrecision;
        table[kAlpha_half_GrPixelConfig] = mediump;
        table[kRGBA_half_GrPixelConfig]  = mediump;

        GR_STATIC_ASSERT(16 == kGrPixelConfigCnt);
    }
}

void GrGLSLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {
}
