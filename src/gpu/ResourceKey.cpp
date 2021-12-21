/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/gpu/ResourceKey.h"

namespace skgpu {

ScratchKey::ResourceType ScratchKey::GenerateResourceType() {
    static std::atomic<int32_t> nextType{ResourceKey::kInvalidDomain + 1};

    int32_t type = nextType.fetch_add(1, std::memory_order_relaxed);
    if (type > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

UniqueKey::Domain UniqueKey::GenerateDomain() {
    static std::atomic<int32_t> nextDomain{ResourceKey::kInvalidDomain + 1};

    int32_t domain = nextDomain.fetch_add(1, std::memory_order_relaxed);
    if (domain > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many skgpu::UniqueKey Domains");
    }

    return static_cast<Domain>(domain);
}

uint32_t ResourceKeyHash(const uint32_t* data, size_t size) {
    return SkOpts::hash(data, size);
}

} // namespace skgpu

