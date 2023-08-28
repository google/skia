/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Texture_DEFINED
#define skgpu_graphite_Texture_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu {
class MutableTextureStateRef;
class RefCntedCallback;
enum class Budgeted : bool;
};

namespace skgpu::graphite {

class Texture : public Resource {
public:
    ~Texture() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    void setReleaseCallback(sk_sp<RefCntedCallback>);

#if defined(GRAPHITE_TEST_UTILS)
    const Texture* asTexture() const override { return this; }
#endif

protected:
    Texture(const SharedContext*,
            SkISize dimensions,
            const TextureInfo& info,
            sk_sp<MutableTextureStateRef> mutableState,
            Ownership,
            skgpu::Budgeted);

    MutableTextureStateRef* mutableState() const;

    void invokeReleaseProc() override;

private:
    SkISize fDimensions;
    TextureInfo fInfo;
    sk_sp<MutableTextureStateRef> fMutableState;
    sk_sp<RefCntedCallback> fReleaseCallback;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_Texture_DEFINED
