/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_TextureProxy_DEFINED
#define skgpu_TextureProxy_DEFINED

#include "experimental/graphite/include/TextureInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"

namespace skgpu {

class ResourceProvider;
class Texture;

class TextureProxy : public SkRefCnt {
public:
    TextureProxy(SkISize dimensions, const TextureInfo& info);

    ~TextureProxy() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return Mipmapped(fInfo.numMipLevels() > 1); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    bool instantiate(ResourceProvider*);

private:
#ifdef SK_DEBUG
    void validateTexture(const Texture*);
#endif

    SkISize fDimensions;
    TextureInfo fInfo;

    sk_sp<Texture> fTexture;
};

} // namepsace skgpu

#endif // skgpu_TextureProxy_DEFINED
