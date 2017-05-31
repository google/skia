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

class GrShaderCaps : public SkRefCnt {
public:
    /** Info about shader variable precision within a given shader stage. That is, this info
        is relevant to a float (or vecNf) variable declared with a GrSLPrecision
        in a given GrShaderType. The info here is hoisted from the OpenGL spec. */
    struct PrecisionInfo {
        PrecisionInfo() {
            fLogRangeLow = 0;
            fLogRangeHigh = 0;
            fBits = 0;
        }

        /** Is this precision level allowed in the shader stage? */
        bool supported() const { return 0 != fBits; }

        bool operator==(const PrecisionInfo& that) const {
            return fLogRangeLow == that.fLogRangeLow && fLogRangeHigh == that.fLogRangeHigh &&
                   fBits == that.fBits;
        }
        bool operator!=(const PrecisionInfo& that) const { return !(*this == that); }

        /** floor(log2(|min_value|)) */
        int fLogRangeLow;
        /** floor(log2(|max_value|)) */
        int fLogRangeHigh;
        /** Number of bits of precision. As defined in OpenGL (with names modified to reflect this
            struct) :
            """
            If the smallest representable value greater than 1 is 1 + e, then fBits will
            contain floor(log2(e)), and every value in the range [2^fLogRangeLow,
            2^fLogRangeHigh] can be represented to at least one part in 2^fBits.
            """
          */
        int fBits;
    };

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

    SkString dump() const;

    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool pathRenderingSupport() const { return fPathRenderingSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool integerSupport() const { return fIntegerSupport; }
    bool texelBufferSupport() const { return fTexelBufferSupport; }
    int imageLoadStoreSupport() const { return fImageLoadStoreSupport; }

    /**
    * Get the precision info for a variable of type kFloat_GrSLType, kVec2f_GrSLType, etc in a
    * given shader type. If the shader type is not supported or the precision level is not
    * supported in that shader type then the returned struct will report false when supported() is
    * called.
    */
    const PrecisionInfo& getFloatShaderPrecisionInfo(GrShaderType shaderType,
                                                     GrSLPrecision precision) const {
        return fFloatPrecisions[shaderType][precision];
    }

    /**
    * Is there any difference between the float shader variable precision types? If this is true
    * then unless the shader type is not supported, any call to getFloatShaderPrecisionInfo() would
    * report the same info for all precisions in all shader types.
    */
    bool floatPrecisionVaries() const { return fShaderPrecisionVaries; }

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

    bool vertexIDSupport() const { return fVertexIDSupport; }

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

    bool mustForceNegatedAtanParamToFloat() const { return fMustForceNegatedAtanParamToFloat; }

    // Returns whether a device incorrectly implements atan(y,x) as atan(y/x)
    bool atan2ImplementedAsAtanYOverX() const { return fAtan2ImplementedAsAtanYOverX; }

    bool requiresLocalOutputColorForFBFetch() const { return fRequiresLocalOutputColorForFBFetch; }

    // On MacBook, geometry shaders break if they have more than one invocation.
    bool mustImplementGSInvocationsWithLoop() const { return fMustImplementGSInvocationsWithLoop; }

    bool mustObfuscateUniformColor() const { return fMustObfuscateUniformColor; }

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

    const char* imageLoadStoreExtensionString() const {
        SkASSERT(this->imageLoadStoreSupport());
        return fImageLoadStoreExtensionString;
    }

    int maxVertexSamplers() const { return fMaxVertexSamplers; }

    int maxGeometrySamplers() const { return fMaxGeometrySamplers; }

    int maxFragmentSamplers() const { return fMaxFragmentSamplers; }

    int maxCombinedSamplers() const { return fMaxCombinedSamplers; }

    int maxVertexImageStorages() const { return fMaxVertexImageStorages; }

    int maxGeometryImageStorages() const { return fMaxGeometryImageStorages; }

    int maxFragmentImageStorages() const { return fMaxFragmentImageStorages; }

    int maxCombinedImageStorages() const { return fMaxCombinedImageStorages; }

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

private:
    /** GrCaps subclasses must call this after filling in the shader precision table. */
    void initSamplerPrecisionTable();

    void applyOptionsOverrides(const GrContextOptions& options);

    GrGLSLGeneration fGLSLGeneration;

    bool fShaderDerivativeSupport   : 1;
    bool fGeometryShaderSupport     : 1;
    bool fPathRenderingSupport      : 1;
    bool fDstReadInShaderSupport    : 1;
    bool fDualSourceBlendingSupport : 1;
    bool fIntegerSupport            : 1;
    bool fTexelBufferSupport        : 1;
    bool fImageLoadStoreSupport     : 1;
    bool fShaderPrecisionVaries     : 1;
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
    bool fVertexIDSupport : 1;

    // Used for specific driver bug work arounds
    bool fCanUseMinAndAbsTogether : 1;
    bool fMustForceNegatedAtanParamToFloat : 1;
    bool fAtan2ImplementedAsAtanYOverX : 1;
    bool fRequiresLocalOutputColorForFBFetch : 1;
    bool fMustImplementGSInvocationsWithLoop : 1;
    bool fMustObfuscateUniformColor : 1;

    PrecisionInfo fFloatPrecisions[kGrShaderTypeCount][kGrSLPrecisionCount];

    const char* fVersionDeclString;

    const char* fShaderDerivativeExtensionString;
    const char* fFragCoordConventionsExtensionString;
    const char* fSecondaryOutputExtensionString;
    const char* fExternalTextureExtensionString;
    const char* fTexelBufferExtensionString;
    const char* fNoPerspectiveInterpolationExtensionString;
    const char* fMultisampleInterpolationExtensionString;
    const char* fSampleVariablesExtensionString;
    const char* fImageLoadStoreExtensionString;

    const char* fFBFetchColorName;
    const char* fFBFetchExtensionString;

    int fMaxVertexSamplers;
    int fMaxGeometrySamplers;
    int fMaxFragmentSamplers;
    int fMaxCombinedSamplers;

    int fMaxVertexImageStorages;
    int fMaxGeometryImageStorages;
    int fMaxFragmentImageStorages;
    int fMaxCombinedImageStorages;

    AdvBlendEqInteraction fAdvBlendEqInteraction;

    GrSwizzle fConfigTextureSwizzle[kGrPixelConfigCnt];
    GrSwizzle fConfigOutputSwizzle[kGrPixelConfigCnt];

    uint8_t fSamplerPrecisions[(1 << kGrShaderTypeCount)][kGrPixelConfigCnt];

    friend class GrGLCaps;  // For initialization.
    friend class GrVkCaps;
    friend class SkSL::ShaderCapsFactory;
};

#endif
