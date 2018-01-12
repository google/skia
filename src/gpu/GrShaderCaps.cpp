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

GrShaderCaps::GrShaderCaps(const GrContextOptions& options) {
    fGLSLGeneration = k330_GrGLSLGeneration;
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fGSInvocationsSupport = false;
    fPathRenderingSupport = false;
    fDstReadInShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fIntegerSupport = false;
    fTexelBufferSupport = false;
    fImageLoadStoreSupport = false;
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
    fMustDoOpBetweenFloorAndAbs = false;
    fRequiresLocalOutputColorForFBFetch = false;
    fMustObfuscateUniformColor = false;
    fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    fCanUseFragCoord = true;
    fFlatInterpolationSupport = false;
    fPreferFlatInterpolation = false;
    fNoPerspectiveInterpolationSupport = false;
    fMultisampleInterpolationSupport = false;
    fSampleVariablesSupport = false;
    fSampleMaskOverrideCoverageSupport = false;
    fExternalTextureSupport = false;
    fTexelFetchSupport = false;
    fVertexIDSupport = false;
    fFloatIs32Bits = true;
    fHalfIs32Bits = false;

    fVersionDeclString = nullptr;
    fShaderDerivativeExtensionString = nullptr;
    fGeometryShaderExtensionString = nullptr;
    fGSInvocationsExtensionString = nullptr;
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
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;

    // TODO: Default this to 0 and only enable image multitexturing when a "safe" threshold is
    // known for a GPU class.
    fDisableImageMultitexturingDstRectAreaThreshold = std::numeric_limits<size_t>::max();
}

void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("Shader Derivative Support", fShaderDerivativeSupport);
    writer->appendBool("Geometry Shader Support", fGeometryShaderSupport);
    writer->appendBool("Geometry Shader Invocations Support", fGSInvocationsSupport);
    writer->appendBool("Path Rendering Support", fPathRenderingSupport);
    writer->appendBool("Dst Read In Shader Support", fDstReadInShaderSupport);
    writer->appendBool("Dual Source Blending Support", fDualSourceBlendingSupport);
    writer->appendBool("Integer Support", fIntegerSupport);
    writer->appendBool("Texel Buffer Support", fTexelBufferSupport);
    writer->appendBool("Image Load Store Support", fImageLoadStoreSupport);

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

    writer->appendBool("FB Fetch Support", fFBFetchSupport);
    writer->appendBool("Drops tile on zero divide", fDropsTileOnZeroDivide);
    writer->appendBool("Bindless texture support", fBindlessTextureSupport);
    writer->appendBool("Uses precision modifiers", fUsesPrecisionModifiers);
    writer->appendBool("Can use any() function", fCanUseAnyFunctionInShader);
    writer->appendBool("Can use min() and abs() together", fCanUseMinAndAbsTogether);
    writer->appendBool("Can use fract() for negative values", fCanUseFractForNegativeValues);
    writer->appendBool("Must force negated atan param to float", fMustForceNegatedAtanParamToFloat);
    writer->appendBool("Must do op between floor and abs", fMustDoOpBetweenFloorAndAbs);
    writer->appendBool("Must use local out color for FBFetch", fRequiresLocalOutputColorForFBFetch);
    writer->appendBool("Must obfuscate uniform color", fMustObfuscateUniformColor);
    writer->appendBool("Must guard division even after explicit zero check",
                       fMustGuardDivisionEvenAfterExplicitZeroCheck);
    writer->appendBool("Can use gl_FragCoord", fCanUseFragCoord);
    writer->appendBool("Flat interpolation support", fFlatInterpolationSupport);
    writer->appendBool("Prefer flat interpolation", fPreferFlatInterpolation);
    writer->appendBool("No perspective interpolation support", fNoPerspectiveInterpolationSupport);
    writer->appendBool("Multisample interpolation support", fMultisampleInterpolationSupport);
    writer->appendBool("Sample variables support", fSampleVariablesSupport);
    writer->appendBool("Sample mask override coverage support", fSampleMaskOverrideCoverageSupport);
    writer->appendBool("External texture support", fExternalTextureSupport);
    writer->appendBool("texelFetch support", fTexelFetchSupport);
    writer->appendBool("sk_VertexID support", fVertexIDSupport);
    writer->appendBool("float == fp32", fFloatIs32Bits);
    writer->appendBool("half == fp32", fHalfIs32Bits);

    writer->appendS32("Max VS Samplers", fMaxVertexSamplers);
    writer->appendS32("Max GS Samplers", fMaxGeometrySamplers);
    writer->appendS32("Max FS Samplers", fMaxFragmentSamplers);
    writer->appendS32("Max Combined Samplers", fMaxFragmentSamplers);
    writer->appendString("Advanced blend equation interaction",
                         kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);
    writer->appendU64("Disable image multitexturing dst area threshold",
                      fDisableImageMultitexturingDstRectAreaThreshold);

    writer->endObject();
}

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
#if GR_TEST_UTILS
    fDualSourceBlendingSupport = fDualSourceBlendingSupport && !options.fSuppressDualSourceBlending;
    if (options.fDisableImageMultitexturing) {
        fDisableImageMultitexturingDstRectAreaThreshold = 0;
    }
#endif
}
