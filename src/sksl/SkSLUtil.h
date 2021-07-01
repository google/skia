/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_UTIL
#define SKSL_UTIL

#include <cstdarg>
#include <memory>
#include "stdlib.h"
#include "string.h"
#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLLexer.h"

#ifndef SKSL_STANDALONE
#include "include/core/SkTypes.h"
#include "include/private/GrTypesPriv.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrShaderCaps.h"
#endif // SK_SUPPORT_GPU
#endif // SKSL_STANDALONE

class GrShaderCaps;

namespace SkSL {

class Context;
class OutputStream;
class StringStream;
class Type;

#if defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

// we're being compiled standalone, so we don't have access to caps...
enum GrGLSLGeneration {
    k110_GrGLSLGeneration,
    k130_GrGLSLGeneration,
    k140_GrGLSLGeneration,
    k150_GrGLSLGeneration,
    k330_GrGLSLGeneration,
    k400_GrGLSLGeneration,
    k420_GrGLSLGeneration,
    k310es_GrGLSLGeneration,
    k320es_GrGLSLGeneration,
};

class StandaloneShaderCaps {
public:
    GrGLSLGeneration fGLSLGeneration = k400_GrGLSLGeneration;
    GrGLSLGeneration generation() const {
        return fGLSLGeneration;
    }

    bool fAtan2ImplementedAsAtanYOverX = false;
    bool atan2ImplementedAsAtanYOverX() const {
        return fAtan2ImplementedAsAtanYOverX;
    }

    bool fCanUseMinAndAbsTogether = true;
    bool canUseMinAndAbsTogether() const {
        return fCanUseMinAndAbsTogether;
    }

    bool fMustForceNegatedAtanParamToFloat = false;
    bool mustForceNegatedAtanParamToFloat() const {
        return fMustForceNegatedAtanParamToFloat;
    }

    bool fMustForceNegatedLdexpParamToMultiply = false;
    bool mustForceNegatedLdexpParamToMultiply() const {
        return fMustForceNegatedLdexpParamToMultiply;
    }

    bool fGeometryShaderSupport = true;
    bool geometryShaderSupport() const {
        return fGeometryShaderSupport;
    }

    bool fShaderDerivativeSupport = true;
    bool shaderDerivativeSupport() const {
        return fShaderDerivativeSupport;
    }

    bool fUsesPrecisionModifiers = false;
    bool usesPrecisionModifiers() const {
        return fUsesPrecisionModifiers;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > k110_GrGLSLGeneration;
    }

    bool fFBFetchSupport = true;
    bool fbFetchSupport() const {
        return fFBFetchSupport;
    }

    bool fFBFetchNeedsCustomOutput = false;
    bool fbFetchNeedsCustomOutput() const {
        return fFBFetchNeedsCustomOutput;
    }

    bool fFlatInterpolationSupport = true;
    bool flatInterpolationSupport() const {
        return fFlatInterpolationSupport;
    }

    bool fNoperspectiveInterpolationSupport = true;
    bool noperspectiveInterpolationSupport() const {
        return fNoperspectiveInterpolationSupport;
    }

    bool fMultisampleInterpolationSupport = true;
    bool multisampleInterpolationSupport() const {
        return fMultisampleInterpolationSupport;
    }

    bool fSampleMaskSupport = true;
    bool sampleMaskSupport() const {
        return fSampleMaskSupport;
    }

    bool fExternalTextureSupport = true;
    bool externalTextureSupport() const {
        return fExternalTextureSupport;
    }

    bool fMustDoOpBetweenFloorAndAbs = false;
    bool mustDoOpBetweenFloorAndAbs() const {
        return fMustDoOpBetweenFloorAndAbs;
    }

    bool fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

    bool fInBlendModesFailRandomlyForAllZeroVec = false;
    bool inBlendModesFailRandomlyForAllZeroVec() const {
        return fInBlendModesFailRandomlyForAllZeroVec;
    }

    bool fMustEnableAdvBlendEqs = false;
    bool mustEnableAdvBlendEqs() const {
        return fMustEnableAdvBlendEqs;
    }

    bool fCanUseAnyFunctionInShader = true;
    bool canUseAnyFunctionInShader() const {
        return fCanUseAnyFunctionInShader;
    }

    bool fNoDefaultPrecisionForExternalSamplers = false;
    bool noDefaultPrecisionForExternalSamplers() const {
        return fNoDefaultPrecisionForExternalSamplers;
    }

    bool fFloatIs32Bits = true;
    bool floatIs32Bits() const {
        return fFloatIs32Bits;
    }

    bool fIntegerSupport = false;
    bool integerSupport() const {
        return fIntegerSupport;
    }

    bool fNonsquareMatrixSupport = false;
    bool nonsquareMatrixSupport() const {
        return fNonsquareMatrixSupport;
    }

    bool fBuiltinFMASupport = false;
    bool builtinFMASupport() const {
        return fBuiltinFMASupport;
    }

    bool fBuiltinDeterminantSupport = false;
    bool builtinDeterminantSupport() const {
        return fBuiltinDeterminantSupport;
    }

    bool fCanUseDoLoops = false;
    bool canUseDoLoops() const {
        // we define this to false in standalone so we don't use do loops while inlining in FP files
        // (which would then, being baked in, end up being used even in contexts where do loops are
        // not allowed)
        return fCanUseDoLoops;
    }

    bool fUseNodePools = true;
    bool useNodePools() const {
        return fUseNodePools;
    }

    const char* fShaderDerivativeExtensionString = nullptr;
    const char* shaderDerivativeExtensionString() const {
        return fShaderDerivativeExtensionString;
    }

    const char* fFragCoordConventionsExtensionString = nullptr;
    const char* fragCoordConventionsExtensionString() const {
        return fFragCoordConventionsExtensionString;
    }

    const char* fGeometryShaderExtensionString = nullptr;
    const char* geometryShaderExtensionString() const {
        return fGeometryShaderExtensionString;
    }

    const char* fGSInvocationsExtensionString = nullptr;
    const char* gsInvocationsExtensionString() const {
        return fGSInvocationsExtensionString;
    }

    const char* fExternalTextureExtensionString = nullptr;
    const char* externalTextureExtensionString() const {
        return fExternalTextureExtensionString;
    }

    const char* fSecondExternalTextureExtensionString = nullptr;
    const char* secondExternalTextureExtensionString() const {
        return fSecondExternalTextureExtensionString;
    }

    const char* fVersionDeclString = "";
    const char* versionDeclString() const {
        return fVersionDeclString;
    }

    bool fGSInvocationsSupport = true;
    bool gsInvocationsSupport() const {
        return fGSInvocationsSupport;
    }

    bool fCanUseFractForNegativeValues = true;
    bool canUseFractForNegativeValues() const {
        return fCanUseFractForNegativeValues;
    }

    bool fCanUseFragCoord = true;
    bool canUseFragCoord() const {
        return fCanUseFragCoord;
    }

    bool fIncompleteShortIntPrecision = false;
    bool incompleteShortIntPrecision() const {
        return fIncompleteShortIntPrecision;
    }

    bool fAddAndTrueToLoopCondition = false;
    bool addAndTrueToLoopCondition() const {
        return fAddAndTrueToLoopCondition;
    }

    bool fUnfoldShortCircuitAsTernary = false;
    bool unfoldShortCircuitAsTernary() const {
        return fUnfoldShortCircuitAsTernary;
    }

    bool fEmulateAbsIntFunction = false;
    bool emulateAbsIntFunction() const {
        return fEmulateAbsIntFunction;
    }

    bool fRewriteDoWhileLoops = false;
    bool rewriteDoWhileLoops() const {
        return fRewriteDoWhileLoops;
    }

    bool fRemovePowWithConstantExponent = false;
    bool removePowWithConstantExponent() const {
        return fRemovePowWithConstantExponent;
    }

    const char* fFBFetchColorName = nullptr;
    const char* fbFetchColorName() const {
        return fFBFetchColorName;
    }

    bool fRewriteMatrixVectorMultiply = false;
    bool rewriteMatrixVectorMultiply() const {
        return fRewriteMatrixVectorMultiply;
    }

    bool fRewriteMatrixComparisons = false;
    bool rewriteMatrixComparisons() const {
        return fRewriteMatrixComparisons;
    }
};

using ShaderCapsClass = StandaloneShaderCaps;
using ShaderCapsPointer = std::shared_ptr<StandaloneShaderCaps>;
extern StandaloneShaderCaps standaloneCaps;

#else

using ShaderCapsClass = GrShaderCaps;
using ShaderCapsPointer = sk_sp<GrShaderCaps>;

#endif  // defined(SKSL_STANDALONE) || !SK_SUPPORT_GPU

// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static ShaderCapsPointer Default() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fBuiltinDeterminantSupport = true;
        result->fCanUseDoLoops = true;
        return result;
    }

    static ShaderCapsPointer Standalone() {
        return MakeShaderCaps();
    }

    static ShaderCapsPointer AddAndTrueToLoopCondition() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fAddAndTrueToLoopCondition = true;
        return result;
    }

    static ShaderCapsPointer BlendModesFailRandomlyForAllZeroVec() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fInBlendModesFailRandomlyForAllZeroVec = true;
        return result;
    }

    static ShaderCapsPointer CannotUseFractForNegativeValues() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFractForNegativeValues = false;
        return result;
    }

    static ShaderCapsPointer CannotUseFragCoord() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFragCoord = false;
        return result;
    }

    static ShaderCapsPointer CannotUseMinAndAbsTogether() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseMinAndAbsTogether = false;
        return result;
    }

    static ShaderCapsPointer EmulateAbsIntFunction() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fEmulateAbsIntFunction = true;
        return result;
    }

    static ShaderCapsPointer GeometryShaderExtensionString() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 310es";
        result->fGeometryShaderSupport = true;
        result->fGeometryShaderExtensionString = "GL_EXT_geometry_shader";
        result->fGSInvocationsSupport = true;
        return result;
    }

    static ShaderCapsPointer GeometryShaderSupport() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fGeometryShaderSupport = true;
        result->fGSInvocationsSupport = true;
        return result;
    }

    static ShaderCapsPointer GSInvocationsExtensionString() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fGeometryShaderSupport = true;
        result->fGSInvocationsSupport = true;
        result->fGSInvocationsExtensionString = "GL_ARB_gpu_shader5";
        return result;
    }

    static ShaderCapsPointer IncompleteShortIntPrecision() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 310es";
        result->fUsesPrecisionModifiers = true;
        result->fIncompleteShortIntPrecision = true;
        return result;
    }

    static ShaderCapsPointer MustForceNegatedAtanParamToFloat() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedAtanParamToFloat = true;
        return result;
    }

    static ShaderCapsPointer MustForceNegatedLdexpParamToMultiply() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedLdexpParamToMultiply = true;
        return result;
    }

    static ShaderCapsPointer MustGuardDivisionEvenAfterExplicitZeroCheck() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
        return result;
    }

    static ShaderCapsPointer NoGSInvocationsSupport() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fGeometryShaderSupport = true;
        result->fGSInvocationsSupport = false;
        return result;
    }

    static ShaderCapsPointer RemovePowWithConstantExponent() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRemovePowWithConstantExponent = true;
        return result;
    }

    static ShaderCapsPointer RewriteDoWhileLoops() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteDoWhileLoops = true;
        return result;
    }

    static ShaderCapsPointer RewriteMatrixComparisons() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fRewriteMatrixComparisons = true;
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer RewriteMatrixVectorMultiply() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteMatrixVectorMultiply = true;
        return result;
    }

    static ShaderCapsPointer SampleMaskSupport() {
        ShaderCapsPointer result = Default();
        result->fSampleMaskSupport = true;
        return result;
    }

    static ShaderCapsPointer ShaderDerivativeExtensionString() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer UnfoldShortCircuitAsTernary() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUnfoldShortCircuitAsTernary = true;
        return result;
    }

    static ShaderCapsPointer UsesPrecisionModifiers() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static ShaderCapsPointer Version110() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = GrGLSLGeneration::k110_GrGLSLGeneration;
        return result;
    }

    static ShaderCapsPointer Version450Core() {
        ShaderCapsPointer result = MakeShaderCaps();
        result->fVersionDeclString = "#version 450 core";
        return result;
    }

private:
    static ShaderCapsPointer MakeShaderCaps();
};

#if !defined(SKSL_STANDALONE)
bool type_to_grsltype(const Context& context, const Type& type, GrSLType* outType);
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

}  // namespace SkSL

#endif  // SKSL_UTIL
