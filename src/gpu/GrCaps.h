
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrCaps_DEFINED
#define GrCaps_DEFINED

#include "../private/GrTypesPriv.h"
#include "GrBlend.h"
#include "GrDriverBugWorkarounds.h"
#include "GrShaderCaps.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkString.h"

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
    bool oversizedStencilSupport() const { return fOversizedStencilSupport; }
    bool textureBarrierSupport() const { return fTextureBarrierSupport; }
    bool sampleLocationsSupport() const { return fSampleLocationsSupport; }
    bool multisampleDisableSupport() const { return fMultisampleDisableSupport; }
    bool instanceAttribSupport() const { return fInstanceAttribSupport; }
    bool usesMixedSamples() const { return fUsesMixedSamples; }

    // Primitive restart functionality is core in ES 3.0, but using it will cause slowdowns on some
    // systems. This cap is only set if primitive restart will improve performance.
    bool usePrimitiveRestart() const { return fUsePrimitiveRestart; }

    bool preferClientSideDynamicBuffers() const { return fPreferClientSideDynamicBuffers; }

    // On tilers, an initial fullscreen clear is an OPTIMIZATION. It allows the hardware to
    // initialize each tile with a constant value rather than loading each pixel from memory.
    bool preferFullscreenClears() const { return fPreferFullscreenClears; }

    bool preferVRAMUseOverFlushes() const { return fPreferVRAMUseOverFlushes; }

    bool blacklistCoverageCounting() const { return fBlacklistCoverageCounting; }

    bool avoidStencilBuffers() const { return fAvoidStencilBuffers; }

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

    bool canUseAdvancedBlendEquation(GrBlendEquation equation) const {
        SkASSERT(GrBlendEquationIsAdvanced(equation));
        return SkToBool(fAdvBlendEqBlacklist & (1 << equation));
    }

    /**
     * Indicates whether GPU->CPU memory mapping for GPU resources such as vertex buffers and
     * textures allows partial mappings or full mappings.
     */
    enum MapFlags {
        kNone_MapFlags   = 0x0,       //<! Cannot map the resource.

        kCanMap_MapFlag  = 0x1,       //<! The resource can be mapped. Must be set for any of
                                      //   the other flags to have meaning.
        kSubset_MapFlag  = 0x2,       //<! The resource can be partially mapped.
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

    int maxRasterSamples() const { return fMaxRasterSamples; }

    int maxWindowRectangles() const { return fMaxWindowRectangles; }

    // Returns whether mixed samples is supported for the given backend render target.
    bool isWindowRectanglesSupportedForRT(const GrBackendRenderTarget& rt) const {
        return this->maxWindowRectangles() > 0 && this->onIsWindowRectanglesSupportedForRT(rt);
    }

    // A tuned, platform-specific value for the maximum number of analytic fragment processors we
    // should use to implement a clip, before falling back on a mask.
    int maxClipAnalyticFPs() const { return fMaxClipAnalyticFPs; }

    virtual bool isConfigTexturable(GrPixelConfig) const = 0;

    // Returns whether a texture of the given config can be copied to a texture of the same config.
    virtual bool isConfigCopyable(GrPixelConfig) const = 0;

    // Returns the maximum supported sample count for a config. 0 means the config is not renderable
    // 1 means the config is renderable but doesn't support MSAA.
    virtual int maxRenderTargetSampleCount(GrPixelConfig) const = 0;

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
    virtual bool surfaceSupportsWritePixels(const GrSurface*) const = 0;

    /**
     * Backends may have restrictions on what types of surfaces support GrGpu::readPixels().
     * If this returns false then the caller should implement a fallback where a temporary texture
     * is created, the surface is drawn or copied into the temporary, and pixels are read from the
     * temporary.
     */
    virtual bool surfaceSupportsReadPixels(const GrSurface*) const = 0;

    /**
     * Given a dst pixel config and a src color type what color type must the caller coax the
     * the data into in order to use GrGpu::writePixels().
     */
    virtual GrColorType supportedWritePixelsColorType(GrPixelConfig config,
                                                      GrColorType /*srcColorType*/) const {
        return GrPixelConfigToColorType(config);
    }

    /**
     * Given a src pixel config and a dst color type what color type must the caller read to using
     * GrGpu::readPixels() and then coax into dstColorType.
     */
    virtual GrColorType supportedReadPixelsColorType(GrPixelConfig config,
                                                     GrColorType /*dstColorType*/) const {
        return GrPixelConfigToColorType(config);
    }

    bool suppressPrints() const { return fSuppressPrints; }

    size_t bufferMapThreshold() const {
        SkASSERT(fBufferMapThreshold >= 0);
        return fBufferMapThreshold;
    }

    /** True in environments that will issue errors if memory uploaded to buffers
        is not initialized (even if not read by draw calls). */
    bool mustClearUploadedBufferData() const { return fMustClearUploadedBufferData; }

    bool wireframeMode() const { return fWireframeMode; }

    bool sampleShadingSupport() const { return fSampleShadingSupport; }

    bool fenceSyncSupport() const { return fFenceSyncSupport; }
    bool crossContextTextureSupport() const { return fCrossContextTextureSupport; }

    /**
     * Returns whether or not we will be able to do a copy given the passed in params
     */
    virtual bool canCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) const = 0;

    /**
     * This is can be called before allocating a texture to be a dst for copySurface. This is only
     * used for doing dst copies needed in blends, thus the src is always a GrRenderTargetProxy. It
     * will populate config and flags fields of the desc such that copySurface can efficiently
     * succeed as well as the proxy origin. rectsMustMatch will be set to true if the copy operation
     * must ensure that the src and dest rects are identical. disallowSubrect will be set to true if
     * copy rect must equal src's bounds.
     */
    virtual bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                                    GrSurfaceOrigin* origin, bool* rectsMustMatch,
                                    bool* disallowSubrect) const = 0;

    bool validateSurfaceDesc(const GrSurfaceDesc&, GrMipMapped) const;

    /**
     * Returns true if the GrBackendTexture can be used with the supplied SkColorType. If it is
     * compatible, the passed in GrPixelConfig will be set to a config that matches the backend
     * format and requested SkColorType.
     */
    virtual bool validateBackendTexture(const GrBackendTexture& tex, SkColorType ct,
                                        GrPixelConfig*) const = 0;
    virtual bool validateBackendRenderTarget(const GrBackendRenderTarget&, SkColorType,
                                             GrPixelConfig*) const = 0;

    // TODO: replace validateBackendTexture and validateBackendRenderTarget with calls to
    // getConfigFromBackendFormat?
    // TODO: it seems like we could pass the full SkImageInfo and validate its colorSpace too
    virtual bool getConfigFromBackendFormat(const GrBackendFormat& format, SkColorType ct,
                                            GrPixelConfig*) const = 0;

#ifdef GR_TEST_UTILS
    /**
     * Creates a GrBackendFormat which matches the backend texture. If the backend texture is
     * invalid, the function will return the default GrBackendFormat.
     */
    GrBackendFormat createFormatFromBackendTexture(const GrBackendTexture&) const;
#endif

    const GrDriverBugWorkarounds& workarounds() const { return fDriverBugWorkarounds; }

protected:
    /** Subclasses must call this at the end of their constructors in order to apply caps
        overrides requested by the client. Note that overrides will only reduce the caps never
        expand them. */
    void applyOptionsOverrides(const GrContextOptions& options);

#ifdef GR_TEST_UTILS
    /**
     * Subclasses implement this to actually create a GrBackendFormat to match backend texture. At
     * this point, the backend texture has already been validated.
     */
    virtual GrBackendFormat onCreateFormatFromBackendTexture(const GrBackendTexture&) const = 0;
#endif

    sk_sp<GrShaderCaps> fShaderCaps;

    bool fNPOTTextureTileSupport                     : 1;
    bool fMipMapSupport                              : 1;
    bool fSRGBSupport                                : 1;
    bool fSRGBWriteControl                           : 1;
    bool fDiscardRenderTargetSupport                 : 1;
    bool fReuseScratchTextures                       : 1;
    bool fReuseScratchBuffers                        : 1;
    bool fGpuTracingSupport                          : 1;
    bool fOversizedStencilSupport                    : 1;
    bool fTextureBarrierSupport                      : 1;
    bool fSampleLocationsSupport                     : 1;
    bool fMultisampleDisableSupport                  : 1;
    bool fInstanceAttribSupport                      : 1;
    bool fUsesMixedSamples                           : 1;
    bool fUsePrimitiveRestart                        : 1;
    bool fPreferClientSideDynamicBuffers             : 1;
    bool fPreferFullscreenClears                     : 1;
    bool fMustClearUploadedBufferData                : 1;

    // Driver workaround
    bool fBlacklistCoverageCounting                  : 1;
    bool fAvoidStencilBuffers                        : 1;

    // ANGLE performance workaround
    bool fPreferVRAMUseOverFlushes                   : 1;

    bool fSampleShadingSupport                       : 1;
    // TODO: this may need to be an enum to support different fence types
    bool fFenceSyncSupport                           : 1;

    // Vulkan doesn't support this (yet) and some drivers have issues, too
    bool fCrossContextTextureSupport                 : 1;

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
    int fMaxRasterSamples;
    int fMaxWindowRectangles;
    int fMaxClipAnalyticFPs;

    GrDriverBugWorkarounds fDriverBugWorkarounds;

private:
    virtual void onApplyOptionsOverrides(const GrContextOptions&) {}
    virtual void onDumpJSON(SkJSONWriter*) const {}

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
