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
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder)) {
    // Disable for now.
    //fPathAtlasFlags |= PathAtlasFlags::kSoftware;
#ifdef SK_ENABLE_VELLO_SHADERS
    if (recorder->priv().caps()->computeSupport()) {
        fPathAtlasFlags |= PathAtlasFlags::kCompute;
    }
#endif  // SK_ENABLE_VELLO_SHADERS
}

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas(Recorder* recorder) const {
#ifdef SK_ENABLE_VELLO_SHADERS
    if (fPathAtlasFlags & PathAtlasFlags::kCompute) {
        return std::make_unique<VelloComputePathAtlas>();
    }
#endif  // SK_ENABLE_VELLO_SHADERS
    return nullptr;
}

std::unique_ptr<SoftwarePathAtlas> AtlasProvider::createSoftwarePathAtlas() const {
    if (fPathAtlasFlags & PathAtlasFlags::kSoftware) {
        return std::make_unique<SoftwarePathAtlas>();
    }
    return nullptr;
}

sk_sp<TextureProxy> AtlasProvider::getAtlasTexture(Recorder* recorder,
                                                   uint16_t width,
                                                   uint16_t height,
                                                   SkColorType colorType) {
    uint64_t key = static_cast<uint64_t>(width)  << 48 |
                   static_cast<uint64_t>(height) << 32 |
                   static_cast<uint64_t>(colorType);
    auto iter = fTexturePool.find(key);
    if (iter != fTexturePool.end()) {
        return iter->second;
    }

    auto proxy = TextureProxy::MakeStorage(recorder->priv().caps(),
                                           SkISize::Make(int32_t(width), int32_t(height)),
                                           colorType,
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
