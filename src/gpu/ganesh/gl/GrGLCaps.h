/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLCaps_DEFINED
#define GrGLCaps_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/gl/GrGLTypesPriv.h"

#include <cstdint>
#include <memory>
#include <vector>

class GrGLContextInfo;
class GrProgramInfo;
class GrRenderTarget;
class GrRenderTargetProxy;
class GrSurface;
class GrSurfaceProxy;
class SkJSONWriter;
enum class SkTextureCompressionType;
struct GrContextOptions;
struct GrGLInterface;
struct GrShaderCaps;
struct SkIRect;
struct SkRect;

namespace GrTest { struct TestFormatColorTypeCombination; }

/**
 * Stores some capabilities of a GL context. Most are determined by the GL
 * version and the extensions string. It also tracks formats that have passed
 * the FBO completeness test.
 */
class GrGLCaps : public GrCaps {
public:
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
        kDiscard_InvalidateFBType,     //<! glDiscardFramebuffer()
        kInvalidate_InvalidateFBType,  //<! glInvalidateFramebuffer()
    };

    enum class InvalidateBufferType {
        kNone,
        kNullData,   // Call glBufferData with a null data pointer.
        kInvalidate  // glInvalidateBufferData
    };

    enum MapBufferType {
        kNone_MapBufferType,
        kMapBuffer_MapBufferType,       // glMapBuffer()
        kMapBufferRange_MapBufferType,  // glMapBufferRange()
        kChromium_MapBufferType,        // GL_CHROMIUM_map_sub
    };

    enum class TransferBufferType {
        kNone,
        kNV_PBO,    // NV_pixel_buffer_object
        kARB_PBO,   // ARB_pixel_buffer_object
        kChromium,  // CHROMIUM_pixel_transfer_buffer_object
    };

    enum class FenceType {
        kNone,
        kSyncObject,
        kNVFence
    };

    enum class MultiDrawType {
        kNone,
        kMultiDrawIndirect,  // ARB_multi_draw_indirect, EXT_multi_draw_indirect, or GL 4.3 core.
        kANGLEOrWebGL  // ANGLE_base_vertex_base_instance or
                       // WEBGL_draw_instanced_base_vertex_base_instance
    };

    enum class RegenerateMipmapType {
        kBaseLevel,
        kBasePlusMaxLevel,
        kBasePlusSync
    };

    /**
     * Initializes the GrGLCaps to the set of features supported in the current
     * OpenGL context accessible via ctxInfo.
     */
    GrGLCaps(const GrContextOptions& contextOptions, const GrGLContextInfo& ctxInfo,
             const GrGLInterface* glInterface);

    bool isFormatSRGB(const GrBackendFormat&) const override;

    bool isFormatTexturable(const GrBackendFormat&, GrTextureType) const override;
    bool isFormatTexturable(GrGLFormat) const;

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;
    bool isFormatRenderable(GrGLFormat format, int sampleCount) const {
        return sampleCount <= this->maxRenderTargetSampleCount(format);
    }

    int getRenderTargetSampleCount(int requestedCount,
                                   const GrBackendFormat& format) const override {
        return this->getRenderTargetSampleCount(requestedCount,
                                                GrBackendFormats::AsGLFormat(format));
    }
    int getRenderTargetSampleCount(int requestedCount, GrGLFormat) const;

    int maxRenderTargetSampleCount(const GrBackendFormat& format) const override {
        return this->maxRenderTargetSampleCount(GrBackendFormats::AsGLFormat(format));
    }
    int maxRenderTargetSampleCount(GrGLFormat) const;

    float maxTextureMaxAnisotropy() const { return fMaxTextureMaxAnisotropy; }

    bool isFormatCopyable(const GrBackendFormat&) const override;

    bool canFormatBeFBOColorAttachment(GrGLFormat) const;

    GrGLFormat getFormatFromColorType(GrColorType colorType) const {
        int idx = static_cast<int>(colorType);
        return fColorTypeToFormatTable[idx];
    }

    /**
     * Gets the internal format to use with glTexImage...() and glTexStorage...(). May be sized or
     * base depending upon the GL. Not applicable to compressed textures.
     */
    GrGLenum getTexImageOrStorageInternalFormat(GrGLFormat format) const {
        return this->getFormatInfo(format).fInternalFormatForTexImageOrStorage;
    }

    /**
     * Gets the external format and type to pass to glTexImage2D with nullptr to create an
     * uninitialized texture. See getTexImageOrStorageInternalFormat() for the internal format.
     */
    void getTexImageExternalFormatAndType(GrGLFormat surfaceFormat, GrGLenum* externalFormat,
                                          GrGLenum* externalType) const;

    /**
     * Given a src data color type and a color type interpretation for a texture of a given format
     * this provides the external GL format and type to use with glTexSubImage2d. The color types
     * should originate from supportedWritePixelsColorType().
     */
    void getTexSubImageExternalFormatAndType(GrGLFormat surfaceFormat, GrColorType surfaceColorType,
                                             GrColorType memoryColorType, GrGLenum* externalFormat,
                                             GrGLenum* externalType) const;

    /**
     * Gets the external format, type, and bytes per pixel to use when uploading solid color data
     * via glTexSubImage...() to clear the texture at creation.
     */
    void getTexSubImageDefaultFormatTypeAndColorType(GrGLFormat format,
                                                     GrGLenum* externalFormat,
                                                     GrGLenum* externalType,
                                                     GrColorType* colorType) const;

    void getReadPixelsFormat(GrGLFormat surfaceFormat, GrColorType surfaceColorType,
                             GrColorType memoryColorType, GrGLenum* externalFormat,
                             GrGLenum* externalType) const;

    /**
    * Gets an array of legal stencil formats. These formats are not guaranteed
    * to be supported by the driver but are legal GLenum names given the GL
    * version and extensions supported.
    */
    const skia_private::TArray<GrGLFormat, true>& stencilFormats() const {
        return fStencilFormats;
    }

    bool formatSupportsTexStorage(GrGLFormat) const;

    /**
     * Would it be useful to check GL_IMPLEMENTATION_READ_FORMAT and _TYPE for this format to
     * detect more efficient glReadPixels arguments?
     */
    bool shouldQueryImplementationReadSupport(GrGLFormat format) const;

    /**
     * Let caps know the result of GL_IMPLEMENTATION_READ_FORMAT and _TYPE query for a format
     * to update supported glReadPixels arguments.
     */
    void didQueryImplementationReadSupport(GrGLFormat format,
                                           GrGLenum readFormat,
                                           GrGLenum readType) const;

    /**
     * Gets the internal format to use with glRenderbufferStorageMultisample...(). May be sized or
     * base depending upon the GL. Not applicable to compressed textures.
     */
    GrGLenum getRenderbufferInternalFormat(GrGLFormat format) const {
        return this->getFormatInfo(format).fInternalFormatForRenderbuffer;
    }

    /**
     * Gets the default external type to use with glTex[Sub]Image... when the data pointer is null.
     */
    GrGLenum getFormatDefaultExternalType(GrGLFormat format) const {
        return this->getFormatInfo(format).fDefaultExternalType;
    }

    /**
     * Has a stencil format index been found for the format (or we've found that no format works).
     */
    bool hasStencilFormatBeenDeterminedForFormat(GrGLFormat format) const {
        return this->getFormatInfo(format).fStencilFormatIndex != FormatInfo::kUnknown_StencilIndex;
    }

    /**
     * Gets the stencil format index for the format. This assumes
     * hasStencilFormatBeenDeterminedForFormat has already been checked. Returns a value < 0 if
     * no stencil format is supported with the format. Otherwise, returned index refers to the array
     * returned by stencilFormats().
     */
    int getStencilFormatIndexForFormat(GrGLFormat format) const {
        SkASSERT(this->hasStencilFormatBeenDeterminedForFormat(format));
        return this->getFormatInfo(format).fStencilFormatIndex;
    }

    /**
     * If index is >= 0 this records an index into stencilFormats() as the best stencil format for
     * the format. If < 0 it records that the format has no supported stencil format index.
     */
    void setStencilFormatIndexForFormat(GrGLFormat, int index);

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
     * Is it unsupported to only resolve a sub-rectangle of a framebuffer?
     */
    bool framebufferResolvesMustBeFullSize() const {
        SkASSERT(fMSFBOType != kNone_MSFBOType);
        return fMSFBOType == kES_Apple_MSFBOType ||
               (fBlitFramebufferFlags & kResolveMustBeFull_BlitFrambufferFlag);
    }

    /**
     * Can we resolve a single-sample framebuffer into an MSAA framebuffer?
     */
    bool canResolveSingleToMSAA() const {
        SkASSERT(fMSFBOType != kNone_MSFBOType);
        return fMSFBOType != kES_Apple_MSFBOType &&
               !(fBlitFramebufferFlags & GrGLCaps::kNoMSAADst_BlitFramebufferFlag);
    }

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

    /** Supports using GrGLsync. */
    bool fenceSyncSupport() const { return fFenceSyncSupport; }

    /// How is GrGLsync implemented?
    FenceType fenceType() const { return fFenceType; }

    /// How are multi draws implemented (if at all)?
    MultiDrawType multiDrawType() const { return fMultiDrawType; }

    /// How is restricting sampled miplevels in onRegenerateMipmapLevels implemented?
    RegenerateMipmapType regenerateMipmapType() const { return fRegenerateMipmapType; }

    /// The maximum number of fragment uniform vectors (GLES has min. 16).
    int maxFragmentUniformVectors() const { return fMaxFragmentUniformVectors; }

    /// Is there support for GL_PACK_REVERSE_ROW_ORDER
    bool packFlipYSupport() const { return fPackFlipYSupport; }

    /// Is there support for texture parameter GL_TEXTURE_USAGE
    bool textureUsageSupport() const { return fTextureUsageSupport; }

    /// Is GL_ARB_IMAGING supported
    bool imagingSupport() const { return fImagingSupport; }

    /// Is there support for Vertex Array Objects?
    bool vertexArrayObjectSupport() const { return fVertexArrayObjectSupport; }

    /// Is there support for GL_KHR_debug?
    bool debugSupport() const { return fDebugSupport; }

    /// Is there support for ES2 compatability?
    bool ES2CompatibilitySupport() const { return fES2CompatibilitySupport; }

    /// Is there support for glDrawRangeElements?
    bool drawRangeElementsSupport() const { return fDrawRangeElementsSupport; }

    /// Are the glDraw*Base(VertexBase)Instance methods, and baseInstance fields in indirect draw
    //commands supported?
    bool baseVertexBaseInstanceSupport() const { return fBaseVertexBaseInstanceSupport; }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    bool isCoreProfile() const { return fIsCoreProfile; }

    bool bindFragDataLocationSupport() const { return fBindFragDataLocationSupport; }

    bool bindUniformLocationSupport() const { return fBindUniformLocationSupport; }

    /// Are textures with GL_TEXTURE_RECTANGLE type supported.
    bool rectangleTextureSupport() const { return fRectangleTextureSupport; }

    /// Can set the BASE and MAX mip map level.
    bool mipmapLevelControlSupport() const { return fMipmapLevelControlSupport; }

    /// Can set the MIN/MAX LOD value.
    bool mipmapLodControlSupport() const { return fMipmapLodControlSupport; }

    bool doManualMipmapping() const { return fDoManualMipmapping; }

    void onDumpJSON(SkJSONWriter*) const override;

    InvalidateBufferType invalidateBufferType() const { return fInvalidateBufferType; }

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

    // Older Android versions seem to have an issue with setting GL_TEXTURE_BASE_LEVEL or
    // GL_TEXTURE_MAX_LEVEL for GL_TEXTURE_EXTERNAL_OES textures.
    bool dontSetBaseOrMaxLevelForExternalTextures() const {
        return fDontSetBaseOrMaxLevelForExternalTextures;
    }

    // PowerVRGX6250 drops every pixel if we modify the sample mask while color writes are disabled.
    bool neverDisableColorWrites() const { return fNeverDisableColorWrites; }

    // Texture parameters must be used to enable MIP mapping even when a sampler object is used.
    bool mustSetAnyTexParameterToEnableMipmapping() const {
        return fMustSetAnyTexParameterToEnableMipmapping;
    }

    // Whether we must reset the blend function to not reference src2 when disabling blending after
    // previously referencing src2.
    bool mustResetBlendFuncBetweenDualSourceAndDisable() const {
        return fMustResetBlendFuncBetweenDualSourceAndDisable;
    }

    // Before changing the sample count of a texture bound to an FBO with
    // glFramebufferTexture2DMultisample() temporarily bind texture 0 to avoid corruption int the
    // texture contents.
    bool bindTexture0WhenChangingTextureFBOMultisampleCount() const {
        return fBindTexture0WhenChangingTextureFBOMultisampleCount;
    }

    // After using glCheckFramebufferStatus() bind 0 to the color attachment and then rebind the
    // original color attachment.
    bool rebindColorAttachmentAfterCheckFramebufferStatus() const {
        return fRebindColorAttachmentAfterCheckFramebufferStatus;
    }

    // During writePixels, call glFlush() before issuing glTexSubImage2D().
    bool flushBeforeWritePixels() const {
        return fFlushBeforeWritePixels;
    }

    // Returns the observed maximum number of instances the driver can handle in a single draw call
    // without crashing, or 'pendingInstanceCount' if this workaround is not necessary.
    // NOTE: the return value may be larger than pendingInstanceCount.
    int maxInstancesPerDrawWithoutCrashing(int pendingInstanceCount) const {
        return (fMaxInstancesPerDrawWithoutCrashing)
                ? fMaxInstancesPerDrawWithoutCrashing : pendingInstanceCount;
    }

    bool canCopyTexSubImage(GrGLFormat dstFormat, bool dstHasMSAARenderBuffer,
                            const GrTextureType* dstTypeIfTexture,
                            GrGLFormat srcFormat, bool srcHasMSAARenderBuffer,
                            const GrTextureType* srcTypeIfTexture) const;
    bool canCopyAsBlit(GrGLFormat dstFormat, int dstSampleCnt,
                       const GrTextureType* dstTypeIfTexture,
                       GrGLFormat srcFormat, int srcSampleCnt,
                       const GrTextureType* srcTypeIfTexture,
                       const SkRect& srcBounds, bool srcBoundsExact,
                       const SkIRect& srcRect, const SkIRect& dstRect) const;
    bool canCopyAsDraw(GrGLFormat dstFormat, bool srcIsTexturable, bool scalingCopy) const;

    DstCopyRestrictions getDstCopyRestrictions(const GrRenderTargetProxy* src,
                                               GrColorType) const override;

    bool programBinarySupport() const { return fProgramBinarySupport; }
    bool programParameterSupport() const { return fProgramParameterSupport; }
    bool programBinaryFormatIsValid(GrGLenum binaryFormat) const;

    /** Are sampler objects available in this GL? */
    bool samplerObjectSupport() const { return fSamplerObjectSupport; }

    /**
     * Are we using sampler objects in favor of texture parameters? (This will only be true if
     * samplerObjectSupport()).
     */
    bool useSamplerObjects() const { return fUseSamplerObjects; }

    bool textureSwizzleSupport() const { return fTextureSwizzleSupport; }

    bool tiledRenderingSupport() const { return fTiledRenderingSupport; }

    bool fbFetchRequiresEnablePerSample() const { return fFBFetchRequiresEnablePerSample; }

    /* Is there support for enabling/disabling sRGB writes for sRGB-capable color buffers? */
    bool srgbWriteControl() const { return fSRGBWriteControl; }

    /**
     * Skip checks for GL errors and framebuffer completeness. Note that this does not skip
     *  checking shader compilation and program linking status.
     */
    bool skipErrorChecks() const { return fSkipErrorChecks; }

    bool clientCanDisableMultisample() const { return fClientCanDisableMultisample; }

    GrBackendFormat getBackendFormatFromCompressionType(SkTextureCompressionType) const override;

    skgpu::Swizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;

#if defined(GR_TEST_UTILS)
    GrGLStandard standard() const { return fStandard; }

    std::vector<GrTest::TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    enum ExternalFormatUsage {
        kTexImage_ExternalFormatUsage,
        kReadPixels_ExternalFormatUsage,
    };
    void getExternalFormat(GrGLFormat surfaceFormat, GrColorType surfaceColorType,
                           GrColorType memoryColorType, ExternalFormatUsage usage,
                           GrGLenum* externalFormat, GrGLenum* externalType) const;

    void init(const GrContextOptions&, const GrGLContextInfo&, const GrGLInterface*);
    void initGLSL(const GrGLContextInfo&, const GrGLInterface*);

    struct FormatWorkarounds {
        bool fDisableSRGBRenderWithMSAAForMacAMD = false;
        bool fDisableRGBA16FTexStorageForCrBug1008003 = false;
        bool fDisableBGRATextureStorageForIntelWindowsES = false;
        bool fDisableLuminance16F = false;
        bool fDisableTexStorage = false;
        bool fDisallowDirectRG8ReadPixels = false;
        bool fDisallowBGRA8ReadPixels = false;
        bool fDisallowR8ForPowerVRSGX54x = false;
        bool fDisallowUnorm16Transfers = false;
        bool fDisallowTextureUnorm16 = false;
        bool fDisallowETC2Compression = false;
    };

    void applyDriverCorrectnessWorkarounds(const GrGLContextInfo&, const GrContextOptions&,
                                           const GrGLInterface*,
                                           GrShaderCaps*, FormatWorkarounds*);

    void onApplyOptionsOverrides(const GrContextOptions& options) override;

    bool onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget&) const override;

    void initFSAASupport(const GrContextOptions& contextOptions, const GrGLContextInfo&,
                         const GrGLInterface*);
    void initBlendEqationSupport(const GrGLContextInfo&);
    void initStencilSupport(const GrGLContextInfo&);
    // This must be called after initFSAASupport().
    void initFormatTable(const GrGLContextInfo&, const GrGLInterface*, const FormatWorkarounds&);
    void setupSampleCounts(const GrGLContextInfo&, const GrGLInterface*);
    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const SkIRect& dstRect,
                          const GrSurfaceProxy* src, const SkIRect& srcRect) const override;
    GrBackendFormat onGetDefaultBackendFormat(GrColorType) const override;
    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

    skgpu::Swizzle onGetReadSwizzle(const GrBackendFormat&, GrColorType) const override;

    GrDstSampleFlags onGetDstSampleFlagsForProxy(const GrRenderTargetProxy*) const override;

    bool onSupportsDynamicMSAA(const GrRenderTargetProxy*) const override;

    GrGLStandard fStandard = kNone_GrGLStandard;

    skia_private::TArray<GrGLFormat, true> fStencilFormats;
    skia_private::TArray<GrGLenum, true> fProgramBinaryFormats;

    int fMaxFragmentUniformVectors = 0;
    float fMaxTextureMaxAnisotropy = 1.f;

    MSFBOType            fMSFBOType            = kNone_MSFBOType;
    InvalidateFBType     fInvalidateFBType     = kNone_InvalidateFBType;
    InvalidateBufferType fInvalidateBufferType = InvalidateBufferType::kNone;
    MapBufferType        fMapBufferType        = kNone_MapBufferType;
    TransferBufferType   fTransferBufferType   = TransferBufferType::kNone;
    FenceType            fFenceType            = FenceType::kNone;
    MultiDrawType        fMultiDrawType        = MultiDrawType::kNone;
    RegenerateMipmapType fRegenerateMipmapType = RegenerateMipmapType::kBaseLevel;

    bool fPackFlipYSupport : 1;
    bool fTextureUsageSupport : 1;
    bool fImagingSupport  : 1;
    bool fVertexArrayObjectSupport : 1;
    bool fDebugSupport : 1;
    bool fES2CompatibilitySupport : 1;
    bool fDrawRangeElementsSupport : 1;
    bool fBaseVertexBaseInstanceSupport : 1;
    bool fIsCoreProfile : 1;
    bool fBindFragDataLocationSupport : 1;
    bool fBindUniformLocationSupport : 1;
    bool fRectangleTextureSupport : 1;
    bool fMipmapLevelControlSupport : 1;
    bool fMipmapLodControlSupport : 1;
    bool fClearTextureSupport : 1;
    bool fProgramBinarySupport : 1;
    bool fProgramParameterSupport : 1;
    bool fSamplerObjectSupport : 1;
    bool fUseSamplerObjects : 1;
    bool fTextureSwizzleSupport : 1;
    bool fTiledRenderingSupport : 1;
    bool fFenceSyncSupport : 1;
    bool fFBFetchRequiresEnablePerSample : 1;
    bool fSRGBWriteControl : 1;
    bool fSkipErrorChecks : 1;
    bool fClientCanDisableMultisample : 1;

    // Driver workarounds
    bool fDoManualMipmapping : 1;
    bool fClearToBoundaryValuesIsBroken : 1;
    bool fDrawArraysBaseVertexIsBroken : 1;
    bool fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO : 1;
    bool fUseDrawInsteadOfAllRenderTargetWrites : 1;
    bool fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines : 1;
    bool fDontSetBaseOrMaxLevelForExternalTextures : 1;
    bool fNeverDisableColorWrites : 1;
    bool fMustSetAnyTexParameterToEnableMipmapping : 1;
    bool fAllowBGRA8CopyTexSubImage : 1;
    bool fAllowSRGBCopyTexSubImage : 1;
    bool fDisallowDynamicMSAA : 1;
    bool fMustResetBlendFuncBetweenDualSourceAndDisable : 1;
    bool fBindTexture0WhenChangingTextureFBOMultisampleCount : 1;
    bool fRebindColorAttachmentAfterCheckFramebufferStatus : 1;
    bool fFlushBeforeWritePixels : 1;
    bool fDisableScalingCopyAsDraws : 1;
    bool fPadRG88TransferAlignment : 1;
    int fMaxInstancesPerDrawWithoutCrashing = 0;

    uint32_t fBlitFramebufferFlags = kNoSupport_BlitFramebufferFlag;

    struct ReadPixelsFormat {
        ReadPixelsFormat() : fFormat(0), fType(0) {}
        GrGLenum fFormat;
        GrGLenum fType;
    };

    /** Number type of the components (with out considering number of bits.) */
    enum class FormatType {
        kUnknown,
        kNormalizedFixedPoint,
        kFloat,
    };

    // ColorTypeInfo for a specific format
    struct ColorTypeInfo {
        GrColorType fColorType = GrColorType::kUnknown;
        enum {
            kUploadData_Flag = 0x1,
            // Does Ganesh itself support rendering to this colorType & format pair. Renderability
            // still additionally depends on if the format can be an FBO color attachment.
            kRenderable_Flag = 0x2,
        };
        uint32_t fFlags = 0;

        skgpu::Swizzle fReadSwizzle;
        skgpu::Swizzle fWriteSwizzle;

        struct ExternalIOFormats {
            GrColorType fColorType = GrColorType::kUnknown;

            /** The external format and type are to be used when uploading/downloading data using
                data of fColorType and uploading to a texture of a given GrGLFormat and its
                intended GrColorType. The fExternalTexImageFormat is the format to use for TexImage
                calls. The fExternalReadFormat is used when calling ReadPixels. If either is zero
                that signals that either TexImage or ReadPixels is not supported for the combination
                of format and color types. */
            GrGLenum fExternalType = 0;
            GrGLenum fExternalTexImageFormat = 0;
            GrGLenum fExternalReadFormat = 0;
            /**
             * Must check whether GL_IMPLEMENTATION_COLOR_READ_FORMAT and _TYPE match
             * fExternalReadFormat and fExternalType before using with glReadPixels.
             */
            bool fRequiresImplementationReadQuery = false;
        };

        GrGLenum externalFormat(GrColorType externalColorType, ExternalFormatUsage usage,
                                bool haveQueriedImplementationReadFormat) const {
            for (int i = 0; i < fExternalIOFormatCount; ++i) {
                if (fExternalIOFormats[i].fColorType == externalColorType) {
                    if (usage == kTexImage_ExternalFormatUsage) {
                        return fExternalIOFormats[i].fExternalTexImageFormat;
                    } else {
                        SkASSERT(usage == kReadPixels_ExternalFormatUsage);
                        if (!haveQueriedImplementationReadFormat &&
                            fExternalIOFormats[i].fRequiresImplementationReadQuery) {
                            return 0;
                        }
                        return fExternalIOFormats[i].fExternalReadFormat;
                    }
                }
            }
            return 0;
        }

        GrGLenum externalType(GrColorType externalColorType) const {
            for (int i = 0; i < fExternalIOFormatCount; ++i) {
                if (fExternalIOFormats[i].fColorType == externalColorType) {
                    return fExternalIOFormats[i].fExternalType;
                }
            }
            return 0;
        }

        std::unique_ptr<ExternalIOFormats[]> fExternalIOFormats;
        int fExternalIOFormatCount = 0;
    };

    struct FormatInfo {
        uint32_t colorTypeFlags(GrColorType colorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == colorType) {
                    return fColorTypeInfos[i].fFlags;
                }
            }
            return 0;
        }

        GrGLenum externalFormat(GrColorType surfaceColorType, GrColorType externalColorType,
                                ExternalFormatUsage usage) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == surfaceColorType) {
                    return fColorTypeInfos[i].externalFormat(externalColorType, usage,
                                                             fHaveQueriedImplementationReadSupport);
                }
            }
            return 0;
        }

        GrGLenum externalType(GrColorType surfaceColorType, GrColorType externalColorType) const {
            for (int i = 0; i < fColorTypeInfoCount; ++i) {
                if (fColorTypeInfos[i].fColorType == surfaceColorType) {
                    return fColorTypeInfos[i].externalType(externalColorType);
                }
            }
            return 0;
        }

        enum {
            kTexturable_Flag                 = 0x01,
            /** kFBOColorAttachment means that even if the format cannot be a GrRenderTarget, we can
                still attach it to a FBO for blitting or reading pixels. */
            kFBOColorAttachment_Flag         = 0x02,
            kFBOColorAttachmentWithMSAA_Flag = 0x04,
            kUseTexStorage_Flag              = 0x08,
            /**
             * Are pixel buffer objects supported in/out of this format? Ignored if PBOs are not
             * supported at all.
             */
            kTransfers_Flag                  = 0x10,
        };
        uint32_t fFlags = 0;

        FormatType fFormatType = FormatType::kUnknown;

        // Not defined for uncompressed formats. Passed to glCompressedTexImage...
        GrGLenum fCompressedInternalFormat = 0;

        // Value to uses as the "internalformat" argument to glTexImage or glTexStorage. It is
        // initialized in coordination with the presence/absence of the kUseTexStorage flag. In
        // other words, it is only guaranteed to be compatible with glTexImage if the flag is not
        // set and or with glTexStorage if the flag is set.
        GrGLenum fInternalFormatForTexImageOrStorage = 0;

        // Value to uses as the "internalformat" argument to glRenderbufferStorageMultisample...
        GrGLenum fInternalFormatForRenderbuffer = 0;

        // Default values to use along with fInternalFormatForTexImageOrStorage for function
        // glTexImage2D when not input providing data (passing nullptr) or when clearing it by
        // uploading a block of solid color data. Not defined for compressed formats.
        GrGLenum fDefaultExternalFormat = 0;
        GrGLenum fDefaultExternalType = 0;
        // When the above two values are used to initialize a texture by uploading cleared data to
        // it the data should be of this color type.
        GrColorType fDefaultColorType = GrColorType::kUnknown;

        bool fHaveQueriedImplementationReadSupport = false;

        enum {
            // This indicates that a stencil format has not yet been determined for the config.
            kUnknown_StencilIndex = -1,
            // This indicates that there is no supported stencil format for the config.
            kUnsupported_StencilFormatIndex = -2
        };

        // Index fStencilFormats.
        int fStencilFormatIndex = kUnknown_StencilIndex;

        SkTDArray<int> fColorSampleCounts;

        std::unique_ptr<ColorTypeInfo[]> fColorTypeInfos;
        int fColorTypeInfoCount = 0;
    };

    FormatInfo fFormatTable[kGrGLColorFormatCount];

    FormatInfo& getFormatInfo(GrGLFormat format) { return fFormatTable[static_cast<int>(format)]; }
    const FormatInfo& getFormatInfo(GrGLFormat format) const {
        return fFormatTable[static_cast<int>(format)];
    }

    GrGLFormat fColorTypeToFormatTable[kGrColorTypeCnt];
    void setColorTypeFormat(GrColorType, GrGLFormat);

    using INHERITED = GrCaps;
};

#endif
