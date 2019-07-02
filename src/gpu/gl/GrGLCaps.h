/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLCaps_DEFINED
#define GrGLCaps_DEFINED

#include <functional>
#include "include/private/SkChecksum.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/gl/GrGLStencilAttachment.h"

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
         * OpenGL 3.0+, OpenGL ES 3.0+, GL_ARB_framebuffer_object,
         * GL_CHROMIUM_framebuffer_multisample, GL_ANGLE_framebuffer_multisample,
         * or GL_EXT_framebuffer_multisample
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

        kLast_MSFBOType = kES_EXT_MsToTexture_MSFBOType
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

    bool isFormatSRGB(const GrBackendFormat& format) const override;

    bool isFormatTexturable(SkColorType, const GrBackendFormat&) const override;

    bool isConfigTexturable(GrPixelConfig config) const override {
        GrGLenum glFormat = this->configSizedInternalFormat(config);
        GrColorType ct = GrPixelConfigToColorType(config);
        return this->isGLFormatTexturable(ct, glFormat);
    }

    int getRenderTargetSampleCount(int requestedCount,
                                   SkColorType, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const override;

    int maxRenderTargetSampleCount(SkColorType, const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(GrPixelConfig config) const override;

    bool isFormatCopyable(SkColorType, const GrBackendFormat&) const override;
    bool isConfigCopyable(GrPixelConfig config) const override {
        // In GL we have three ways to be able to copy. CopyTexImage, blit, and draw. CopyTexImage
        // requires the src to be an FBO attachment, blit requires both src and dst to be FBO
        // attachments, and draw requires the dst to be an FBO attachment. Thus to copy from and to
        // the same config, we need that config to be bindable to an FBO.
        return this->canConfigBeFBOColorAttachment(config);
    }

    bool canGLFormatBeFBOColorAttachment(GrGLenum glFormat) const;

    bool canConfigBeFBOColorAttachment(GrPixelConfig config) const {
        GrGLenum format = this->configSizedInternalFormat(config);
        if (!format) {
            return false;
        }
        return this->canGLFormatBeFBOColorAttachment(format);
    }

    bool configSupportsTexStorage(GrPixelConfig config) const {
        GrGLenum format = this->configSizedInternalFormat(config);
        if (!format) {
            return false;
        }
        return this->glFormatSupportsTexStorage(format);
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

    void getRenderbufferFormat(GrPixelConfig config, GrGLenum* internalFormat) const;
    void getSizedInternalFormat(GrPixelConfig config, GrGLenum* internalFormat) const;

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
        fConfigTable[config].fVerifiedColorAttachment = true;
    }

    /**
     * Call to check whether a config has been verified as a valid color
     * attachment.
     */
    bool isConfigVerifiedColorAttachment(GrPixelConfig config) const {
        return fConfigTable[config].fVerifiedColorAttachment;
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
               kES_EXT_MsToTexture_MSFBOType != fMSFBOType;
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

    /// Is there support for GL_PACK_REVERSE_ROW_ORDER
    bool packFlipYSupport() const { return fPackFlipYSupport; }

    /// Is there support for texture parameter GL_TEXTURE_USAGE
    bool textureUsageSupport() const { return fTextureUsageSupport; }

    /// Is GL_ALPHA8 renderable
    bool alpha8IsRenderable() const { return fAlpha8IsRenderable; }

    /// Is GL_ARB_IMAGING supported
    bool imagingSupport() const { return fImagingSupport; }

    /// Is there support for Vertex Array Objects?
    bool vertexArrayObjectSupport() const { return fVertexArrayObjectSupport; }

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

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;
    SupportedRead supportedReadPixelsColorType(GrPixelConfig, const GrBackendFormat&,
                                               GrColorType) const override;

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

    bool mipMapLevelAndLodControlSupport() const { return fMipMapLevelAndLodControlSupport; }

    bool doManualMipmapping() const { return fDoManualMipmapping; }

    void onDumpJSON(SkJSONWriter*) const override;

    bool rgba8888PixelsOpsAreSlow() const { return fRGBA8888PixelsOpsAreSlow; }
    bool partialFBOReadIsSlow() const { return fPartialFBOReadIsSlow; }
    bool rgbaToBgraReadbackConversionsAreSlow() const {
        return fRGBAToBGRAReadbackConversionsAreSlow;
    }

    bool useBufferDataNullHint() const { return fUseBufferDataNullHint; }

    // Certain Intel GPUs on Mac fail to clear if the glClearColor is made up of only 1s and 0s.
    bool clearToBoundaryValuesIsBroken() const { return fClearToBoundaryValuesIsBroken; }

    /// glClearTex(Sub)Image support
    bool clearTextureSupport() const { return fClearTextureSupport; }

    // Adreno/MSAA drops a draw on the imagefiltersbase GM if the base vertex param to
    // glDrawArrays is nonzero.
    // https://bugs.chromium.org/p/skia/issues/detail?id=6650
    bool drawArraysBaseVertexIsBroken() const { return fDrawArraysBaseVertexIsBroken; }

    // If true then we must use an intermediate surface to perform partial updates to unorm textures
    // that have ever been bound to a FBO.
    bool disallowTexSubImageForUnormConfigTexturesEverBoundToFBO() const {
        return fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO;
    }

    // Use an intermediate surface to write pixels (full or partial overwrite) to into a texture
    // that is bound to an FBO.
    bool useDrawInsteadOfAllRenderTargetWrites() const {
        return fUseDrawInsteadOfAllRenderTargetWrites;
    }

    // At least some Adreno 3xx drivers draw lines incorrectly after drawing non-lines. Toggling
    // face culling on and off seems to resolve this.
    bool requiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines() const {
        return fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines;
    }

    // Some Adreno drivers refuse to ReadPixels from an MSAA buffer that has stencil attached.
    bool detachStencilFromMSAABuffersBeforeReadPixels() const {
        return fDetachStencilFromMSAABuffersBeforeReadPixels;
    }

    // Older Android versions seem to have an issue with setting GL_TEXTURE_BASE_LEVEL or
    // GL_TEXTURE_MAX_LEVEL for GL_TEXTURE_EXTERNAL_OES textures.
    bool dontSetBaseOrMaxLevelForExternalTextures() const {
        return fDontSetBaseOrMaxLevelForExternalTextures;
    }

    // Returns the observed maximum number of instances the driver can handle in a single draw call
    // without crashing, or 'pendingInstanceCount' if this workaround is not necessary.
    // NOTE: the return value may be larger than pendingInstanceCount.
    int maxInstancesPerDrawWithoutCrashing(int pendingInstanceCount) const {
        return (fMaxInstancesPerDrawWithoutCrashing)
                ? fMaxInstancesPerDrawWithoutCrashing : pendingInstanceCount;
    }

    bool canCopyTexSubImage(GrPixelConfig dstConfig, bool dstHasMSAARenderBuffer,
                            const GrTextureType* dstTypeIfTexture,
                            GrPixelConfig srcConfig, bool srcHasMSAARenderBuffer,
                            const GrTextureType* srcTypeIfTexture) const;
    bool canCopyAsBlit(GrPixelConfig dstConfig, int dstSampleCnt,
                       const GrTextureType* dstTypeIfTexture,
                       GrPixelConfig srcConfig, int srcSampleCnt,
                        const GrTextureType* srcTypeIfTexture,
                       const SkRect& srcBounds, bool srcBoundsExact,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) const;
    bool canCopyAsDraw(GrPixelConfig dstConfig, bool srcIsTextureable) const;

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override;

    bool programBinarySupport() const { return fProgramBinarySupport; }
    bool programParameterSupport() const { return fProgramParameterSupport; }

    bool samplerObjectSupport() const { return fSamplerObjectSupport; }

    bool fbFetchRequiresEnablePerSample() const { return fFBFetchRequiresEnablePerSample; }

    GrPixelConfig validateBackendRenderTarget(const GrBackendRenderTarget&,
                                              GrColorType) const override;

    bool areColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    GrPixelConfig getConfigFromBackendFormat(const GrBackendFormat&, GrColorType) const override;
    GrPixelConfig getYUVAConfigFromBackendFormat(const GrBackendFormat&) const override;

    GrBackendFormat getBackendFormatFromGrColorType(GrColorType ct,
                                                    GrSRGBEncoded srgbEncoded) const override;
    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override;
    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

#if GR_TEST_UTILS
    GrGLStandard standard() const { return fStandard; }
#endif

private:
    enum ExternalFormatUsage {
        kTexImage_ExternalFormatUsage,
        kReadPixels_ExternalFormatUsage,

        kLast_ExternalFormatUsage = kReadPixels_ExternalFormatUsage
    };
    static const int kExternalFormatUsageCnt = kLast_ExternalFormatUsage + 1;
    bool getExternalFormat(GrPixelConfig surfaceConfig, GrPixelConfig memoryConfig,
                           ExternalFormatUsage usage, GrGLenum* externalFormat,
                           GrGLenum* externalType) const;

    void init(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*);
    void initGLSL(const GrGLContextInfo&, const GrGLInterface*);
    bool hasPathRenderingSupport(const GrGLContextInfo&, const GrGLInterface*);

    struct FormatWorkarounds {
        bool fDisableTextureRedForMesa = false;
        bool fDisableSRGBRenderWithMSAAForMacAMD = false;
        bool fDisablePerFormatTextureStorageForCommandBufferES2 = false;
        bool fDisableNonRedSingleChannelTexStorageForANGLEGL = false;
        bool fDisableBGRATextureStorageForIntelWindowsES = false;
        bool fDisableRGB8ForMali400 = false;
    };

    void applyDriverCorrectnessWorkarounds(const GrGLContextInfo&, const GrContextOptions&,
                                           GrShaderCaps*, FormatWorkarounds*);

    void onApplyOptionsOverrides(const GrContextOptions& options) override;

    bool onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget&) const override;

    void initFSAASupport(const GrContextOptions& contextOptions, const GrGLContextInfo&,
                         const GrGLInterface*);
    void initBlendEqationSupport(const GrGLContextInfo&);
    void initStencilSupport(const GrGLContextInfo&);
    // This must be called after initFSAASupport().
    void initConfigTable(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*);
    void initFormatTable(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*,
                         const FormatWorkarounds&);
    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    size_t onTransferFromOffsetAlignment(GrColorType bufferColorType) const override;

    bool isGLFormatTexturable(GrColorType, GrGLenum glFormat) const;
    bool glFormatSupportsTexStorage(GrGLenum glFormat) const;

    GrGLStandard fStandard;

    SkTArray<StencilFormat, true> fStencilFormats;

    int fMaxFragmentUniformVectors;

    MSFBOType           fMSFBOType;
    InvalidateFBType    fInvalidateFBType;
    MapBufferType       fMapBufferType;
    TransferBufferType  fTransferBufferType;

    bool fPackFlipYSupport : 1;
    bool fTextureUsageSupport : 1;
    bool fAlpha8IsRenderable: 1;
    bool fImagingSupport  : 1;
    bool fVertexArrayObjectSupport : 1;
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
    bool fMipMapLevelAndLodControlSupport : 1;
    bool fRGBAToBGRAReadbackConversionsAreSlow : 1;
    bool fUseBufferDataNullHint                : 1;
    bool fClearTextureSupport : 1;
    bool fProgramBinarySupport : 1;
    bool fProgramParameterSupport : 1;
    bool fSamplerObjectSupport : 1;
    bool fFBFetchRequiresEnablePerSample : 1;

    // Driver workarounds
    bool fDoManualMipmapping : 1;
    bool fClearToBoundaryValuesIsBroken : 1;
    bool fDrawArraysBaseVertexIsBroken : 1;
    bool fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO : 1;
    bool fUseDrawInsteadOfAllRenderTargetWrites : 1;
    bool fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines : 1;
    bool fDetachStencilFromMSAABuffersBeforeReadPixels : 1;
    bool fDontSetBaseOrMaxLevelForExternalTextures : 1;
    int fMaxInstancesPerDrawWithoutCrashing;

    uint32_t fBlitFramebufferFlags;

    /** Number type of the components (with out considering number of bits.) */
    enum FormatType {
        kNormalizedFixedPoint_FormatType,
        kFloat_FormatType,
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
        int fStencilFormatIndex;

        // If data from a surface of this config is read back to a GrColorType with all four
        // color channels this indicates how each channel should be interpreted. May contain
        // 0s and 1s.
        GrSwizzle fRGBAReadSwizzle = GrSwizzle("rgba");

        SkTDArray<int> fColorSampleCounts;

        enum {
            kRenderable_Flag              = 0x1,
            kRenderableWithMSAA_Flag      = 0x2,
        };
        uint32_t fFlags;

        // verification of color attachment validity is done while flushing. Although only ever
        // used in the (sole) rendering thread it can cause races if it is glommed into fFlags.
        bool fVerifiedColorAttachment = false;
    };

    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    struct ColorTypeInfo {
        ColorTypeInfo(GrColorType colorType, uint32_t flags)
                : fColorType(colorType)
                , fFlags(flags) {}

        GrColorType fColorType;
        enum {
            kUploadData_Flag = 0x1,
        };
        uint32_t fFlags;
    };

    struct FormatInfo {
        uint32_t colorTypeFlags(GrColorType colorType) const {
            for (int i = 0; i < fColorTypeInfos.count(); ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        enum {
            kTextureable_Flag                = 0x1,
            /** kFBOColorAttachment means that even if the format cannot be a GrRenderTarget, we can
                still attach it to a FBO for blitting or reading pixels. */
            kFBOColorAttachment_Flag         = 0x2,
            kFBOColorAttachmentWithMSAA_Flag = 0x4,
            kCanUseTexStorage_Flag           = 0x8,
        };
        uint32_t fFlags = 0;

        SkSTArray<1, ColorTypeInfo> fColorTypeInfos;
    };

    static const size_t kNumGLFormats = 21;
    FormatInfo fFormatTable[kNumGLFormats];

    FormatInfo& getFormatInfo(GrGLenum glFormat);
    const FormatInfo& getFormatInfo(GrGLenum glFormat) const;

    typedef GrCaps INHERITED;
};

#endif
