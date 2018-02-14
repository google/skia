/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrShaderCaps_DEFINED
#define GrShaderCaps_DEFINED

#include "../private/GrSwizzle.h"
#include "../private/GrGLSL.h"

namespace SkSL {
    class ShaderCapsFactory;
}
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
        kSpecificEnables_AdvBlendEqInteraction,  //<! Specific layout qualifiers per equation

        kLast_AdvBlendEqInteraction = kSpecificEnables_AdvBlendEqInteraction
    };

    GrShaderCaps(const GrContextOptions&);

    void dumpJSON(SkJSONWriter*) const;

    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool gsInvocationsSupport() const { return fGSInvocationsSupport; }
    bool pathRenderingSupport() const { return fPathRenderingSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool integerSupport() const { return fIntegerSupport; }
    bool texelBufferSupport() const { return fTexelBufferSupport; }
    int imageLoadStoreSupport() const { return fImageLoadStoreSupport; }

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

    bool dropsTileOnZeroDivide() const { return fDropsTileOnZeroDivide; }

    bool flatInterpolationSupport() const { return fFlatInterpolationSupport; }

    bool preferFlatInterpolation() const { return fPreferFlatInterpolation; }

    bool noperspectiveInterpolationSupport() const { return fNoPerspectiveInterpolationSupport; }

    bool externalTextureSupport() const { return fExternalTextureSupport; }

    bool texelFetchSupport() const { return fTexelFetchSupport; }

    bool vertexIDSupport() const { return fVertexIDSupport; }

    bool floatIs32Bits() const { return fFloatIs32Bits; }

    bool halfIs32Bits() const { return fHalfIs32Bits; }

    AdvBlendEqInteraction advBlendEqInteraction() const { return fAdvBlendEqInteraction; }

    bool mustEnableAdvBlendEqs() const {
        return fAdvBlendEqInteraction >= kGeneralEnable_AdvBlendEqInteraction;
    }

    bool mustEnableSpecificAdvBlendEqs() const {
        return fAdvBlendEqInteraction == kSpecificEnables_AdvBlendEqInteraction;
    }

    bool mustDeclareFragmentShaderOutput() const {
        return fGLSLGeneration > k110_GrGLSLGeneration;
    }

    bool usesPrecisionModifiers() const { return fUsesPrecisionModifiers; }

    // Returns whether we can use the glsl function any() in our shader code.
    bool canUseAnyFunctionInShader() const { return fCanUseAnyFunctionInShader; }

    bool canUseMinAndAbsTogether() const { return fCanUseMinAndAbsTogether; }

    bool canUseFractForNegativeValues() const { return fCanUseFractForNegativeValues; }

    bool mustForceNegatedAtanParamToFloat() const { return fMustForceNegatedAtanParamToFloat; }

    // Returns whether a device incorrectly implements atan(y,x) as atan(y/x)
    bool atan2ImplementedAsAtanYOverX() const { return fAtan2ImplementedAsAtanYOverX; }

    // If this returns true some operation (could be a no op) must be called between floor and abs
    // to make sure the driver compiler doesn't inline them together which can cause a driver bug in
    // the shader.
    bool mustDoOpBetweenFloorAndAbs() const { return fMustDoOpBetweenFloorAndAbs; }

    // If false, SkSL uses a workaround so that sk_FragCoord doesn't actually query gl_FragCoord
    bool canUseFragCoord() const { return fCanUseFragCoord; }

    // If true interpolated vertex shader outputs are inaccurate.
    bool interpolantsAreInaccurate() const { return fInterpolantsAreInaccurate; }

    bool requiresLocalOutputColorForFBFetch() const { return fRequiresLocalOutputColorForFBFetch; }

    bool mustObfuscateUniformColor() const { return fMustObfuscateUniformColor; }

    // The D3D shader compiler, when targeting PS 3.0 (ie within ANGLE) fails to compile certain
    // constructs. See detailed comments in GrGLCaps.cpp.
    bool mustGuardDivisionEvenAfterExplicitZeroCheck() const {
        return fMustGuardDivisionEvenAfterExplicitZeroCheck;
    }

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

    // Returns the string of an extension that will do all necessary coord transfomations needed
    // when reading the fragment position. If such an extension does not exisits, this function
    // returns a nullptr, and all transforms of the frag position must be done manually in the
    // shader.
    const char* fragCoordConventionsExtensionString() const {
        return fFragCoordConventionsExtensionString;
    }

    // This returns the name of an extension that must be enabled in the shader, if such a thing is
    // required in order to use a secondary output in the shader. This returns a nullptr if no such
    // extension is required. However, the return value of this function does not say whether dual
    // source blending is supported.
    const char* secondaryOutputExtensionString() const {
        return fSecondaryOutputExtensionString;
    }

    const char* externalTextureExtensionString() const {
        SkASSERT(this->externalTextureSupport());
        return fExternalTextureExtensionString;
    }

    const char* texelBufferExtensionString() const {
        SkASSERT(this->texelBufferSupport());
        return fTexelBufferExtensionString;
    }

    const char* noperspectiveInterpolationExtensionString() const {
        SkASSERT(this->noperspectiveInterpolationSupport());
        return fNoPerspectiveInterpolationExtensionString;
    }

    const char* imageLoadStoreExtensionString() const {
        SkASSERT(this->imageLoadStoreSupport());
        return fImageLoadStoreExtensionString;
    }

    int maxVertexSamplers() const { return fMaxVertexSamplers; }

    int maxGeometrySamplers() const { return fMaxGeometrySamplers; }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    int maxCombinedSamplers() const { return fMaxCombinedSamplers; }

    /**
     * In general using multiple texture units for image rendering seems to be a win at smaller
     * sizes of dst rects and a loss at larger sizes. Dst rects above this pixel area threshold will
     * not use multitexturing.
     */
    size_t disableImageMultitexturingDstRectAreaThreshold() const {
        return fDisableImageMultitexturingDstRectAreaThreshold;
    }

    /**
     * Given a texture's config, this determines what swizzle must be appended to accesses to the
     * texture in generated shader code. Swizzling may be implemented in texture parameters or a
     * sampler rather than in the shader. In this case the returned swizzle will always be "rgba".
     */
    const GrSwizzle& configTextureSwizzle(GrPixelConfig config) const {
        return fConfigTextureSwizzle[config];
    }

    /** Swizzle that should occur on the fragment shader outputs for a given config. */
    const GrSwizzle& configOutputSwizzle(GrPixelConfig config) const {
        return fConfigOutputSwizzle[config];
    }

    GrGLSLGeneration generation() const { return fGLSLGeneration; }

private:
    void applyOptionsOverrides(const GrContextOptions& options);

    GrGLSLGeneration fGLSLGeneration;

    bool fShaderDerivativeSupport   : 1;
    bool fGeometryShaderSupport     : 1;
    bool fGSInvocationsSupport      : 1;
    bool fPathRenderingSupport      : 1;
    bool fDstReadInShaderSupport    : 1;
    bool fDualSourceBlendingSupport : 1;
    bool fIntegerSupport            : 1;
    bool fTexelBufferSupport        : 1;
    bool fImageLoadStoreSupport     : 1;
    bool fDropsTileOnZeroDivide : 1;
    bool fFBFetchSupport : 1;
    bool fFBFetchNeedsCustomOutput : 1;
    bool fUsesPrecisionModifiers : 1;
    bool fFlatInterpolationSupport : 1;
    bool fPreferFlatInterpolation : 1;
    bool fNoPerspectiveInterpolationSupport : 1;
    bool fExternalTextureSupport : 1;
    bool fTexelFetchSupport : 1;
    bool fVertexIDSupport : 1;
    bool fFloatIs32Bits : 1;
    bool fHalfIs32Bits : 1;

    // Used for specific driver bug work arounds
    bool fCanUseAnyFunctionInShader : 1;
    bool fCanUseMinAndAbsTogether : 1;
    bool fCanUseFractForNegativeValues : 1;
    bool fMustForceNegatedAtanParamToFloat : 1;
    bool fAtan2ImplementedAsAtanYOverX : 1;
    bool fMustDoOpBetweenFloorAndAbs : 1;
    bool fRequiresLocalOutputColorForFBFetch : 1;
    bool fMustObfuscateUniformColor : 1;
    bool fMustGuardDivisionEvenAfterExplicitZeroCheck : 1;
    bool fCanUseFragCoord : 1;
    bool fInterpolantsAreInaccurate : 1;

    const char* fVersionDeclString;

    const char* fShaderDerivativeExtensionString;
    const char* fGeometryShaderExtensionString;
    const char* fGSInvocationsExtensionString;
    const char* fFragCoordConventionsExtensionString;
    const char* fSecondaryOutputExtensionString;
    const char* fExternalTextureExtensionString;
    const char* fTexelBufferExtensionString;
    const char* fNoPerspectiveInterpolationExtensionString;
    const char* fImageLoadStoreExtensionString;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    int fMaxVertexSamplers;
    int fMaxGeometrySamplers;
    int fMaxFragmentSamplers;
    int fMaxCombinedSamplers;

    size_t fDisableImageMultitexturingDstRectAreaThreshold;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    GrSwizzle fConfigTextureSwizzle[kGrPixelConfigCnt];
    GrSwizzle fConfigOutputSwizzle[kGrPixelConfigCnt];

    friend class GrCaps;  // For initialization.
    friend class GrGLCaps;
    friend class GrMockCaps;
    friend class GrMtlCaps;
    friend class GrVkCaps;
    friend class SkSL::ShaderCapsFactory;
};

#endif
