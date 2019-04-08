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
    fImageLoadStoreSupport = false;
    fDropsTileOnZeroDivide = false;
    fFBFetchSupport = false;
    fFBFetchNeedsCustomOutput = false;
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
    fIncompleteShortIntPrecision = false;
    fAddAndTrueToLoopCondition = false;
    fUnfoldShortCircuitAsTernary = false;
    fEmulateAbsIntFunction = false;
    fRewriteDoWhileLoops = false;
    fRemovePowWithConstantExponent = false;
    fMustWriteToFragColor = false;
    fFlatInterpolationSupport = false;
    fPreferFlatInterpolation = false;
    fNoPerspectiveInterpolationSupport = false;
    fSampleVariablesSupport = false;
    fExternalTextureSupport = false;
    fVertexIDSupport = false;
    fFPManipulationSupport = false;
    fFloatIs32Bits = true;
    fHalfIs32Bits = false;
    fHasLowFragmentPrecision = false;
    fUnsignedSupport = false;
    fBuiltinFMASupport = false;

    fVersionDeclString = nullptr;
    fShaderDerivativeExtensionString = nullptr;
    fGeometryShaderExtensionString = nullptr;
    fGSInvocationsExtensionString = nullptr;
    fFragCoordConventionsExtensionString = nullptr;
    fSecondaryOutputExtensionString = nullptr;
    fExternalTextureExtensionString = nullptr;
    fSecondExternalTextureExtensionString = nullptr;
    fNoPerspectiveInterpolationExtensionString = nullptr;
    fSampleVariablesExtensionString = nullptr;
    fFBFetchColorName = nullptr;
    fFBFetchExtensionString = nullptr;
    fImageLoadStoreExtensionString = nullptr;
    fMaxFragmentSamplers = 0;
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
}

#ifdef SK_ENABLE_DUMP_GPU
void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("Shader Derivative Support", fShaderDerivativeSupport);
    writer->appendBool("Geometry Shader Support", fGeometryShaderSupport);
    writer->appendBool("Geometry Shader Invocations Support", fGSInvocationsSupport);
    writer->appendBool("Path Rendering Support", fPathRenderingSupport);
    writer->appendBool("Dst Read In Shader Support", fDstReadInShaderSupport);
    writer->appendBool("Dual Source Blending Support", fDualSourceBlendingSupport);
    writer->appendBool("Integer Support", fIntegerSupport);
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
    writer->appendBool("Incomplete short int precision", fIncompleteShortIntPrecision);
    writer->appendBool("Add and true to loops workaround", fAddAndTrueToLoopCondition);
    writer->appendBool("Unfold short circuit as ternary", fUnfoldShortCircuitAsTernary);
    writer->appendBool("Emulate abs(int) function", fEmulateAbsIntFunction);
    writer->appendBool("Rewrite do while loops", fRewriteDoWhileLoops);
    writer->appendBool("Rewrite pow with constant exponent", fRemovePowWithConstantExponent);
    writer->appendBool("Must write to sk_FragColor [workaround]", fMustWriteToFragColor);
    writer->appendBool("Flat interpolation support", fFlatInterpolationSupport);
    writer->appendBool("Prefer flat interpolation", fPreferFlatInterpolation);
    writer->appendBool("No perspective interpolation support", fNoPerspectiveInterpolationSupport);
    writer->appendBool("Sample variables support", fSampleVariablesSupport);
    writer->appendBool("External texture support", fExternalTextureSupport);
    writer->appendBool("sk_VertexID support", fVertexIDSupport);
    writer->appendBool("Floating point manipulation support", fFPManipulationSupport);
    writer->appendBool("float == fp32", fFloatIs32Bits);
    writer->appendBool("half == fp32", fHalfIs32Bits);
    writer->appendBool("Has poor fragment precision", fHasLowFragmentPrecision);
    writer->appendBool("Builtin fma() support", fBuiltinFMASupport);

    writer->appendS32("Max FS Samplers", fMaxFragmentSamplers);
    writer->appendString("Advanced blend equation interaction",
                         kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);

    writer->endObject();
}
#else
void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const { }
#endif

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(fCanUseAnyFunctionInShader);
        SkASSERT(fCanUseMinAndAbsTogether);
        SkASSERT(fCanUseFractForNegativeValues);
        SkASSERT(!fMustForceNegatedAtanParamToFloat);
        SkASSERT(!fAtan2ImplementedAsAtanYOverX);
        SkASSERT(!fMustDoOpBetweenFloorAndAbs);
        SkASSERT(!fRequiresLocalOutputColorForFBFetch);
        SkASSERT(!fMustObfuscateUniformColor);
        SkASSERT(!fMustGuardDivisionEvenAfterExplicitZeroCheck);
        SkASSERT(fCanUseFragCoord);
        SkASSERT(!fIncompleteShortIntPrecision);
        SkASSERT(!fAddAndTrueToLoopCondition);
        SkASSERT(!fUnfoldShortCircuitAsTernary);
        SkASSERT(!fEmulateAbsIntFunction);
        SkASSERT(!fRewriteDoWhileLoops);
        SkASSERT(!fRemovePowWithConstantExponent);
        SkASSERT(!fMustWriteToFragColor);
    }
#if GR_TEST_UTILS
    fDualSourceBlendingSupport = fDualSourceBlendingSupport && !options.fSuppressDualSourceBlending;
#endif
}
