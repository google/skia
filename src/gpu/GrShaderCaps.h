/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderCaps_DEFINED
#define GrShaderCaps_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/glsl/GrGLSL.h"

namespace SkSL {
class ShaderCapsFactory;
class SharedCompiler;
}  // namespace SkSL

struct GrContextOptions;
class SkJSONWriter;

class GrShaderCaps : public SkRefCnt {
public:
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

    GrShaderCaps(const GrContextOptions&);

    void dumpJSON(SkJSONWriter*) const;

    bool supportsDistanceFieldText() const { return fShaderDerivativeSupport; }

    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool gsInvocationsSupport() const { return fGSInvocationsSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool integerSupport() const { return fIntegerSupport; }
    bool nonsquareMatrixSupport() const { return fNonsquareMatrixSupport; }

    /**
     * Some helper functions for encapsulating various extensions to read FB Buffer on openglES
     *
     * TODO(joshualitt) On desktop opengl 4.2+ we can achieve something similar to this effect
     */
    bool fbFetchSupport() const { return fFBFetchSupport; }

    bool fbFetchNeedsCustomOutput() const { return fFBFetchNeedsCustomOutput; }

    const char* versionDeclString() const { return fVersionDeclString; }

    const char* fbFetchColorName() const { return fFBFetchColorName; }

    const char* fbFetchExtensionString() const { return fFBFetchExtensionString; }

    bool flatInterpolationSupport() const { return fFlatInterpolationSupport; }

    bool preferFlatInterpolation() const { return fPreferFlatInterpolation; }

    bool noperspectiveInterpolationSupport() const { return fNoPerspectiveInterpolationSupport; }

    bool sampleMaskSupport() const { return fSampleMaskSupport; }

    bool externalTextureSupport() const { return fExternalTextureSupport; }

    bool vertexIDSupport() const { return fVertexIDSupport; }

    // isinf() is defined, and floating point infinities are handled according to IEEE standards.
    bool infinitySupport() const { return fInfinitySupport; }

    // frexp(), ldexp(), findMSB(), findLSB().
    bool bitManipulationSupport() const { return fBitManipulationSupport; }

    bool floatIs32Bits() const { return fFloatIs32Bits; }

    bool halfIs32Bits() const { return fHalfIs32Bits; }

    bool hasLowFragmentPrecision() const { return fHasLowFragmentPrecision; }

    // Use a reduced set of rendering algorithms or less optimal effects in order to
    // reduce the number of unique shaders generated.
    bool reducedShaderMode() const { return fReducedShaderMode; }

    // SkSL only.
    bool builtinFMASupport() const { return fBuiltinFMASupport; }

    bool builtinDeterminantSupport() const { return fBuiltinDeterminantSupport; }

    AdvBlendEqInteraction advBlendEqInteraction() const { return fAdvBlendEqInteraction; }

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustDeclareFragmentShaderOutput() const { return fGLSLGeneration > k110_GrGLSLGeneration; }

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

    bool colorSpaceMathNeedsFloat() const { return fColorSpaceMathNeedsFloat; }

    // If true, then conditions in for loops need "&& true" to work around driver bugs.
    bool addAndTrueToLoopCondition() const { return fAddAndTrueToLoopCondition; }

    // If true, then expressions such as "x && y" or "x || y" are rewritten as
    // ternary to work around driver bugs.
    bool unfoldShortCircuitAsTernary() const { return fUnfoldShortCircuitAsTernary; }

    bool emulateAbsIntFunction() const { return fEmulateAbsIntFunction; }

    bool rewriteDoWhileLoops() const { return fRewriteDoWhileLoops; }

    bool removePowWithConstantExponent() const { return fRemovePowWithConstantExponent; }

    bool requiresLocalOutputColorForFBFetch() const { return fRequiresLocalOutputColorForFBFetch; }

    bool mustObfuscateUniformColor() const { return fMustObfuscateUniformColor; }

    // The D3D shader compiler, when targeting PS 3.0 (ie within ANGLE) fails to compile certain
    // constructs. See detailed comments in GrGLCaps.cpp.
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

    // On Pixel 3, 3a, and 4 devices we've noticed that the simple function:
    // half4 blend(half4 a, half4 b) { return a.a * b; }
    // may return (0, 0, 0, 1) when b is (0, 0, 0, 0).
    bool inBlendModesFailRandomlyForAllZeroVec() const {
        return fInBlendModesFailRandomlyForAllZeroVec;
    }

    // On Nexus 6, the GL context can get lost if a shader does not write a value to gl_FragColor.
    // https://bugs.chromium.org/p/chromium/issues/detail?id=445377
    bool mustWriteToFragColor() const { return fMustWriteToFragColor; }

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

    // Some GPUs produce poor results when enabling Metal's fastmath option
    bool canUseFastMath() const { return fCanUseFastMath; }

    // By default, SkSL pools IR nodes per-program. To debug memory corruption, it is sometimes
    // helpful to disable that feature.
    bool useNodePools() const { return fUseNodePools; }

    // When we have the option of using either dFdx or dfDy in a shader, this returns whether we
    // should avoid using dFdx. We have found some drivers have bugs or lower precision when using
    // dFdx.
    bool avoidDfDxForGradientsWhenPossible() const { return fAvoidDfDxForGradientsWhenPossible; }

    // Returns the string of an extension that must be enabled in the shader to support
    // derivatives. If nullptr is returned then no extension needs to be enabled. Before calling
    // this function, the caller should check that shaderDerivativeSupport exists.
    const char* shaderDerivativeExtensionString() const {
        SkASSERT(this->shaderDerivativeSupport());
        return fShaderDerivativeExtensionString;
    }

    // Returns the string of an extension that must be enabled in the shader to support geometry
    // shaders. If nullptr is returned then no extension needs to be enabled. Before calling this
    // function, the caller must verify that geometryShaderSupport exists.
    const char* geometryShaderExtensionString() const {
        SkASSERT(this->geometryShaderSupport());
        return fGeometryShaderExtensionString;
    }

    // Returns the string of an extension that must be enabled in the shader to support
    // geometry shader invocations. If nullptr is returned then no extension needs to be enabled.
    // Before calling this function, the caller must verify that gsInvocationsSupport exists.
    const char* gsInvocationsExtensionString() const {
        SkASSERT(this->gsInvocationsSupport());
        return fGSInvocationsExtensionString;
    }

    // This returns the name of an extension that must be enabled in the shader, if such a thing is
    // required in order to use a secondary output in the shader. This returns a nullptr if no such
    // extension is required. However, the return value of this function does not say whether dual
    // source blending is supported.
    const char* secondaryOutputExtensionString() const { return fSecondaryOutputExtensionString; }

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

    const char* noperspectiveInterpolationExtensionString() const {
        SkASSERT(this->noperspectiveInterpolationSupport());
        return fNoPerspectiveInterpolationExtensionString;
    }

    const char* sampleVariablesExtensionString() const {
        SkASSERT(this->sampleMaskSupport());
        return fSampleVariablesExtensionString;
    }

    const char* tessellationExtensionString() const {
        SkASSERT(this->tessellationSupport());
        return fTessellationExtensionString;
    }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    // Maximum number of segments a tessellation edge can be divided into.
    int maxTessellationSegments() const { return fMaxTessellationSegments; }

    bool tessellationSupport() const { return SkToBool(fMaxTessellationSegments);}

    GrGLSLGeneration generation() const { return fGLSLGeneration; }

private:
    void applyOptionsOverrides(const GrContextOptions& options);

    GrGLSLGeneration fGLSLGeneration;

    bool fShaderDerivativeSupport           : 1;
    bool fGeometryShaderSupport             : 1;
    bool fGSInvocationsSupport              : 1;
    bool fDstReadInShaderSupport            : 1;
    bool fDualSourceBlendingSupport         : 1;
    bool fIntegerSupport                    : 1;
    bool fNonsquareMatrixSupport            : 1;
    bool fFBFetchSupport                    : 1;
    bool fFBFetchNeedsCustomOutput          : 1;
    bool fUsesPrecisionModifiers            : 1;
    bool fFlatInterpolationSupport          : 1;
    bool fPreferFlatInterpolation           : 1;
    bool fNoPerspectiveInterpolationSupport : 1;
    bool fSampleMaskSupport                 : 1;
    bool fExternalTextureSupport            : 1;
    bool fVertexIDSupport                   : 1;
    bool fInfinitySupport                   : 1;
    bool fBitManipulationSupport            : 1;
    bool fFloatIs32Bits                     : 1;
    bool fHalfIs32Bits                      : 1;
    bool fHasLowFragmentPrecision           : 1;
    bool fReducedShaderMode                 : 1;

    // Used by SkSL to know when to generate polyfills.
    bool fBuiltinFMASupport : 1;
    bool fBuiltinDeterminantSupport : 1;

    // Used for specific driver bug work arounds
    bool fCanUseAnyFunctionInShader                   : 1;
    bool fCanUseMinAndAbsTogether                     : 1;
    bool fCanUseFractForNegativeValues                : 1;
    bool fMustForceNegatedAtanParamToFloat            : 1;
    bool fMustForceNegatedLdexpParamToMultiply        : 1;
    bool fAtan2ImplementedAsAtanYOverX                : 1;
    bool fMustDoOpBetweenFloorAndAbs                  : 1;
    bool fRequiresLocalOutputColorForFBFetch          : 1;
    bool fMustObfuscateUniformColor                   : 1;
    bool fMustGuardDivisionEvenAfterExplicitZeroCheck : 1;
    bool fInBlendModesFailRandomlyForAllZeroVec       : 1;
    bool fCanUseFragCoord                             : 1;
    bool fIncompleteShortIntPrecision                 : 1;
    bool fAddAndTrueToLoopCondition                   : 1;
    bool fUnfoldShortCircuitAsTernary                 : 1;
    bool fEmulateAbsIntFunction                       : 1;
    bool fRewriteDoWhileLoops                         : 1;
    bool fRemovePowWithConstantExponent               : 1;
    bool fMustWriteToFragColor                        : 1;
    bool fNoDefaultPrecisionForExternalSamplers       : 1;
    bool fRewriteMatrixVectorMultiply                 : 1;
    bool fRewriteMatrixComparisons                    : 1;
    bool fColorSpaceMathNeedsFloat                    : 1;
    bool fCanUseDoLoops                               : 1;
    bool fCanUseFastMath                              : 1;
    bool fAvoidDfDxForGradientsWhenPossible           : 1;

    // This controls behavior of the SkSL compiler, not the code we generate
    bool fUseNodePools : 1;

    const char* fVersionDeclString;

    const char* fShaderDerivativeExtensionString;
    const char* fGeometryShaderExtensionString;
    const char* fGSInvocationsExtensionString;
    const char* fSecondaryOutputExtensionString;
    const char* fExternalTextureExtensionString;
    const char* fSecondExternalTextureExtensionString;
    const char* fNoPerspectiveInterpolationExtensionString;
    const char* fSampleVariablesExtensionString;
    const char* fTessellationExtensionString;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    int fMaxFragmentSamplers;
    int fMaxTessellationSegments;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    friend class GrCaps;  // For initialization.
    friend class GrDawnCaps;
    friend class GrD3DCaps;
    friend class GrGLCaps;
    friend class GrMockCaps;
    friend class GrMtlCaps;
    friend class GrVkCaps;
    friend class SkSL::ShaderCapsFactory;
    friend class SkSL::SharedCompiler;
};

#endif
