/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphiteResourceKey.h"

namespace skgpu::graphite {

ResourceType GraphiteResourceKey::GenerateResourceType() {
    static std::atomic<int32_t> nextType{ResourceKey::kInvalidDomain + 1};

    int32_t type = nextType.fetch_add(1, std::memory_order_relaxed);
    if (type > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many Graphite Resource Types");
    }

    return static_cast<ResourceType>(type);
}

} // namespace skgpu::graphite
