/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/ClipAtlasManager.h"
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
    // The raster-backend path atlas is always supported.
    PathAtlasFlagsBitMask flags = PathAtlasFlags::kRaster;
    if (RendererProvider::IsVelloRendererSupported(caps)) {
        flags |= PathAtlasFlags::kCompute;
    }
    return flags;
}

AtlasProvider::AtlasProvider(Recorder* recorder)
        : fTextAtlasManager(std::make_unique<TextAtlasManager>(recorder))
        , fRasterPathAtlas(std::make_unique<RasterPathAtlas>(recorder))
        , fClipAtlasManager(std::make_unique<ClipAtlasManager>(recorder))
        , fPathAtlasFlags(QueryPathAtlasSupport(recorder->priv().caps())) {}

AtlasProvider::~AtlasProvider() = default;

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas(Recorder* recorder) const {
    if (this->isAvailable(PathAtlasFlags::kCompute)) {
        return ComputePathAtlas::CreateDefault(recorder);
    }
    return nullptr;
}

RasterPathAtlas* AtlasProvider::getRasterPathAtlas() const {
    return fRasterPathAtlas.get();
}

ClipAtlasManager* AtlasProvider::getClipAtlasManager() const {
    return fClipAtlasManager.get();
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
                                                   "AtlasProviderTexture",
                                                   Budgeted::kYes);
    if (!proxy) {
        return nullptr;
    }

    fTexturePool[key] = proxy;
    return proxy;
}

void AtlasProvider::freeGpuResources() {
    // Clear out any pages not in use or needed for any pending work on the Recorder.
    // In the event this is called right after a snap(), all pages would be eligible
    // for cleanup anyways.
    fTextAtlasManager->freeGpuResources();
    if (fRasterPathAtlas) {
        fRasterPathAtlas->freeGpuResources();
    }
    if (fClipAtlasManager) {
        fClipAtlasManager->freeGpuResources();
    }
    // Release any textures held directly by the provider. These textures are used by transient
    // ComputePathAtlases that are reset every time a DrawContext snaps a DrawTask so there is no
    // need to reset those atlases explicitly here. Since the AtlasProvider gives out refs to the
    // TextureProxies in the pool, it should be safe to clear the pool in the middle of Recording.
    // Draws that use the previous TextureProxies will have refs on them.
    fTexturePool.clear();
}

void AtlasProvider::recordUploads(DrawContext* dc) {
    if (!fTextAtlasManager->recordUploads(dc)) {
        SKGPU_LOG_E("TextAtlasManager uploads have failed -- may see invalid results.");
    }

    if (fRasterPathAtlas) {
        fRasterPathAtlas->recordUploads(dc);
    }
    if (fClipAtlasManager) {
        fClipAtlasManager->recordUploads(dc);
    }
}

void AtlasProvider::compact() {
    fTextAtlasManager->compact();
    if (fRasterPathAtlas) {
        fRasterPathAtlas->compact();
    }
    if (fClipAtlasManager) {
        fClipAtlasManager->compact();
    }
}

void AtlasProvider::invalidateAtlases() {
    // We must also evict atlases on a failure. The failed tasks can include uploads that the
    // atlas was depending on for its caches. Failing to prepare means they will never run so
    // future "successful" Recorder snaps would otherwise reference atlas pages that had stale
    // contents.
    fTextAtlasManager->evictAtlases();
    if (fRasterPathAtlas) {
        fRasterPathAtlas->evictAtlases();
    }
    if (fClipAtlasManager) {
        fClipAtlasManager->evictAtlases();
    }
}

}  // namespace skgpu::graphite
