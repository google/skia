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
    fBufferTextureSupport = false;
    fVersionDeclString = nullptr;
    fShaderDerivativeExtensionString = nullptr;
    fFragCoordConventionsExtensionString = nullptr;
    fSecondaryOutputExtensionString = nullptr;
    fExternalTextureExtensionString = nullptr;
    fBufferTextureExtensionString = nullptr;
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
    r.appendf("Buffer texture support: %s\n", (fBufferTextureSupport ? "YES" : "NO"));
    r.appendf("Max VS Samplers: %d\n", fMaxVertexSamplers);
    r.appendf("Max GS Samplers: %d\n", fMaxGeometrySamplers);
    r.appendf("Max FS Samplers: %d\n", fMaxFragmentSamplers);
    r.appendf("Max Combined Samplers: %d\n", fMaxFragmentSamplers);
    r.appendf("Advanced blend equation interaction: %s\n",
              kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);
    return r;
}

void GrGLSLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {
}
