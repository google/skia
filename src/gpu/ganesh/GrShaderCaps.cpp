/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/ganesh/GrShaderCaps.h"

#include "include/gpu/GrContextOptions.h"

////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"

void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

    writer->appendBool("Shader Derivative Support", fShaderDerivativeSupport);
    writer->appendBool("Dst Read In Shader Support", fDstReadInShaderSupport);
    writer->appendBool("Dual Source Blending Support", fDualSourceBlendingSupport);
    writer->appendBool("Integer Support", fIntegerSupport);
    writer->appendBool("Nonsquare Matrix Support", fNonsquareMatrixSupport);
    writer->appendBool("Inverse Hyperbolic Support", fInverseHyperbolicSupport);

    static const char* kAdvBlendEqInteractionStr[] = {
        "Not Supported",
        "Automatic",
        "General Enable",
    };
    static_assert(0 == kNotSupported_AdvBlendEqInteraction);
    static_assert(1 == kAutomatic_AdvBlendEqInteraction);
    static_assert(2 == kGeneralEnable_AdvBlendEqInteraction);
    static_assert(std::size(kAdvBlendEqInteractionStr) == kLast_AdvBlendEqInteraction + 1);

    writer->appendBool("FB Fetch Support", fFBFetchSupport);
    writer->appendBool("Uses precision modifiers", fUsesPrecisionModifiers);
    writer->appendBool("Can use void-typed expressions in a sequence expression",
                       fCanUseVoidInSequenceExpressions);
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
    writer->appendBool("Can use gl_FragCoord", fCanUseFragCoord);
    writer->appendBool("Add and true to loops workaround", fAddAndTrueToLoopCondition);
    writer->appendBool("Unfold short circuit as ternary", fUnfoldShortCircuitAsTernary);
    writer->appendBool("Emulate abs(int) function", fEmulateAbsIntFunction);
    writer->appendBool("Rewrite do while loops", fRewriteDoWhileLoops);
    writer->appendBool("Rewrite switch statements", fRewriteSwitchStatements);
    writer->appendBool("Rewrite pow with constant exponent", fRemovePowWithConstantExponent);
    writer->appendBool("Must write to sk_FragColor [workaround]", fMustWriteToFragColor);
    writer->appendBool("Don't add default precision statement for samplerExternalOES",
                       fNoDefaultPrecisionForExternalSamplers);
    writer->appendBool("Rewrite matrix-vector multiply", fRewriteMatrixVectorMultiply);
    writer->appendBool("Rewrite matrix equality comparisons", fRewriteMatrixComparisons);
    writer->appendBool("Rounding fix required for Perlin noise", fPerlinNoiseRoundingFix);
    writer->appendBool("Must declare fragment front-facing", fMustDeclareFragmentFrontFacing);
    writer->appendBool("Flat interpolation support", fFlatInterpolationSupport);
    writer->appendBool("Prefer flat interpolation", fPreferFlatInterpolation);
    writer->appendBool("No perspective interpolation support", fNoPerspectiveInterpolationSupport);
    writer->appendBool("Sample mask support", fSampleMaskSupport);
    writer->appendBool("External texture support", fExternalTextureSupport);
    writer->appendBool("sk_VertexID support", fVertexIDSupport);
    writer->appendBool("Infinity support", fInfinitySupport);
    writer->appendBool("Non-constant array index support", fNonconstantArrayIndexSupport);
    writer->appendBool("Bit manipulation support", fBitManipulationSupport);
    writer->appendBool("float == fp32", fFloatIs32Bits);
    writer->appendBool("half == fp32", fHalfIs32Bits);
    writer->appendBool("Has poor fragment precision", fHasLowFragmentPrecision);
    writer->appendBool("Builtin fma() support", fBuiltinFMASupport);
    writer->appendBool("Builtin determinant() support", fBuiltinDeterminantSupport);

    writer->appendS32("Max FS Samplers", fMaxFragmentSamplers);
    writer->appendCString("Advanced blend equation interaction",
                          kAdvBlendEqInteractionStr[fAdvBlendEqInteraction]);

    writer->endObject();
}
#else
void GrShaderCaps::dumpJSON(SkJSONWriter* writer) const { }
#endif

void GrShaderCaps::applyOptionsOverrides(const GrContextOptions& options) {
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(fCanUseVoidInSequenceExpressions);
        SkASSERT(fCanUseMinAndAbsTogether);
        SkASSERT(fCanUseFractForNegativeValues);
        SkASSERT(!fMustForceNegatedAtanParamToFloat);
        SkASSERT(!fMustForceNegatedLdexpParamToMultiply);
        SkASSERT(!fAtan2ImplementedAsAtanYOverX);
        SkASSERT(!fMustDoOpBetweenFloorAndAbs);
        SkASSERT(!fRequiresLocalOutputColorForFBFetch);
        SkASSERT(!fMustObfuscateUniformColor);
        SkASSERT(!fMustGuardDivisionEvenAfterExplicitZeroCheck);
        SkASSERT(fCanUseFragCoord);
        SkASSERT(!fAddAndTrueToLoopCondition);
        SkASSERT(!fUnfoldShortCircuitAsTernary);
        SkASSERT(!fEmulateAbsIntFunction);
        SkASSERT(!fRewriteDoWhileLoops);
        SkASSERT(!fRewriteSwitchStatements);
        SkASSERT(!fRemovePowWithConstantExponent);
        SkASSERT(!fMustWriteToFragColor);
        SkASSERT(!fNoDefaultPrecisionForExternalSamplers);
        SkASSERT(!fRewriteMatrixVectorMultiply);
        SkASSERT(!fRewriteMatrixComparisons);
        SkASSERT(!fPerlinNoiseRoundingFix);
        SkASSERT(!fMustDeclareFragmentFrontFacing);
    }
    if (options.fReducedShaderVariations) {
        fReducedShaderMode = true;
    }
#if defined(GR_TEST_UTILS)
    if (options.fSuppressDualSourceBlending) {
        fDualSourceBlendingSupport = false;
    }
    if (options.fSuppressFramebufferFetch) {
        fFBFetchSupport = false;
    }
#endif
}
