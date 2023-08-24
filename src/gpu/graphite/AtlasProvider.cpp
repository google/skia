/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

AtlasProvider::AtlasProvider(Recorder* recorder)
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder)) {}

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas(Recorder* recorder) const {
#ifdef SK_ENABLE_VELLO_SHADERS
    if (recorder->priv().caps()->computeSupport()) {
        return std::make_unique<VelloComputePathAtlas>();
    }
#endif  // SK_ENABLE_VELLO_SHADERS
    return nullptr;
}

sk_sp<TextureProxy> AtlasProvider::getAtlasTexture(Recorder* recorder,
                                                   uint32_t width,
                                                   uint32_t height) {
    uint64_t key = (static_cast<uint64_t>(width) << 32) | static_cast<uint64_t>(height);
    auto iter = fTexturePool.find(key);
    if (iter != fTexturePool.end()) {
        return iter->second;
    }

    // TODO(chromium:1856): WebGPU does not support the "storage binding" usage for the R8Unorm
    // texture format. This means that we may have to use RGBA8 on Dawn until it provides an
    // optional feature.
    auto proxy = TextureProxy::MakeStorage(recorder->priv().caps(),
                                           SkISize::Make(int32_t(width), int32_t(height)),
                                           kAlpha_8_SkColorType,
                                           skgpu::Budgeted::kYes);
    if (!proxy) {
        return nullptr;
    }

    fTexturePool[key] = proxy;
    return proxy;
}

void AtlasProvider::clearTexturePool() {
    fTexturePool.clear();
}

}  // namespace skgpu::graphite
