/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Caps_DEFINED
#define skgpu_graphite_Caps_DEFINED

#include <optional>

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAlign.h"
#include "src/base/SkEnumBitMask.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/text/gpu/SDFTControl.h"

enum class SkBlendMode;
class SkCapabilities;

namespace SkSL { struct ShaderCaps; }

namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

enum class BufferType : int;
struct ContextOptions;
class ComputePipelineDesc;
class GraphicsPipelineDesc;
class GraphiteResourceKey;
struct RenderPassDesc;
class TextureInfo;
class TextureProxy;

struct ResourceBindingRequirements {
    // The required data layout rules for the contents of a uniform buffer.
    Layout fUniformBufferLayout = Layout::kInvalid;

    // The required data layout rules for the contents of a storage buffer.
    Layout fStorageBufferLayout = Layout::kInvalid;

    // Whether combined texture-sampler types are supported. Backends that do not support
    // combined image samplers (i.e. sampler2D) require a texture and sampler object to be bound
    // separately and their binding indices explicitly specified in the shader text.
    bool fSeparateTextureAndSamplerBinding = false;

    // Whether buffer, texture, and sampler resource bindings use distinct index ranges.
    bool fDistinctIndexRanges = false;
};

enum class DstReadRequirement {
    kNone,
    kTextureCopy,
    kTextureSample,
    kFramebufferFetch,
};

class Caps {
public:
    virtual ~Caps();

    const SkSL::ShaderCaps* shaderCaps() const { return fShaderCaps.get(); }

    sk_sp<SkCapabilities> capabilities() const;

    virtual TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                     Mipmapped mipmapped,
                                                     Protected,
                                                     Renderable) const = 0;

    virtual TextureInfo getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                     Mipmapped mipmapped) const = 0;

    virtual TextureInfo getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                  Discardable discardable) const = 0;

    virtual TextureInfo getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                          uint32_t sampleCount,
                                                          Protected) const = 0;

    virtual TextureInfo getDefaultStorageTextureInfo(SkColorType) const = 0;

    virtual UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                              const RenderPassDesc&) const = 0;
    virtual UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const = 0;

    bool areColorTypeAndTextureInfoCompatible(SkColorType, const TextureInfo&) const;
    virtual uint32_t channelMask(const TextureInfo&) const = 0;

    bool isTexturable(const TextureInfo&) const;
    virtual bool isRenderable(const TextureInfo&) const = 0;
    virtual bool isStorage(const TextureInfo&) const = 0;

    int maxTextureSize() const { return fMaxTextureSize; }
    int defaultMSAASamplesCount() const { return fDefaultMSAASamples; }

    virtual void buildKeyForTexture(SkISize dimensions,
                                    const TextureInfo&,
                                    ResourceType,
                                    Shareable,
                                    GraphiteResourceKey*) const = 0;

    const ResourceBindingRequirements& resourceBindingRequirements() const {
        return fResourceBindingReqs;
    }

    // Returns the required alignment in bytes for the offset into a uniform buffer when binding it
    // to a draw.
    size_t requiredUniformBufferAlignment() const { return fRequiredUniformBufferAlignment; }

    // Returns the required alignment in bytes for the offset into a storage buffer when binding it
    // to a draw.
    size_t requiredStorageBufferAlignment() const { return fRequiredStorageBufferAlignment; }

    // Returns the required alignment in bytes for the offset and size of copies involving a buffer.
    size_t requiredTransferBufferAlignment() const { return fRequiredTransferBufferAlignment; }

    // Returns the aligned rowBytes when transfering to or from a Texture
    size_t getAlignedTextureDataRowBytes(size_t rowBytes) const {
        return SkAlignTo(rowBytes, fTextureDataRowBytesAlignment);
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
     */
    virtual SkColorType supportedWritePixelsColorType(SkColorType dstColorType,
                                                      const TextureInfo& dstTextureInfo,
                                                      SkColorType srcColorType) const = 0;

    /**
     * Given a src surface's color type and its texture info as well as a color type the caller
     * would like read into, this provides a legal color type that the caller can use for
     * readPixels. The returned color type may differ from the passed dstColorType, in
     * which case the caller must convert the read pixel data (see GrConvertPixels). When converting
     * to dstColorType the swizzle in the returned struct should be applied. The caller must check
     * the returned color type for kUnknown.
     */
    virtual SkColorType supportedReadPixelsColorType(SkColorType srcColorType,
                                                     const TextureInfo& srcTextureInfo,
                                                     SkColorType dstColorType) const = 0;

    /**
     * Checks whether the passed color type is renderable. If so, the same color type is passed
     * back. If not, provides an alternative (perhaps lower bit depth and/or unorm instead of float)
     * color type that is supported or kUnknown if there no renderable fallback format.
     */
    SkColorType getRenderableColorType(SkColorType) const;

    bool clampToBorderSupport() const { return fClampToBorderSupport; }

    bool protectedSupport() const { return fProtectedSupport; }

    // Supports BackendSemaphores
    bool semaphoreSupport() const { return fSemaphoreSupport; }

    // Returns whether storage buffers are supported.
    bool storageBufferSupport() const { return fStorageBufferSupport; }

    // Returns whether storage buffers are preferred over uniform buffers, when both will yield
    // correct results.
    bool storageBufferPreferred() const { return fStorageBufferPreferred; }

    // Returns whether a draw buffer can be mapped.
    bool drawBufferCanBeMapped() const { return fDrawBufferCanBeMapped; }

    // Returns whether multisampled render to single sampled is supported.
    bool msaaRenderToSingleSampledSupport() const { return fMSAARenderToSingleSampledSupport; }

    // Returns whether compute shaders are supported.
    bool computeSupport() const { return fComputeSupport; }

    // Returns the skgpu::Swizzle to use when sampling or reading back from a texture with the
    // passed in SkColorType and TextureInfo.
    skgpu::Swizzle getReadSwizzle(SkColorType, const TextureInfo&) const;

    // Returns the skgpu::Swizzle to use when writing colors to a surface with the passed in
    // SkColorType and TextureInfo.
    skgpu::Swizzle getWriteSwizzle(SkColorType, const TextureInfo&) const;

    skgpu::ShaderErrorHandler* shaderErrorHandler() const { return fShaderErrorHandler; }

    // Returns what method of dst read is required for a draw using the dst color.
    DstReadRequirement getDstReadRequirement() const;

    float minDistanceFieldFontSize() const { return fMinDistanceFieldFontSize; }
    float glyphsAsPathsFontSize() const { return fGlyphsAsPathsFontSize; }

    size_t glyphCacheTextureMaximumBytes() const { return fGlyphCacheTextureMaximumBytes; }

    bool allowMultipleGlyphCacheTextures() const { return fAllowMultipleGlyphCacheTextures; }
    bool supportBilerpFromGlyphAtlas() const { return fSupportBilerpFromGlyphAtlas; }

    sktext::gpu::SDFTControl getSDFTControl(bool useSDFTForSmallText) const;

protected:
    Caps();

    // Subclasses must call this at the end of their init method in order to do final processing on
    // the caps.
    void finishInitialization(const ContextOptions&);

    // There are only a few possible valid sample counts (1, 2, 4, 8, 16). So we can key on those 5
    // options instead of the actual sample value.
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

    // ColorTypeInfo for a specific format.
    // Used in format tables.
    struct ColorTypeInfo {
        SkColorType fColorType = kUnknown_SkColorType;
        SkColorType fTransferColorType = kUnknown_SkColorType;
        enum {
            kUploadData_Flag = 0x1,
            // Does Graphite itself support rendering to this colorType & format pair. Renderability
            // still additionally depends on if the format itself is renderable.
            kRenderable_Flag = 0x2,
        };
        uint32_t fFlags = 0;

        skgpu::Swizzle fReadSwizzle;
        skgpu::Swizzle fWriteSwizzle;
    };

    int fMaxTextureSize = 0;
    int fDefaultMSAASamples = 4;
    size_t fRequiredUniformBufferAlignment = 0;
    size_t fRequiredStorageBufferAlignment = 0;
    size_t fRequiredTransferBufferAlignment = 0;
    size_t fTextureDataRowBytesAlignment = 1;

    std::unique_ptr<SkSL::ShaderCaps> fShaderCaps;

    bool fClampToBorderSupport = true;
    bool fProtectedSupport = false;
    bool fSemaphoreSupport = false;
    bool fStorageBufferSupport = false;
    bool fStorageBufferPreferred = false;
    bool fDrawBufferCanBeMapped = true;
    bool fMSAARenderToSingleSampledSupport = false;

    bool fComputeSupport = false;

    ResourceBindingRequirements fResourceBindingReqs;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Client-provided Caps

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    ShaderErrorHandler* fShaderErrorHandler = nullptr;

#if defined(GRAPHITE_TEST_UTILS)
    int  fMaxTextureAtlasSize = 2048;
#endif
    size_t fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;

    float fMinDistanceFieldFontSize = 18;
    float fGlyphsAsPathsFontSize = 324;

    bool fAllowMultipleGlyphCacheTextures = true;
    bool fSupportBilerpFromGlyphAtlas = false;

private:
    virtual bool onIsTexturable(const TextureInfo&) const = 0;
    virtual const ColorTypeInfo* getColorTypeInfo(SkColorType, const TextureInfo&) const = 0;

    sk_sp<SkCapabilities> fCapabilities;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Caps_DEFINED
