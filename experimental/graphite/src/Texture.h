/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Texture_DEFINED
#define skgpu_Texture_DEFINED

#include "experimental/graphite/include/TextureInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

namespace skgpu {

class Texture : public SkRefCnt {
public:
    ~Texture() override;

    enum class UsageFlags : uint8_t {
        kSampledTexture    = 0x1,
        kColorAttachment   = 0x2,
        kStencilAttachment = 0x4,
        kDepthAttachment   = 0x8,
    };
    SKGPU_DECL_BITFIELD_CLASS_OPS_FRIENDS(UsageFlags);

    UsageFlags supportedUsages() const { return fSupportedUsages; }

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return Mipmapped(fInfo.numMipLevels() > 1); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

protected:
    Texture(SkISize dimensions, const TextureInfo& info, UsageFlags);

private:
    SkISize fDimensions;
    TextureInfo fInfo;
    UsageFlags fSupportedUsages;
};

SKGPU_MAKE_BITFIELD_CLASS_OPS(Texture::UsageFlags)

} // namepsace skgpu

#endif // skgpu_Texture_DEFINED
