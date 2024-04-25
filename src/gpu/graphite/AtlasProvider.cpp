/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/ComputePathAtlas.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

AtlasProvider::PathAtlasFlagsBitMask AtlasProvider::QueryPathAtlasSupport(const Caps* caps) {
    PathAtlasFlagsBitMask flags = PathAtlasFlags::kNone;
    flags |= PathAtlasFlags::kRaster;
    if (RendererProvider::IsVelloRendererSupported(caps)) {
        flags |= PathAtlasFlags::kCompute;
    }
    return flags;
}

AtlasProvider::AtlasProvider(Recorder* recorder)
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder))
        , fRasterPathAtlas(std::make_unique<RasterPathAtlas>(recorder))
        , fPathAtlasFlags(QueryPathAtlasSupport(recorder->priv().caps())) {}

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas(Recorder* recorder) const {
    if (this->isAvailable(PathAtlasFlags::kCompute)) {
        return ComputePathAtlas::CreateDefault(recorder);
    }
    return nullptr;
}

RasterPathAtlas* AtlasProvider::getRasterPathAtlas() const {
    return fRasterPathAtlas.get();
}

sk_sp<TextureProxy> AtlasProvider::getAtlasTexture(Recorder* recorder,
                                                   uint16_t width,
                                                   uint16_t height,
                                                   SkColorType colorType,
                                                   uint16_t identifier,
                                                   bool requireStorageUsage) {
    uint64_t key = static_cast<uint64_t>(width)  << 48 |
                   static_cast<uint64_t>(height) << 32 |
                   static_cast<uint64_t>(colorType) << 16 |
                   static_cast<uint64_t>(identifier);
    auto iter = fTexturePool.find(key);
    if (iter != fTexturePool.end()) {
        return iter->second;
    }

    // We currently only make the distinction between a storage texture (written by a
    // compute pass) and a plain sampleable texture (written via upload) that won't be
    // used as a render attachment.
    const Caps* caps = recorder->priv().caps();
    auto textureInfo = requireStorageUsage
            ? caps->getDefaultStorageTextureInfo(colorType)
            : caps->getDefaultSampledTextureInfo(colorType,
                                                 Mipmapped::kNo,
                                                 recorder->priv().isProtected(),
                                                 Renderable::kNo);
    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   SkISize::Make((int32_t) width, (int32_t) height),
                                                   textureInfo,
                                                   Budgeted::kYes);
    if (!proxy) {
        return nullptr;
    }

    fTexturePool[key] = proxy;
    return proxy;
}

void AtlasProvider::clearTexturePool() {
    fTexturePool.clear();
}

void AtlasProvider::recordUploads(DrawContext* dc) {
    if (!fTextAtlasManager->recordUploads(dc)) {
        SKGPU_LOG_E("TextAtlasManager uploads have failed -- may see invalid results.");
    }

    if (fRasterPathAtlas) {
        fRasterPathAtlas->recordUploads(dc);
    }
}

void AtlasProvider::postFlush() {
    fTextAtlasManager->postFlush();
    if (fRasterPathAtlas) {
        fRasterPathAtlas->postFlush();
    }
}

}  // namespace skgpu::graphite
