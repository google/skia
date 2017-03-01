/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasUtils.h"
#include "GrContext.h"
#include "GrResourceProvider.h"

std::unique_ptr<GrDrawOpAtlas> GrAtlasUtils::MakeAtlas(GrContext* ctx, GrPixelConfig config,
                                                       int width, int height,
                                                       int numPlotsX, int numPlotsY,
                                                       GrDrawOpAtlas::EvictionFunc func,
                                                       void* data) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    // TODO: Determine how to avoid having to do this. (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    sk_sp<GrTexture> texture(ctx->textureProvider()->createApproxTexture(desc, kFlags));
    if (!texture) {
        return nullptr;
    }

    // MDB TODO: for now, wrap an instantiated texture. Having the deferred instantiation
    // possess the correct properties (e.g., no pendingIO) should fall out of the system but
    // should receive special attention.
    // Note: When switching over to the deferred proxy, use the kExact flag to create
    // the atlas and assert that the width & height are powers of 2.
    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(texture));
    if (!proxy) {
        return nullptr;
    }

    std::unique_ptr<GrDrawOpAtlas> atlas(
            new GrDrawOpAtlas(ctx, std::move(proxy), numPlotsX, numPlotsY));
    atlas->registerEvictionCallback(func, data);
    return atlas;
}
