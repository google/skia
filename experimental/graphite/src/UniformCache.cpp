/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformCache.h"

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "src/core/SkOpts.h"

namespace skgpu {

namespace {

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == skgpu::UniformData::kInvalidUniformID);
    return id;
}

} // anonymous namespace

size_t UniformCache::Hash::operator()(sk_sp<UniformData> ud) const {
    return SkOpts::hash_fn(ud->data(), ud->dataSize(), 0);
}

bool UniformCache::Eq::operator()(sk_sp<UniformData> a, sk_sp<UniformData> b) const {
    if (a->count() != b->count() ||
        a->uniforms() != b->uniforms() ||
        a->dataSize() != b->dataSize()) {
        return false;
    }

    return !memcmp(a->data(), b->data(), a->dataSize()) &&
           !memcmp(a->offsets(), b->offsets(), a->count()*sizeof(uint32_t));
};

sk_sp<UniformData> UniformCache::findOrCreate(sk_sp<UniformData> ud) {

    auto iter = fUniformData.find(ud);
    if (iter != fUniformData.end()) {
        SkASSERT((*iter)->id() != UniformData::kInvalidUniformID);
        return *iter;
    }

    ud->setID(next_id());
    fUniformData.insert(ud);
    return ud;
}

} // namespace skgpu
