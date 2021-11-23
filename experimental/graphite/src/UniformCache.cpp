/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformCache.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "src/core/SkOpts.h"

namespace skgpu {

size_t UniformCache::Hash::operator()(UniformData* ud) const {
    if (!ud) {
        return 0;
    }
    return SkOpts::hash_fn(ud->data(), ud->dataSize(), 0);
}

bool UniformCache::Eq::operator()(UniformData* a, UniformData* b) const {
    if (!a || !b) {
        return !a && !b;
    }
    if (a->count() != b->count() ||
        a->uniforms() != b->uniforms() ||
        a->dataSize() != b->dataSize()) {
        return false;
    }

    return !memcmp(a->data(), b->data(), a->dataSize()) &&
           !memcmp(a->offsets(), b->offsets(), a->count()*sizeof(uint32_t));
};

UniformCache::UniformCache() {
    // kInvalidUniformID is reserved
    static_assert(kInvalidUniformID == 0);
    fUniformData.push_back(nullptr);
    fUniformDataIDs.insert({nullptr, 0});
}

#ifdef SK_DEBUG
void UniformCache::validate() const {
    for (size_t i = 0; i < fUniformData.size(); ++i) {
        auto kv = fUniformDataIDs.find(fUniformData[i].get());
        SkASSERT(kv != fUniformDataIDs.end());
        SkASSERT(kv->first == fUniformData[i].get());
        SkASSERT(SkTo<uint32_t>(i) == kv->second);
    }
}
#endif

uint32_t UniformCache::insert(sk_sp<UniformData> data) {
    auto kv = fUniformDataIDs.find(data.get());
    if (kv != fUniformDataIDs.end()) {
        return kv->second;
    }

    uint32_t id = SkTo<uint32_t>(fUniformData.size());
    SkASSERT(data && id != kInvalidUniformID);

    fUniformDataIDs.insert({data.get(), id});
    fUniformData.push_back(std::move(data));
    this->validate();
    return id;
}

sk_sp<UniformData> UniformCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fUniformData.size());
    return fUniformData[uniqueID];
}

} // namespace skgpu
