/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_UTIL
#define SKSL_UTIL

#include "include/core/SkTypes.h"
#include "include/sksl/SkSLVersion.h"
#include "src/sksl/SkSLGLSL.h"

#include <memory>

enum class SkSLType : char;

namespace SkSL {

class Context;
class OutputStream;
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

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > SkSL::GLSLGeneration::k110;
    }

    // Returns the string of an extension that must be enabled in the shader to support
    // derivatives. If nullptr is returned then no extension needs to be enabled. Before calling
    // this function, the caller should check that shaderDerivativeSupport exists.
    const char* shaderDerivativeExtensionString() const {
        SkASSERT(this->fShaderDerivativeSupport);
        return fShaderDerivativeExtensionString;
    }

    // This returns the name of an extension that must be enabled in the shader to support external
    // textures. In some cases, two extensions must be enabled - the second extension is returned
    // by secondExternalTextureExtensionString(). If that function returns nullptr, then only one
    // extension is required.
    const char* externalTextureExtensionString() const {
        SkASSERT(this->fExternalTextureSupport);
        return fExternalTextureExtensionString;
    }

    const char* secondExternalTextureExtensionString() const {
        SkASSERT(this->fExternalTextureSupport);
        return fSecondExternalTextureExtensionString;
    }

    /**
     * SkSL 300 requires support for derivatives, nonsquare matrices and bitwise integer operations.
     */
    SkSL::Version supportedSkSLVerion() const {
        if (fShaderDerivativeSupport && fNonsquareMatrixSupport && fIntegerSupport &&
            fGLSLGeneration >= SkSL::GLSLGeneration::k330) {
            return SkSL::Version::k300;
        }
        return SkSL::Version::k100;
    }

    bool supportsDistanceFieldText() const { return fShaderDerivativeSupport; }

    SkSL::GLSLGeneration fGLSLGeneration = SkSL::GLSLGeneration::k330;

    bool fDualSourceBlendingSupport = false;
    bool fShaderDerivativeSupport = false;
    /** Enables sampleGrad and sampleLod functions that don't rely on implicit derivatives */
    bool fExplicitTextureLodSupport = false;
    /** Indicates true 32-bit integer support, with unsigned types and bitwise operations */
    bool fIntegerSupport = false;
    bool fNonsquareMatrixSupport = false;
    /** asinh(), acosh(), atanh() */
    bool fInverseHyperbolicSupport = false;
    bool fFBFetchSupport = false;
    bool fFBFetchNeedsCustomOutput = false;
    bool fUsesPrecisionModifiers = false;
    bool fFlatInterpolationSupport = false;
    bool fNoPerspectiveInterpolationSupport = false;
    bool fSampleMaskSupport = false;
    bool fExternalTextureSupport = false;
    bool fFloatIs32Bits = true;

    // isinf() is defined, and floating point infinities are handled according to IEEE standards.
    bool fInfinitySupport = false;

    // Used by SkSL to know when to generate polyfills.
    bool fBuiltinFMASupport = true;
    bool fBuiltinDeterminantSupport = true;

    // Used for specific driver bug work arounds
    bool fCanUseVoidInSequenceExpressions = true;
    bool fCanUseMinAndAbsTogether = true;
    bool fCanUseFractForNegativeValues = true;
    bool fMustForceNegatedAtanParamToFloat = false;
    bool fMustForceNegatedLdexpParamToMultiply = false;  // http://skbug.com/12076
    // Returns whether a device incorrectly implements atan(y,x) as atan(y/x)
    bool fAtan2ImplementedAsAtanYOverX = false;
    // If this returns true some operation (could be a no op) must be called between floor and abs
    // to make sure the driver compiler doesn't inline them together which can cause a driver bug in
    // the shader.
    bool fMustDoOpBetweenFloorAndAbs = false;
    // The D3D shader compiler, when targeting PS 3.0 (ie within ANGLE) fails to compile certain
    // constructs. See detailed comments in GrGLCaps.cpp.
    bool fMustGuardDivisionEvenAfterExplicitZeroCheck = false;
    // If false, SkSL uses a workaround so that sk_FragCoord doesn't actually query gl_FragCoord
    bool fCanUseFragCoord = true;
    // If true, then conditions in for loops need "&& true" to work around driver bugs.
    bool fAddAndTrueToLoopCondition = false;
    // If true, then expressions such as "x && y" or "x || y" are rewritten as ternary to work
    // around driver bugs.
    bool fUnfoldShortCircuitAsTernary = false;
    bool fEmulateAbsIntFunction = false;
    bool fRewriteDoWhileLoops = false;
    bool fRewriteSwitchStatements = false;
    bool fRemovePowWithConstantExponent = false;
    // The Android emulator claims samplerExternalOES is an unknown type if a default precision
    // statement is made for the type.
    bool fNoDefaultPrecisionForExternalSamplers = false;
    // ARM GPUs calculate `matrix * vector` in SPIR-V at full precision, even when the inputs are
    // RelaxedPrecision. Rewriting the multiply as a sum of vector*scalar fixes this. (skia:11769)
    bool fRewriteMatrixVectorMultiply = false;
    // Rewrites matrix equality comparisons to avoid an Adreno driver bug. (skia:11308)
    bool fRewriteMatrixComparisons = false;
    // Strips const from function parameters in the GLSL code generator. (skia:13858)
    bool fRemoveConstFromFunctionParameters = false;
    // On some Android devices colors aren't accurate enough for the double lookup in the
    // Perlin noise shader. This workaround aggressively snaps colors to multiples of 1/255.
    bool fPerlinNoiseRoundingFix = false;
    // Vulkan requires certain builtin variables be present, even if they're unused. At one time,
    // validation errors would result if sk_Clockwise was missing. Now, it's just (Adreno) driver
    // bugs that drop or corrupt draws if they're missing.
    bool fMustDeclareFragmentFrontFacing = false;

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
    static const ShaderCaps* Default() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fShaderDerivativeSupport = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const ShaderCaps* Standalone() {
        static const SkSL::ShaderCaps* sCaps = MakeShaderCaps().release();
        return sCaps;
    }

protected:
    static std::unique_ptr<ShaderCaps> MakeShaderCaps();
};

#if !defined(SKSL_STANDALONE) && (defined(SK_GANESH) || defined(SK_GRAPHITE))
bool type_to_sksltype(const Context& context, const Type& type, SkSLType* outType);
#endif

void write_stringstream(const StringStream& d, OutputStream& out);

}  // namespace SkSL

#endif  // SKSL_UTIL
