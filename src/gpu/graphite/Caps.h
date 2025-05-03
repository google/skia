/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Caps_DEFINED
#define skgpu_graphite_Caps_DEFINED

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAlign.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/text/gpu/SubRunControl.h"

#if defined(GPU_TEST_UTILS)
#include "src/gpu/graphite/ContextOptionsPriv.h"
#endif

enum class SkBlendMode;
enum class SkTextureCompressionType;
class SkCapabilities;
class SkStream;
class SkWStream;

namespace SkSL { struct ShaderCaps; }

namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

struct AttachmentDesc;
enum class BufferType : int;
struct ContextOptions;
class ComputePipelineDesc;
class GraphicsPipelineDesc;
class GraphiteResourceKey;
class RendererProvider;
struct RenderPassDesc;
class TextureInfo;

struct ResourceBindingRequirements {
    /* The required data layout rules for the contents of a uniform buffer. */
    Layout fUniformBufferLayout = Layout::kInvalid;

    /* The required data layout rules for the contents of a storage buffer. */
    Layout fStorageBufferLayout = Layout::kInvalid;

    /**
     * Whether combined texture-sampler types are supported. Backends that do not support combined
     * image samplers (i.e. sampler2D) require a texture and sampler object to be bound separately
     * and their binding indices explicitly specified in the shader text.
     */
    bool fSeparateTextureAndSamplerBinding = false;

    /**
     * Whether intrinsic constant information is stored as push constants (rather than normal UBO).
     * Currently only relevant or possibly true for Vulkan.
     */
    bool fUseVulkanPushConstantsForIntrinsicConstants = false;

    /**
     * Whether compute shader textures use separate index ranges from other resources (i.e. buffers)
     */
    bool fComputeUsesDistinctIdxRangesForTextures = false;

    /**
     * Define set indices. We assume that even if textures and samplers must be bound separately,
     * they will still be contained within the same set/group.
     */
    static constexpr int kUnassigned = -1;
    int fUniformsSetIdx              = kUnassigned;
    int fTextureSamplerSetIdx        = kUnassigned;
    int fInputAttachmentSetIdx       = kUnassigned;
    /* Define uniform buffer bindings */
    int fIntrinsicBufferBinding      = kUnassigned;
    int fRenderStepBufferBinding     = kUnassigned;
    int fPaintParamsBufferBinding    = kUnassigned;
    int fGradientBufferBinding       = kUnassigned;
};

class Caps {
public:
    virtual ~Caps();

    const SkSL::ShaderCaps* shaderCaps() const { return fShaderCaps.get(); }

    sk_sp<SkCapabilities> capabilities() const;

#if defined(GPU_TEST_UTILS)
    std::string_view deviceName() const { return fDeviceName; }

    PathRendererStrategy requestedPathRendererStrategy() const {
        return fRequestedPathRendererStrategy;
    }
#endif

    /**
     * TODO(b/390473370): Once backends initialize a Caps-level format table, these will not need
     * to be virtual anymore:
     */
    virtual bool isSampleCountSupported(TextureFormat, uint8_t requestedSampleCount) const = 0;
    /* Return the TextureFormat that satisfies `dsFlags`. */
    virtual TextureFormat getDepthStencilFormat(SkEnumBitMask<DepthStencilFlags>) const = 0;

    virtual TextureInfo getDefaultAttachmentTextureInfo(AttachmentDesc,
                                                        Protected,
                                                        Discardable) const = 0;

    virtual TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                     Mipmapped mipmapped,
                                                     Protected,
                                                     Renderable) const = 0;

    virtual TextureInfo getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                     Mipmapped mipmapped) const = 0;

    virtual TextureInfo getDefaultCompressedTextureInfo(SkTextureCompressionType,
                                                        Mipmapped mipmapped,
                                                        Protected) const = 0;

    virtual TextureInfo getDefaultStorageTextureInfo(SkColorType) const = 0;

    /* Get required depth attachment dimensions for a givin color attachment info and dimensions. */
    virtual SkISize getDepthAttachmentDimensions(const TextureInfo&,
                                                 const SkISize colorAttachmentDimensions) const;

    virtual UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                              const RenderPassDesc&) const = 0;
    virtual UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const = 0;


    virtual bool extractGraphicsDescs(const UniqueKey&,
                                      GraphicsPipelineDesc*,
                                      RenderPassDesc*,
                                      const RendererProvider*) const { return false; }

    bool areColorTypeAndTextureInfoCompatible(SkColorType, const TextureInfo&) const;

    bool isTexturable(const TextureInfo&) const;
    virtual bool isRenderable(const TextureInfo&) const = 0;
    virtual bool isStorage(const TextureInfo&) const = 0;

    virtual bool loadOpAffectsMSAAPipelines() const { return false; }

    int maxTextureSize() const { return fMaxTextureSize; }
    uint8_t defaultMSAASamplesCount() const { return fDefaultMSAASamples; }

    /**
     * Returns the maximum number of varyings allowed in a render pipeline. Note that this is the
     * number of varying variables, not the total number of varying scalars.
     */
    int maxVaryings() const { return fMaxVaryings; }

    virtual void buildKeyForTexture(SkISize dimensions,
                                    const TextureInfo&,
                                    ResourceType,
                                    GraphiteResourceKey*) const = 0;

    const ResourceBindingRequirements& resourceBindingRequirements() const {
        return fResourceBindingReqs;
    }

    /**
     * Returns the required alignment in bytes for the offset into a uniform buffer when binding it
     * to a draw.
     */
    size_t requiredUniformBufferAlignment() const { return fRequiredUniformBufferAlignment; }

    /**
     * Returns the required alignment in bytes for the offset into a storage buffer when binding it
     * to a draw.
     */
    size_t requiredStorageBufferAlignment() const { return fRequiredStorageBufferAlignment; }

    /**
     * Returns the required alignment in bytes for the offset and size of copies involving a buffer.
     */
    size_t requiredTransferBufferAlignment() const { return fRequiredTransferBufferAlignment; }

    /* Returns the aligned rowBytes when transfering to or from a Texture */
    size_t getAlignedTextureDataRowBytes(size_t rowBytes) const {
        return SkAlignTo(rowBytes, fTextureDataRowBytesAlignment);
    }

    /**
     * Backends can optionally override this method to return meaningful sampler conversion info.
     * By default, simply return a default ImmutableSamplerInfo (e.g. no immutable sampler).
     */
    virtual ImmutableSamplerInfo getImmutableSamplerInfo(const TextureInfo&) const {
        return {};
    }

    /**
     * Backends may have restrictions on what types of textures support Device::writePixels().
     * If this returns false then the caller should implement a fallback where a temporary texture
     * is created, pixels are written to it, and then that is copied or drawn into the the surface.
     */
    virtual bool supportsWritePixels(const TextureInfo& textureInfo) const = 0;

    /**
     * Backends may have restrictions on what types of textures support Device::readPixels().
     * If this returns false then the caller should implement a fallback where a temporary texture
     * is created, the original texture is copied or drawn into it, and then pixels read from
     * the temporary texture.
     */
    virtual bool supportsReadPixels(const TextureInfo& textureInfo) const = 0;

    /**
     * Given a dst pixel config and a src color type what color type must the caller coax the
     * the data into in order to use writePixels.
     *
     * We currently don't have an SkColorType for a 3 channel RGB format. Additionally the current
     * implementation of raster pipeline requires power of 2 channels, so it is not easy to add such
     * an SkColorType. Thus we need to check for data that is 3 channels using the isRGBFormat
     * return value and handle it manually
     */
    virtual std::pair<SkColorType, bool /*isRGB888Format*/> supportedWritePixelsColorType(
            SkColorType dstColorType,
            const TextureInfo& dstTextureInfo,
            SkColorType srcColorType) const = 0;

    /**
     * Given a src surface's color type and its texture info as well as a color type the caller
     * would like read into, this provides a legal color type that the caller can use for
     * readPixels. The returned color type may differ from the passed dstColorType, in
     * which case the caller must convert the read pixel data (see GrConvertPixels). When converting
     * to dstColorType the swizzle in the returned struct should be applied. The caller must check
     * the returned color type for kUnknown.
     *
     * We currently don't have an SkColorType for a 3 channel RGB format. Additionally the current
     * implementation of raster pipeline requires power of 2 channels, so it is not easy to add such
     * an SkColorType. Thus we need to check for data that is 3 channels using the isRGBFormat
     * return value and handle it manually
     */
    virtual std::pair<SkColorType, bool /*isRGBFormat*/> supportedReadPixelsColorType(
            SkColorType srcColorType,
            const TextureInfo& srcTextureInfo,
            SkColorType dstColorType) const = 0;

    /**
     * Checks whether the passed color type is renderable. If so, the same color type is passed
     * back. If not, provides an alternative (perhaps lower bit depth and/or unorm instead of float)
     * color type that is supported or kUnknown if there no renderable fallback format.
     */
    SkColorType getRenderableColorType(SkColorType) const;

    /**
     * Determines the orientation of the NDC coordinates emitted by the vertex stage relative to
     * both Skia's presumed top-left Y-down system and the viewport coordinates (which are also
     * always top-left, Y-down for all supported backends).)
     *
     * If true is returned, then (-1,-1) in normalized device coords maps to the top-left of the
     * configured viewport and positive Y points down. This aligns with Skia's conventions.
     * If false is returned, then (-1,-1) in NDC maps to the bottom-left of the viewport and
     * positive Y points up (so NDC is flipped relative to sk_Position and the viewport coords).
     *
     * There is no backend difference in handling the X axis so it's assumed -1 maps to the left
     * edge and +1 maps to the right edge.
     */
    bool ndcYAxisPointsDown() const { return fNDCYAxisPointsDown; }

    bool clampToBorderSupport() const { return fClampToBorderSupport; }

    bool protectedSupport() const { return fProtectedSupport; }

    /* Supports BackendSemaphores */
    bool semaphoreSupport() const { return fSemaphoreSupport; }

    /* If false then calling Context::submit with SyncToCpu::kYes is an error. */
    bool allowCpuSync() const { return fAllowCpuSync; }

    /* Returns whether storage buffers are supported and to be preferred over uniform buffers. */
    bool storageBufferSupport() const { return fStorageBufferSupport; }

    /**
     * The gradient buffer is an unsized float array so it is only optimal memory-wise to use it if
     * the storage buffer memory layout is std430 or in metal, which is also the only supported
     * way the data is packed.
     */
    bool gradientBufferSupport() const {
        return fStorageBufferSupport &&
               (fResourceBindingReqs.fStorageBufferLayout == Layout::kStd430 ||
                fResourceBindingReqs.fStorageBufferLayout == Layout::kMetal);
    }

    /* Returns whether a draw buffer can be mapped. */
    bool drawBufferCanBeMapped() const { return fDrawBufferCanBeMapped; }

#if defined(GPU_TEST_UTILS)
    bool drawBufferCanBeMappedForReadback() const { return fDrawBufferCanBeMappedForReadback; }
#endif

    /**
     * Returns whether using Buffer::asyncMap() must be used to map buffers. map() may only be
     * called after asyncMap() is called and will fail if the asynchronous map is not complete. This
     * excludes premapped buffers for which map() can be called freely until the first unmap() call.
     */
    bool bufferMapsAreAsync() const { return fBufferMapsAreAsync; }

    /* Returns whether multisampled render to single sampled is supported. */
    bool msaaRenderToSingleSampledSupport() const { return fMSAARenderToSingleSampledSupport; }

    /**
     * Returns whether a render pass can have MSAA/depth/stencil attachments and a resolve
     * attachment with mismatched sizes. Note: the MSAA attachment and the depth/stencil attachment
     * still need to match their sizes.
     * This also implies supporting partial load/resolve.
     */
    bool differentResolveAttachmentSizeSupport() const {
        return fDifferentResolveAttachmentSizeSupport;
    }

    /* Returns whether compute shaders are supported. */
    bool computeSupport() const { return fComputeSupport; }

    /**
     * Returns true if the given backend supports importing AHardwareBuffers. This will only
     * ever be supported on Android devices with API level >= 26.
     */
    bool supportsAHardwareBufferImages() const { return fSupportsAHardwareBufferImages; }

    /**
     * Enum representing the capabilities of the fixed function blend unit.
     */
    enum BlendEquationSupport : uint8_t {
        kBasic = 0,           /* Default bare minimum support. Allows selecting the operator that
                                 combines src + dst terms.*/
        kAdvancedNoncoherent, /* Additional fixed function support for specific SVG/PDF blend modes.
                                 Requires blend barriers.*/
        kAdvancedCoherent     /* Advanced blend equation support that does not require blend
                                 barriers and permits overlap.*/
    };
    /**
     * Return the level of hardware advanced blend mode support.
     */
    BlendEquationSupport blendEquationSupport() const { return fBlendEqSupport; }
    /**
     * Simple helper for indicating whether the hardware supports advanced blend modes at all
     * (coherent or noncoherent).
     */
    bool supportsHardwareAdvancedBlending() const {
        return fBlendEqSupport > BlendEquationSupport::kBasic;
    }

    /**
     * Returns the skgpu::Swizzle to use when sampling or reading back from a texture with the
     * passed in SkColorType and TextureInfo.
     */
    skgpu::Swizzle getReadSwizzle(SkColorType, const TextureInfo&) const;

    /**
     * Returns the skgpu::Swizzle to use when writing colors to a surface with the passed in
     * SkColorType and TextureInfo.
     */
    skgpu::Swizzle getWriteSwizzle(SkColorType, const TextureInfo&) const;

    /**
     * Includes the following dynamic state:
     *
     * * Line width, depth bias, depth bounds, stencil compare mask, stencil write mask and stencil
     *   reference.
     *   This set corresponds to Vulkan 1.0 dynamic state.  Blend constants does not depend on this
     *   flag as it is always dynamic with all graphite backends.
     *
     * * Depth test enable, depth write enable, depth compare op, depth bounds test enable, depth
     *   bias enable, stencil test enable and stencil op.
     *   This set corresponds to depth and stencil related state from VK_EXT_extended_dynamic_state
     *   and VK_EXT_extended_dynamic_state2.
     *
     * * Primitive topology and primitive restart enable.
     *   Note that the primitive topology _class_ is not dynamic.
     *   This set corresponds to input assembly state from VK_EXT_extended_dynamic_state and
     *   VK_EXT_extended_dynamic_state2.
     *
     * * Cull mode, front face and rasterizer discard.
     *   This set corresponds to rasterizer state from VK_EXT_extended_dynamic_state and
     *   VK_EXT_extended_dynamic_state2.
     */
    bool useBasicDynamicState() const { return fUseBasicDynamicState; }
    /**
     * Whether all vertex input state is dynamic.
     * This set corresponds to state from VK_EXT_vertex_input_dynamic_state.  This state is
     * equivalently pulled out of the shaders pipeline via VK_EXT_graphics_pipeline_library
     * (usePipelineLibraries()).
     */
    bool useVertexInputDynamicState() const { return fUseVertexInputDynamicState; }
    /**
     * Whether VK_EXT_graphics_pipeline_library should be used.  In this case, the "shaders" subset
     * of the pipeline is compiled separately, then fast-linked with the vertex input and fragment
     * output state to create the final library.  Currently, this is a detail of the Vulkan backend,
     * which helps VkPipelineCache hits (because the shaders pipeline hits the cache, and blend
     * state is patched in).  However, this is most useful once exposed to the front-end, such that
     * it can track the (fewer) shaders pipeline separately, have the complete pipelines point to
     * the shaders pipeline, avoid unnecessary cache look ups, and more.  (skbug.com/414645289)
     */
    bool usePipelineLibraries() const { return fUsePipelineLibraries; }

    skgpu::ShaderErrorHandler* shaderErrorHandler() const { return fShaderErrorHandler; }

    /**
     * Returns what method of dst read a draw should use for obtaining the dst color. Backends can
     * use the default implementation or override this method as needed.
     */
    virtual DstReadStrategy getDstReadStrategy() const;

    float minDistanceFieldFontSize() const { return fMinDistanceFieldFontSize; }
    float glyphsAsPathsFontSize() const { return fGlyphsAsPathsFontSize; }

    size_t glyphCacheTextureMaximumBytes() const { return fGlyphCacheTextureMaximumBytes; }
    int maxPathAtlasTextureSize() const { return fMaxPathAtlasTextureSize; }

    bool allowMultipleAtlasTextures() const { return fAllowMultipleAtlasTextures; }
    bool supportBilerpFromGlyphAtlas() const { return fSupportBilerpFromGlyphAtlas; }

    bool requireOrderedRecordings() const { return fRequireOrderedRecordings; }

    /**
     * When uploading to a full compressed texture do we need to pad the size out to a multiple of
     * the block width and height.
     */
    bool fullCompressedUploadSizeMustAlignToBlockDims() const {
        return fFullCompressedUploadSizeMustAlignToBlockDims;
    }

    sktext::gpu::SubRunControl getSubRunControl(bool useSDFTForSmallText) const;

    bool setBackendLabels() const { return fSetBackendLabels; }

    GpuStatsFlags supportedGpuStats() const { return fSupportedGpuStats; }

protected:
    Caps();

    /**
     * Subclasses must call this at the end of their init method in order to do final processing on
     * the caps.
     */
    void finishInitialization(const ContextOptions&);

#if defined(GPU_TEST_UTILS)
    void setDeviceName(std::string n) {
        fDeviceName = std::move(n);
    }
#endif

    /**
     * There are only a few possible valid sample counts (1, 2, 4, 8, 16). So we can key on those 5
     * options instead of the actual sample value.
     */
    static inline uint32_t SamplesToKey(uint32_t numSamples) {
        switch (numSamples) {
            case 1:
                return 0;
            case 2:
                return 1;
            case 4:
                return 2;
            case 8:
                return 3;
            case 16:
                return 4;
            default:
                SkUNREACHABLE;
        }
    }

    /* ColorTypeInfo for a specific format. Used in format tables. */
    struct ColorTypeInfo {
        ColorTypeInfo() = default;
        ColorTypeInfo(SkColorType ct, SkColorType transferCt, uint32_t flags,
                      skgpu::Swizzle readSwizzle, skgpu::Swizzle writeSwizzle)
                : fColorType(ct)
                , fTransferColorType(transferCt)
                , fFlags(flags)
                , fReadSwizzle(readSwizzle)
                , fWriteSwizzle(writeSwizzle) {}

        SkColorType fColorType = kUnknown_SkColorType;
        SkColorType fTransferColorType = kUnknown_SkColorType;
        enum {
            kUploadData_Flag = 0x1,
            /**
             * Does Graphite itself support rendering to this colorType & format pair. Renderability
             * still additionally depends on if the format itself is renderable.
             */
            kRenderable_Flag = 0x2,
        };
        uint32_t fFlags = 0;

        skgpu::Swizzle fReadSwizzle;
        skgpu::Swizzle fWriteSwizzle;
    };

    int fMaxTextureSize = 0;
    uint8_t fDefaultMSAASamples = 4;
    size_t fRequiredUniformBufferAlignment = 0;
    size_t fRequiredStorageBufferAlignment = 0;
    size_t fRequiredTransferBufferAlignment = 0;
    size_t fTextureDataRowBytesAlignment = 1;

    int fMaxVaryings = 0;

    std::unique_ptr<SkSL::ShaderCaps> fShaderCaps;

    bool fNDCYAxisPointsDown = false; // Most backends have NDC +Y pointing up
    bool fClampToBorderSupport = true;
    bool fProtectedSupport = false;
    bool fSemaphoreSupport = false;
    bool fAllowCpuSync = true;
    bool fStorageBufferSupport = false;
    bool fDrawBufferCanBeMapped = true;
    bool fBufferMapsAreAsync = false;
    bool fMSAARenderToSingleSampledSupport = false;
    bool fDifferentResolveAttachmentSizeSupport = false;

    bool fComputeSupport = false;
    bool fSupportsAHardwareBufferImages = false;
    BlendEquationSupport fBlendEqSupport = BlendEquationSupport::kBasic;
    bool fFullCompressedUploadSizeMustAlignToBlockDims = false;

#if defined(GPU_TEST_UTILS)
    bool fDrawBufferCanBeMappedForReadback = true;
#endif

    ResourceBindingRequirements fResourceBindingReqs;

    GpuStatsFlags fSupportedGpuStats = GpuStatsFlags::kNone;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Client-provided Caps

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    ShaderErrorHandler* fShaderErrorHandler = nullptr;

#if defined(GPU_TEST_UTILS)
    std::string fDeviceName;
    int fMaxTextureAtlasSize = 2048;
    PathRendererStrategy fRequestedPathRendererStrategy;
#endif
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    float fMinDistanceFieldFontSize = 18;
    float fGlyphsAsPathsFontSize = 324;

    int fMaxPathAtlasTextureSize = 8192;

    bool fAllowMultipleAtlasTextures = true;
    bool fSupportBilerpFromGlyphAtlas = false;

    bool fRequireOrderedRecordings = false;

    bool fSetBackendLabels = false;

    // Dynamic state.  The granularity is less fine than Vulkan's, but there is still some
    // granularity to allow for some dynamic state to be disabled due to driver bugs without having
    // to disable everything.  Eventually, these can be used to create fewer pipelines in the first
    // place (b/414645289).
    bool fUseBasicDynamicState = false;
    bool fUseVertexInputDynamicState = false;
    bool fUsePipelineLibraries = false;

private:
    virtual bool onIsTexturable(const TextureInfo&) const = 0;
    virtual const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const = 0;

    sk_sp<SkCapabilities> fCapabilities;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Caps_DEFINED
