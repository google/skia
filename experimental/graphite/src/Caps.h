/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Caps_DEFINED
#define skgpu_Caps_DEFINED

#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

namespace SkSL {
struct ShaderCaps;
}

namespace skgpu {

class TextureInfo;

class Caps : public SkRefCnt {
public:
    ~Caps() override;

    const SkSL::ShaderCaps* shaderCaps() const { return fShaderCaps.get(); }

    virtual TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                     uint32_t levelCount,
                                                     Protected,
                                                     Renderable) const = 0;

    virtual TextureInfo getDefaultMSAATextureInfo(SkColorType,
                                                  uint32_t sampleCount,
                                                  Protected) const = 0;

    virtual TextureInfo getDefaultDepthStencilTextureInfo(DepthStencilType,
                                                          uint32_t sampleCount,
                                                          Protected) const = 0;

    bool areColorTypeAndTextureInfoCompatible(SkColorType, const TextureInfo&) const;

    virtual bool isTexturable(const TextureInfo&) const = 0;
    virtual bool isRenderable(const TextureInfo&) const = 0;

    int maxTextureSize() const { return fMaxTextureSize; }

    // Returns the required alignment in bytes for the offset into a uniform buffer when binding it
    // to a draw.
    size_t requiredUniformBufferAlignment() const { return fRequiredUniformBufferAlignment; }

protected:
    Caps();

    int fMaxTextureSize = 0;
    size_t fRequiredUniformBufferAlignment = 0;

    std::unique_ptr<SkSL::ShaderCaps> fShaderCaps;

private:
    virtual bool onAreColorTypeAndTextureInfoCompatible(SkColorType, const TextureInfo&) const = 0;
};

} // namespace skgpu

#endif // skgpu_Caps_DEFINED
