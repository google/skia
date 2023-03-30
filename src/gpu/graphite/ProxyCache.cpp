/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ProxyCache.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"
#include "include/gpu/GpuTypes.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace {

void make_bitmap_key(skgpu::UniqueKey* key, const SkBitmap& bm, skgpu::Mipmapped mipmapped) {
    SkASSERT(key);

    SkIPoint origin = bm.pixelRefOrigin();
    SkIRect subset = SkIRect::MakePtSize(origin, bm.dimensions());

    static const skgpu::UniqueKey::Domain kProxyCacheDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, kProxyCacheDomain, 6, "ProxyCache");
    builder[0] = bm.pixelRef()->getGenerationID();
    builder[1] = subset.fLeft;
    builder[2] = subset.fTop;
    builder[3] = subset.fRight;
    builder[4] = subset.fBottom;
    builder[5] = SkToBool(mipmapped);
}

} // anonymous namespace

namespace skgpu::graphite {

ProxyCache::~ProxyCache() {}

uint32_t ProxyCache::UniqueKeyHash::operator()(const skgpu::UniqueKey& key) const {
    return key.hash();
}

sk_sp<TextureProxy> ProxyCache::findOrCreateCachedProxy(Recorder* recorder,
                                                        const SkBitmap& bitmap,
                                                        Mipmapped mipmapped) {
    if (bitmap.dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    skgpu::UniqueKey key;

    if (mipmapped == Mipmapped::kNo) {
        make_bitmap_key(&key, bitmap, Mipmapped::kYes);

        if (sk_sp<TextureProxy>* cached = fCache.find(key)) {
            return *cached;
        }
    }

    make_bitmap_key(&key, bitmap, mipmapped);

    if (sk_sp<TextureProxy>* cached = fCache.find(key)) {
        return *cached;
    }

    auto [ view, ct ] = MakeBitmapProxyView(recorder, bitmap, nullptr,
                                            mipmapped, skgpu::Budgeted::kYes);
    if (view) {
        fCache.set(key, view.refProxy());
    }
    return view.refProxy();
}

} // namespace skgpu::graphite
