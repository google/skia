/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrShaderCaps.h"

#include "include/gpu/GrContextOptions.h"
#include "src/utils/SkJSONWriter.h"

////////////////////////////////////////////////////////////////////////////////////////////

GrShaderCaps::GrShaderCaps(const GrContextOptions& options) {
    fGLSLGeneration = k330_GrGLSLGeneration;
    fShaderDerivativeSupport = false;
    fGeometryShaderSupport = false;
    fGSInvocationsSupport = false;
    fDstReadInShaderSupport = false;
    fDualSourceBlendingSupport = false;
    fIntegerSupport = false;
    fNonsquareMatrixSupport = false;
    fFBFetchSupport = false;
    fFBFetchNeedsCustomOutput = false;
    fUsesPrecisionModifiers = false;
    fCanUseAnyFunctionInShader = true;
    fCanUseMinAndAbsTogether = true;
    fCanUseFractForNegativeValues = true;
    fMustForceNegatedAtanParamToFloat = false;
    fMustForceNegatedLdexpParamToMultiply = false;
    fAtan2ImplementedAsAtanYOverX = false;
    fMustDoOpBetweenFloorAndAbs = false;
    fRequiresLocalOutputColorForFBFetch = false;
    fMustObfuscateUniformColor = false;
    fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    fInBlendModesFailRandomlyForAllZeroVec = false;
    fCanUseFragCoord = true;
    fIncompleteShortIntPrecision = false;
    fAddAndTrueToLoopCondition = false;
    fUnfoldShortCircuitAsTernary = false;
    fEmulateAbsIntFunction = false;
    fRewriteDoWhileLoops = false;
    fRemovePowWithConstantExponent = false;
    fMustWriteToFragColor = false;
    fNoDefaultPrecisionForExternalSamplers = false;
    fRewriteMatrixVectorMultiply = false;
    fRewriteMatrixComparisons = false;
    fFlatInterpolationSupport = false;
    fPreferFlatInterpolation = false;
    fNoPerspectiveInterpolationSupport = false;
    fSampleMaskSupport = false;
    fExternalTextureSupport = false;
    fVertexIDSupport = false;
    fInfinitySupport = false;
    fBitManipulationSupport = false;
    fFloatIs32Bits = true;
    fHalfIs32Bits = false;
    fHasLowFragmentPrecision = false;
    fReducedShaderMode = false;
    fColorSpaceMathNeedsFloat = false;
    fBuiltinFMASupport = false;
    fBuiltinDeterminantSupport = false;
    fCanUseDoLoops = true;
    fCanUseFastMath = false;
    fUseNodePools = true;
    fAvoidDfDxForGradientsWhenPossible = false;

    fVersionDeclString = nullptr;
    fShaderDerivativeExtensionString = nullptr;
    fGeometryShaderExtensionString = nullptr;
    fGSInvocationsExtensionString = nullptr;
    fSecondaryOutputExtensionString = nullptr;
    fExternalTextureExtensionString = nullptr;
    fSecondExternalTextureExtensionString = nullptr;
    fNoPerspectiveInterpolationExtensionString = nullptr;
    fSampleVariablesExtensionString = nullptr;
    fTessellationExtensionString = nullptr;
    fFBFetchColorName = nullptr;
    fFBFetchExtensionString = nullptr;
    fMaxFragmentSamplers = 0;
    fMaxTessellationSegments = 0;
    fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
}

#ifdef SK_ENABLE_DUMP_GPU
void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("Shader Derivative Support", fShaderDerivativeSupport);
    writer->appendBool("Geometry Shader Support", fGeometryShaderSupport);
    writer->appendBool("Geometry Shader Invocations Support", fGSInvocationsSupport);
    writer->appendBool("Dst Read In Shader Support", fDstReadInShaderSupport);
    writer->appendBool("Dual Source Blending Support", fDualSourceBlendingSupport);
    writer->appendBool("Integer Support", fIntegerSupport);
    writer->appendBool("Nonsquare Matrix Support", fNonsquareMatrixSupport);

    static const char* kAdvBlendEqInteractionStr[] = {
        "Not Supported",
        "Automatic",
        "General Enable",
    };
    static_assert(0 == kNotSupported_AdvBlendEqInteraction);
    static_assert(1 == kAutomatic_AdvBlendEqInteraction);
    static_assert(2 == kGeneralEnable_AdvBlendEqInteraction);
    static_assert(SK_ARRAY_COUNT(kAdvBlendEqInteractionStr) == kLast_AdvBlendEqInteraction + 1);

    writer->appendBool("FB Fetch Support", fFBFetchSupport);
    writer->appendBool("Uses precision modifiers", fUsesPrecisionModifiers);
    writer->appendBool("Can use any() function", fCanUseAnyFunctionInShader);
    writer->appendBool("Can use min() and abs() together", fCanUseMinAndAbsTogether);
    writer->appendBool("Can use fract() for negative values", fCanUseFractForNegativeValues);
    writer->appendBool("Must force negated atan param to float", fMustForceNegatedAtanParamToFloat);
    writer->appendBool("Must force negated ldexp param to multiply",
                       fMustForceNegatedLdexpParamToMultiply);
    writer->appendBool("Must do op between floor and abs", fMustDoOpBetweenFloorAndAbs);
    writer->appendBool("Must use local out color for FBFetch", fRequiresLocalOutputColorForFBFetch);
    writer->appendBool("Must obfuscate uniform color", fMustObfuscateUniformColor);
    writer->appendBool("Must guard division even after explicit zero check",
                       fMustGuardDivisionEvenAfterExplicitZeroCheck);
    writer->appendBool(
            "src-in and dst-in blend modes may return (0,0,0,1) when dst/src is all zeros",
            fInBlendModesFailRandomlyForAllZeroVec);
    writer->appendBool("Can use gl_FragCoord", fCanUseFragCoord);
    writer->appendBool("Incomplete short int precision", fIncompleteShortIntPrecision);
    writer->appendBool("Add and true to loops workaround", fAddAndTrueToLoopCondition);
    writer->appendBool("Unfold short circuit as ternary", fUnfoldShortCircuitAsTernary);
    writer->appendBool("Emulate abs(int) function", fEmulateAbsIntFunction);
    writer->appendBool("Rewrite do while loops", fRewriteDoWhileLoops);
    writer->appendBool("Rewrite pow with constant exponent", fRemovePowWithConstantExponent);
    writer->appendBool("Must write to sk_FragColor [workaround]", fMustWriteToFragColor);
    writer->appendBool("Don't add default precision statement for samplerExternalOES",
                       fNoDefaultPrecisionForExternalSamplers);
    writer->appendBool("Rewrite matrix-vector multiply", fRewriteMatrixVectorMultiply);
    writer->appendBool("Rewrite matrix equality comparisons", fRewriteMatrixComparisons);
    writer->appendBool("Flat interpolation support", fFlatInterpolationSupport);
    writer->appendBool("Prefer flat interpolation", fPreferFlatInterpolation);
    writer->appendBool("No perspective interpolation support", fNoPerspectiveInterpolationSupport);
    writer->appendBool("Sample mask support", fSampleMaskSupport);
    writer->appendBool("External texture support", fExternalTextureSupport);
    writer->appendBool("sk_VertexID support", fVertexIDSupport);
    writer->appendBool("Infinity support", fInfinitySupport);
    writer->appendBool("Bit manipulation support", fBitManipulationSupport);
    writer->appendBool("float == fp32", fFloatIs32Bits);
    writer->appendBool("half == fp32", fHalfIs32Bits);
    writer->appendBool("Has poor fragment precision", fHasLowFragmentPrecision);
    writer->appendBool("Color space math needs float", fColorSpaceMathNeedsFloat);
    writer->appendBool("Builtin fma() support", fBuiltinFMASupport);
    writer->appendBool("Builtin determinant() support", fBuiltinDeterminantSupport);
    writer->appendBool("Can use do-while loops", fCanUseDoLoops);
    writer->appendBool("Use node pools", fUseNodePools);

    writer->appendS32("Max FS Samplers", fMaxFragmentSamplers);
    writer->appendS32("Max Tessellation Segments", fMaxTessellationSegments);
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
        SkASSERT(!fMustForceNegatedLdexpParamToMultiply);
        SkASSERT(!fAtan2ImplementedAsAtanYOverX);
        SkASSERT(!fMustDoOpBetweenFloorAndAbs);
        SkASSERT(!fRequiresLocalOutputColorForFBFetch);
        SkASSERT(!fMustObfuscateUniformColor);
        SkASSERT(!fMustGuardDivisionEvenAfterExplicitZeroCheck);
        SkASSERT(!fInBlendModesFailRandomlyForAllZeroVec);
        SkASSERT(fCanUseFragCoord);
        SkASSERT(!fIncompleteShortIntPrecision);
        SkASSERT(!fAddAndTrueToLoopCondition);
        SkASSERT(!fUnfoldShortCircuitAsTernary);
        SkASSERT(!fEmulateAbsIntFunction);
        SkASSERT(!fRewriteDoWhileLoops);
        SkASSERT(!fRemovePowWithConstantExponent);
        SkASSERT(!fMustWriteToFragColor);
        SkASSERT(!fNoDefaultPrecisionForExternalSamplers);
        SkASSERT(!fRewriteMatrixVectorMultiply);
        SkASSERT(!fRewriteMatrixComparisons);
    }
    if (!options.fEnableExperimentalHardwareTessellation) {
        fMaxTessellationSegments = 0;
    }
    if (options.fReducedShaderVariations) {
        fReducedShaderMode = true;
    }
#if GR_TEST_UTILS
    if (options.fSuppressDualSourceBlending) {
        fDualSourceBlendingSupport = false;
    }
    if (options.fSuppressFramebufferFetch) {
        fFBFetchSupport = false;
    }
    if (options.fSuppressGeometryShaders) {
        fGeometryShaderSupport = false;
    }
    if (options.fMaxTessellationSegmentsOverride > 0) {
        fMaxTessellationSegments = std::min(options.fMaxTessellationSegmentsOverride,
                                            fMaxTessellationSegments);
    }
#endif
}
