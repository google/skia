
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
#include "include/gpu/GrBlend.h"
#include "include/gpu/GrDriverBugWorkarounds.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrShaderCaps.h"

class GrBackendFormat;
class GrBackendRenderTarget;
class GrBackendTexture;
struct GrContextOptions;
class GrRenderTargetProxy;
class GrSurface;
class GrSurfaceProxy;
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
    bool discardRenderTargetSupport() const { return fDiscardRenderTargetSupport; }
    bool gpuTracingSupport() const { return fGpuTracingSupport; }
    bool compressedTexSubImageSupport() const { return fCompressedTexSubImageSupport; }
    bool oversizedStencilSupport() const { return fOversizedStencilSupport; }
    bool textureBarrierSupport() const { return fTextureBarrierSupport; }
    bool sampleLocationsSupport() const { return fSampleLocationsSupport; }
    bool multisampleDisableSupport() const { return fMultisampleDisableSupport; }
    bool instanceAttribSupport() const { return fInstanceAttribSupport; }
    bool mixedSamplesSupport() const { return fMixedSamplesSupport; }
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
        kAsyncRead_MapFlag  = 0x4,   //<! Are maps for reading asynchronous WRT GrGpuCommandBuffers
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

    virtual bool isFormatTexturable(SkColorType, const GrBackendFormat&) const = 0;
    virtual bool isConfigTexturable(GrPixelConfig) const = 0;

    // Returns whether a texture of the given config can be copied to a texture of the same config.
    virtual bool isFormatCopyable(SkColorType, const GrBackendFormat&) const = 0;
    virtual bool isConfigCopyable(GrPixelConfig) const = 0;

    // Returns the maximum supported sample count for a config. 0 means the config is not renderable
    // 1 means the config is renderable but doesn't support MSAA.
    virtual int maxRenderTargetSampleCount(SkColorType, const GrBackendFormat&) const = 0;
    virtual int maxRenderTargetSampleCount(GrPixelConfig) const = 0;

    // Returns the number of samples to use when performing internal draws to the given config with
    // MSAA or mixed samples. If 0, Ganesh should not attempt to use internal multisampling.
    int internalMultisampleCount(GrPixelConfig config) const {
        return SkTMin(fInternalMultisampleCount, this->maxRenderTargetSampleCount(config));
    }

    bool isConfigRenderable(GrPixelConfig config) const {
        return this->maxRenderTargetSampleCount(config) > 0;
    }

    // TODO: Remove this after Flutter updated to no longer use it.
    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const {
        return this->maxRenderTargetSampleCount(config) > (withMSAA ? 1 : 0);
    }

    // Find a sample count greater than or equal to the requested count which is supported for a
    // color buffer of the given config or 0 if no such sample count is supported. If the requested
    // sample count is 1 then 1 will be returned if non-MSAA rendering is supported, otherwise 0.
    // For historical reasons requestedCount==0 is handled identically to requestedCount==1.
    virtual int getRenderTargetSampleCount(int requestedCount,
                                           SkColorType, const GrBackendFormat&) const = 0;
    virtual int getRenderTargetSampleCount(int requestedCount, GrPixelConfig) const = 0;
    // TODO: Remove. Legacy name used by Chrome.
    int getSampleCount(int requestedCount, GrPixelConfig config) const {
        return this->getRenderTargetSampleCount(requestedCount, config);
    }

    /**
     * Backends may have restrictions on what types of surfaces support GrGpu::writePixels().
     * If this returns false then the caller should implement a fallback where a temporary texture
     * is created, pixels are written to it, and then that is copied or drawn into the the surface.
     */
    bool surfaceSupportsWritePixels(const GrSurface*) const;


    /**
     * Indicates whether surface supports readPixels or the alternatives.
     */
    enum ReadFlags {
        kSupported_ReadFlag     = 0x0,
        kRequiresCopy_ReadFlag  = 0x1,
        kProtected_ReadFlag     = 0x2,
    };

    /**
     * Backends may have restrictions on what types of surfaces support GrGpu::readPixels().
     * If this returns kRequiresCopy_ReadFlag then the caller should implement a fallback where a
     * temporary texture is created, the surface is drawn or copied into the temporary, and
     * pixels are read from the temporary. If this returns kProtected_ReadFlag, then the caller
     * should not attempt reading it.
     */
    virtual ReadFlags surfaceSupportsReadPixels(const GrSurface*) const = 0;

    /**
     * Given a dst pixel config and a src color type what color type must the caller coax the
     * the data into in order to use GrGpu::writePixels().
     */
    virtual GrColorType supportedWritePixelsColorType(GrPixelConfig config,
                                                      GrColorType srcColorType) const {
        return GrPixelConfigToColorType(config);
    }

    struct SupportedRead {
        GrSwizzle fSwizzle;
        GrColorType fColorType;
    };

    /**
     * Given a src surface's pixel config and its backend format as well as a color type the caller
     * would like read into, this provides a legal color type that the caller may pass to
     * GrGpu::readPixels(). The returned color type may differ from the passed dstColorType, in
     * which case the caller must convert the read pixel data (see GrConvertPixels). When converting
     * to dstColorType the swizzle in the returned struct should be applied. The caller must check
     * the returned color type for kUnknown.
     */
    virtual SupportedRead supportedReadPixelsColorType(GrPixelConfig srcConfig,
                                                       const GrBackendFormat& srcFormat,
                                                       GrColorType dstColorType) const;

    /** Are transfer buffers (to textures and from surfaces) supported? */
    bool transferBufferSupport() const { return fTransferBufferSupport; }

    /**
     * Gets the alignment requirement for the buffer offset used with GrGpu::transferPixelsFrom for
     * a given GrColorType. To check whether a pixels as GrColorType can be read for a given surface
     * see supportedReadPixelsColorType() and surfaceSupportsReadPixels().
     *
     * @param bufferColorType The color type of the pixel data that will be stored in the transfer
     *                        buffer.
     * @return minimum required alignment for the buffer offset or zero if reading to the color type
     *         is not supported.
     */
    size_t transferFromOffsetAlignment(GrColorType bufferColorType) const;

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

    /**
     * This is can be called before allocating a texture to be a dst for copySurface. This is only
     * used for doing dst copies needed in blends, thus the src is always a GrRenderTargetProxy. It
     * will populate config and flags fields of the desc such that copySurface can efficiently
     * succeed as well as the proxy origin. rectsMustMatch will be set to true if the copy operation
     * must ensure that the src and dest rects are identical. disallowSubrect will be set to true if
     * copy rect must equal src's bounds.
     */
    virtual bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                                    bool* rectsMustMatch, bool* disallowSubrect) const = 0;

    bool validateSurfaceDesc(const GrSurfaceDesc&, GrMipMapped) const;

    /**
     * If the GrBackendRenderTarget can be used with the supplied SkColorType the return will be
     * the config that matches the backend format and requested SkColorType. Otherwise, kUnknown is
     * returned.
     */
    virtual GrPixelConfig validateBackendRenderTarget(const GrBackendRenderTarget&,
                                                      SkColorType) const = 0;

    // TODO: replace validateBackendRenderTarget with calls to getConfigFromBackendFormat?
    // TODO: it seems like we could pass the full SkImageInfo and validate its colorSpace too
    // Returns kUnknown if a valid config could not be determined.
    virtual GrPixelConfig getConfigFromBackendFormat(const GrBackendFormat& format,
                                                     SkColorType ct) const = 0;

    /**
     * Special method only for YUVA images. Returns a config that matches the backend format or
     * kUnknown if a config could not be determined.
     */
    virtual GrPixelConfig getYUVAConfigFromBackendFormat(const GrBackendFormat& format) const = 0;

    /** These are used when creating a new texture internally. */
    virtual GrBackendFormat getBackendFormatFromGrColorType(GrColorType ct,
                                                            GrSRGBEncoded srgbEncoded) const = 0;
    GrBackendFormat getBackendFormatFromColorType(SkColorType ct) const;

    virtual GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const = 0;

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
    bool fDiscardRenderTargetSupport                 : 1;
    bool fReuseScratchTextures                       : 1;
    bool fReuseScratchBuffers                        : 1;
    bool fGpuTracingSupport                          : 1;
    bool fCompressedTexSubImageSupport               : 1;
    bool fOversizedStencilSupport                    : 1;
    bool fTextureBarrierSupport                      : 1;
    bool fSampleLocationsSupport                     : 1;
    bool fMultisampleDisableSupport                  : 1;
    bool fInstanceAttribSupport                      : 1;
    bool fMixedSamplesSupport                        : 1;
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

    // Driver workaround
    bool fDriverBlacklistCCPR                        : 1;
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
    virtual size_t onTransferFromOffsetAlignment(GrColorType bufferColorType) const = 0;

    // Backends should implement this if they have any extra requirements for use of window
    // rectangles for a specific GrBackendRenderTarget outside of basic support.
    virtual bool onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget&) const {
        return true;
    }

    bool fSuppressPrints : 1;
    bool fWireframeMode  : 1;

    typedef SkRefCnt INHERITED;
};

#endif
