/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrTextureProxyCacheAccess.h"

#include "src/gpu/ganesh/GrProxyProvider.h"

void GrTextureProxy::CacheAccess::setUniqueKey(
        sk_sp<GrUniquelyKeyedProxyRegistry> uniquelyKeyedProxyRegistry,
        const skgpu::UniqueKey& key) {
    fTextureProxy->setUniqueKey(std::move(uniquelyKeyedProxyRegistry), key);
}

