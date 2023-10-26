/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/AtlasProvider.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PathAtlas.h"
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
        , fRasterPathAtlas(std::make_unique<RasterPathAtlas>())
        , fPathAtlasFlags(QueryPathAtlasSupport(recorder->priv().caps())) {}

std::unique_ptr<ComputePathAtlas> AtlasProvider::createComputePathAtlas() const {
    if (this->isAvailable(PathAtlasFlags::kCompute)) {
        return ComputePathAtlas::CreateDefault();
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

    sk_sp<TextureProxy> proxy;
    if (requireStorageUsage) {
        proxy = TextureProxy::MakeStorage(recorder->priv().caps(),
                                          SkISize::Make(int32_t(width), int32_t(height)),
                                          colorType,
                                          skgpu::Budgeted::kYes);
    } else {
        // We currently only make the distinction between a storage texture (written by a
        // compute pass) and a plain sampleable texture (written via upload) that won't be
        // used as a render attachment.
        proxy = TextureProxy::Make(recorder->priv().caps(),
                                   SkISize::Make(int32_t(width), int32_t(height)),
                                   colorType,
                                   skgpu::Mipmapped::kNo,
                                   skgpu::Protected::kNo,
                                   skgpu::Renderable::kNo,
                                   skgpu::Budgeted::kYes);
    }
    if (!proxy) {
        return nullptr;
    }

    fTexturePool[key] = proxy;
    return proxy;
}

void AtlasProvider::clearTexturePool() {
    fTexturePool.clear();
}

void AtlasProvider::recordUploads(DrawContext* dc, Recorder* recorder) {
    if (!dc->recordTextUploads(fTextAtlasManager.get())) {
        SKGPU_LOG_E("TextAtlasManager uploads have failed -- may see invalid results.");
    }

    if (fRasterPathAtlas) {
        fRasterPathAtlas->recordUploads(dc, recorder);
    }
}

}  // namespace skgpu::graphite
