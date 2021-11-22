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
#include <stdlib.h>
#include <string.h>
#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLGLSL.h"
#include "src/sksl/SkSLLexer.h"

#ifndef SKSL_STANDALONE
#include "include/core/SkTypes.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrContextOptions.h"
#include "include/private/GrTypesPriv.h"
#endif // SK_SUPPORT_GPU
#endif // SKSL_STANDALONE

namespace SkSL {

class Context;
class OutputStream;
class ShaderCapsFactory;
class StringStream;
class Type;

struct ShaderCaps {
    /**
     * Indicates how GLSL must interact with advanced blend equations. The KHR extension requires
     * special layout qualifiers in the fragment shader.
     */
    enum AdvBlendEqInteraction {
        kNotSupported_AdvBlendEqInteraction,     //<! No _blend_equation_advanced extension
        kAutomatic_AdvBlendEqInteraction,        //<! No interaction required
        kGeneralEnable_AdvBlendEqInteraction,    //<! layout(blend_support_all_equations) out

        kLast_AdvBlendEqInteraction = kGeneralEnable_AdvBlendEqInteraction
    };

    //
    // TODO: Remove these accessors
    //
    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool nonsquareMatrixSupport() const { return fNonsquareMatrixSupport; }

    /** Indicates true 32-bit integer support, with unsigned types and bitwise operations */
    bool integerSupport() const { return fIntegerSupport; }

    /** asinh(), acosh(), atanh() */
    bool inverseHyperbolicSupport() const { return fInverseHyperbolicSupport; }

    /**
     * Some helper functions for encapsulating various extensions to read FB Buffer on openglES
     *
     * TODO: On desktop opengl 4.2+ we can achieve something similar to this effect
     */
    bool fbFetchSupport() const { return fFBFetchSupport; }

    bool fbFetchNeedsCustomOutput() const { return fFBFetchNeedsCustomOutput; }

    const char* versionDeclString() const { return fVersionDeclString; }

    const char* fbFetchColorName() const { return fFBFetchColorName; }

    bool flatInterpolationSupport() const { return fFlatInterpolationSupport; }

    bool noperspectiveInterpolationSupport() const { return fNoPerspectiveInterpolationSupport; }

    bool sampleMaskSupport() const { return fSampleMaskSupport; }

    bool externalTextureSupport() const { return fExternalTextureSupport; }

    bool floatIs32Bits() const { return fFloatIs32Bits; }

    // SkSL only.
    bool builtinFMASupport() const { return fBuiltinFMASupport; }

    bool builtinDeterminantSupport() const { return fBuiltinDeterminantSupport; }

    AdvBlendEqInteraction advBlendEqInteraction() const { return fAdvBlendEqInteraction; }

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > SkSL::GLSLGeneration::k110;
    }

    bool usesPrecisionModifiers() const { return fUsesPrecisionModifiers; }

    // Returns whether we can use the glsl function any() in our shader code.
    bool canUseAnyFunctionInShader() const { return fCanUseAnyFunctionInShader; }

    bool canUseMinAndAbsTogether() const { return fCanUseMinAndAbsTogether; }

    bool canUseFractForNegativeValues() const { return fCanUseFractForNegativeValues; }

    bool mustForceNegatedAtanParamToFloat() const { return fMustForceNegatedAtanParamToFloat; }

    // http://skbug.com/12076
    bool mustForceNegatedLdexpParamToMultiply() const {
        return fMustForceNegatedLdexpParamToMultiply;
    }

    // Returns whether a device incorrectly implements atan(y,x) as atan(y/x)
    bool atan2ImplementedAsAtanYOverX() const { return fAtan2ImplementedAsAtanYOverX; }

    // If this returns true some operation (could be a no op) must be called between floor and abs
    // to make sure the driver compiler doesn't inline them together which can cause a driver bug in
    // the shader.
    bool mustDoOpBetweenFloorAndAbs() const { return fMustDoOpBetweenFloorAndAbs; }

    // If false, SkSL uses a workaround so that sk_FragCoord doesn't actually query gl_FragCoord
    bool canUseFragCoord() const { return fCanUseFragCoord; }

    // If true, short ints can't represent every integer in the 16-bit two's complement range as
    // required by the spec. SKSL will always emit full ints.
    bool incompleteShortIntPrecision() const { return fIncompleteShortIntPrecision; }

    // If true, then conditions in for loops need "&& true" to work around driver bugs.
    bool addAndTrueToLoopCondition() const { return fAddAndTrueToLoopCondition; }

    // If true, then expressions such as "x && y" or "x || y" are rewritten as
    // ternary to work around driver bugs.
    bool unfoldShortCircuitAsTernary() const { return fUnfoldShortCircuitAsTernary; }

    bool emulateAbsIntFunction() const { return fEmulateAbsIntFunction; }

    bool rewriteDoWhileLoops() const { return fRewriteDoWhileLoops; }

    bool rewriteSwitchStatements() const { return fRewriteSwitchStatements; }

    bool removePowWithConstantExponent() const { return fRemovePowWithConstantExponent; }

    // The D3D shader compiler, when targeting PS 3.0 (ie within ANGLE) fails to compile certain
    // constructs. See detailed comments in GrGLCaps.cpp.
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

    // The Android emulator claims samplerExternalOES is an unknown type if a default precision
    // statement is made for the type.
    bool noDefaultPrecisionForExternalSamplers() const {
        return fNoDefaultPrecisionForExternalSamplers;
    }

    // ARM GPUs calculate `matrix * vector` in SPIR-V at full precision, even when the inputs are
    // RelaxedPrecision. Rewriting the multiply as a sum of vector*scalar fixes this. (skia:11769)
    bool rewriteMatrixVectorMultiply() const {
        return fRewriteMatrixVectorMultiply;
    }

    // Rewrites matrix equality comparisons to avoid an Adreno driver bug. (skia:11308)
    bool rewriteMatrixComparisons() const { return fRewriteMatrixComparisons; }

    // ANGLE disallows do loops altogether, and we're seeing crashes on Tegra3 with do loops in at
    // least some cases.
    bool canUseDoLoops() const { return fCanUseDoLoops; }

    // By default, SkSL pools IR nodes per-program. To debug memory corruption, it is sometimes
    // helpful to disable that feature.
    bool useNodePools() const { return fUseNodePools; }

    // Returns the string of an extension that must be enabled in the shader to support
    // derivatives. If nullptr is returned then no extension needs to be enabled. Before calling
    // this function, the caller should check that shaderDerivativeSupport exists.
    const char* shaderDerivativeExtensionString() const {
        SkASSERT(this->shaderDerivativeSupport());
        return fShaderDerivativeExtensionString;
    }

    // This returns the name of an extension that must be enabled in the shader to support external
    // textures. In some cases, two extensions must be enabled - the second extension is returned
    // by secondExternalTextureExtensionString(). If that function returns nullptr, then only one
    // extension is required.
    const char* externalTextureExtensionString() const {
        SkASSERT(this->externalTextureSupport());
        return fExternalTextureExtensionString;
    }

    const char* secondExternalTextureExtensionString() const {
        SkASSERT(this->externalTextureSupport());
        return fSecondExternalTextureExtensionString;
    }

    SkSL::GLSLGeneration generation() const { return fGLSLGeneration; }

    SkSL::GLSLGeneration fGLSLGeneration = SkSL::GLSLGeneration::k330;

    bool fShaderDerivativeSupport = false;
    bool fIntegerSupport = false;
    bool fNonsquareMatrixSupport = false;
    bool fInverseHyperbolicSupport = false;
    bool fFBFetchSupport = false;
    bool fFBFetchNeedsCustomOutput = false;
    bool fUsesPrecisionModifiers = false;
    bool fFlatInterpolationSupport = false;
    bool fNoPerspectiveInterpolationSupport = false;
    bool fSampleMaskSupport = false;
    bool fExternalTextureSupport = false;
    bool fFloatIs32Bits = true;

    // Used by SkSL to know when to generate polyfills.
    bool fBuiltinFMASupport = false;
    bool fBuiltinDeterminantSupport = false;

    // Used for specific driver bug work arounds
    bool fCanUseAnyFunctionInShader = true;
    bool fCanUseMinAndAbsTogether = true;
    bool fCanUseFractForNegativeValues = true;
    bool fMustForceNegatedAtanParamToFloat = false;
    bool fMustForceNegatedLdexpParamToMultiply = false;
    bool fAtan2ImplementedAsAtanYOverX = false;
    bool fMustDoOpBetweenFloorAndAbs = false;
    bool fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    bool fCanUseFragCoord = true;
    bool fIncompleteShortIntPrecision = false;
    bool fAddAndTrueToLoopCondition = false;
    bool fUnfoldShortCircuitAsTernary = false;
    bool fEmulateAbsIntFunction = false;
    bool fRewriteDoWhileLoops = false;
    bool fRewriteSwitchStatements = false;
    bool fRemovePowWithConstantExponent = false;
    bool fNoDefaultPrecisionForExternalSamplers = false;
    bool fRewriteMatrixVectorMultiply = false;
    bool fRewriteMatrixComparisons = false;
    bool fCanUseDoLoops = true;

    // This controls behavior of the SkSL compiler, not the code we generate
    bool fUseNodePools = true;

    const char* fVersionDeclString = "";

    const char* fShaderDerivativeExtensionString = nullptr;
    const char* fExternalTextureExtensionString = nullptr;
    const char* fSecondExternalTextureExtensionString = nullptr;
    const char* fFBFetchColorName = nullptr;

    AdvBlendEqInteraction fAdvBlendEqInteraction = kNotSupported_AdvBlendEqInteraction;
};

// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static std::unique_ptr<ShaderCaps> Default() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fBuiltinDeterminantSupport = true;
        result->fCanUseDoLoops = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> Standalone() {
        return MakeShaderCaps();
    }

    static std::unique_ptr<ShaderCaps> AddAndTrueToLoopCondition() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fAddAndTrueToLoopCondition = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> CannotUseFractForNegativeValues() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFractForNegativeValues = false;
        return result;
    }

    static std::unique_ptr<ShaderCaps> CannotUseFragCoord() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseFragCoord = false;
        return result;
    }

    static std::unique_ptr<ShaderCaps> CannotUseMinAndAbsTogether() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fCanUseMinAndAbsTogether = false;
        return result;
    }

    static std::unique_ptr<ShaderCaps> EmulateAbsIntFunction() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fEmulateAbsIntFunction = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> FramebufferFetchSupport() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fFBFetchSupport = true;
        result->fFBFetchColorName = "gl_LastFragData[0]";
        return result;
    }

    static std::unique_ptr<ShaderCaps> IncompleteShortIntPrecision() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 310es";
        result->fUsesPrecisionModifiers = true;
        result->fIncompleteShortIntPrecision = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> MustForceNegatedAtanParamToFloat() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedAtanParamToFloat = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> MustForceNegatedLdexpParamToMultiply() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fMustForceNegatedLdexpParamToMultiply = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> MustGuardDivisionEvenAfterExplicitZeroCheck() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> RemovePowWithConstantExponent() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRemovePowWithConstantExponent = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> RewriteDoWhileLoops() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteDoWhileLoops = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> RewriteMatrixComparisons() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fRewriteMatrixComparisons = true;
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> RewriteMatrixVectorMultiply() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteMatrixVectorMultiply = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> RewriteSwitchStatements() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fRewriteSwitchStatements = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> SampleMaskSupport() {
        std::unique_ptr<ShaderCaps> result = Default();
        result->fSampleMaskSupport = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> ShaderDerivativeExtensionString() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fShaderDerivativeSupport = true;
        result->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> UnfoldShortCircuitAsTernary() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUnfoldShortCircuitAsTernary = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> UsesPrecisionModifiers() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 400";
        result->fUsesPrecisionModifiers = true;
        return result;
    }

    static std::unique_ptr<ShaderCaps> Version110() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 110";
        result->fGLSLGeneration = SkSL::GLSLGeneration::k110;
        return result;
    }

    static std::unique_ptr<ShaderCaps> Version450Core() {
        std::unique_ptr<ShaderCaps> result = MakeShaderCaps();
        result->fVersionDeclString = "#version 450 core";
        return result;
    }

private:
    static std::unique_ptr<ShaderCaps> MakeShaderCaps();
};

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
bool type_to_grsltype(const Context& context, const Type& type, GrSLType* outType);
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

}  // namespace SkSL

#endif  // SKSL_UTIL
