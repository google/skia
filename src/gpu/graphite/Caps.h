/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Caps_DEFINED
#define skgpu_graphite_Caps_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkEnumBitMask.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/text/gpu/SDFTControl.h"

class SkCapabilities;

namespace SkSL { struct ShaderCaps; }

namespace skgpu { class ShaderErrorHandler; }

namespace skgpu::graphite {

struct ContextOptions;
class ComputePipelineDesc;
class GraphicsPipelineDesc;
class GraphiteResourceKey;
struct RenderPassDesc;
class TextureInfo;
class TextureProxy;

class Caps {
public:
    virtual ~Caps();

    const SkSL::ShaderCaps* shaderCaps() const { return fShaderCaps.get(); }

    sk_sp<SkCapabilities> capabilities() const;

    virtual TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                     Mipmapped mipmapped,
                                                     Protected,
                                                     Renderable) const = 0;

    virtual TextureInfo getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                  Discardable discardable) const = 0;

    virtual TextureInfo getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                          uint32_t sampleCount,
                                                          Protected) const = 0;

    virtual UniqueKey makeGraphicsPipelineKey(const GraphicsPipelineDesc&,
                                              const RenderPassDesc&) const = 0;
    virtual UniqueKey makeComputePipelineKey(const ComputePipelineDesc&) const = 0;

    bool areColorTypeAndTextureInfoCompatible(SkColorType, const TextureInfo&) const;

    bool isTexturable(const TextureInfo&) const;
    virtual bool isRenderable(const TextureInfo&) const = 0;

    int maxTextureSize() const { return fMaxTextureSize; }

    virtual void buildKeyForTexture(SkISize dimensions,
                                    const TextureInfo&,
                                    ResourceType,
                                    Shareable,
                                    GraphiteResourceKey*) const = 0;

    // Returns the required alignment in bytes for the offset into a uniform buffer when binding it
    // to a draw.
    size_t requiredUniformBufferAlignment() const { return fRequiredUniformBufferAlignment; }

    // Returns the required alignment in bytes for the offset into a storage buffer when binding it
    // to a draw.
    size_t requiredStorageBufferAlignment() const { return fRequiredStorageBufferAlignment; }

    // Returns the alignment in bytes for the offset into a Buffer when using it
    // to transfer to or from a Texture with the given bytes per pixel.
    virtual size_t getTransferBufferAlignment(size_t bytesPerPixel) const = 0;

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

    bool clampToBorderSupport() const { return fClampToBorderSupport; }

    // Returns whether storage buffers are supported.
    bool storageBufferSupport() const { return fStorageBufferSupport; }

    // Returns whether storage buffers are preferred over uniform buffers, when both will yield
    // correct results.
    bool storageBufferPreferred() const { return fStorageBufferPreferred; }

    // Returns the skgpu::Swizzle to use when sampling or reading back from a texture with the
    // passed in SkColorType and TextureInfo.
    skgpu::Swizzle getReadSwizzle(SkColorType, const TextureInfo&) const;

    // Returns the skgpu::Swizzle to use when writing colors to a surface with the passed in
    // SkColorType and TextureInfo.
    skgpu::Swizzle getWriteSwizzle(SkColorType, const TextureInfo&) const;

    skgpu::ShaderErrorHandler* shaderErrorHandler() const { return fShaderErrorHandler; }

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

    // TODO: This value should be set by some context option. For now just making it 4.
    uint32_t defaultMSAASamples() const { return 4; }

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
    size_t fRequiredUniformBufferAlignment = 0;
    size_t fRequiredStorageBufferAlignment = 0;
    size_t fTextureDataRowBytesAlignment = 1;

    std::unique_ptr<SkSL::ShaderCaps> fShaderCaps;

    bool fClampToBorderSupport = true;

    bool fStorageBufferSupport = false;
    bool fStorageBufferPreferred = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Client-provided Caps

    /**
     * If present, use this object to report shader compilation failures. If not, report failures
     * via SkDebugf and assert.
     */
    ShaderErrorHandler* fShaderErrorHandler = nullptr;

#if GRAPHITE_TEST_UTILS
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
