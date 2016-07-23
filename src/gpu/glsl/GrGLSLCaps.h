/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLSLCaps_DEFINED
#define GrGLSLCaps_DEFINED

#include "GrCaps.h"
#include "GrGLSL.h"
#include "GrSwizzle.h"

class GrGLSLCaps : public GrShaderCaps {
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

    /**
     * Initializes the GrGLSLCaps to a default set of features
     */
    GrGLSLCaps(const GrContextOptions&);

    /**
     * Some helper functions for encapsulating various extensions to read FB Buffer on openglES
     *
     * TODO(joshualitt) On desktop opengl 4.2+ we can achieve something similar to this effect
     */
    bool fbFetchSupport() const { return fFBFetchSupport; }

    bool fbFetchNeedsCustomOutput() const { return fFBFetchNeedsCustomOutput; }

    bool bindlessTextureSupport() const { return fBindlessTextureSupport; }

    const char* versionDeclString() const { return fVersionDeclString; }

    const char* fbFetchColorName() const { return fFBFetchColorName; }

    const char* fbFetchExtensionString() const { return fFBFetchExtensionString; }

    bool dropsTileOnZeroDivide() const { return fDropsTileOnZeroDivide; }

    bool flatInterpolationSupport() const { return fFlatInterpolationSupport; }

    bool noperspectiveInterpolationSupport() const { return fNoPerspectiveInterpolationSupport; }

    bool multisampleInterpolationSupport() const { return fMultisampleInterpolationSupport; }

    bool sampleVariablesSupport() const { return fSampleVariablesSupport; }

    bool sampleMaskOverrideCoverageSupport() const { return fSampleMaskOverrideCoverageSupport; }

    bool externalTextureSupport() const { return fExternalTextureSupport; }

    bool texelFetchSupport() const { return fTexelFetchSupport; }

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

    // Returns whether we can use the glsl funciton any() in our shader code.
    bool canUseAnyFunctionInShader() const { return fCanUseAnyFunctionInShader; }

    bool canUseMinAndAbsTogether() const { return fCanUseMinAndAbsTogether; }

    bool mustForceNegatedAtanParamToFloat() const { return fMustForceNegatedAtanParamToFloat; }

    // Returns the string of an extension that must be enabled in the shader to support
    // derivatives. If nullptr is returned then no extension needs to be enabled. Before calling
    // this function, the caller should check that shaderDerivativeSupport exists.
    const char* shaderDerivativeExtensionString() const {
        SkASSERT(this->shaderDerivativeSupport());
        return fShaderDerivativeExtensionString;
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

    const char* multisampleInterpolationExtensionString() const {
        SkASSERT(this->multisampleInterpolationSupport());
        return fMultisampleInterpolationExtensionString;
    }

    const char* sampleVariablesExtensionString() const {
        SkASSERT(this->sampleVariablesSupport());
        return fSampleVariablesExtensionString;
    }

    int maxVertexSamplers() const { return fMaxVertexSamplers; }

    int maxGeometrySamplers() const { return fMaxGeometrySamplers; }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    int maxCombinedSamplers() const { return fMaxCombinedSamplers; }

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

    /** Precision qualifier that should be used with a sampler, given its config and visibility. */
    GrSLPrecision samplerPrecision(GrPixelConfig config, GrShaderFlags visibility) const {
        return static_cast<GrSLPrecision>(fSamplerPrecisions[visibility][config]);
    }

    GrGLSLGeneration generation() const { return fGLSLGeneration; }

    /**
    * Returns a string containing the caps info.
    */
    SkString dump() const override;

private:
    /** GrCaps subclasses must call this after filling in the shader precision table. */
    void initSamplerPrecisionTable();

    void onApplyOptionsOverrides(const GrContextOptions& options) override;

    GrGLSLGeneration fGLSLGeneration;

    bool fDropsTileOnZeroDivide : 1;
    bool fFBFetchSupport : 1;
    bool fFBFetchNeedsCustomOutput : 1;
    bool fBindlessTextureSupport : 1;
    bool fUsesPrecisionModifiers : 1;
    bool fCanUseAnyFunctionInShader : 1;
    bool fFlatInterpolationSupport : 1;
    bool fNoPerspectiveInterpolationSupport : 1;
    bool fMultisampleInterpolationSupport : 1;
    bool fSampleVariablesSupport : 1;
    bool fSampleMaskOverrideCoverageSupport : 1;
    bool fExternalTextureSupport : 1;
    bool fTexelFetchSupport : 1;

    // Used for specific driver bug work arounds
    bool fCanUseMinAndAbsTogether : 1;
    bool fMustForceNegatedAtanParamToFloat : 1;

    const char* fVersionDeclString;

    const char* fShaderDerivativeExtensionString;
    const char* fFragCoordConventionsExtensionString;
    const char* fSecondaryOutputExtensionString;
    const char* fExternalTextureExtensionString;
    const char* fTexelBufferExtensionString;
    const char* fNoPerspectiveInterpolationExtensionString;
    const char* fMultisampleInterpolationExtensionString;
    const char* fSampleVariablesExtensionString;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    uint32_t fMaxVertexSamplers;
    uint32_t fMaxGeometrySamplers;
    uint32_t fMaxFragmentSamplers;
    uint32_t fMaxCombinedSamplers;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    GrSwizzle fConfigTextureSwizzle[kGrPixelConfigCnt];
    GrSwizzle fConfigOutputSwizzle[kGrPixelConfigCnt];

    uint8_t fSamplerPrecisions[(1 << kGrShaderTypeCount)][kGrPixelConfigCnt];

    friend class GrGLCaps;  // For initialization.
    friend class GrVkCaps;

    typedef GrShaderCaps INHERITED;
};

#endif
