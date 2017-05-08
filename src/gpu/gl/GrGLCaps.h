/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLCaps_DEFINED
#define GrGLCaps_DEFINED

#include <functional>

#include "GrCaps.h"
#include "GrGLStencilAttachment.h"
#include "GrSwizzle.h"
#include "SkChecksum.h"
#include "SkTHash.h"
#include "SkTArray.h"
#include "../private/GrGLSL.h"

class GrGLContextInfo;
class GrGLRenderTarget;

/**
 * Stores some capabilities of a GL context. Most are determined by the GL
 * version and the extensions string. It also tracks formats that have passed
 * the FBO completeness test.
 */
class GrGLCaps : public GrCaps {
public:
    typedef GrGLStencilAttachment::Format StencilFormat;

    /**
     * The type of MSAA for FBOs supported. Different extensions have different
     * semantics of how / when a resolve is performed.
     */
    enum MSFBOType {
        /**
         * no support for MSAA FBOs
         */
        kNone_MSFBOType = 0,
        /**
         * OpenGL < 3.0 with GL_EXT_framebuffer_object. Doesn't allow rendering to ALPHA.
         */
        kEXT_MSFBOType,
        /**
         * OpenGL 3.0+, OpenGL ES 3.0+, and GL_ARB_framebuffer_object.
         */
        kStandard_MSFBOType,
        /**
         * GL_APPLE_framebuffer_multisample ES extension
         */
        kES_Apple_MSFBOType,
        /**
         * GL_IMG_multisampled_render_to_texture. This variation does not have MSAA renderbuffers.
         * Instead the texture is multisampled when bound to the FBO and then resolved automatically
         * when read. It also defines an alternate value for GL_MAX_SAMPLES (which we call
         * GR_GL_MAX_SAMPLES_IMG).
         */
        kES_IMG_MsToTexture_MSFBOType,
        /**
         * GL_EXT_multisampled_render_to_texture. Same as the IMG one above but uses the standard
         * GL_MAX_SAMPLES value.
         */
        kES_EXT_MsToTexture_MSFBOType,
        /**
         * GL_NV_framebuffer_mixed_samples.
         */
        kMixedSamples_MSFBOType,

        kLast_MSFBOType = kMixedSamples_MSFBOType
    };

    enum BlitFramebufferFlags {
        kNoSupport_BlitFramebufferFlag                    = 1 << 0,
        kNoScalingOrMirroring_BlitFramebufferFlag         = 1 << 1,
        kResolveMustBeFull_BlitFrambufferFlag             = 1 << 2,
        kNoMSAADst_BlitFramebufferFlag                    = 1 << 3,
        kNoFormatConversion_BlitFramebufferFlag           = 1 << 4,
        kNoFormatConversionForMSAASrc_BlitFramebufferFlag = 1 << 5,
        kRectsMustMatchForMSAASrc_BlitFramebufferFlag     = 1 << 6,
    };

    enum InvalidateFBType {
        kNone_InvalidateFBType,
        kDiscard_InvalidateFBType,       //<! glDiscardFramebuffer()
        kInvalidate_InvalidateFBType,    //<! glInvalidateFramebuffer()

        kLast_InvalidateFBType = kInvalidate_InvalidateFBType
    };

    enum MapBufferType {
        kNone_MapBufferType,
        kMapBuffer_MapBufferType,         // glMapBuffer()
        kMapBufferRange_MapBufferType,    // glMapBufferRange()
        kChromium_MapBufferType,          // GL_CHROMIUM_map_sub

        kLast_MapBufferType = kChromium_MapBufferType,
    };

    enum TransferBufferType {
        kNone_TransferBufferType,
        kPBO_TransferBufferType,          // ARB_pixel_buffer_object
        kChromium_TransferBufferType,     // CHROMIUM_pixel_transfer_buffer_object

        kLast_TransferBufferType = kChromium_TransferBufferType,
    };

    /**
     * Initializes the GrGLCaps to the set of features supported in the current
     * OpenGL context accessible via ctxInfo.
     */
    GrGLCaps(const GrContextOptions& contextOptions, const GrGLContextInfo& ctxInfo,
             const GrGLInterface* glInterface);

    bool isConfigTexturable(GrPixelConfig config) const override {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kTextureable_Flag);
    }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        if (withMSAA) {
            return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kRenderableWithMSAA_Flag);
        } else {
            return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag);
        }
    }
    bool canConfigBeImageStorage(GrPixelConfig config) const override {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kCanUseAsImageStorage_Flag);
    }
    bool canConfigBeFBOColorAttachment(GrPixelConfig config) const {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kFBOColorAttachment_Flag);
    }

    bool isConfigTexSupportEnabled(GrPixelConfig config) const {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kCanUseTexStorage_Flag);
    }

    bool canUseConfigWithTexelBuffer(GrPixelConfig config) const {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kCanUseWithTexelBuffer_Flag);
    }

    /** Returns the mapping between GrPixelConfig components and GL internal format components. */
    const GrSwizzle& configSwizzle(GrPixelConfig config) const {
        return fConfigTable[config].fSwizzle;
    }

    GrGLenum configSizedInternalFormat(GrPixelConfig config) const {
        return fConfigTable[config].fFormats.fSizedInternalFormat;
    }

    bool getTexImageFormats(GrPixelConfig surfaceConfig, GrPixelConfig externalConfig,
                            GrGLenum* internalFormat, GrGLenum* externalFormat,
                            GrGLenum* externalType) const;

    bool getCompressedTexImageFormats(GrPixelConfig surfaceConfig, GrGLenum* internalFormat) const;

    bool getReadPixelsFormat(GrPixelConfig surfaceConfig, GrPixelConfig externalConfig,
                             GrGLenum* externalFormat, GrGLenum* externalType) const;

    bool getRenderbufferFormat(GrPixelConfig config, GrGLenum* internalFormat) const;

    /** The format to use read/write a texture as an image in a shader */
    GrGLenum getImageFormat(GrPixelConfig config) const {
        return fConfigTable[config].fFormats.fSizedInternalFormat;
    }

    /**
    * Gets an array of legal stencil formats. These formats are not guaranteed
    * to be supported by the driver but are legal GLenum names given the GL
    * version and extensions supported.
    */
    const SkTArray<StencilFormat, true>& stencilFormats() const {
        return fStencilFormats;
    }

    /**
     * Has a stencil format index been found for the config (or we've found that no format works).
     */
    bool hasStencilFormatBeenDeterminedForConfig(GrPixelConfig config) const {
        return fConfigTable[config].fStencilFormatIndex != ConfigInfo::kUnknown_StencilIndex;
    }

    /**
     * Gets the stencil format index for the config. This assumes
     * hasStencilFormatBeenDeterminedForConfig has already been checked. Returns a value < 0 if
     * no stencil format is supported with the config. Otherwise, returned index refers to the array
     * returned by stencilFormats().
     */
    int getStencilFormatIndexForConfig(GrPixelConfig config) const {
        SkASSERT(this->hasStencilFormatBeenDeterminedForConfig(config));
        return fConfigTable[config].fStencilFormatIndex;
    }

    /**
     * If index is >= 0 this records an index into stencilFormats() as the best stencil format for
     * the config. If < 0 it records that the config has no supported stencil format index.
     */
    void setStencilFormatIndexForConfig(GrPixelConfig config, int index) {
        SkASSERT(!this->hasStencilFormatBeenDeterminedForConfig(config));
        if (index < 0) {
            fConfigTable[config].fStencilFormatIndex = ConfigInfo::kUnsupported_StencilFormatIndex;
        } else {
            fConfigTable[config].fStencilFormatIndex = index;
        }
    }

    /**
     * Call to note that a color config has been verified as a valid color
     * attachment. This may save future calls to glCheckFramebufferStatus
     * using isConfigVerifiedColorAttachment().
     */
    void markConfigAsValidColorAttachment(GrPixelConfig config) {
        fConfigTable[config].fFlags |= ConfigInfo::kVerifiedColorAttachment_Flag;
    }

    /**
     * Call to check whether a config has been verified as a valid color
     * attachment.
     */
    bool isConfigVerifiedColorAttachment(GrPixelConfig config) const {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kVerifiedColorAttachment_Flag);
    }

    /**
     * Reports the type of MSAA FBO support.
     */
    MSFBOType msFBOType() const { return fMSFBOType; }

    /**
     * Does the preferred MSAA FBO extension have MSAA renderbuffers?
     */
    bool usesMSAARenderBuffers() const {
        return kNone_MSFBOType != fMSFBOType &&
               kES_IMG_MsToTexture_MSFBOType != fMSFBOType &&
               kES_EXT_MsToTexture_MSFBOType != fMSFBOType &&
               kMixedSamples_MSFBOType != fMSFBOType;
    }

    /**
     * What functionality is supported by glBlitFramebuffer.
     */
    uint32_t blitFramebufferSupportFlags() const { return fBlitFramebufferFlags; }

    /**
     * Is the MSAA FBO extension one where the texture is multisampled when bound to an FBO and
     * then implicitly resolved when read.
     */
    bool usesImplicitMSAAResolve() const {
        return kES_IMG_MsToTexture_MSFBOType == fMSFBOType ||
               kES_EXT_MsToTexture_MSFBOType == fMSFBOType;
    }

    InvalidateFBType invalidateFBType() const { return fInvalidateFBType; }

    /// What type of buffer mapping is supported?
    MapBufferType mapBufferType() const { return fMapBufferType; }

    /// What type of transfer buffer is supported?
    TransferBufferType transferBufferType() const { return fTransferBufferType; }

    /// The maximum number of fragment uniform vectors (GLES has min. 16).
    int maxFragmentUniformVectors() const { return fMaxFragmentUniformVectors; }

    /**
     * Depending on the ES extensions present the BGRA external format may
     * correspond to either a BGRA or RGBA internalFormat. On desktop GL it is
     * RGBA.
     */
    bool bgraIsInternalFormat() const;

    /// Is there support for GL_UNPACK_ROW_LENGTH
    bool unpackRowLengthSupport() const { return fUnpackRowLengthSupport; }

    /// Is there support for GL_UNPACK_FLIP_Y
    bool unpackFlipYSupport() const { return fUnpackFlipYSupport; }

    /// Is there support for GL_PACK_ROW_LENGTH
    bool packRowLengthSupport() const { return fPackRowLengthSupport; }

    /// Is there support for GL_PACK_REVERSE_ROW_ORDER
    bool packFlipYSupport() const { return fPackFlipYSupport; }

    /// Is there support for texture parameter GL_TEXTURE_USAGE
    bool textureUsageSupport() const { return fTextureUsageSupport; }

    /// Is there support for GL_RED and GL_R8
    bool textureRedSupport() const { return fTextureRedSupport; }

    /// Is GL_ARB_IMAGING supported
    bool imagingSupport() const { return fImagingSupport; }

    /// Is there support for Vertex Array Objects?
    bool vertexArrayObjectSupport() const { return fVertexArrayObjectSupport; }

    /// Is there support for GL_EXT_direct_state_access?
    bool directStateAccessSupport() const { return fDirectStateAccessSupport; }

    /// Is there support for GL_KHR_debug?
    bool debugSupport() const { return fDebugSupport; }

    /// Is there support for ES2 compatability?
    bool ES2CompatibilitySupport() const { return fES2CompatibilitySupport; }

    /// Is there support for glDraw*Instanced?
    bool drawInstancedSupport() const { return fDrawInstancedSupport; }

    /// Is there support for glDraw*Indirect? Note that the baseInstance fields of indirect draw
    /// commands cannot be used unless we have base instance support.
    bool drawIndirectSupport() const { return fDrawIndirectSupport; }

    /// Is there support for glMultiDraw*Indirect? Note that the baseInstance fields of indirect
    /// draw commands cannot be used unless we have base instance support.
    bool multiDrawIndirectSupport() const { return fMultiDrawIndirectSupport; }

    /// Is there support for glDrawRangeElements?
    bool drawRangeElementsSupport() const { return fDrawRangeElementsSupport; }

    /// Are the baseInstance fields supported in indirect draw commands?
    bool baseInstanceSupport() const { return fBaseInstanceSupport; }

    /// Use indices or vertices in CPU arrays rather than VBOs for dynamic content.
    bool useNonVBOVertexAndIndexDynamicData() const { return fUseNonVBOVertexAndIndexDynamicData; }

    /// Does ReadPixels support reading readConfig pixels from a FBO that is surfaceConfig?
    bool readPixelsSupported(GrPixelConfig surfaceConfig,
                             GrPixelConfig readConfig,
                             std::function<void (GrGLenum, GrGLint*)> getIntegerv,
                             std::function<bool ()> bindRenderTarget,
                             std::function<void ()> unbindRenderTarget) const;

    bool isCoreProfile() const { return fIsCoreProfile; }

    bool bindFragDataLocationSupport() const { return fBindFragDataLocationSupport; }

    bool bindUniformLocationSupport() const { return fBindUniformLocationSupport; }

    /// Are textures with GL_TEXTURE_RECTANGLE type supported.
    bool rectangleTextureSupport() const { return fRectangleTextureSupport; }

    /// GL_ARB_texture_swizzle
    bool textureSwizzleSupport() const { return fTextureSwizzleSupport; }

    bool mipMapLevelAndLodControlSupport() const { return fMipMapLevelAndLodControlSupport; }

    bool doManualMipmapping() const { return fDoManualMipmapping; }

    bool srgbDecodeDisableSupport() const { return fSRGBDecodeDisableSupport; }
    bool srgbDecodeDisableAffectsMipmaps() const { return fSRGBDecodeDisableAffectsMipmaps; }

    /**
     * Returns a string containing the caps info.
     */
    SkString dump() const override;

    bool rgba8888PixelsOpsAreSlow() const { return fRGBA8888PixelsOpsAreSlow; }
    bool partialFBOReadIsSlow() const { return fPartialFBOReadIsSlow; }
    bool rgbaToBgraReadbackConversionsAreSlow() const {
        return fRGBAToBGRAReadbackConversionsAreSlow;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override;

private:
    enum ExternalFormatUsage {
        kTexImage_ExternalFormatUsage,
        kOther_ExternalFormatUsage,

        kLast_ExternalFormatUsage = kOther_ExternalFormatUsage
    };
    static const int kExternalFormatUsageCnt = kLast_ExternalFormatUsage + 1;
    bool getExternalFormat(GrPixelConfig surfaceConfig, GrPixelConfig memoryConfig,
                           ExternalFormatUsage usage, GrGLenum* externalFormat,
                           GrGLenum* externalType) const;

    void init(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*);
    void initGLSL(const GrGLContextInfo&);
    bool hasPathRenderingSupport(const GrGLContextInfo&, const GrGLInterface*);

    void onApplyOptionsOverrides(const GrContextOptions& options) override;

    void initFSAASupport(const GrGLContextInfo&, const GrGLInterface*);
    void initBlendEqationSupport(const GrGLContextInfo&);
    void initStencilFormats(const GrGLContextInfo&);
    // This must be called after initFSAASupport().
    void initConfigTable(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*,
                         GrShaderCaps*);

    void initShaderPrecisionTable(const GrGLContextInfo&, const GrGLInterface*, GrShaderCaps*);

    GrGLStandard fStandard;

    SkTArray<StencilFormat, true> fStencilFormats;

    int fMaxFragmentUniformVectors;

    MSFBOType           fMSFBOType;
    InvalidateFBType    fInvalidateFBType;
    MapBufferType       fMapBufferType;
    TransferBufferType  fTransferBufferType;

    bool fUnpackRowLengthSupport : 1;
    bool fUnpackFlipYSupport : 1;
    bool fPackRowLengthSupport : 1;
    bool fPackFlipYSupport : 1;
    bool fTextureUsageSupport : 1;
    bool fTextureRedSupport : 1;
    bool fImagingSupport  : 1;
    bool fVertexArrayObjectSupport : 1;
    bool fDirectStateAccessSupport : 1;
    bool fDebugSupport : 1;
    bool fES2CompatibilitySupport : 1;
    bool fDrawInstancedSupport : 1;
    bool fDrawIndirectSupport : 1;
    bool fDrawRangeElementsSupport : 1;
    bool fMultiDrawIndirectSupport : 1;
    bool fBaseInstanceSupport : 1;
    bool fUseNonVBOVertexAndIndexDynamicData : 1;
    bool fIsCoreProfile : 1;
    bool fBindFragDataLocationSupport : 1;
    bool fRGBA8888PixelsOpsAreSlow : 1;
    bool fPartialFBOReadIsSlow : 1;
    bool fBindUniformLocationSupport : 1;
    bool fRectangleTextureSupport : 1;
    bool fTextureSwizzleSupport : 1;
    bool fMipMapLevelAndLodControlSupport : 1;
    bool fRGBAToBGRAReadbackConversionsAreSlow : 1;
    bool fDoManualMipmapping : 1;
    bool fSRGBDecodeDisableSupport : 1;
    bool fSRGBDecodeDisableAffectsMipmaps : 1;

    uint32_t fBlitFramebufferFlags;

    /** Number type of the components (with out considering number of bits.) */
    enum FormatType {
        kNormalizedFixedPoint_FormatType,
        kFloat_FormatType,
        kInteger_FormatType,
    };

    struct ReadPixelsFormat {
        ReadPixelsFormat() : fFormat(0), fType(0) {}
        GrGLenum fFormat;
        GrGLenum fType;
    };

    struct ConfigFormats {
        ConfigFormats() {
            // Inits to known bad GL enum values.
            memset(this, 0xAB, sizeof(ConfigFormats));
        }
        GrGLenum fBaseInternalFormat;
        GrGLenum fSizedInternalFormat;

        /** The external format and type are to be used when uploading/downloading data using this
            config where both the CPU data and GrSurface are the same config. To get the external
            format and type when converting between configs while copying to/from memory use
            getExternalFormat().
            The kTexImage external format is usually the same as kOther except for kSRGBA on some
            GL contexts. */
        GrGLenum fExternalFormat[kExternalFormatUsageCnt];
        GrGLenum fExternalType;

        // Either the base or sized internal format depending on the GL and config.
        GrGLenum fInternalFormatTexImage;
        GrGLenum fInternalFormatRenderbuffer;
    };

    struct ConfigInfo {
        ConfigInfo() : fStencilFormatIndex(kUnknown_StencilIndex), fFlags(0) {}

        ConfigFormats fFormats;

        FormatType fFormatType;

        // On ES contexts there are restrictions on type type/format that may be used for
        // ReadPixels. One is implicitly specified by the current FBO's format. The other is
        // queryable. This stores the queried option (lazily).
        ReadPixelsFormat fSecondReadPixelsFormat;

        enum {
            // This indicates that a stencil format has not yet been determined for the config.
            kUnknown_StencilIndex = -1,
            // This indicates that there is no supported stencil format for the config.
            kUnsupported_StencilFormatIndex = -2
        };

        // Index fStencilFormats.
        int      fStencilFormatIndex;

        enum {
            kVerifiedColorAttachment_Flag = 0x1,
            kTextureable_Flag             = 0x2,
            kRenderable_Flag              = 0x4,
            kRenderableWithMSAA_Flag      = 0x8,
            /** kFBOColorAttachment means that even if the config cannot be a GrRenderTarget, we can
                still attach it to a FBO for blitting or reading pixels. */
            kFBOColorAttachment_Flag      = 0x10,
            kCanUseTexStorage_Flag        = 0x20,
            kCanUseWithTexelBuffer_Flag   = 0x40,
            kCanUseAsImageStorage_Flag    = 0x80,
        };
        uint32_t fFlags;

        GrSwizzle fSwizzle;
    };

    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    typedef GrCaps INHERITED;
};

#endif
