
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrCaps_DEFINED
#define GrCaps_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/gpu/GrDriverBugWorkarounds.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrBlend.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxy.h"

class GrBackendFormat;
class GrBackendRenderTarget;
class GrBackendTexture;
struct GrContextOptions;
class GrRenderTargetProxy;
class GrSurface;
class SkJSONWriter;

/**
 * Represents the capabilities of a GrContext.
 */
class GrCaps : public SkRefCnt {
public:
    GrCaps(const GrContextOptions&);

    void dumpJSON(SkJSONWriter*) const;

    const GrShaderCaps* shaderCaps() const { return fShaderCaps.get(); }

    bool npotTextureTileSupport() const { return fNPOTTextureTileSupport; }
    /** To avoid as-yet-unnecessary complexity we don't allow any partial support of MIP Maps (e.g.
        only for POT textures) */
    bool mipMapSupport() const { return fMipMapSupport; }

    /**
     * Skia convention is that a device only has sRGB support if it supports sRGB formats for both
     * textures and framebuffers.
     */
    bool srgbSupport() const { return fSRGBSupport; }
    /**
     * Is there support for enabling/disabling sRGB writes for sRGB-capable color buffers?
     */
    bool srgbWriteControl() const { return fSRGBWriteControl; }
    bool gpuTracingSupport() const { return fGpuTracingSupport; }
    bool oversizedStencilSupport() const { return fOversizedStencilSupport; }
    bool textureBarrierSupport() const { return fTextureBarrierSupport; }
    bool sampleLocationsSupport() const { return fSampleLocationsSupport; }
    bool multisampleDisableSupport() const { return fMultisampleDisableSupport; }
    bool instanceAttribSupport() const { return fInstanceAttribSupport; }
    bool mixedSamplesSupport() const { return fMixedSamplesSupport; }
    // This flag indicates that we never have to resolve MSAA. In practice, it means that we have
    // an MSAA-render-to-texture extension: Any render target we create internally will use the
    // extension, and any wrapped render target is the client's responsibility.
    bool msaaResolvesAutomatically() const { return fMSAAResolvesAutomatically; }
    bool halfFloatVertexAttributeSupport() const { return fHalfFloatVertexAttributeSupport; }

    // Primitive restart functionality is core in ES 3.0, but using it will cause slowdowns on some
    // systems. This cap is only set if primitive restart will improve performance.
    bool usePrimitiveRestart() const { return fUsePrimitiveRestart; }

    bool preferClientSideDynamicBuffers() const { return fPreferClientSideDynamicBuffers; }

    // On tilers, an initial fullscreen clear is an OPTIMIZATION. It allows the hardware to
    // initialize each tile with a constant value rather than loading each pixel from memory.
    bool preferFullscreenClears() const { return fPreferFullscreenClears; }

    bool preferVRAMUseOverFlushes() const { return fPreferVRAMUseOverFlushes; }

    bool preferTrianglesOverSampleMask() const { return fPreferTrianglesOverSampleMask; }

    bool avoidStencilBuffers() const { return fAvoidStencilBuffers; }

    bool avoidWritePixelsFastPath() const { return fAvoidWritePixelsFastPath; }

    /**
     * Indicates the capabilities of the fixed function blend unit.
     */
    enum BlendEquationSupport {
        kBasic_BlendEquationSupport,             //<! Support to select the operator that
                                                 //   combines src and dst terms.
        kAdvanced_BlendEquationSupport,          //<! Additional fixed function support for specific
                                                 //   SVG/PDF blend modes. Requires blend barriers.
        kAdvancedCoherent_BlendEquationSupport,  //<! Advanced blend equation support that does not
                                                 //   require blend barriers, and permits overlap.

        kLast_BlendEquationSupport = kAdvancedCoherent_BlendEquationSupport
    };

    BlendEquationSupport blendEquationSupport() const { return fBlendEquationSupport; }

    bool advancedBlendEquationSupport() const {
        return fBlendEquationSupport >= kAdvanced_BlendEquationSupport;
    }

    bool advancedCoherentBlendEquationSupport() const {
        return kAdvancedCoherent_BlendEquationSupport == fBlendEquationSupport;
    }

    bool isAdvancedBlendEquationBlacklisted(GrBlendEquation equation) const {
        SkASSERT(GrBlendEquationIsAdvanced(equation));
        SkASSERT(this->advancedBlendEquationSupport());
        return SkToBool(fAdvBlendEqBlacklist & (1 << equation));
    }

    /**
     * Indicates whether GPU->CPU memory mapping for GPU resources such as vertex buffers and
     * textures allows partial mappings or full mappings.
     */
    enum MapFlags {
        kNone_MapFlags      = 0x0,   //<! Cannot map the resource.

        kCanMap_MapFlag     = 0x1,   //<! The resource can be mapped. Must be set for any of
                                     //   the other flags to have meaning.
        kSubset_MapFlag     = 0x2,   //<! The resource can be partially mapped.
        kAsyncRead_MapFlag  = 0x4,   //<! Are maps for reading asynchronous WRT GrOpsRenderPass
                                     //   submitted to GrGpu.
    };

    uint32_t mapBufferFlags() const { return fMapBufferFlags; }

    // Scratch textures not being reused means that those scratch textures
    // that we upload to (i.e., don't have a render target) will not be
    // recycled in the texture cache. This is to prevent ghosting by drivers
    // (in particular for deferred architectures).
    bool reuseScratchTextures() const { return fReuseScratchTextures; }
    bool reuseScratchBuffers() const { return fReuseScratchBuffers; }

    /// maximum number of attribute values per vertex
    int maxVertexAttributes() const { return fMaxVertexAttributes; }

    int maxRenderTargetSize() const { return fMaxRenderTargetSize; }

    /** This is the largest render target size that can be used without incurring extra perfomance
        cost. It is usually the max RT size, unless larger render targets are known to be slower. */
    int maxPreferredRenderTargetSize() const { return fMaxPreferredRenderTargetSize; }

    int maxTextureSize() const { return fMaxTextureSize; }

    /** This is the maximum tile size to use by GPU devices for rendering sw-backed images/bitmaps.
        It is usually the max texture size, unless we're overriding it for testing. */
    int maxTileSize() const {
        SkASSERT(fMaxTileSize <= fMaxTextureSize);
        return fMaxTileSize;
    }

    int maxWindowRectangles() const { return fMaxWindowRectangles; }

    // Returns whether mixed samples is supported for the given backend render target.
    bool isWindowRectanglesSupportedForRT(const GrBackendRenderTarget& rt) const {
        return this->maxWindowRectangles() > 0 && this->onIsWindowRectanglesSupportedForRT(rt);
    }

    virtual bool isFormatSRGB(const GrBackendFormat&) const = 0;
    virtual bool isFormatCompressed(const GrBackendFormat&) const = 0;

    // TODO: Once we use the supportWritePixels call for uploads, we can remove this function and
    // instead only have the version that takes a GrBackendFormat.
    virtual bool isFormatTexturableAndUploadable(GrColorType, const GrBackendFormat&) const = 0;
    // Can a texture be made with the GrBackendFormat, and then be bound and sampled in a shader.
    virtual bool isFormatTexturable(const GrBackendFormat&) const = 0;

    // Returns whether a texture of the given format can be copied to a texture of the same format.
    virtual bool isFormatCopyable(const GrBackendFormat&) const = 0;

    // Returns the maximum supported sample count for a format. 0 means the format is not renderable
    // 1 means the format is renderable but doesn't support MSAA. This call only refers to the
    // format itself. A caller should also confirm if the format is renderable with a given
    // GrColorType by calling isFormatRenderable.
    virtual int maxRenderTargetSampleCount(const GrBackendFormat&) const = 0;

    // Returns the number of samples to use when performing internal draws to the given config with
    // MSAA or mixed samples. If 0, Ganesh should not attempt to use internal multisampling.
    int internalMultisampleCount(const GrBackendFormat& format) const {
        return SkTMin(fInternalMultisampleCount, this->maxRenderTargetSampleCount(format));
    }

    virtual bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                               int sampleCount = 1) const = 0;

    virtual bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const = 0;

    // Find a sample count greater than or equal to the requested count which is supported for a
    // render target of the given format or 0 if no such sample count is supported. If the requested
    // sample count is 1 then 1 will be returned if non-MSAA rendering is supported, otherwise 0.
    // For historical reasons requestedCount==0 is handled identically to requestedCount==1.
    virtual int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const = 0;

    /**
     * Backends may have restrictions on what types of surfaces support GrGpu::writePixels().
     * If this returns false then the caller should implement a fallback where a temporary texture
     * is created, pixels are written to it, and then that is copied or drawn into the the surface.
     */
    bool surfaceSupportsWritePixels(const GrSurface*) const;

    /**
     * Indicates whether surface supports GrGpu::readPixels, must be copied, or cannot be read.
     */
    enum class SurfaceReadPixelsSupport {
        /** GrGpu::readPixels is supported by the surface. */
        kSupported,
        /**
         * GrGpu::readPixels is not supported by this surface but this surface can be drawn
         * or copied to a Ganesh-created GrTextureType::kTexture2D and then that surface will be
         * readable.
         */
        kCopyToTexture2D,
        /**
         * Not supported
         */
        kUnsupported,
    };
    /**
     * Backends may have restrictions on what types of surfaces support GrGpu::readPixels(). We may
     * either be able to read directly from the surface, read from a copy of the surface, or not
     * read at all.
     */
    virtual SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const = 0;

    struct SupportedWrite {
        GrColorType fColorType;
        // If the write is occurring using GrGpu::transferPixelsTo then this provides the
        // minimum alignment of the offset into the transfer buffer.
        size_t fOffsetAlignmentForTransferBuffer;
    };

    /**
     * Given a dst pixel config and a src color type what color type must the caller coax the
     * the data into in order to use GrGpu::writePixels().
     */
    virtual SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                         const GrBackendFormat& surfaceFormat,
                                                         GrColorType srcColorType) const = 0;

    struct SupportedRead {
        GrColorType fColorType;
        // If the read is occurring using GrGpu::transferPixelsFrom then this provides the
        // minimum alignment of the offset into the transfer buffer.
        size_t fOffsetAlignmentForTransferBuffer;
    };

    /**
     * Given a src surface's color type and its backend format as well as a color type the caller
     * would like read into, this provides a legal color type that the caller may pass to
     * GrGpu::readPixels(). The returned color type may differ from the passed dstColorType, in
     * which case the caller must convert the read pixel data (see GrConvertPixels). When converting
     * to dstColorType the swizzle in the returned struct should be applied. The caller must check
     * the returned color type for kUnknown.
     */
    SupportedRead supportedReadPixelsColorType(GrColorType srcColorType,
                                               const GrBackendFormat& srcFormat,
                                               GrColorType dstColorType) const;

    /**
     * Do GrGpu::writePixels() and GrGpu::transferPixelsTo() support a src buffer where the row
     * bytes is not equal to bpp * w?
     */
    bool writePixelsRowBytesSupport() const { return fWritePixelsRowBytesSupport; }
    /**
     * Does GrGpu::readPixels() support a dst buffer where the row bytes is not equal to bpp * w?
     */
    bool readPixelsRowBytesSupport() const { return fReadPixelsRowBytesSupport; }

    /** Are transfer buffers (to textures and from surfaces) supported? */
    bool transferBufferSupport() const { return fTransferBufferSupport; }

    bool suppressPrints() const { return fSuppressPrints; }

    size_t bufferMapThreshold() const {
        SkASSERT(fBufferMapThreshold >= 0);
        return fBufferMapThreshold;
    }

    /** True in environments that will issue errors if memory uploaded to buffers
        is not initialized (even if not read by draw calls). */
    bool mustClearUploadedBufferData() const { return fMustClearUploadedBufferData; }

    /** For some environments, there is a performance or safety concern to not
        initializing textures. For example, with WebGL and Firefox, there is a large
        performance hit to not doing it.
     */
    bool shouldInitializeTextures() const { return fShouldInitializeTextures; }

    /**
     * When this is true it is required that all textures are initially cleared. However, the
     * clearing must be implemented by passing level data to GrGpu::createTexture() rather than
     * be implemeted by GrGpu::createTexture().
     *
     * TODO: Make this take GrBacknedFormat when canClearTextureOnCreation() does as well.
     */
    bool createTextureMustSpecifyAllLevels() const {
        return this->shouldInitializeTextures() && !this->canClearTextureOnCreation();
    }

    /** Returns true if the given backend supports importing AHardwareBuffers via the
     * GrAHardwarebufferImageGenerator. This will only ever be supported on Android devices with API
     * level >= 26.
     * */
    bool supportsAHardwareBufferImages() const { return fSupportsAHardwareBufferImages; }

    bool wireframeMode() const { return fWireframeMode; }

    /** Supports using GrFence. */
    bool fenceSyncSupport() const { return fFenceSyncSupport; }

    /** Supports using GrSemaphore. */
    bool semaphoreSupport() const { return fSemaphoreSupport; }

    bool crossContextTextureSupport() const { return fCrossContextTextureSupport; }
    /**
     * Returns whether or not we will be able to do a copy given the passed in params
     */
    bool canCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                        const SkIRect& srcRect, const SkIPoint& dstPoint) const;

    bool dynamicStateArrayGeometryProcessorTextureSupport() const {
        return fDynamicStateArrayGeometryProcessorTextureSupport;
    }

    // Not all backends support clearing with a scissor test (e.g. Metal), this will always
    // return true if performColorClearsAsDraws() returns true.
    bool performPartialClearsAsDraws() const {
        return fPerformColorClearsAsDraws || fPerformPartialClearsAsDraws;
    }

    // Many drivers have issues with color clears.
    bool performColorClearsAsDraws() const { return fPerformColorClearsAsDraws; }

    /// Adreno 4xx devices experience an issue when there are a large number of stencil clip bit
    /// clears. The minimal repro steps are not precisely known but drawing a rect with a stencil
    /// op instead of using glClear seems to resolve the issue.
    bool performStencilClearsAsDraws() const { return fPerformStencilClearsAsDraws; }

    // Can we use coverage counting shortcuts to render paths? Coverage counting can cause artifacts
    // along shared edges if care isn't taken to ensure both contours wind in the same direction.
    bool allowCoverageCounting() const { return fAllowCoverageCounting; }

    // Should we disable the CCPR code due to a faulty driver?
    bool driverBlacklistCCPR() const { return fDriverBlacklistCCPR; }
    bool driverBlacklistMSAACCPR() const { return fDriverBlacklistMSAACCPR; }

    /**
     * This is used to try to ensure a successful copy a dst in order to perform shader-based
     * blending.
     *
     * fRectsMustMatch will be set to true if the copy operation must ensure that the src and dest
     * rects are identical.
     *
     * fMustCopyWholeSrc will be set to true if copy rect must equal src's bounds.
     *
     * Caller will detect cases when copy cannot succeed and try copy-as-draw as a fallback.
     */
    struct DstCopyRestrictions {
        GrSurfaceProxy::RectsMustMatch fRectsMustMatch = GrSurfaceProxy::RectsMustMatch::kNo;
        bool fMustCopyWholeSrc = false;
    };
    virtual DstCopyRestrictions getDstCopyRestrictions(const GrRenderTargetProxy* src,
                                                       GrColorType ct) const {
        return {};
    }

    bool validateSurfaceParams(const SkISize&, const GrBackendFormat&, GrPixelConfig,
                               GrRenderable renderable, int renderTargetSampleCnt,
                               GrMipMapped) const;

    bool areColorTypeAndFormatCompatible(GrColorType grCT,
                                         const GrBackendFormat& format) const {
        if (GrColorType::kUnknown == grCT) {
            return false;
        }

        return this->onAreColorTypeAndFormatCompatible(grCT, format);
    }

    // TODO: it seems like we could pass the full SkImageInfo and validate its colorSpace too
    // Returns kUnknown if a valid config could not be determined.
    GrPixelConfig getConfigFromBackendFormat(const GrBackendFormat& format,
                                             GrColorType grCT) const {
        if (GrColorType::kUnknown == grCT) {
            return kUnknown_GrPixelConfig;
        }

        return this->onGetConfigFromBackendFormat(format, grCT);
    }

    /**
     * Special method only for YUVA images. Returns a colortype that matches the backend format or
     * kUnknown if a colortype could not be determined.
     */
    virtual GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat&,
                                                          bool isAlphaChannel) const = 0;

    /** These are used when creating a new texture internally. */
    GrBackendFormat getDefaultBackendFormat(GrColorType, GrRenderable) const;

    virtual GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const = 0;

    /**
     * Used by implementation of shouldInitializeTextures(). Indicates whether GrGpu implements the
     * clear in GrGpu::createTexture() or if false then the caller must provide cleared MIP level
     * data or GrGpu::createTexture() will fail.
     *
     * TODO: Make this take a GrBackendFormat so that GL can make this faster for cases
     * when the format is renderable and glTexClearImage is not available. Doing this
     * is overly complicated until the GrPixelConfig/format mess is straightened out..
     */
    virtual bool canClearTextureOnCreation() const = 0;

    /**
     * The CLAMP_TO_BORDER wrap mode for texture coordinates was added to desktop GL in 1.3, and
     * GLES 3.2, but is also available in extensions. Vulkan and Metal always have support.
     */
    bool clampToBorderSupport() const { return fClampToBorderSupport; }

    /**
     * Returns the GrSwizzle to use when sampling from a texture with the passed in GrBackendFormat
     * and GrColorType.
     */
    virtual GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const = 0;

    /**
     * Returns the GrSwizzle to use when outputting to a render target with the passed in
     * GrBackendFormat and GrColorType.
     */
    virtual GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const = 0;

    const GrDriverBugWorkarounds& workarounds() const { return fDriverBugWorkarounds; }

    /**
     * Given a possibly generic GrPixelConfig and a backend format return a specific
     * GrPixelConfig.
     */
    GrPixelConfig makeConfigSpecific(GrPixelConfig config, const GrBackendFormat& format) const {
        auto ct = GrPixelConfigToColorType(config);
        auto result = this->getConfigFromBackendFormat(format, ct);
        SkASSERT(config == result || AreConfigsCompatible(config, result));
        return result;
    }

#ifdef SK_DEBUG
    // This is just a debugging entry point until we're weaned off of GrPixelConfig. It
    // should be used to verify that the pixel config from user-level code (the genericConfig)
    // is compatible with a pixel config we've computed from scratch (the specificConfig).
    static bool AreConfigsCompatible(GrPixelConfig genericConfig, GrPixelConfig specificConfig);
#endif

#if GR_TEST_UTILS
    struct TestFormatColorTypeCombination {
        GrColorType fColorType;
        GrBackendFormat fFormat;
    };

    virtual std::vector<TestFormatColorTypeCombination> getTestingCombinations() const = 0;
#endif

protected:
    /** Subclasses must call this at the end of their constructors in order to apply caps
        overrides requested by the client. Note that overrides will only reduce the caps never
        expand them. */
    void applyOptionsOverrides(const GrContextOptions& options);

    sk_sp<GrShaderCaps> fShaderCaps;

    bool fNPOTTextureTileSupport                     : 1;
    bool fMipMapSupport                              : 1;
    bool fSRGBSupport                                : 1;
    bool fSRGBWriteControl                           : 1;
    bool fReuseScratchTextures                       : 1;
    bool fReuseScratchBuffers                        : 1;
    bool fGpuTracingSupport                          : 1;
    bool fOversizedStencilSupport                    : 1;
    bool fTextureBarrierSupport                      : 1;
    bool fSampleLocationsSupport                     : 1;
    bool fMultisampleDisableSupport                  : 1;
    bool fInstanceAttribSupport                      : 1;
    bool fMixedSamplesSupport                        : 1;
    bool fMSAAResolvesAutomatically                  : 1;
    bool fUsePrimitiveRestart                        : 1;
    bool fPreferClientSideDynamicBuffers             : 1;
    bool fPreferFullscreenClears                     : 1;
    bool fMustClearUploadedBufferData                : 1;
    bool fShouldInitializeTextures                   : 1;
    bool fSupportsAHardwareBufferImages              : 1;
    bool fHalfFloatVertexAttributeSupport            : 1;
    bool fClampToBorderSupport                       : 1;
    bool fPerformPartialClearsAsDraws                : 1;
    bool fPerformColorClearsAsDraws                  : 1;
    bool fPerformStencilClearsAsDraws                : 1;
    bool fAllowCoverageCounting                      : 1;
    bool fTransferBufferSupport                      : 1;
    bool fWritePixelsRowBytesSupport                 : 1;
    bool fReadPixelsRowBytesSupport                  : 1;

    // Driver workaround
    bool fDriverBlacklistCCPR                        : 1;
    bool fDriverBlacklistMSAACCPR                    : 1;
    bool fAvoidStencilBuffers                        : 1;
    bool fAvoidWritePixelsFastPath                   : 1;

    // ANGLE performance workaround
    bool fPreferVRAMUseOverFlushes                   : 1;

    // On some platforms it's better to make more triangles than to use the sample mask (MSAA only).
    bool fPreferTrianglesOverSampleMask              : 1;

    bool fFenceSyncSupport                           : 1;
    bool fSemaphoreSupport                           : 1;

    // Requires fence sync support in GL.
    bool fCrossContextTextureSupport                 : 1;

    // Not (yet) implemented in VK backend.
    bool fDynamicStateArrayGeometryProcessorTextureSupport : 1;

    BlendEquationSupport fBlendEquationSupport;
    uint32_t fAdvBlendEqBlacklist;
    GR_STATIC_ASSERT(kLast_GrBlendEquation < 32);

    uint32_t fMapBufferFlags;
    int fBufferMapThreshold;

    int fMaxRenderTargetSize;
    int fMaxPreferredRenderTargetSize;
    int fMaxVertexAttributes;
    int fMaxTextureSize;
    int fMaxTileSize;
    int fMaxWindowRectangles;
    int fInternalMultisampleCount;

    GrDriverBugWorkarounds fDriverBugWorkarounds;

private:
    virtual void onApplyOptionsOverrides(const GrContextOptions&) {}
    virtual void onDumpJSON(SkJSONWriter*) const {}
    virtual bool onSurfaceSupportsWritePixels(const GrSurface*) const = 0;
    virtual bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                  const SkIRect& srcRect, const SkIPoint& dstPoint) const = 0;
    virtual GrBackendFormat onGetDefaultBackendFormat(GrColorType, GrRenderable) const = 0;

    // Backends should implement this if they have any extra requirements for use of window
    // rectangles for a specific GrBackendRenderTarget outside of basic support.
    virtual bool onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget&) const {
        return true;
    }

    virtual GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                       GrColorType ct) const = 0;

    virtual bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const = 0;

    virtual SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType,
                                                         const GrBackendFormat& srcFormat,
                                                         GrColorType dstColorType) const = 0;


    bool fSuppressPrints : 1;
    bool fWireframeMode  : 1;

    typedef SkRefCnt INHERITED;
};

#endif
