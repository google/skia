
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
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrBlend.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSwizzle.h"

class GrBackendFormat;
class GrBackendRenderTarget;
class GrBackendTexture;
struct GrContextOptions;
class GrProgramDesc;
class GrProgramInfo;
class GrRenderTargetProxy;
class GrSurface;
class SkJSONWriter;

namespace skgpu {
class KeyBuilder;
}

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
    bool mipmapSupport() const { return fMipmapSupport; }

    bool gpuTracingSupport() const { return fGpuTracingSupport; }
    bool oversizedStencilSupport() const { return fOversizedStencilSupport; }
    bool textureBarrierSupport() const { return fTextureBarrierSupport; }
    bool sampleLocationsSupport() const { return fSampleLocationsSupport; }
    bool drawInstancedSupport() const { return fDrawInstancedSupport; }
    // Is there hardware support for indirect draws? (Ganesh always supports indirect draws as long
    // as it can polyfill them with instanced calls, but this cap tells us if they are supported
    // natively.)
    bool nativeDrawIndirectSupport() const { return fNativeDrawIndirectSupport; }
    bool useClientSideIndirectBuffers() const {
#ifdef SK_DEBUG
        if (!fNativeDrawIndirectSupport || fNativeDrawIndexedIndirectIsBroken) {
            // We might implement indirect draws with a polyfill, so the commands need to reside in
            // CPU memory.
            SkASSERT(fUseClientSideIndirectBuffers);
        }
#endif
        return fUseClientSideIndirectBuffers;
    }
    bool conservativeRasterSupport() const { return fConservativeRasterSupport; }
    bool wireframeSupport() const { return fWireframeSupport; }
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

    // Should we discard stencil values after a render pass? (Tilers get better performance if we
    // always load stencil buffers with a "clear" op, and then discard the content when finished.)
    bool discardStencilValuesAfterRenderPass() const {
        // b/160958008
        return false;
#if 0
        // This method is actually just a duplicate of preferFullscreenClears(), with a descriptive
        // name for the sake of readability.
        return this->preferFullscreenClears();
#endif
    }

    // D3D does not allow the refs or masks to differ on a two-sided stencil draw.
    bool twoSidedStencilRefsAndMasksMustMatch() const {
        return fTwoSidedStencilRefsAndMasksMustMatch;
    }

    bool preferVRAMUseOverFlushes() const { return fPreferVRAMUseOverFlushes; }

    bool avoidStencilBuffers() const { return fAvoidStencilBuffers; }

    bool avoidWritePixelsFastPath() const { return fAvoidWritePixelsFastPath; }

    // http://skbug.com/9739
    bool requiresManualFBBarrierAfterTessellatedStencilDraw() const {
        return fRequiresManualFBBarrierAfterTessellatedStencilDraw;
    }

    // glDrawElementsIndirect fails GrMeshTest on every Win10 Intel bot.
    bool nativeDrawIndexedIndirectIsBroken() const { return fNativeDrawIndexedIndirectIsBroken; }

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

    bool isAdvancedBlendEquationDisabled(GrBlendEquation equation) const {
        SkASSERT(GrBlendEquationIsAdvanced(equation));
        SkASSERT(this->advancedBlendEquationSupport());
        return SkToBool(fAdvBlendEqDisableFlags & (1 << equation));
    }

    // On some GPUs it is a performance win to disable blending instead of doing src-over with a src
    // alpha equal to 1. To disable blending we collapse src-over to src and the backends will
    // handle the disabling of blending.
    bool shouldCollapseSrcOverToSrcWhenAble() const {
        return fShouldCollapseSrcOverToSrcWhenAble;
    }

    // When abandoning the GrDirectContext do we need to sync the GPU before we start abandoning
    // resources.
    bool mustSyncGpuDuringAbandon() const {
        return fMustSyncGpuDuringAbandon;
    }

    // Shortcut for shaderCaps()->reducedShaderMode().
    bool reducedShaderMode() const { return this->shaderCaps()->reducedShaderMode(); }

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

    // This returns the general mapping support for the GPU. However, even if this returns a flag
    // that says buffers can be mapped, it does NOT mean that every buffer will be mappable. Thus
    // calls of map should still check to see if a valid pointer was returned from the map call and
    // handle fallbacks appropriately. If this does return kNone_MapFlags then all calls to map() on
    // any buffer will fail.
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

    int maxWindowRectangles() const { return fMaxWindowRectangles; }

    // Returns whether window rectangles are supported for the given backend render target.
    bool isWindowRectanglesSupportedForRT(const GrBackendRenderTarget& rt) const {
        return this->maxWindowRectangles() > 0 && this->onIsWindowRectanglesSupportedForRT(rt);
    }

    // Hardware tessellation seems to have a fixed upfront cost. If there is a somewhat small number
    // of verbs, we seem to be faster emulating tessellation with instanced draws instead.
    int minPathVerbsForHwTessellation() const { return fMinPathVerbsForHwTessellation; }
    int minStrokeVerbsForHwTessellation() const { return fMinStrokeVerbsForHwTessellation; }

    uint32_t maxPushConstantsSize() const { return fMaxPushConstantsSize; }

    size_t transferBufferAlignment() const { return fTransferBufferAlignment; }

    virtual bool isFormatSRGB(const GrBackendFormat&) const = 0;

    bool isFormatCompressed(const GrBackendFormat& format) const;

    // Can a texture be made with the GrBackendFormat and texture type, and then be bound and
    // sampled in a shader.
    virtual bool isFormatTexturable(const GrBackendFormat&, GrTextureType) const = 0;

    // Returns whether a texture of the given format can be copied to a texture of the same format.
    virtual bool isFormatCopyable(const GrBackendFormat&) const = 0;

    // Returns the maximum supported sample count for a format. 0 means the format is not renderable
    // 1 means the format is renderable but doesn't support MSAA.
    virtual int maxRenderTargetSampleCount(const GrBackendFormat&) const = 0;

    // Returns the number of samples to use when performing draws to the given config with internal
    // MSAA. If 0, Ganesh should not attempt to use internal multisampling.
    int internalMultisampleCount(const GrBackendFormat& format) const {
        return std::min(fInternalMultisampleCount, this->maxRenderTargetSampleCount(format));
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
     * Does GrGpu::writePixels() support a src buffer where the row bytes is not equal to bpp * w?
     */
    bool writePixelsRowBytesSupport() const { return fWritePixelsRowBytesSupport; }

    /**
     * Does GrGpu::transferPixelsTo() support a src buffer where the row bytes is not equal to
     * bpp * w?
     */
    bool transferPixelsToRowBytesSupport() const { return fTransferPixelsToRowBytesSupport; }

    /**
     * Does GrGpu::readPixels() support a dst buffer where the row bytes is not equal to bpp * w?
     */
    bool readPixelsRowBytesSupport() const { return fReadPixelsRowBytesSupport; }

    bool transferFromSurfaceToBufferSupport() const { return fTransferFromSurfaceToBufferSupport; }
    bool transferFromBufferToTextureSupport() const { return fTransferFromBufferToTextureSupport; }

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

    bool avoidLargeIndexBufferDraws() const { return fAvoidLargeIndexBufferDraws; }

    /// Adreno 4xx devices experience an issue when there are a large number of stencil clip bit
    /// clears. The minimal repro steps are not precisely known but drawing a rect with a stencil
    /// op instead of using glClear seems to resolve the issue.
    bool performStencilClearsAsDraws() const { return fPerformStencilClearsAsDraws; }

    // Should we disable the clip mask atlas due to a faulty driver?
    bool driverDisableMSAAClipAtlas() const { return fDriverDisableMSAAClipAtlas; }

    // Should we disable TessellationPathRenderer due to a faulty driver?
    bool disableTessellationPathRenderer() const { return fDisableTessellationPathRenderer; }

    // Returns how to sample the dst values for the passed in GrRenderTargetProxy.
    GrDstSampleFlags getDstSampleFlagsForProxy(const GrRenderTargetProxy*, bool drawUsesMSAA) const;

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

    bool validateSurfaceParams(const SkISize&, const GrBackendFormat&, GrRenderable renderable,
                               int renderTargetSampleCnt, GrMipmapped, GrTextureType) const;

    bool areColorTypeAndFormatCompatible(GrColorType grCT, const GrBackendFormat& format) const;

    /** These are used when creating a new texture internally. */
    GrBackendFormat getDefaultBackendFormat(GrColorType, GrRenderable) const;

    virtual GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const = 0;

    /**
     * The CLAMP_TO_BORDER wrap mode for texture coordinates was added to desktop GL in 1.3, and
     * GLES 3.2, but is also available in extensions. Vulkan and Metal always have support.
     */
    bool clampToBorderSupport() const { return fClampToBorderSupport; }

    /**
     * Returns the GrSwizzle to use when sampling or reading back from a texture with the passed in
     * GrBackendFormat and GrColorType.
     */
    GrSwizzle getReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const;

    /**
     * Returns the GrSwizzle to use when writing colors to a surface with the passed in
     * GrBackendFormat and GrColorType.
     */
    virtual GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const = 0;

    virtual uint64_t computeFormatKey(const GrBackendFormat&) const = 0;

    const GrDriverBugWorkarounds& workarounds() const { return fDriverBugWorkarounds; }

    /**
     * Adds fields to the key to represent the sampler that will be created for the passed
     * in parameters. Currently this extra keying is only needed when building a vulkan pipeline
     * with immutable samplers.
     */
    virtual void addExtraSamplerKey(skgpu::KeyBuilder*,
                                    GrSamplerState,
                                    const GrBackendFormat&) const {}

    enum class ProgramDescOverrideFlags {
        kNone = 0,
        // If using discardable msaa surfaces in vulkan, when we break up a render pass for an
        // inline upload, we must do a load msaa subpass for the second render pass. However, if the
        // original render pass did not have this load subpass (e.g. clear or discard load op), then
        // all the GrProgramInfos for draws that end up in the second render pass will have been
        // recorded thinking they will be in a render pass with only 1 subpass. Thus we add an
        // override flag to the makeDesc call to force the actually VkPipeline that gets created to
        // be created using a render pass with 2 subpasses. We do miss on the pre-compile with this
        // approach, but inline uploads are very rare and already slow.
        kVulkanHasResolveLoadSubpass = 0x1,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ProgramDescOverrideFlags);


    virtual GrProgramDesc makeDesc(
            GrRenderTarget*, const GrProgramInfo&,
            ProgramDescOverrideFlags overrideFlags = ProgramDescOverrideFlags::kNone) const = 0;

    // This method specifies, for each backend, the extra properties of a RT when Ganesh creates one
    // internally. For example, for Vulkan, Ganesh always creates RTs that can be used as input
    // attachments.
    virtual GrInternalSurfaceFlags getExtraSurfaceFlagsForDeferredRT() const {
        return GrInternalSurfaceFlags::kNone;
    }

    bool supportsDynamicMSAA(const GrRenderTargetProxy*) const;

    virtual bool dmsaaResolveCanBeUsedAsTextureInSameRenderPass() const { return true; }

    // skbug.com/11935. Task reordering is disabled for some GPUs on GL due to driver bugs.
    bool avoidReorderingRenderTasks() const {
        return fAvoidReorderingRenderTasks;
    }

    bool avoidDithering() const {
        return fAvoidDithering;
    }

    /**
     * Checks whether the passed color type is renderable. If so, the same color type is passed
     * back along with the default format used for the color type. If not, provides an alternative
     * (perhaps lower bit depth and/or unorm instead of float) color type that is supported
     * along with it's default format or kUnknown if there no renderable fallback format.
     */
    std::tuple<GrColorType, GrBackendFormat> getFallbackColorTypeAndFormat(GrColorType,
                                                                           int sampleCount) const;

#if GR_TEST_UTILS
    struct TestFormatColorTypeCombination {
        GrColorType fColorType;
        GrBackendFormat fFormat;
    };

    virtual std::vector<TestFormatColorTypeCombination> getTestingCombinations() const = 0;
#endif

protected:
    // Subclasses must call this at the end of their init method in order to do final processing on
    // the caps (including overrides requested by the client).
    // NOTE: this method will only reduce the caps, never expand them.
    void finishInitialization(const GrContextOptions& options);

    virtual bool onSupportsDynamicMSAA(const GrRenderTargetProxy*) const { return false; }

    std::unique_ptr<GrShaderCaps> fShaderCaps;

    bool fNPOTTextureTileSupport                     : 1;
    bool fMipmapSupport                              : 1;
    bool fReuseScratchTextures                       : 1;
    bool fReuseScratchBuffers                        : 1;
    bool fGpuTracingSupport                          : 1;
    bool fOversizedStencilSupport                    : 1;
    bool fTextureBarrierSupport                      : 1;
    bool fSampleLocationsSupport                     : 1;
    bool fDrawInstancedSupport                       : 1;
    bool fNativeDrawIndirectSupport                  : 1;
    bool fUseClientSideIndirectBuffers               : 1;
    bool fConservativeRasterSupport                  : 1;
    bool fWireframeSupport                           : 1;
    bool fMSAAResolvesAutomatically                  : 1;
    bool fUsePrimitiveRestart                        : 1;
    bool fPreferClientSideDynamicBuffers             : 1;
    bool fPreferFullscreenClears                     : 1;
    bool fTwoSidedStencilRefsAndMasksMustMatch       : 1;
    bool fMustClearUploadedBufferData                : 1;
    bool fShouldInitializeTextures                   : 1;
    bool fSupportsAHardwareBufferImages              : 1;
    bool fHalfFloatVertexAttributeSupport            : 1;
    bool fClampToBorderSupport                       : 1;
    bool fPerformPartialClearsAsDraws                : 1;
    bool fPerformColorClearsAsDraws                  : 1;
    bool fAvoidLargeIndexBufferDraws                 : 1;
    bool fPerformStencilClearsAsDraws                : 1;
    bool fTransferFromBufferToTextureSupport         : 1;
    bool fTransferFromSurfaceToBufferSupport         : 1;
    bool fWritePixelsRowBytesSupport                 : 1;
    bool fTransferPixelsToRowBytesSupport            : 1;
    bool fReadPixelsRowBytesSupport                  : 1;
    bool fShouldCollapseSrcOverToSrcWhenAble         : 1;
    bool fMustSyncGpuDuringAbandon                   : 1;

    // Driver workaround
    bool fDriverDisableMSAAClipAtlas                 : 1;
    bool fDisableTessellationPathRenderer            : 1;
    bool fAvoidStencilBuffers                        : 1;
    bool fAvoidWritePixelsFastPath                   : 1;
    bool fRequiresManualFBBarrierAfterTessellatedStencilDraw : 1;
    bool fNativeDrawIndexedIndirectIsBroken          : 1;
    bool fAvoidReorderingRenderTasks                 : 1;
    bool fAvoidDithering                             : 1;

    // ANGLE performance workaround
    bool fPreferVRAMUseOverFlushes                   : 1;

    bool fFenceSyncSupport                           : 1;
    bool fSemaphoreSupport                           : 1;

    // Requires fence sync support in GL.
    bool fCrossContextTextureSupport                 : 1;

    // Not (yet) implemented in VK backend.
    bool fDynamicStateArrayGeometryProcessorTextureSupport : 1;

    BlendEquationSupport fBlendEquationSupport;
    uint32_t fAdvBlendEqDisableFlags;
    static_assert(kLast_GrBlendEquation < 32);

    uint32_t fMapBufferFlags;
    int fBufferMapThreshold;

    int fMaxRenderTargetSize;
    int fMaxPreferredRenderTargetSize;
    int fMaxVertexAttributes;
    int fMaxTextureSize;
    int fMaxWindowRectangles;
    int fInternalMultisampleCount;
    int fMinPathVerbsForHwTessellation = 25;
    int fMinStrokeVerbsForHwTessellation = 50;
    uint32_t fMaxPushConstantsSize = 0;
    size_t fTransferBufferAlignment = 1;

    GrDriverBugWorkarounds fDriverBugWorkarounds;

private:
    void applyOptionsOverrides(const GrContextOptions& options);

    virtual void onApplyOptionsOverrides(const GrContextOptions&) {}
    virtual void onDumpJSON(SkJSONWriter*) const {}
    virtual bool onSurfaceSupportsWritePixels(const GrSurface*) const = 0;
    virtual bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                  const SkIRect& srcRect, const SkIPoint& dstPoint) const = 0;
    virtual GrBackendFormat onGetDefaultBackendFormat(GrColorType) const = 0;

    // Backends should implement this if they have any extra requirements for use of window
    // rectangles for a specific GrBackendRenderTarget outside of basic support.
    virtual bool onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget&) const {
        return true;
    }

    virtual bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const = 0;

    virtual SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType,
                                                         const GrBackendFormat& srcFormat,
                                                         GrColorType dstColorType) const = 0;

    virtual GrSwizzle onGetReadSwizzle(const GrBackendFormat&, GrColorType) const = 0;

    virtual GrDstSampleFlags onGetDstSampleFlagsForProxy(const GrRenderTargetProxy*) const {
        return GrDstSampleFlags::kNone;
    }

    bool fSuppressPrints : 1;
    bool fWireframeMode  : 1;

    using INHERITED = SkRefCnt;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrCaps::ProgramDescOverrideFlags)

#endif
